#include <sys/types.h>     
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/in.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
 
 
int main(int argc, char **argv)
{
	int sockfd;
	int ret;
	int n_read;
	int n_write;
	char readbuf[128];
	char msg[128];
 
	int fd; //fifo
	char fifo_readbuf[20] = {0};
	char *fifo_msg = "quit";
 
	pid_t fork_return;
 
	if(argc != 3){
		printf("param error!\n");
		return 1;
	}
 
 
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(struct sockaddr_in));
 
	//socket
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		perror("socket");
		return 1;
	}else{
		printf("socket success, sockfd = %d\n",sockfd);
	}
 
	//connect
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));//host to net (2 bytes)
	inet_aton(argv[1],&server_addr.sin_addr); 
	ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if(ret == -1){
		perror("connect");
		return 1;
	}else{
		printf("connect success!\n");
	}
 
	//fifo
	if(mkfifo("./fifo",S_IRWXU) == -1 && errno != EEXIST)
	{
		perror("fifo");
	}
 
	//fork
	fork_return = fork();
 
	if(fork_return > 0){//father keeps writing msg
		while(1){
			//write
			memset(&msg,0,sizeof(msg));
			//printf("\ntype msg:");
			scanf("%s",(char *)msg);
			n_write = write(sockfd,&msg,strlen(msg));
			if(msg[0]=='q' && msg[1]=='u' && msg[2]=='i' && msg[3]=='t'){
				printf("quit detected!\n");
				fd = open("./fifo",O_WRONLY);
				write(fd,fifo_msg,strlen(fifo_msg));
				close(fd);
				close(sockfd);
				wait(NULL);
				break;
			}
			if(n_write == -1){
				perror("write");
				return 1;
			}else{
				printf("%d bytes msg sent\n",n_write);
			}
		}
	}else if(fork_return < 0){
		perror("fork");
		return 1;
	}else{//son keeps reading 
		while(1){
			fd = open("./fifo",O_RDONLY|O_NONBLOCK);
			lseek(fd, 0, SEEK_SET);
			read(fd,&fifo_readbuf,20);
			//printf("read from fifo:%s\n",fifo_readbuf);
			if(fifo_readbuf[0]=='q' && fifo_readbuf[1]=='u' && fifo_readbuf[2]=='i' && fifo_readbuf[3]=='t'){
				exit(1);
			}
 
			//read
			memset(&readbuf,0,sizeof(readbuf));
			n_read = read(sockfd,&readbuf,128);
			if(n_read == -1){
				perror("read");
				return 1;
			}else{
				printf("\nserver: %s\n",readbuf);
			}
		}
 
	}
 
 
	return 0;
}
