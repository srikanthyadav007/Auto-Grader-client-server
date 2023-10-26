#include <sys/types.h>
#include <sys/stat.h>      
#include <sys/socket.h>
#include <sys/time.h>
#include <error.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <strings.h>
#include<limits.h>
#include<pthread.h>
#include <netdb.h>
using namespace std;

void error(const char* msg){
	perror(msg);
	exit(1);
}

bool checkerr(){
	return errno == EWOULDBLOCK || errno == EAGAIN;
}

void performance(int Error, int timeout_err, double req, double loop_time, int success, double sum, double avg_res_time, double throughput){
	cout<<"Error:" << Error << endl;
	cout<<"Timeout:" << timeout_err << endl;
	cout<<"request rate:" << req / loop_time << endl;
	cout<<"Successful responses:" << success << endl;
	cout<<"Requests:" << req << endl;
	cout<<"Total time:" << sum << endl;
	cout<<"Loop time:" << loop_time << endl;
	cout<<"Avg response time:" << avg_res_time << endl;
	cout<<"Throughput:" << throughput << endl; 	
}

double getTime(){
	struct timeval curr_time;
	if(gettimeofday(&curr_time,NULL)!=0)
		error("Error getting the time");
	double seconds=(double) curr_time.tv_sec;
	double microseconds=(double) curr_time.tv_usec/1e6;
	return (double) seconds+ (double) microseconds;
}

int main(int argc,char* argv[]){
	
	if (argc < 7) {
    		fprintf(stderr, "usage %s hostname port <sourceCodeFileTobeGraded>  <loopNum> <sleepTimeSeconds> <TimeOutSeconds>\n", argv[0]);
    		exit(0);
  	}
	
	int socketfd, portno, n;
  	int success=0;
  	double sum=0;
  	struct sockaddr_in serv_addr; 
  	struct hostent *server; 
  	
  	char buffer[40000]; 
  	
  	portno = atoi(argv[2]); 
  	int loopNum=atoi(argv[4]);
  	int sleepTime=atoi(argv[5]);
 	int time_out=atoi(argv[6]);
 	
 	struct timeval timeout;
  	timeout.tv_sec = time_out;   
  	timeout.tv_usec =0 ;
  	int error=0;
  	int timeout_err=0;
  	
  	int req=loopNum;
  	double start_loop=getTime();
  	
  	while(loopNum>0){
  		loopNum--;  
  		socketfd = socket(AF_INET, SOCK_STREAM, 0); 
  		if (socketfd < 0){
    			perror("ERROR opening socket");
    			continue;
  		}
  		if(setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		{
  			perror("ERROR setting socket options");
        		close(socketfd);
        		continue;
    		}		
    		server = gethostbyname(argv[1]);
    		if (server == NULL) {
    			fprintf(stderr, "ERROR, no such host\n");
    			exit(0);
  		}
  		bzero((char *)&serv_addr, sizeof(serv_addr)); 

  		serv_addr.sin_family = AF_INET; 
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port = htons(portno);
		
		if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
  			if (checkerr()) {
        			perror("Connection timeout occurred");
        			timeout_err++;
        			error++;
        			close(socketfd);
       				continue;
    			} 
    			else {
        			perror("ERROR connecting");
        			error++;
        			close(socketfd);
        			continue;
    			}
  		}
  		
  		bzero(buffer, 40000); 

  		char* input=argv[3];                        
 	 	int fd=open(input,O_RDONLY,S_IRWXU);        
  		long size=lseek(fd,0,SEEK_END);           
  		lseek(fd,0,SEEK_SET);
  		char input_file[10000];
  		read(fd,input_file,size);             
  		
  		double Tsend=getTime();
  		n = write(socketfd, input_file, strlen(input_file));
  		
  		if (n < 0){
    			if (checkerr()) {
        			perror("Sending timeout occurred");
        			timeout_err++;
        			error++;
        			close(socketfd);
        			continue;
    			} 
    			else {
        			perror("ERROR Sending");
        			error++;
        			close(socketfd);
        			continue;
    			}
  		}
  		
  		bzero(buffer, 40000);
  		n = read(socketfd, buffer, 40000);   
  		double Trecv=getTime();
  		if (n < 0)
  		{
    			if (checkerr()) {
        			perror("Receive timeout occurred");
        			timeout_err++;
        			error++;
        			close(socketfd);
        			continue;
    			} 
    			else {
        			perror("ERROR Receiving");
        			error++;
        			close(socketfd);
        			continue;
    			}
  		}
  		printf("Server response: %s\n", buffer);
  		double tot_time=Trecv-Tsend;
  		sum+=tot_time;
  		success++;
  		close(socketfd);
  		//sleep(sleepTime);
	}
	double end_loop=getTime();
  	double loop_time=end_loop-start_loop;
  	double avg_res_time=sum/success;
  	double throughput=success/loop_time;
  	
  	performance(error,timeout_err,req,loop_time,success,sum,avg_res_time,throughput);
  	return 0;
  }
  	

