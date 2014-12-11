#include "WinServer.h"
#include <iostream>
using namespace std;


class SendRecv:public DealSocket
{
	int i;
public:
	dealSocketRtn initState(){return DEAL_GO_ON_RECV;};
	virtual dealSocketRtn dealSend(){
		char buf[] = "HTTP/1.1 200 OK\r\n\
Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n\
Server: Apache\r\n\
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n\
ETag: \"34aa387-d-1568eb00\"\r\n\
Accept-Ranges: bytes\r\n\
Content-Length: 32\r\n\
Vary: Accept-Encoding\r\n\
Content-Type: text/plain\r\n\
\r\n\
	  输出结果：\n\
	  Hello World!\r\n\
	  ";
		cout<<"->发送数据："<<buf<<endl;
		if (SOCKET_ERROR == send(m_pCliSock->GetSOCK(),buf,strlen(buf)+1,0))
		{
			cout<<"->close"<<buf<<endl;
			return DEAL_CLOSE;
		}
		else
		{
			return DEAL_GO_ON_RECV;
		}
	};
	virtual dealSocketRtn dealRecv(){
		char buf[1024]={0};
		int nRecv = ::recv(m_pCliSock->GetSOCK(), buf, 1024,0);
		if(nRecv>0)
		{
			cout<<"->接收到数据：\n"<<buf<<endl;
			return DEAL_GO_ON_RECV_SEND;
		}
		else
		{
			cout<<"->close"<<buf<<endl;
			return DEAL_CLOSE;
		}
		
	};
};

#include <cstdlib>
int main()
{
	WinServerTCP ws;
	try
	{
		ws.Work(DealSocketMaker<SendRecv>(),"127.0.0.1",8090);
	}
	catch (...)
	{
		system("pause");
	}
}