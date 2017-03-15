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
#include<errno.h>
#include<pthread.h>

#define MYPORT 9877
#define BUFFER_SIZE 1024
#define OPEN_MAX 1000


void echo(int fd)
{
	char sbuff[BUFFER_SIZE],rbuff[BUFFER_SIZE];
	//fd_set fds;
	//FD_ZERO(&fds);
	//FD_SET(fd,&fds);
	strcpy(sbuff,"ab\n");
	while(1)
	{
		send(fd,sbuff,sizeof(sbuff),0);
		usleep(1000);
		/*while(select(FD_MAXSIZE,&fds,NULL,NULL,(struct timeval*)0)==0)
		{
			usleep(1000);
		}*/
		int len=recv(fd,rbuff,sizeof(rbuff),0);
		printf("%d --",fd);
		if(len<=0)
		{
			printf("error!\n");
			return;
		}
		fputs(rbuff,stdout);
		memset(rbuff,0,BUFFER_SIZE);
	}
	
	//sleep(5);
	//sleep(20);
	return;
}

static void* doit(void *arg)
{
	int fd=*(int*)arg;
	free(arg);
	pthread_detach(pthread_self());
	echo(fd);
	close(fd);
	return NULL;
}

int main()
{
	struct sockaddr_in servaddr;
	int *fd;
	
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(MYPORT);
	servaddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	pthread_t pid;
	for(int i=0;i<100;i++)
	{
		fd=(int*)malloc(sizeof(int));
		*fd=socket(AF_INET,SOCK_STREAM,0);
		int len=sizeof(servaddr);
		if(connect(*fd,(struct sockaddr*)&servaddr,len)==-1)
		{
			printf("connect error!\n");
			break;
		}
		printf("thread:%d\n",i);
		if(pthread_create(&pid,NULL,&doit,fd)!=0)
		{
			printf("create error!\n");
		}
	}
	while(1)
	{
		
	}
	sleep(10);
	return 0;
}























