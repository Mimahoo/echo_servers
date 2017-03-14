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
#include<pthread.h>
#include<sys/wait.h>

#define MYPORT 9877
#define QUEUE 20
#define BUFFER_SIZE 1024

void doEcho(int sockfd)
{
	char buff[BUFFER_SIZE];
	while(1)
	{
		memset(buff,0,sizeof(buff));
		int len=recv(sockfd,buff,BUFFER_SIZE,0);
		if(strcmp(buff,"exit\n")==0||len==0)
		{
			printf("%d exit!\n",sockfd);
			return;
		}
		fputs(buff,stdout);
		send(sockfd,buff,len,0);
	}
}

static void* doit(void* arg)
{
	int sockfd=*(int*)arg;
	free(arg);
	pthread_detach(pthread_self());
	doEcho(sockfd);
	close(sockfd);
	return NULL;
} 


int main()
{
	//定义socket
	int server_sockfd = socket(AF_INET,SOCK_STREAM,0);	
	pthread_t tid;
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
	while(1)
	{
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
		int *pconnfd=(int*)malloc(sizeof(int));
		*pconnfd=conn;
		pthread_create(&tid,NULL,&doit,pconnfd);
	}
	return 0;
}
