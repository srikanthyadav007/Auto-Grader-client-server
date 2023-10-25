/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netdb.h>
#include<string>
#include<iostream>
#include<sys/time.h>

using namespace std;
void error(const char *msg) {
  perror(msg);
  exit(0);
}

double GetTime(){
	struct timeval tv;
	if(gettimeofday(&tv,NULL)!=0)
	     error("Gettimeofday failed!");
	
	double seconds=(double)tv.tv_sec;
	double microseconds=(double)tv.tv_usec/1000000;
	return seconds+microseconds;
}

int main(int argc, char *argv[]) {
  int sockfd, portno, n;

  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname

  char buffer[40960]; //buffer for message

  if (argc < 6) {
    fprintf(stderr, "usage %s hostname port filename <loopNum> <sleepTimeSeconds>\n", argv[0]);
    exit(0);
  }
  
  portno = atoi(argv[2]); // 2nd argument of the command is port number
  int loopNum=atoi(argv[4]);
  int timetosleep=atoi(argv[5]);
  /* create socket, get sockfd handle */

  string filename=argv[3];
  const char* fname = filename.c_str();
  double sum_time=0;
  int success=0;
  double start=GetTime();
while(loopNum--){
  sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket. 
  //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)


  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in server address in sockaddr_in datastructure */

  server = gethostbyname(argv[1]);
  //finds the IP address of a hostname. 
  //Address is returned in the 'h_addr' field of the hostend struct

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

  serv_addr.sin_family = AF_INET; // Address Family is IP

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
/*Copy server IP address being held in h_addr field of server variable
to sin_addr.s_addr of serv_addr structure */

//convert host order port number to network order
  serv_addr.sin_port = htons(portno);

  /* connect to server 
  First argument is the half-socket, second is the server address structure
  which includes IP address and port number, third is size of 2nd argument
  */

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  //If here means connection was complete

  /* ask user for input */

 // printf("Please enter the message: ");
  bzero(buffer, 40960); //reset buffer to zero
 // fgets(buffer, 255, stdin); //read message from stdin, into buffer

  /* send user message to server 
  write call: first argument is socket FD, 2nd is the string to write, 3rd is length of 2nd
  */
  int fd=open(fname,O_RDONLY,S_IRWXU);
  long size=lseek(fd,0,SEEK_END);
  lseek(fd,0,SEEK_SET);
  read(fd,buffer,size);
  double start_time=GetTime();
  n = write(sockfd, buffer, strlen(buffer));

  if (n < 0){
    perror("ERROR writing to socket");
    sleep(timetosleep);
    continue;
  }
  bzero(buffer, 256);

  /* read reply from server 
  First argument is socket, 2nd is string to read into, 3rd is number of bytes to read
  */

  n = read(sockfd, buffer, 255);
  double end_time=GetTime();
  if (n < 0){
    perror("ERROR reading from socket");
    sleep(timetosleep);
    continue;
  }
  printf("Server response: %s\n", buffer);
  double total_time=end_time-start_time;
  success++;
  sum_time+=total_time;
  close(sockfd);
  sleep(timetosleep);
}
double end=GetTime();
printf("Total Time:%lf\n",sum_time);
double avg_response=0;
if(success!=0)
  avg_response=sum_time/success;

printf("Avg. response time:%lf\n",avg_response);
printf("Loop time:%lf\n",end-start);
printf("Successfull response:%d\n",success);
  return 0;
}
