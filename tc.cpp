#include "tc.h"
#include "getopt.h"

#define CONN_LOST -1
#define SEND_ERROR -2
#define RECV_ERROR -3
#define SELECT_ERROR -4

int wait=0;		//wait time(second)
int repeat=0;		//Repeat times
bool u_flag=false;		//UDP flags
bool autoconn=false;	//Automatic Connection when connection lost
bool showtime=false;
bool listenmode=false;
bool verbose=false;
timeval tv;
const char* ptr_probe="......";		//Probe Datagram

int serv_process(sockaddr_in* ptr_saddr);
int tcp_handler(sockaddr_in* ptr_saddr);
int udp_handler(sockaddr_in* ptr_saddr);
int tcp_listener(sockaddr_in* ptr_saddr);
int udp_listener(sockaddr_in* ptr_saddr);
int getcmd(char* cmd);
int tu_send(SOCKET* ptr_sock,char* ptr_send,bool u_flag,sockaddr_in* servAddr);
int tu_recv(SOCKET* ptr_sock,char* ptr_recv,timeval* timeout,bool u_flag);
bool isconn(SOCKET* ptr_sock);
int tcpworkonce(SOCKET* ptr_sock,char* command);
int udpworkonce(SOCKET* ptr_sock,sockaddr_in* ptr_remoteaddr,char* command);
void connlostwarn(bool autoconn);

int main(int argc,char* argv[])
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |
            FOREGROUND_GREEN);
	cout<<"<---------------TC tool by Michael Jiang @ UESTC--------------->"<<endl;
	cout<<"<---------------Free use, for network interface test----------->"<<endl;
	cout<<"<---------------Version:1.1  Release Data:2012 Aug.20---------->"<<endl;
	
	//参数处理
	int tmp_argv;
	char serv_ip[16]={0},listenip[16]={0};
	unsigned int serv_port=0;

	while((tmp_argv=getopt(argc,argv,"s:p:w:r:i:lvucth"))!=-1)
	{
		switch(tmp_argv)
		{
		case 's':
			strcpy(serv_ip,optarg);
			break;
		case 'p':
			serv_port=atoi(optarg);
			break;
		case 'i':
			strcpy(serv_ip,optarg);
			break;
		case 'l':
			listenmode=true;
			break;
		case 'w':
			wait=atoi(optarg);
			break;
		case 'r':
			repeat=atoi(optarg);
			break;
		case 'u':
			u_flag=true;
			break;
		case 'c':
			autoconn=true;
			break;
		case 't':
			showtime=true;
			break;
		case 'v':
			verbose=true;
			break;
		case 'h':
			;
		default:
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |
            FOREGROUND_RED);
			cout<<"Input error!"<<endl;
			cout<<"Usage for Client Mode:tc -s <SERVER_IP> -p <SERVER_PORT> [-w <wait time>] [-r <repeat times>] [-u] [-c] [-t] [-v]"<<endl;
			cout<<"Usage for Server Mode:tc [-i <LISTEN_IP>] -p <LISTEN_PORT> -l [-u] [-v]"<<endl;
			exit(EXIT_FAILURE);
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	//套接字初始化工作
	DWORD wVersion;
	WSADATA wsaData;

	wVersion=MAKEWORD(2,2);
	if(WSAStartup(wVersion,&wsaData))
	{	
		fputs("Load Winsock lib error!\n",stderr);
		exit(1);
	}

	sockaddr_in servAddr;

	memset(&servAddr,0,sizeof(servAddr));

	servAddr.sin_family=AF_INET;
	servAddr.sin_port=htons(serv_port);
	if(strlen(serv_ip))
		servAddr.sin_addr.s_addr=inet_addr(serv_ip);
	else
		servAddr.sin_addr.s_addr=htonl(INADDR_ANY);

	if(listenmode)
		serv_process(&servAddr);
	else
	{
		if(u_flag)
			udp_handler(&servAddr);
		else
			tcp_handler(&servAddr);
	}

	return 0;
}
//服务器模式处理流程
int serv_process(sockaddr_in* ptr_saddr)
{
	if(u_flag)
		udp_listener(ptr_saddr);
	else
		tcp_listener(ptr_saddr);

	return 0;
}
//TCP监听模式
int tcp_listener(sockaddr_in* ptr_saddr)
{
	SOCKET socklisten=socket(AF_INET,SOCK_STREAM,0);

	if(socklisten==INVALID_SOCKET)
	{
		fprintf(stderr,"Create socket error!The error code=%d.\n",WSAGetLastError());
		return -1;
	}

	if(bind(socklisten,(sockaddr*)ptr_saddr,sizeof(sockaddr))==SOCKET_ERROR)
	{
		fprintf(stderr,"Bind socket error!The error code=%d.\n",WSAGetLastError());
		return -1;
	}

	if(listen(socklisten,10)==SOCKET_ERROR)
	{
		fprintf(stderr,"Listen error!The error code=%d.\n",WSAGetLastError());
		return -1;
	}

	cout<<"Tc is listening on "<<inet_ntoa(ptr_saddr->sin_addr)<<endl;

	SOCKET sockconn;
	sockaddr_in cliAddr;
	int clientLen=sizeof(cliAddr);
	
	sockconn=accept(socklisten,(sockaddr*)&cliAddr,&clientLen);
	if(sockconn==INVALID_SOCKET)
	{
		fprintf(stderr,"Listen error!The error code=%d.\n",WSAGetLastError());
		return -1;
	}
	
	if(verbose)
	{
		cout<<"Client:"<<inet_ntoa(cliAddr.sin_addr)<<" : "<<ntohs(cliAddr.sin_port)<<" connected!"<<endl;
	}
	
	while(true)
	{
		char recvbuf[4096]={0},sendbuf[4096]={0};
		int recvBytes=0;
		
		recvBytes=recv(sockconn,recvbuf,sizeof(recvbuf),0);
		
		if(recvBytes==SOCKET_ERROR)
		{
			fprintf(stderr,"Recv error!The error code=%d.\n",WSAGetLastError());
			return -1;
		}
		else if(!recvBytes)
		{
			fputs("Client close the connection!",stdout);
			break;
		}
		
		cout<<"Client says:"<<recvbuf<<endl;
		if(verbose)
			cout<<"Total Bytes:"<<strlen(recvbuf)<<endl;

		cout<<"Your reply:";
		
		cin>>sendbuf;
		//若输入:exit,则关闭连接
		if(_strnicmp(sendbuf,":exit",strlen(":exit")))
		{
			if(SOCKET_ERROR==send(sockconn,sendbuf,strlen(sendbuf),0))
			{
				fprintf(stderr,"Send error!The error code=%d.\n",WSAGetLastError());
				break;
			}
		}
		else
			closesocket(sockconn);
	}

	return 0;
}
//UDP监听模式
int udp_listener(sockaddr_in* ptr_saddr)
{
	SOCKET socklisten=socket(AF_INET,SOCK_DGRAM,0);

	if(socklisten==INVALID_SOCKET)
	{
		fprintf(stderr,"Create socket error!The error code=%d.\n",WSAGetLastError());
		return -1;
	}

	if(bind(socklisten,(sockaddr*)ptr_saddr,sizeof(sockaddr))==SOCKET_ERROR)
	{
		fprintf(stderr,"Bind socket error!The error code=%d.\n",WSAGetLastError());
		return -1;
	}

	cout<<"Tc is waiting on "<<inet_ntoa(ptr_saddr->sin_addr)<<endl;
	
	while(true)
	{
		sockaddr_in cliAddr;
		int clientLen=sizeof(cliAddr);
		char recvbuf[4096]={0},sendbuf[4096]={0};
		int recvBytes=0;
		
		memset(&cliAddr,0,sizeof(cliAddr));
		recvBytes=recvfrom(socklisten,recvbuf,sizeof(recvbuf),0,(sockaddr*)&cliAddr,&clientLen);
		
		if(recvBytes==SOCKET_ERROR)
		{
			fprintf(stderr,"Recv error!The error code=%d.\n",WSAGetLastError());
			return -1;
		}
		else if(!recvBytes)
		{
			fputs("Client sent you a 0 byte datagram!",stdout);
			break;
		}

		if(verbose)
		{
			cout<<"Client\t"<<inet_ntoa(cliAddr.sin_addr)<<":"<<ntohs(cliAddr.sin_port)<<" say:"<<recvbuf<<endl;
			cout<<"Total Bytes:"<<strlen(recvbuf)<<endl;
		}
		else
			cout<<"Client says:"<<recvbuf<<endl;
		
		cout<<"Your reply:";
		
		cin>>sendbuf;
		//若输入:exit,则关闭连接
		if(_strnicmp(sendbuf,":exit",strlen(":exit")))
		{
			if(SOCKET_ERROR==sendto(socklisten,sendbuf,strlen(sendbuf),0,(sockaddr*)&cliAddr,clientLen))
			{
				fprintf(stderr,"Send error!The error code=%d.\n",WSAGetLastError());
				break;
			}
		}
		else
			break;
	}

	closesocket(socklisten);
	return 0;
}
//TCP业务处理
int tcp_handler(sockaddr_in* ptr_saddr)
{
	int trytimes=repeat;
	char command[4096]={0};

	RECONNECT:
	SOCKET clientsock;
	char recvBuf[4096]={0};

	if((clientsock=socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET)
	{
		fprintf(stderr,"Create socket error!The error code=%d.\n",GetLastError());
		return -1;
	}

	if(connect(clientsock,(sockaddr*)ptr_saddr,sizeof(sockaddr))==SOCKET_ERROR)
	{
		fprintf(stderr,"Connect to server error!The error code=%d.\n",GetLastError());
		return -1;
	}

	if(wait>0)
	{
		tv.tv_sec=wait;
		tv.tv_usec=0;
	}

	if(!repeat)
	{
		while(getcmd(command))
		{
			if(tcpworkonce(&clientsock,command)==CONN_LOST)
				goto RECONNECT;

			memset(command,0,sizeof(command));
		}
	}
	else if(repeat>0)
	{
		if(trytimes==repeat)
		{
			cout<<"Please input commands:";
			gets(command);
		}
	//	cout<<"trytimes="<<trytimes<<endl;
		while(trytimes--)
		{
			if(tcpworkonce(&clientsock,command)==CONN_LOST)
			{
			//	Sleep(1000);
				goto RECONNECT;
			}
		}
	}

	closesocket(clientsock);

	return 0;

}
//TCP一次通信业务
int tcpworkonce(SOCKET* ptr_sock,char* command)
{
	int retval;
	char recvBuf[4096]={0};
	DWORD dwStart,dwEnd;

	if(showtime)
		dwStart=timeGetTime();

	if(tu_send(ptr_sock,command,u_flag,NULL)==-1)
				return CONN_LOST;

	if(!wait)
	{
		retval=tu_recv(ptr_sock,recvBuf,NULL,u_flag);
		if(retval==-1)
			return SELECT_ERROR;
		else if(!retval)
		{
			connlostwarn(autoconn);
			return CONN_LOST;
		}
		cout<<recvBuf<<endl;
		if(verbose)
			cout<<"Total Bytes:"<<strlen(recvBuf)<<endl;
	}
	else
	{
		timeval tmp=tv;
		int retval=tu_recv(ptr_sock,recvBuf,&tmp,u_flag);

		if(retval==-1)
			return SELECT_ERROR;
		else if(!retval)
			cout<<"Wait timeout!"<<endl;
		else
		{
			cout<<recvBuf<<endl;
			if(verbose)
				cout<<"Total Bytes:"<<strlen(recvBuf)<<endl;
		}
	}

	if(showtime)
	{
		dwEnd=timeGetTime();
		cout<<"Recv time use:"<<dwEnd-dwStart<<" ms."<<endl;
	}
			
/*	if(!isconn(ptr_sock))
	{
		connlostwarn(autoconn);
		
		return CONN_LOST;
	}*/

	return 0;
}
//UDP业务处理
int udp_handler(sockaddr_in* ptr_saddr)
{
	SOCKET clientsock;
	timeval tv;
	int trytimes=repeat;
	char command[4096]={0};
	char recvBuf[4096]={0};

	if((clientsock=socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET)
	{
		fprintf(stderr,"Create socket error!The error code=%d.\n",GetLastError());
		return -1;
	}

	if(wait>0)
	{
		tv.tv_sec=wait;
		tv.tv_usec=0;
	}

	if(!repeat)
	{
		while(getcmd(command))
		{
			if(udpworkonce(&clientsock,ptr_saddr,command))
				return -1;
		}
	}
	else if(repeat>0)
	{
		if(trytimes==repeat)
		{
			cout<<"Please input commands:";
			gets(command);
		}
		while(trytimes--)
		{
			if(udpworkonce(&clientsock,ptr_saddr,command))
				return -1;
		}
	}

	closesocket(clientsock);

	return 0;

}
//UDP一次通信业务
int udpworkonce(SOCKET* ptr_sock,sockaddr_in* ptr_remoteaddr,char* command)
{
	char recvBuf[4096]={0};
	DWORD dwStart,dwEnd;

	if(showtime)
		dwStart=timeGetTime();

	if(tu_send(ptr_sock,command,u_flag,ptr_remoteaddr)==-1)
		return SEND_ERROR;
	
	if(!wait)
	{
		if(tu_recv(ptr_sock,recvBuf,NULL,u_flag)==-1)
			return RECV_ERROR;

		cout<<recvBuf<<endl;
		if(verbose)
			cout<<"Total Bytes:"<<strlen(recvBuf)<<endl;
	}
	else
	{
		timeval tmp=tv;
		int retval=tu_recv(ptr_sock,recvBuf,&tmp,u_flag);
		
		if(retval==-1)
			return SELECT_ERROR;
		else if(!retval)
			cout<<"Wait timeout!"<<endl;
		else
		{
			cout<<recvBuf<<endl;
			if(verbose)
				cout<<"Total Bytes:"<<strlen(recvBuf)<<endl;
		}

	}
	
	if(showtime)
	{
		dwEnd=timeGetTime();
		cout<<"Recv time use:"<<dwEnd-dwStart<<" ms."<<endl;
	}

	return 0;
}
//TCP和UDP复用以下发送函数
int tu_send(SOCKET* ptr_sock,char* ptr_send,bool u_flag,sockaddr_in* servAddr)
{
	if(u_flag)
	{
		if(sendto(*ptr_sock,ptr_send,strlen(ptr_send),0,(sockaddr*)servAddr,sizeof(sockaddr))==SOCKET_ERROR)
		{
			fprintf(stderr,"Send error!The error code is %d.\n",WSAGetLastError());
			return -1;
		}
	}
	else
	{
		if(send(*ptr_sock,ptr_send,strlen(ptr_send),0)==SOCKET_ERROR)
		{
			fprintf(stderr,"Send error!The error code is %d.\n",WSAGetLastError());
			return -1;
		}
	}

	return 1;
}
//TCP和UDP复用以下接收函数
int tu_recv(SOCKET* ptr_sock,char* ptr_recv,timeval* timeout,bool u_flag)
{
	int retval,recvBytes=0;
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(*ptr_sock,&rfds);

	retval=select(-1,&rfds,NULL,NULL,timeout);

	if(retval==SOCKET_ERROR)
	{
		fputs("Select error!\n",stderr);
		return -1;
	}
	else if(retval)
	{
		if(u_flag)
		{
			if(SOCKET_ERROR==(recvBytes=recvfrom(*ptr_sock,ptr_recv,4096,0,NULL,NULL)))
			{
				fputs("Recevie error!",stderr);
				return -1;
			}
		}
		else
		{
			if(SOCKET_ERROR==(recvBytes=recv(*ptr_sock,ptr_recv,4096,0)))
			{
				fputs("Recevie error!",stderr);
				return -1;
			}
		}

		return recvBytes;
	}

	return 0;
}
//获取用户输入
int getcmd(char* cmd)
{
	cout<<"Please input command:";
	gets(cmd);
	if(!strcmp(cmd,":exit"))	//输入:exit则退出
		return 0;
	return 1;
}
//判断TCP连接是否断开
bool isconn(SOCKET* ptr_sock)
{
	int i=2;
	while(i--)
	{
		if(send(*ptr_sock,ptr_probe,strlen(ptr_probe),0)==SOCKET_ERROR)
			return false;

		if(send(*ptr_sock,ptr_probe,strlen(ptr_probe),0)==SOCKET_ERROR)
			return false;
	}

	return true;
}
//TCP连接断开警告
void connlostwarn(bool autoconn)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |
            FOREGROUND_RED);

	cout<<"Remote Server close the connection."<<endl;
	if(autoconn)
		cout<<"Auto connection enable.Connecting to the remote server..."<<endl;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_INTENSITY |
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}