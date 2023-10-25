#include <sys/types.h>
#include <sys/wait.h>
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
#include<iostream>

using namespace std;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

string check(const string &filename){
	int x=fork();
	if(x==0){
		string a="g++ -w -o received "+filename+" 2> compile_error.txt";
		execl("/bin/sh", "sh", "-c", a.c_str(), (char *) NULL);

	}
		
	else{
		wait(NULL);
		int fd2=open("compile_error.txt",O_RDONLY,S_IRWXU);
		int file_size=lseek(fd2,0,SEEK_END);
		lseek(fd2,0,SEEK_SET);
		char buffer[10000];
		bzero(buffer,10000);
		if(file_size>0)
		{
			string heading="---Compile Error---\n";
			int n=read(fd2,buffer,file_size);
			close(fd2);
			heading=heading+buffer;
			return heading;
		}
		close(fd2);
		int y=fork();
		if(y==0){
			
			int fd3=open("Runtime_error.txt",O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);
			int xd=open("Output.txt",O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);
			dup2(fd3,STDERR_FILENO);
			dup2(xd,STDOUT_FILENO);
			execlp("sh","sh","-c","./received",NULL);
		}
		else{
			wait(NULL);
			int fd4=open("Runtime_error.txt",O_RDONLY,S_IRWXU);
			int file_size=lseek(fd4,0,SEEK_END);
			lseek(fd4,0,SEEK_SET);
			char buffer[10000];
			bzero(buffer,10000);
			if(file_size>0)
			{
				string heading="---Runtime Error---\n";
				int n=read(fd4,buffer,file_size);
				close(fd4);
				heading=heading+buffer;
				return heading;
			}
			close(fd4);
			int z=fork();
			if(z==0)
			{
				close(STDOUT_FILENO);
				int fd5=open("difference.txt",O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);
				execlp("diff","diff","Expected_output.txt","Output.txt",NULL);
			}
			else{
				wait(NULL);
				int fd6=open("difference.txt",O_RDONLY,S_IRWXU);
				int file_size=lseek(fd6,0,SEEK_END);
				lseek(fd6,0,SEEK_SET);
				char buffer[10000];
				bzero(buffer,10000);
				if(file_size>0)
				{
					string heading="---Differnt Output---";
					int n=read(fd6,buffer,file_size);
					close(fd6);
					heading=heading+buffer;
					return heading;
				}
				
				if(file_size==0){
					return "Passed Away";
					close(fd6);
				}
			}
		} 
	}
	return "";
	
}
		

int main(int argc, char* argv[]){
	int socketfd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server_address, client_address;
	socklen_t client_length;
	int file_fd;
	int portno=atoi(argv[1]);
	char buffer[1024];
	if(socketfd<0)
		error("Error creating a socket");
	bzero((char*)&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(portno);
        int binded=bind(socketfd, (struct sockaddr*) &server_address, sizeof(server_address));
	if(binded<0)
		error("Error Binding");
	if(listen(socketfd,1)==-1)
    		error("Error listening at server");
    	client_length=sizeof(client_address);
    	while(true){
    		int newsocketfd=accept(socketfd,(struct sockaddr*) &client_address,&client_length);
    		if(newsocketfd==-1)
			perror("Error accepting at server");
		bzero(buffer,1024);
		int message_length=read(newsocketfd,buffer,1024);
		if(message_length==-1)
			perror("Error on reading");
		int file_fd=open("./received_file.cpp",O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
		if(file_fd==-1)
			perror("Error opening the file");
		if(write(file_fd,buffer,message_length)<0)
			perror("Error writing on file");
		close(file_fd);
		bzero(buffer,1024);
		string result = check("received_file.cpp");
		int stringlength=result.length();
		if(write(newsocketfd,result.c_str(),stringlength)<0)
			perror("Error writing on file");
		close(newsocketfd);
	}
	return 0;
}
		
	
