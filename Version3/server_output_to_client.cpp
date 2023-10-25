#include "server_functions.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <strings.h>

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