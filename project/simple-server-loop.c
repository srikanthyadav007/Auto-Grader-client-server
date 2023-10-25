/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netinet/in.h>


void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[]) {
  int sockfd, //the listen socket descriptor (half-socket)
   newsockfd, //the full socket after the client connection is made
   portno; //port number at which server listens

  socklen_t clilen; //a type of an integer for holding length of the socket address
  char buffer[10000]; //buffer for reading and writing the messages
  struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
 // This is just a holder right now, note no port number either. It needs a 'bind' call


  if (sockfd < 0)
    error("ERROR opening socket");

 
  bzero((char *)&serv_addr, sizeof(serv_addr)); // initialize serv_address bytes to all zeros
  
  serv_addr.sin_family = AF_INET; // Address Family of INTERNET
  serv_addr.sin_addr.s_addr = INADDR_ANY;  //Any IP address. 

//Port number is the first argument of the server command
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  // Need to convert number from host order to network order

  /* bind the socket created earlier to this port number on this machine 
 First argument is the socket descriptor, second is the address structure (including port number).
 Third argument is size of the second argument */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 1); // 1 means 1 connection requests can be in queue. 
  //now server is listening for connections


  clilen = sizeof(cli_addr);  //length of struct sockaddr_in


  /* accept a new request, now the socket is complete.
  Create a newsockfd for this socket.
  First argument is the 'listen' socket, second is the argument 
  in which the client address will be held, third is length of second
  */
  char content[10000];
  while (1){
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    /* read message from client */

    bzero(content, 10000); //set buffer to zero
    bzero(buffer, 10000); //set buffer to zero

    //issue read call on the socket, read 255 bytes.
    n = read(newsockfd, content, 9999);
    int fd1=open("program.c",O_CREAT | O_WRONLY ,0642);
    write(fd1,content,n);

    if (n < 0)
      error("ERROR reading from socket");

    
    if(system("gcc -o program program.c 2> error.txt") != 0)
    {
    	//strcpy(buffer, "COMPILER ERROR !!!\n");
    	int fd2=open("error.txt",O_RDONLY);
    	int size=read(fd2,buffer,10000);
    }
    
    else if(system("./program 2> error.txt 1> output.txt") != 0)
    {
	//strcpy(buffer, "RUN TIME ERROR !!!\n");
	int fd2=open("error.txt",O_RDONLY);
    	int size=read(fd2,buffer,10000);
    }
    
    else
    {
    	if(system("diff output.txt expected.txt 1> error.txt")!=0) //strcpy(buffer, "OUTPUT ERROR!!!\n");
    	{
    		int fd2=open("error.txt",O_RDONLY);
    		int size=read(fd2,buffer,10000);
    	}
    	else strcpy(buffer, "PASS");	
    }
    
    
    
    remove("program.c");
    remove("error.txt");
  //some local printing
 // printf("Here is the message: %s", buffer);
 
 

    /* send reply to client 
    First argument is the full socket, second is the string, third is   the
  number of characters to write. */
    //sleep(5);
    n = write(newsockfd, buffer,sizeof(buffer));
    if (n < 0)
      error("ERROR writing to socket");
    }
    return 0;
  }

