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
#include<queue>

using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wait_consumer=PTHREAD_COND_INITIALIZER;
pthread_cond_t wait_producer=PTHREAD_COND_INITIALIZER;

int items_in_queue;
queue <int> fdqueue;

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
	char diffcommand[100];
	snprintf(diffcommand,100,"diff Exp_output.txt run_output_%d.txt >difference_%d.txt",sockfd,sockfd);
	//string diffcommand="diff Exp_output.txt "+;
	if(system(diffcommand)!=0)
		return answer(3,diffoutput.c_str());
	return "File Passed Successfully";
}

void *workerthread(void *args){
    while(true){
        int newsocketfd;
        pthread_mutex_lock(&mutex);
        while (fdqueue.size()==0) {
            pthread_cond_wait(&wait_consumer, &mutex);
        }
        newsocketfd = fdqueue.front();
        fdqueue.pop();
        //items_in_queue--;
        pthread_cond_signal(&wait_producer);
        pthread_mutex_unlock(&mutex);

        char buffer[1024];
        bzero(buffer,1024);
        int message_length=read(newsocketfd,buffer,1024);
        if(message_length==-1) {
            perror("Error on reading");
            close(newsocketfd);
            continue;
        }

        string filename="received_file_"+to_string(newsocketfd)+".cpp";
        int fd=open(filename.c_str(),O_WRONLY|O_RDONLY|O_CREAT|O_TRUNC,S_IRWXU);
        if(fd==-1){
            perror("Error creating the file");
            close(newsocketfd);
            continue;
        }       

        if(write(fd,buffer,message_length)<0) {
            perror("Error writing on the cpp file");
            close(newsocketfd);
            close(fd);
            continue;
        }
        close(fd);

        string result = check(filename.c_str(),newsocketfd);
        int stringlength=result.length();
        if(write(newsocketfd,result.c_str(),stringlength)<0) {
            perror("Error writing on client");
            close(newsocketfd);
            continue;
        }

        close(newsocketfd);
    }
}


int main(int argc, char* argv[]){
	
	if (argc < 3) {
    		printf("usage %s <port no> <max_pool_size>\n",argv[0]);
    		exit(1);
  	}
  
	int socketfd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in serv_addr,client_address;
	socklen_t client_length;
	
	int portno;
	char buffer[1024];
	
	if(socketfd<0)
		error("Error creating the socket");
	
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        
    	portno = atoi(argv[1]);
    	
    	serv_addr.sin_port = htons(portno);
    	
    	int binded=bind(socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	if(binded<0)
		error("Error Binding");
		
    	if(listen(socketfd,100)==-1)
    		error("Error listening at server");
    		
    	client_length=sizeof(client_address);
    	
    	int i=1;
    	int pool_size=atoi(argv[2]);
    	pthread_t thread[pool_size];
    	items_in_queue=0;
    	
    	while(i<=pool_size){
    		if (pthread_create(&thread[i], NULL, workerthread, NULL) != 0){
       			perror("Failed to create Thread");
     		}
     		i++;
    	}	
    	int newsocketfd;
    	while(true){
    		newsocketfd=accept(socketfd,(struct sockaddr*) &client_address,&client_length);
    		if(newsocketfd==-1)
			perror("Error accepting at server");
    		pthread_mutex_lock(&mutex);
    		while(fdqueue.size()>=pool_size){
    			pthread_cond_wait(&wait_producer,&mutex);}
        	fdqueue.push(newsocketfd);
        	items_in_queue++;
        	pthread_cond_signal(&wait_consumer);
        	pthread_mutex_unlock(&mutex);
        	
	}
	close(socketfd);
    	return 0;
}
