#include <sys/types.h>
#include <sys/stat.h>      
#include <sys/socket.h>
#include <error.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <strings.h>
#include <limits.h>
#include <pthread.h>

using namespace std;

pthread_mutex_t mutex;


int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("usage %s <port no> <max_pool_size>\n",argv[0]);
        exit(1);
    }
    int queuelength=atoi(argv[2]);
	int socketfd=socket(AF_INET,SOCK_STREAM,0);
	if(socketfd<0)
		error("Error creating the socket");
	struct sockaddr_in server_address,client_address;
	socklen_t client_length;
	int portno=atoi(argv[1]);
	char buffer[1024];
	bzero((char *)&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(portno);
	int binded=bind(socketfd, (struct sockaddr*) &server_address, sizeof(server_address));
	if(binded<0)
		error("Error Binding");
	if(listen(socketfd,100)==-1)
    	error("Error listening at server");
    	client_length=sizeof(client_address);
    	pthread_mutex_init(&mutex,NULL);
    while (1){
    	pthread_mutex_lock(&lock);   
    	int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
   
    	if (newsockfd < 0){
      		perror("ERROR on accept"); 
      		pthread_mutex_unlock(&lock);
      		continue;
    	}

     	pthread_t thread;
     	if (pthread_create(&thread, NULL, start_function, &newsockfd) != 0){
       		perror("Failed to create Thread");
       		close(newsockfd);
       		pthread_mutex_unlock(&lock);
     	}
    }
    close(sockfd);
    return 0;
}