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
#include<signal.h>
#include<errno.h>

#define MYPORT 9877
#define QUEUE 20
#define BUFFER_SIZE 1024

void sigfunc(int signo)
{
	pid_t pid;
	int status;
	while((pid=waitpid(-1,&status,WNOHANG))>0)
		printf("child %d terminated!\n",pid);
}

int main()
{
	//定义socket
	int server_sockfd = socket(AF_INET,SOCK_STREAM,0);	
	//定义sockaddr_in
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(MYPORT);
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//bind，成功返回0,出错返回-1
	if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
	{
		perror("bind error!\n");
		exit(1);
	}
	//lisen,成功返回0,出错返回-1
	if(listen(server_sockfd,QUEUE)==-1)
	{
		perror("lisen error!\n");
		exit(1);
	}
	
	//成功返回非负描述字，出错返回-1
	int count=0;
	signal(SIGCHLD,sigfunc);
	while(1)
	{
		//客户端套接字
		char buffer[BUFFER_SIZE];
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);
		int conn = accept(server_sockfd,(struct sockaddr*)&client_addr,&length);
		if(conn<0)
		{
			if(errno==EINTR)
			{
				continue;
			}
			perror("connect");
	 		exit(1);
		}
		int pid;
		if((pid=fork())==0)
		{
			close(server_sockfd);
			while(1)
			{
				memset(buffer,0,sizeof(buffer));
				int len = recv(conn,buffer,sizeof(buffer),0);
				if(strcmp(buffer,"exit\n")==0||len==0)
				{
					break;
				}
				fputs(buffer,stdout);
				send(conn,buffer,len,0);
			}
			close(conn);
			exit(0);
		}
		close(conn);
		count++;
	}
	return 0;
}
