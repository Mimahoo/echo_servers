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
#include<poll.h>
#include<errno.h>

#define MYPORT 9877
#define QUEUE 1024
#define BUFFER_SIZE 1024
#define OPEN_MAX 1000


int main()
{
	//定义socket
	int server_sockfd = socket(AF_INET,SOCK_STREAM,0);	
	
	//定义struct pollfd数组
	struct pollfd fds[OPEN_MAX];
	int result;
	
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
	fds[0].fd=server_sockfd;
	fds[0].events=POLLIN;
	
	for(int i=1;i<OPEN_MAX;i++)
		fds[i].fd=-1;
	int max=0;
	while(1)
	{
		int conn;
		int i;
		char buffer[BUFFER_SIZE];
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);
		int nready=poll(fds,max+1,-1);
		if(fds[0].revents&POLLIN)
		{
			//成功返回非负描述字，出错返回-1
			conn = accept(server_sockfd,(struct sockaddr*)&client_addr,&length);
			if(conn<0)
			{
				if(errno==EINTR)
				{
					continue;
				}
				printf("connect error!\n");
	 			exit(1);
			}
			for(i=1;i<OPEN_MAX;++i)
			{
				if(fds[i].fd<0)
				{
					fds[i].fd=conn;
					break;
				}
			}
			if(i==OPEN_MAX)
			{
				printf("too many clients!\n");
				exit(1);
			}
			fds[i].events=POLLIN;
			if(i>max) max=i;
			if(--nready<=0) continue;
		}
		for(i=1;i<OPEN_MAX;i++)
		{
			if(fds[i].fd<0) continue;
			if(fds[i].revents&POLLIN)
			{
				memset(buffer,0,sizeof(buffer));
				int len = recv(fds[i].fd,buffer,sizeof(buffer),0);
				if(strcmp(buffer,"exit\n")==0||len==0)
				{
					printf("%d exit!\n",fds[i].fd);
					close(fds[i].fd);
					fds[i].fd=-1;
					continue;
				}
				fputs(buffer,stdout);
				send(fds[i].fd,buffer,len,0);
			}
		}	
	}
	close(fds[0].fd);
	return 0;
}
