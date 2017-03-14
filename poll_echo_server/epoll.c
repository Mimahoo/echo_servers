#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<netinet/in.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/shm.h>
#include<sys/epoll.h>
#include<errno.h>

#define MYPORT 9877
#define QUEUE 1024
#define BUFFER_SIZE 1024
#define OPEN_MAX 1000

int main()
{
	int server_sockfd=socket(AF_INET,SOCK_STREAM,0);
	
	struct sockaddr_in servaddr,cliaddr;
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htons(INADDR_ANY);
	servaddr.sin_port=htons(MYPORT);
	
	if(bind(server_sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1)
	{
		printf("bind error!\n");
		exit(1);
	}
	
	if(listen(server_sockfd,QUEUE)==-1)
	{
		printf("listen error!\n");
		exit(1);
	}
	
	struct epoll_event ev,events[OPEN_MAX];
	int epollfd=epoll_create(OPEN_MAX);
	
	ev.data.fd=server_sockfd;
	ev.events=EPOLLIN|EPOLLET;
	
	epoll_ctl(epollfd,EPOLL_CTL_ADD,server_sockfd,&ev);
	char buff[BUFFER_SIZE];
	
	while(1)
	{
		int n=epoll_wait(epollfd,events,OPEN_MAX,500);
		for(int i=0;i<n;i++)
		{
			if(events[i].data.fd==server_sockfd)
			{
				int length=sizeof(cliaddr);
				int conn=accept(server_sockfd,(struct sockaddr*)&cliaddr,&length);
				if(conn==-1)
				{
					printf("connect error!\n");
					continue;
				}
				printf("connection from %s ,port %d.\n",inet_ntop(AF_INET,(void*)&cliaddr.sin_addr,buff,sizeof(buff)),ntohs(cliaddr.sin_port));
				ev.data.fd=conn;
				ev.events=EPOLLIN|EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_ADD,conn,&ev);
			}
			else if(events[i].events&EPOLLIN)
			{
				memset(buff,0,sizeof(buff));
				int fd=events[i].data.fd;
				if(fd<0) continue;
				
				int len=recv(fd,buff,sizeof(buff),0);
				if(len==0)
				{
					printf("%d exit!\n",fd);
					epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
					close(fd);
					//events[i].data.fd=-1;
				}
				fputs(buff,stdout);
				send(fd,buff,sizeof(buff),0);
			}
		}
	}
	return 0;
}






















