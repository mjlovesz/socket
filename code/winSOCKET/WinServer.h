#pragma once

#include <winsock2.h>
#include <map>
#include <vector>
#include <functional>
typedef enum{
	WINSEV_E_MEM		//内存申请出错
	,WINSEV_E_NOT_SUPPORT//暂时不支持
	,WINSEV_E_INIT_SOCK	//初始化错误
	,WINSEV_E_SOCKET	//socket函数出错
	,WINSEV_E_BIND		//bind 函数出错
	,WINSEV_E_LISTEN	//listen 函数出错
	,WINSEV_E_ACCEPT	//accept 函数出错
	,WINSEV_E_SELECT	//select 函数出错
	,WINSEV_E_BEGINTHREAD//开始新线程出错
}WinServerError;//抛出错误

typedef enum{
	DEAL_NO					//啥都不做				0
	,DEAL_GO_ON_RECV		//要接受数据			1
	,DEAL_GO_ON_SEND		//要发送数据			10
	,DEAL_GO_ON_RECV_SEND	//要接受&发送数据		11
	,DEAL_CLOSE				//关闭连接				100
	,DEAL_ERROR_1	= 8 * 1	//错误1，增加错误加8
	,DEAL_ERROR_2	= 8 * 2	//错误2，增加错误加8
}dealSocketRtn;//返回值

enum{
	WinServer_TCP
	,WinServer_UDP
};

enum{
	SEVER_WILL_CLOSE
	,SEVER_CLOSED

};//状态
class DealSocket;
class DealManage;
typedef bool (*DealError)(void*,WinServerError) ;//返回ture表示已处理，不用throw,否则就throw错误



/************************************************************************/
/*	用来产生DealSocket的类。（后改为使用std::function实现）				
/*	写法 new DealSocketMaker<yourDealSocketClass>(),					
/*	然后这个指针就可以不断的生成你的类的对象了							
/************************************************************************/
struct DealSocketMaker_Parent
{
	virtual DealSocket*  operator()() = 0;
};
template <typename deal>
struct DealSocketMaker:public DealSocketMaker_Parent
{
	DealSocket* operator()(){
		return new deal();	
	};
};


/************************************************************************/
/*	用来处理接收发送的基类的。											
/*	所有的处理类都继承自此类											
/************************************************************************/
class DealSocket//处理发送的类
{
protected:
	DealManage * m_pCliSock;//保存调用本类的对象的指针。注，只可调用里面的ChangeDeal(),否则会死循环。
public:
	void SetClientSocket(DealManage* p){m_pCliSock = p;};
	virtual dealSocketRtn initState(){return DEAL_GO_ON_RECV_SEND;};
	virtual dealSocketRtn dealSend()=0;
	virtual dealSocketRtn dealRecv()=0;
	virtual ~DealSocket(){};
};


/************************************************************************/
/*	包含数据处理类（DealSocket）的类。									
/*	可以根据需求更换数据处理类，更换处理方式							
/************************************************************************/
class DealManage//处理上面那个“处理发送的类”
{
	DealSocket *	m_pDeal;
	DealSocket *	m_pNextDeal;
	UINT			m_FreeFlag;
	dealSocketRtn	m_State;	//接收状态
	struct sockaddr_in ClientAddr;//地址信息
	SOCKET			m_sock;
	void ChangeDeal();
public:
	DealManage():m_pDeal(NULL),m_pNextDeal(NULL),m_FreeFlag(0),m_sock(INVALID_SOCKET),m_State(DEAL_NO){};
	DealManage(SOCKET sock,DealSocket *p,UINT willFree = 1);//若第二个参数为0，则不释放这个指针；否则对这个指针负责
	~DealManage();
	dealSocketRtn dealSend();
	dealSocketRtn dealRecv();
	void ChangeDeal(DealSocket* p,UINT willFree = 3);//若第二个参数为0，则不释放这个指针；否则对这个指针负责
	dealSocketRtn GetState(){return m_State;};
	void SetSockaddr_in(struct sockaddr_in& s){ClientAddr = s;};
	struct sockaddr_in GetSockaddr_in(){return ClientAddr;};
	SOCKET GetSOCK(){return m_sock;};
};


/************************************************************************/
/*	服务器类															
/*																		
/************************************************************************/
class WinServer
{
public:
	WinServer(void);//抛出INT错误
	~WinServer(void);
protected://数据
	SOCKET	m_sock;		//套接字的描述符
	UINT	m_af;		//协议域	AF_UNIX(本地),AF_INET(IPv4),AF_INET6(IPv6),……

	DealError m_efunc;	//处理出错程序指针
	void*	m_efuncParam;//保存出错程序指针的一个参数

	std::function<DealSocket*()> m_MakeDeal;

	UINT	m_State;	//状态
public://函数
	bool virtual Work(std::function<DealSocket*()> dealSocketMkr) = 0;
	void SetAF(UINT u){m_af = u;};
	void SetErrorFunc(DealError fn,void* fnPra = NULL){m_efunc = fn;m_efuncParam = fnPra;};//void*是传入的函数的第一个参数

	void virtual close();
protected://函数
	void Bind_AF_INET_AF_UNIX(ULONG ip, USHORT port);
	inline void DEALERROR(WinServerError);
	void Select();
	void Thread();
	void cleanSock(std::map<SOCKET,DealManage*>* mapClientSock = NULL);
	
};

class WinServerTCP:
	public WinServer
{
public:
	WinServerTCP(void);//抛出INT错误
	~WinServerTCP(void){};
private://数据
	ULONG	m_ip;		//ip
	USHORT	m_port;		//port
	UINT	m_connectNum;//可连接数

public://函数
	bool Work(std::function<DealSocket*()> dealSocketMkr);
	bool Work(std::function<DealSocket*()> dealSocketMkr,bool isThread);//懒得搞智能指针，默认由此类帮助析构这个指针
	bool Work(std::function<DealSocket*()> dealSocketMkr,LPCTSTR ip,unsigned short port,bool isThread = false);//懒得搞智能指针，默认由此类帮助析构这个指针
	void SetIP(LPCTSTR);
	void SetPORT(unsigned short u);
	void SetConnectNum(UINT u){m_connectNum = u;};

protected://函数
	void Select();
	void Thread();
	static unsigned int WINAPI threadCallBack(LPVOID);

};

class WinServerUDP:
	public WinServer
{
public:
	WinServerUDP(void);//抛出INT错误
	~WinServerUDP(void){};
private://数据
	ULONG	m_ip;		//ip
	USHORT	m_port;		//port

public://函数
	bool Work(std::function<DealSocket*()> dealSocketMkr);
	bool Work(std::function<DealSocket*()> dealSocketMkr,LPCTSTR ip,unsigned short port);
	void SetIP(LPCTSTR);
	void SetPORT(unsigned short u);

protected://函数
	void Select();
};