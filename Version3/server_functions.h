#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include <string>  // Include any necessary headers
using namespace std; 


extern pthread_mutex_t mutex;

string answer(int type, const char filename[]);
string check(const char filename[], int sockfd);
void error(const char* msg);

#endif