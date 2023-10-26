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
#include<limits.h>
#include<pthread.h>

using namespace std;

pthread_mutex_t mutex;

void error(const char* msg){
	perror(msg);
	exit(1);
}

string answer(int type,const char filename[]){
	string heading;
	if(type==1)
		heading="----Compilation Error----\n";
	else if(type==2)
		heading="----Runtime Error----\n";
	else
		heading="----Difference in Output----\n";

	int filefd=open(filename, O_RDONLY,S_IRWXU);
	char result[10000];
	long filesize=lseek(filefd,0,SEEK_END);
	lseek(filefd,0,SEEK_SET);
	read(filefd,result,10000);
	close(filefd);
	heading=heading+result;
	return heading;
}

string check(const char filename[], int sockfd){
	
	string compileerrorfname="complie_error_"+to_string(sockfd)+".txt";
	string compilecommand="g++ -o file_"+to_string(sockfd)+" "+filename+" 2>"+compileerrorfname;
	if(system(compilecommand.c_str())!=0)
		return answer(1,compileerrorfname.c_str());
	
	string runtimeerrorfname="runtime_error_"+to_string(sockfd)+".txt";
	string runoutputfname="run_output_"+to_string(sockfd)+".txt";
	string runcommand="./file_"+to_string(sockfd)+" 2>"+runtimeerrorfname+" 1>"+runoutputfname;
	if(system(runcommand.c_str())!=0)
		return answer(2,runtimeerrorfname.c_str());
	
	string diffoutput="difference_"+to_string(sockfd)+".txt";
	//string diffcommand="diff expected_output "+runcommand;
	char diffcommand[100];
	snprintf(diffcommand,100,"diff expected_output.txt run_output_%d.txt >difference_%d.txt",sockfd,sockfd);
	if(system(diffcommand)!=0)
		return answer(3,diffoutput.c_str());
	return "File Passed Successfully";
}

	

void* multithreading(void *sockfdaddress)
{
	int socketfd=*(int *) sockfdaddress;
	pthread_mutex_unlock(&mutex);
	char buffer[1024];
	bzero(buffer,1024);
	int message_length=read(socketfd,buffer,1024);
	if(message_length==-1)
		perror("Error on reading");
	string filename="received_file_"+to_string(socketfd)+".cpp";
	int fd=open(filename.c_str(),O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);
	if(fd==-1)
		perror("Error opening the file");
	if(write(fd,buffer,message_length)<0)
		perror("Error writing on file");
	close(fd);
	string result = check(filename.c_str(),socketfd);
	int stringlength=result.length();
	if(write(socketfd,result.c_str(),stringlength)<0)
		perror("Error writing on file");
	close(socketfd);
	pthread_exit(NULL);
}
	
	

int main(int argc, char* argv[])
{
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
    	while(true){
    		pthread_mutex_lock(&mutex);
    		int newsocketfd=accept(socketfd,(struct sockaddr*) &client_address,&client_length);
    		if(newsocketfd==-1){
    			perror("Error accepting at server");
    		}
    		pthread_t thread;
    		if(pthread_create(&thread,NULL, multithreading, &newsocketfd)!=0){
    			perror("Error creating threads");
    			close(newsocketfd);
    			pthread_mutex_unlock(&mutex);
    		}
    	}
    	return 0;
}
    			
    			
    			
    			
