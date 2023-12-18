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
	int conn_num = 0;
	int flag = 0;
	int sockfd;
	int conn_sockfd;
	int ret;
	int n_read;
	int n_write;
	int len = sizeof(struct sockaddr_in);
	char readbuf[128];
	char msg[128];
 
	int fd; //fifo
	char fifo_readbuf[20] = {0};
	char *fifo_msg = "quit";
 
	pid_t fork_return;
	pid_t fork_return_1;
 
	struct sockaddr_in my_addr;
	struct sockaddr_in client_addr;
	memset(&my_addr,0,sizeof(struct sockaddr_in));
	memset(&client_addr,0,sizeof(struct sockaddr_in));
 
	if(argc != 3){
		printf("param error!\n");
		return 1;
	}
 
	//socket
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		perror("socket");
		return 1;
	}else{
		printf("socket success, sockfd = %d\n",sockfd);
	}
 
	//bind
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(argv[2]));//host to net (2 bytes)
	inet_aton(argv[1],&my_addr.sin_addr); //char* format -> net format
 
	ret = bind(sockfd, (struct sockaddr *)&my_addr, len);
	if(ret == -1){
		perror("bind");
		return 1;
	}else{
		printf("bind success\n");
	}
 
	//listen
	ret = listen(sockfd,10);
	if(ret == -1){
		perror("listen");
		return 1;
	}else{
		printf("listening...\n");
	}
 
	//fifo
	if(mkfifo("./fifo1",S_IRWXU) == -1 && errno != EEXIST)
	{
		perror("fifo");
	}
 
 
	while(1){
		//accept
		conn_sockfd = accept(sockfd,(struct sockaddr *)&client_addr,&len);
		if(conn_sockfd == -1){
			perror("accept");
			return 1;
		}else{
			conn_num++;
			if(conn_num > 1){
				printf("there are more then one client, msg may not be sent accuratly!\n");
			}
			printf("accept success, no.%d client IP = %s\n",conn_num,inet_ntoa(client_addr.sin_addr));
 
		}
 
		fork_return = fork();
 
		if(fork_return > 0){//father keeps waiting for new request
			//wait(NULL); //cant wait,will block	
		}else if(fork_return < 0){
			perror("fork");
			return 1;
		}else{//son deals with request
			fork_return_1 = fork();
			if(fork_return_1 > 0){//father keeps writing msg
				while(1){
					fd = open("./fifo1",O_RDONLY|O_NONBLOCK);
					lseek(fd, 0, SEEK_SET);
					read(fd,&fifo_readbuf,20);
					//printf("read from fifo:%s\n",fifo_readbuf);
					if(fifo_readbuf[0]=='q' && fifo_readbuf[1]=='u' && fifo_readbuf[2]=='i' && fifo_readbuf[3]=='t'){
						printf("sorry,the last msg sent fail,client has quit\n");
						close(fd);
						close(conn_sockfd);
						wait(NULL);
						break;
					}
 
 
					//write
					memset(&msg,0,sizeof(msg));
					//printf("\ntype msg:");
					scanf("%s",(char *)msg);
					n_write = write(conn_sockfd,&msg,strlen(msg));
					if(n_write == -1){
						perror("write");
						return 1;
					}else{
						printf("%d bytes msg sent\n",n_write);
 
					}
				}
 
			}else if(fork_return_1 < 0){
				perror("fork");
				return 1;
			}else{//son keeps reading msg
				while(1){
					//read
					memset(&readbuf,0,sizeof(readbuf));
					n_read = read(conn_sockfd,&readbuf,128);
					if(readbuf[0]=='q' && readbuf[1]=='u' && readbuf[2]=='i' && readbuf[3]=='t'){
						printf("client quit\n");
						conn_num--;
						printf("%d client remain\n",conn_num);
						write(conn_sockfd,"BYE",3);
						fd = open("./fifo1",O_WRONLY);
						write(fd,fifo_msg,strlen(fifo_msg));
						exit(1);
					}
					if(n_read == -1){
						perror("read");
						return 1;
					}else{
						printf("\nclient: %s\n",readbuf);
					}
 
				}
			}
			exit(2);
		}
 
	}
 
 
	return 0;
}
