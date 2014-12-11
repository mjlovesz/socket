#pragma once

#include <winsock2.h>
#include <map>
#include <vector>
#include <functional>
typedef enum{
	WINSEV_E_MEM		//�ڴ��������
	,WINSEV_E_NOT_SUPPORT//��ʱ��֧��
	,WINSEV_E_INIT_SOCK	//��ʼ������
	,WINSEV_E_SOCKET	//socket��������
	,WINSEV_E_BIND		//bind ��������
	,WINSEV_E_LISTEN	//listen ��������
	,WINSEV_E_ACCEPT	//accept ��������
	,WINSEV_E_SELECT	//select ��������
	,WINSEV_E_BEGINTHREAD//��ʼ���̳߳���
}WinServerError;//�׳�����

typedef enum{
	DEAL_NO					//ɶ������				0
	,DEAL_GO_ON_RECV		//Ҫ��������			1
	,DEAL_GO_ON_SEND		//Ҫ��������			10
	,DEAL_GO_ON_RECV_SEND	//Ҫ����&��������		11
	,DEAL_CLOSE				//�ر�����				100
	,DEAL_ERROR_1	= 8 * 1	//����1�����Ӵ����8
	,DEAL_ERROR_2	= 8 * 2	//����2�����Ӵ����8
}dealSocketRtn;//����ֵ

enum{
	WinServer_TCP
	,WinServer_UDP
};

enum{
	SEVER_WILL_CLOSE
	,SEVER_CLOSED

};//״̬
class DealSocket;
class DealManage;
typedef bool (*DealError)(void*,WinServerError) ;//����ture��ʾ�Ѵ�������throw,�����throw����



/************************************************************************/
/*	��������DealSocket���ࡣ�����Ϊʹ��std::functionʵ�֣�				
/*	д�� new DealSocketMaker<yourDealSocketClass>(),					
/*	Ȼ�����ָ��Ϳ��Բ��ϵ����������Ķ�����							
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
/*	����������շ��͵Ļ���ġ�											
/*	���еĴ����඼�̳��Դ���											
/************************************************************************/
class DealSocket//�����͵���
{
protected:
	DealManage * m_pCliSock;//������ñ���Ķ����ָ�롣ע��ֻ�ɵ��������ChangeDeal(),�������ѭ����
public:
	void SetClientSocket(DealManage* p){m_pCliSock = p;};
	virtual dealSocketRtn initState(){return DEAL_GO_ON_RECV_SEND;};
	virtual dealSocketRtn dealSend()=0;
	virtual dealSocketRtn dealRecv()=0;
	virtual ~DealSocket(){};
};


/************************************************************************/
/*	�������ݴ����ࣨDealSocket�����ࡣ									
/*	���Ը�������������ݴ����࣬��������ʽ							
/************************************************************************/
class DealManage//���������Ǹ��������͵��ࡱ
{
	DealSocket *	m_pDeal;
	DealSocket *	m_pNextDeal;
	UINT			m_FreeFlag;
	dealSocketRtn	m_State;	//����״̬
	struct sockaddr_in ClientAddr;//��ַ��Ϣ
	SOCKET			m_sock;
	void ChangeDeal();
public:
	DealManage():m_pDeal(NULL),m_pNextDeal(NULL),m_FreeFlag(0),m_sock(INVALID_SOCKET),m_State(DEAL_NO){};
	DealManage(SOCKET sock,DealSocket *p,UINT willFree = 1);//���ڶ�������Ϊ0�����ͷ����ָ�룻��������ָ�븺��
	~DealManage();
	dealSocketRtn dealSend();
	dealSocketRtn dealRecv();
	void ChangeDeal(DealSocket* p,UINT willFree = 3);//���ڶ�������Ϊ0�����ͷ����ָ�룻��������ָ�븺��
	dealSocketRtn GetState(){return m_State;};
	void SetSockaddr_in(struct sockaddr_in& s){ClientAddr = s;};
	struct sockaddr_in GetSockaddr_in(){return ClientAddr;};
	SOCKET GetSOCK(){return m_sock;};
};


/************************************************************************/
/*	��������															
/*																		
/************************************************************************/
class WinServer
{
public:
	WinServer(void);//�׳�INT����
	~WinServer(void);
protected://����
	SOCKET	m_sock;		//�׽��ֵ�������
	UINT	m_af;		//Э����	AF_UNIX(����),AF_INET(IPv4),AF_INET6(IPv6),����

	DealError m_efunc;	//����������ָ��
	void*	m_efuncParam;//����������ָ���һ������

	std::function<DealSocket*()> m_MakeDeal;

	UINT	m_State;	//״̬
public://����
	bool virtual Work(std::function<DealSocket*()> dealSocketMkr) = 0;
	void SetAF(UINT u){m_af = u;};
	void SetErrorFunc(DealError fn,void* fnPra = NULL){m_efunc = fn;m_efuncParam = fnPra;};//void*�Ǵ���ĺ����ĵ�һ������

	void virtual close();
protected://����
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
	WinServerTCP(void);//�׳�INT����
	~WinServerTCP(void){};
private://����
	ULONG	m_ip;		//ip
	USHORT	m_port;		//port
	UINT	m_connectNum;//��������

public://����
	bool Work(std::function<DealSocket*()> dealSocketMkr);
	bool Work(std::function<DealSocket*()> dealSocketMkr,bool isThread);//���ø�����ָ�룬Ĭ���ɴ�������������ָ��
	bool Work(std::function<DealSocket*()> dealSocketMkr,LPCTSTR ip,unsigned short port,bool isThread = false);//���ø�����ָ�룬Ĭ���ɴ�������������ָ��
	void SetIP(LPCTSTR);
	void SetPORT(unsigned short u);
	void SetConnectNum(UINT u){m_connectNum = u;};

protected://����
	void Select();
	void Thread();
	static unsigned int WINAPI threadCallBack(LPVOID);

};

class WinServerUDP:
	public WinServer
{
public:
	WinServerUDP(void);//�׳�INT����
	~WinServerUDP(void){};
private://����
	ULONG	m_ip;		//ip
	USHORT	m_port;		//port

public://����
	bool Work(std::function<DealSocket*()> dealSocketMkr);
	bool Work(std::function<DealSocket*()> dealSocketMkr,LPCTSTR ip,unsigned short port);
	void SetIP(LPCTSTR);
	void SetPORT(unsigned short u);

protected://����
	void Select();
};