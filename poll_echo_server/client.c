#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/shm.h>
#include<poll.h>


#define MYPORT 9877
#define BUFFER_SIZE 1024

int main()
{
	//定义socket
	int sock_cli = socket(AF_INET,SOCK_STREAM,0);
	//定义sockaddr_in
	struct sockaddr_in servaddr;
	
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(MYPORT);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	//连接服务器,成功返回0,出错返回-1
	if(connect(sock_cli,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
	{
		perror("connect error!\n");
		exit(1);
	}
	
	struct pollfd fds[2];
	fds[0].fd=fileno(stdin);
	fds[0].events=POLLIN;
	fds[1].fd=sock_cli;
	fds[1].events=POLLIN;
	
	char sendbuf[BUFFER_SIZE];
	char recvbuf[BUFFER_SIZE];
	memset(sendbuf,0,sizeof(sendbuf));
	memset(recvbuf,0,sizeof(recvbuf));
	while(1)
	{
		poll(fds,2,-1);
		if(fds[1].revents&POLLIN)
		{
			int len=recv(sock_cli,recvbuf,sizeof(recvbuf),0);
			{
				if(len==0)
				{
					printf("server error!\n");
					return 0;
				}
			}
			fputs(recvbuf,stdout);
		}
		if(fds[0].revents&POLLIN)
		{
			fgets(sendbuf,sizeof(sendbuf),stdin);
			send(sock_cli,sendbuf,strlen(sendbuf),0);
			if(strcmp(sendbuf,"exit\n")==0)
			{
				break;
			}
		}
		memset(sendbuf,0,sizeof(sendbuf));
		memset(recvbuf,0,sizeof(recvbuf));
	}
	close(sock_cli);
	return 0;
}
