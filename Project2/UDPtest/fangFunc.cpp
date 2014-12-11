#include <iostream>
#include <cstdlib>
using namespace std;
#include "WinServer.h"

class SendRecv:public DealSocket
{
	UINT m_Sendto;
public:
	SendRecv(UINT Sendto = 0){
		m_Sendto = Sendto;
	};
	virtual dealSocketRtn initState(){
		if (m_Sendto != 0)
		{
			struct sockaddr_in LocalAddr;
			LocalAddr.sin_family = AF_INET;
			LocalAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			LocalAddr.sin_port = htons(m_Sendto);
			memset(LocalAddr.sin_zero, 0x00, 8);
			m_pCliSock->SetSockaddr_in(LocalAddr);
			return DEAL_GO_ON_SEND;
		}
		else
		{
			return DEAL_GO_ON_RECV;
		}
	}
	virtual dealSocketRtn dealSend(){
		cout<<"please input:"<<endl;
		struct sockaddr_in saddr_in = m_pCliSock->GetSockaddr_in();
		char buf[1024];
		cin>>buf;
		if (SOCKET_ERROR == sendto(m_pCliSock->GetSOCK(),buf,1024,0,
			(sockaddr*)&saddr_in,sizeof(struct sockaddr_in)))
		{
			return DEAL_CLOSE;
		}
		else
		{
			return DEAL_GO_ON_RECV;
		}
	};
	virtual dealSocketRtn dealRecv(){
		char buf[1024]={0};
		struct sockaddr_in saddr_in ;
		int n = sizeof(struct sockaddr);
		int nRecv = recvfrom(m_pCliSock->GetSOCK(), buf, 1024,0,(sockaddr*)&saddr_in,&n);
		if(nRecv>0)
		{
			cout<<m_pCliSock->GetSOCK();
			cout<<"->recv data:\n"<<buf<<endl;
			m_pCliSock->SetSockaddr_in(saddr_in);
			return DEAL_GO_ON_SEND;
		}
		else
		{
			return DEAL_GO_ON_RECV;
		}

	};
};

int main()
{
	WinServerUDP ws;
	int recvHost = 8890,sendHost= 8891;
	int flag;
	cout<<"输入初始状态："<<endl;
	cin>>flag;
	try
	{
		if (flag)
		{
			ws.Work([recvHost](){return new SendRecv(recvHost);},"127.0.0.1",sendHost);
			//发送 本地：sendHost=8891 ,接收端 recvHost = 8890
		}
		else
		{
			ws.Work([](){return new SendRecv();},"127.0.0.1",recvHost);
			//接收 本地：recvHost=8890 ,接收端 sendHost = 8891
		}
	}
	catch (...)
	{
		system("pause");
	}
	system("pause");
}
