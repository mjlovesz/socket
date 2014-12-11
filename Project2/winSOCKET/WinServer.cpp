#include "WinServer.h"
#include <iostream>
#include <algorithm>
#include <process.h>
#include <set>
#pragma comment(lib, "WS2_32")
#define  PORT_DEFAULT 80
#define  IP_DEFAULT "127.0.0.1"


bool DefDealError(void* p,WinServerError Etype)
{
	switch (Etype)
	{
	case WINSEV_E_MEM:		//内存申请出错
		std::cout<<"内存申请出错"<<std::endl;
		break;
	case WINSEV_E_NOT_SUPPORT:		//内存申请出错
		std::cout<<"暂时不支持"<<std::endl;
		break;
	case WINSEV_E_INIT_SOCK:		//内存申请出错
		std::cout<<"初始化错误"<<std::endl;
		break;
	case WINSEV_E_SOCKET:		//内存申请出错
		std::cout<<"socket函数出错"<<std::endl;
		break;
	case WINSEV_E_BIND:		//内存申请出错
		std::cout<<"bind 函数出错"<<std::endl;
		break;
	case WINSEV_E_LISTEN:		//内存申请出错
		std::cout<<"listen 函数出错"<<std::endl;
		break;
	case WINSEV_E_ACCEPT:		//内存申请出错
		std::cout<<"accept 函数出错"<<std::endl;
		break;
	case WINSEV_E_SELECT:		//内存申请出错
		std::cout<<"select 函数出错"<<std::endl;
		break;
	case WINSEV_E_BEGINTHREAD:		//内存申请出错
		std::cout<<"开始新线程出错"<<std::endl;
		break;
	default:
		break;
	}
	return false;
}
struct ThreadParam
{
	ThreadParam(){};
	ThreadParam(ThreadParam* p):threadid(p->threadid),pDealM(p->pDealM){};
	DWORD			threadid;
	HANDLE			threadhandle;
	DealManage*		pDealM;
};



DealManage::DealManage(SOCKET sock,DealSocket *p,UINT willFree):
	m_pDeal(p),m_pNextDeal(NULL),m_FreeFlag(willFree),m_sock(sock)
{
	p->SetClientSocket(this);
	m_State = p->initState();
}
DealManage::~DealManage()
{
	if (m_pDeal)
		delete m_pDeal;
}
dealSocketRtn DealManage::dealSend(){
	m_State = m_pDeal->dealSend();
	ChangeDeal();
	return m_State;
};
dealSocketRtn DealManage::dealRecv(){
	m_State = m_pDeal->dealRecv();
	ChangeDeal();
	return m_State;
};
void DealManage::ChangeDeal(){
	if (m_pNextDeal)
	{
		if (m_FreeFlag&1)
		{
			delete m_pDeal;
		}
		m_FreeFlag >>= 1;
		m_pDeal = m_pNextDeal;
		m_pNextDeal = NULL;
	}
}
void DealManage::ChangeDeal(DealSocket* p,UINT willFree){
	if (p)
	{
		m_pNextDeal = p;
		p->SetClientSocket(this);
		m_State = p->initState();
	}
}

WinServer::WinServer(void)
{
	WSADATA  Ws;
	if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
	{
		throw WINSEV_E_INIT_SOCK;
	}
	m_sock	=	INVALID_SOCKET;
	m_af	=	AF_INET;
	m_efunc =	DefDealError;
	m_State =	SEVER_CLOSED;
}

WinServer::~WinServer(void)
{
	if (m_sock != INVALID_SOCKET)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}

	WSACleanup();
}
void WinServer::cleanSock(std::map<SOCKET,DealManage*>* mapClientSock)
{
	if (mapClientSock)
	{	
		for (auto n: *mapClientSock)
		{
			delete n.second;
			closesocket(n.first);
		}
		mapClientSock->clear();
	}

	if (m_sock != INVALID_SOCKET)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
}

void WinServer::Bind_AF_INET_AF_UNIX(ULONG ip, USHORT port)
{
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = ip;
	LocalAddr.sin_port = port;
	memset(LocalAddr.sin_zero, 0x00, 8);

	int Ret = bind(m_sock, (struct sockaddr*)&LocalAddr, sizeof(LocalAddr));
	if ( Ret != 0 )
	{
		DEALERROR(WINSEV_E_BIND);
	}
}

void WinServer::DEALERROR(WinServerError e)
{
	if(m_efunc && m_efunc(m_efuncParam,e))return;
	throw e;//如果没有处理，就throw错误
}
void WinServer::close()
{
	m_State = WM_CLOSE;
}

/////////////////////////////////////////TCP/////////////////////
WinServerTCP::WinServerTCP()
{
	m_port	=	htons(PORT_DEFAULT);	
	m_ip	=	inet_addr(IP_DEFAULT);
	m_connectNum =	11;
}

bool WinServerTCP::Work(std::function<DealSocket*()> dealSocketMkr)
{
	return Work(dealSocketMkr,false);
}
bool WinServerTCP::Work(std::function<DealSocket*()> dealSocketMkr,LPCTSTR ip,unsigned short port,bool isThread)
{
	SetIP(ip);
	SetPORT(port);
	return Work(dealSocketMkr,isThread);
}
bool WinServerTCP::Work(std::function<DealSocket*()> dealSocketMkr, bool isThread)
{
	if (m_State != SEVER_CLOSED)
	{
		return false;
	}
	m_MakeDeal = dealSocketMkr;

	m_sock = socket(m_af, SOCK_STREAM, IPPROTO_TCP);

	if (m_sock == INVALID_SOCKET )
	{
		DEALERROR(WINSEV_E_SOCKET);
	}
	switch (m_af)
	{
	case AF_INET:
	case AF_UNIX:
		Bind_AF_INET_AF_UNIX(m_ip,m_port);
		break;
	default:
		throw WINSEV_E_NOT_SUPPORT;
	}

	if (listen(m_sock, m_connectNum) != 0 )
	{
		throw WINSEV_E_LISTEN;
	}

	if (isThread)
	{
		Thread();
	}
	else
	{
		Select();
	}
	return true;
}
void WinServerTCP::Select()//异步
{
	int Ret;
	struct sockaddr_in ClientAddr;
	int AddrLen = sizeof(ClientAddr);
	SOCKET CientSocket;
	struct timeval tv = {0, 50};/*反应老快了*/
	fd_set writeFd, readFd;
	std::map<SOCKET,DealManage*> mapClientSock;//保存所有的socket和它的处理类
	while (true)
	{
		if (m_State == SEVER_WILL_CLOSE)
		{
			cleanSock(&mapClientSock);//强制关掉所有的sock
			m_State = SEVER_CLOSED;
			break;
		}
		FD_ZERO(&writeFd);
		FD_ZERO(&readFd);

		for (std::map<SOCKET,DealManage*>::iterator it = mapClientSock.begin(); it != mapClientSock.end();)
		{
			if (it->second->GetState()&DEAL_CLOSE)
			{
				closesocket(it->first);
				delete it->second;
				mapClientSock.erase(it++);
				continue;
			}
			if (it->second->GetState()&DEAL_GO_ON_RECV)
			{
				FD_SET(it->first,&readFd);
			}
			if (it->second->GetState()&DEAL_GO_ON_SEND)
			{
				FD_SET(it->first,&writeFd);
			}
			 ++it;
		}
		FD_SET(m_sock,&readFd);

		Ret = select(NULL, &readFd, &writeFd, NULL, &tv);
		if (Ret < 0)
		{
			DEALERROR(WINSEV_E_SELECT);
			continue;
		}
		else if (Ret == 0)
		{
			continue;
		}
			//处理可读的内容
		for (auto n : mapClientSock)
		{
			if (FD_ISSET(n.first,&readFd))
			{
				n.second->dealRecv();
			}
			if (FD_ISSET(n.first,&writeFd))
			{
				n.second->dealSend();
			}
		}
		if (FD_ISSET(m_sock,&readFd))
		{
			CientSocket = accept(m_sock, (struct sockaddr*)&ClientAddr, &AddrLen);
			if ( CientSocket == INVALID_SOCKET )
			{
				DEALERROR(WINSEV_E_ACCEPT);
			}
			else
			{
				DealManage *pDM = new DealManage(CientSocket,m_MakeDeal());
				pDM->SetSockaddr_in(ClientAddr);
				mapClientSock.insert(std::make_pair(CientSocket,pDM));
			}
		}
	}
}
#define WM_CLOSE_ONE_SOCKET WM_USER + 1 //LPARAM 为HWND
void WinServerTCP::Thread()//多线程异步
{
	u_long mode = 1;
	ioctlsocket(m_sock,FIONBIO,&mode);//设置为非阻塞
	struct sockaddr_in ClientAddr;
	int AddrLen = sizeof(ClientAddr);
	SOCKET CientSocket;
	HANDLE  hwnd;
	std::set<HANDLE> vHandle;
	MSG msg;
	while (true)
	{
		//1.处理结束
		if (m_State == SEVER_WILL_CLOSE && vHandle.size() == 0)
		{
			m_State = SEVER_CLOSED;
			break;
		}
		if (m_State == SEVER_WILL_CLOSE)
		{
			for (auto n : vHandle)
			{
				PostThreadMessage(GetThreadId(n), WM_CLOSE_ONE_SOCKET, 0, 0);//发送消息告诉所有的线程停止工作
			}
			cleanSock();
			continue;
		}
		//2.处理新的请求
		CientSocket = accept(m_sock, (struct sockaddr*)&ClientAddr, &AddrLen);
		if (CientSocket != INVALID_SOCKET )
		{
			DealManage *pDM = new DealManage(CientSocket,m_MakeDeal());
			pDM->SetSockaddr_in(ClientAddr);
			ThreadParam* pParam = new ThreadParam;
			pParam->pDealM = pDM;
			pParam->threadid = GetCurrentThreadId();
			hwnd = (HANDLE)_beginthreadex( NULL, 0, WinServerTCP::threadCallBack, (LPVOID)pParam, 1, NULL);
			pParam->threadhandle = hwnd;
			ResumeThread(hwnd);
			if (hwnd)
				vHandle.insert(hwnd);
			else
				DEALERROR(WINSEV_E_BEGINTHREAD);
		}
		//3.处理消息
		if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			if (msg.message == WM_CLOSE_ONE_SOCKET)
			{
				vHandle.erase((HANDLE)msg.lParam);
				CloseHandle((HANDLE)msg.lParam);
			}
		}

	}
}
unsigned int WINAPI WinServerTCP::threadCallBack(LPVOID pvoid)
{
	ThreadParam* pParam = (ThreadParam*)pvoid;
	struct timeval tv = {0, 50};/*反应老快了*/
	MSG msg;
	int Ret;	
	fd_set writeFd, readFd;
	while (true)
	{
		FD_ZERO(&writeFd);
		FD_ZERO(&readFd);
		if (pParam->pDealM->GetState()&DEAL_GO_ON_RECV)
		{
			FD_SET(pParam->pDealM->GetSOCK(),&readFd);
		}
		if (pParam->pDealM->GetState()&DEAL_GO_ON_SEND)
		{
			FD_SET(pParam->pDealM->GetSOCK(),&writeFd);
		}
		//处理发送接收
		Ret = select(NULL, &readFd, &writeFd, NULL, &tv);
		if (Ret > 0)
		{
			if (FD_ISSET(pParam->pDealM->GetSOCK(),&readFd))
			{
				pParam->pDealM->dealRecv();
			}
			if (FD_ISSET(pParam->pDealM->GetSOCK(),&writeFd))
			{
				pParam->pDealM->dealSend();
			}
		}

		//处理返回状态
		if (pParam->pDealM->GetState()&DEAL_CLOSE)
		{
			break;
		}
		//处理消息
		if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			if (msg.message == WM_CLOSE_ONE_SOCKET)
			{
				break;
			}
		}

	}
	closesocket(pParam->pDealM->GetSOCK());
	delete pParam->pDealM;
	PostThreadMessage(pParam->threadid,WM_CLOSE_ONE_SOCKET,0,(LPARAM)pParam->threadhandle);
	delete pParam;
	return 0;
}

void WinServerTCP::SetIP(LPCTSTR pcstr)
{
	m_ip = inet_addr(pcstr);
}

void WinServerTCP::SetPORT(unsigned short u)
{
	m_port = htons(u);
}

/////////////////////////////////////UDP/////////////////////////
WinServerUDP::WinServerUDP()
{
	m_port	=	htons(PORT_DEFAULT);	
	m_ip	=	inet_addr(IP_DEFAULT);
}
bool WinServerUDP::Work(std::function<DealSocket*()> dealSocketMkr,LPCTSTR ip,unsigned short port)
{
	SetIP(ip);
	SetPORT(port);
	return Work(dealSocketMkr);
}
bool WinServerUDP::Work(std::function<DealSocket*()> dealSocketMkr)
{
	if (m_State != SEVER_CLOSED)
	{
		return false;
	}
	m_MakeDeal = dealSocketMkr;

	m_sock = socket(m_af, SOCK_DGRAM, IPPROTO_UDP);
	if ( m_sock == INVALID_SOCKET )
	{
		DEALERROR(WINSEV_E_SOCKET);
	}
	switch (m_af)
	{
	case AF_INET:
	case AF_UNIX:
		Bind_AF_INET_AF_UNIX(m_ip,m_port);
		break;
	default:
		throw WINSEV_E_NOT_SUPPORT;
	}

	Select();

	return true;
}
void WinServerUDP::Select()//异步
{
	int Ret;
	struct timeval tv = {0, 50};/*反应老快了*/
	fd_set readFd,writeFd;
	DealManage *pDM = new DealManage(m_sock,m_MakeDeal());
	while (true)
	{
		if (m_State == SEVER_WILL_CLOSE)
		{
			delete pDM;
			m_State = SEVER_CLOSED;
			break;
		}
		FD_ZERO(&readFd);
		FD_ZERO(&writeFd);
		if (pDM->GetState()&DEAL_GO_ON_RECV)
		{
			FD_SET(m_sock,&readFd);
		}
		if (pDM->GetState()&DEAL_GO_ON_SEND)
		{
			FD_SET(m_sock,&writeFd);
		}

		Ret = select(NULL, &readFd, &writeFd, NULL, &tv);
		if (FD_ISSET(m_sock,&readFd) && pDM)
		{
			pDM->dealRecv();//非连接，只是一次性的。收到后就处理。处理完就完了。
		}
		if (FD_ISSET(m_sock,&writeFd) && pDM)
		{
			pDM->dealSend();//估计一直可以发。
		}
	}
}

void WinServerUDP::SetIP(LPCTSTR pcstr)
{
	m_ip = inet_addr(pcstr);
}

void WinServerUDP::SetPORT(unsigned short u)
{
	m_port = htons(u);
}
