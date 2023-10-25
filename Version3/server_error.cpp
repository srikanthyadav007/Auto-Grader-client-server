#include <error.h>
#include <stdlib.h>
#include "serverfunctions.h"

void error(const char* msg){
	perror(msg);
	exit(1);
}