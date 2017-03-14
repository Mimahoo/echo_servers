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
#include<errno.h>
#include<sys/shm.h>

#define MYPORT 9877
#define QUEUE 20
#define BUFFER_SIZE 1024

int main()
{
	//定义socket
	int server_sockfd = socket(AF_INET,SOCK_STREAM,0);	
	
	fd_set readfds,testfds;
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
	//设置readfds
	FD_ZERO(&readfds);
	//把server_sockfd加入readfds
	FD_SET(server_sockfd,&readfds);
	while(1)
	{
		int fd;
		int nread;
		testfds=readfds;
		
		result=select(FD_SETSIZE,&testfds,(fd_set*)0,(fd_set*)0,(struct timeval*)0);
		//客户端套接字
		char buffer[BUFFER_SIZE];
		struct sockaddr_in client_addr;
		socklen_t length = sizeof(client_addr);
		int conn;
		//轮询处理集合中的fd
		for(fd=0;fd<FD_SETSIZE;fd++)
		{
			if(FD_ISSET(fd,&testfds))
			{
				if(fd==server_sockfd)
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
					//把conn加入到集合之中
					FD_SET(conn,&readfds);
				}
				else
				{
					ioctl(fd,FIONREAD,&nread);
					if(nread==0)
					{
						close(fd);
						FD_CLR(fd,&readfds);
						printf("%d exit!\n",fd);
					}
					else
					{
						//处理客户端请求
						memset(buffer,0,sizeof(buffer));
						int len = recv(fd,buffer,sizeof(buffer),0);
						if(strcmp(buffer,"exit\n")==0||len==0)
						{
							continue;
						}
						fputs(buffer,stdout);
						send(conn,buffer,len,0);
					}
				}
			}
		}
	}
	close(server_sockfd);
	return 0;
}
