#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netdb.h>
#include<string>
#include<iostream>
#include <sys/time.h>

using namespace std;

void error(const char *msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char* argv[]){
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (sockfd < 0)
    		error("ERROR opening socket");
	struct sockaddr_in serv_addr; 
  	struct hostent *server;
  	char buffer[40960]; 
	int portno = atoi(argv[2]); 
  	string filename=argv[3];
  	server = gethostbyname(argv[1]);

  	if (server == NULL) {
    		fprintf(stderr, "ERROR, no such host\n");
    		exit(0);
  	}

  	bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

  	serv_addr.sin_family = AF_INET; // Address Family is IP

  	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
  	serv_addr.sin_port = htons(portno);
  	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
 	   error("ERROR connecting");
 	bzero(buffer, 40960); //reset buffer to zero

  	int fd=open(filename.c_str(),O_RDONLY,S_IRWXU);
  	long size=lseek(fd,0,SEEK_END);
  	lseek(fd,0,SEEK_SET);
  	read(fd,buffer,size);
  	int n = write(sockfd, buffer, strlen(buffer));

  	if (n < 0)
    		error("ERROR writing to socket");
  	bzero(buffer, 256);

  	n = read(sockfd, buffer, 255);
 	 if (n < 0)
 	   error("ERROR reading from socket");
	printf("Server response: %s\n", buffer);

 	return 0;
}
