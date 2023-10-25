#include "server_functions.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <strings.h>

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
	string diffcommand="diff expected_output "+runcommand;
	if(system(diffcommand.c_str())!=0)
		return answer(3,diffoutput.c_str());
	return "File Passed Successfully";
}