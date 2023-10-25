#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

using namespace std;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

string ans(int type, const char* filename) {
    string str;
    if(type == 1)
        str = "Compile Error\n";
    else if(type == 2)
        str = "Runtime Error\n";
    else
        str = "Different Output\n";

    char str3[10000];
    bzero(str3, 10000);
    int fd = open(filename, O_RDONLY, S_IRWXU);
    long size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, str3, size);
    close(fd);

    str += str3;

    return str;
}

const char* check(string filename){
    string str = "g++ -o received ";
    str += filename;
    string str5 = " 2> Compile_Out.txt";
    str += str5;
    const char* command = str.c_str();
    int compilerror = system(command);
    if(compilerror){
        return ans(1, "Compile_out.txt").c_str();
    }

    int runerr = system("./received 1> Run_Out.txt 2> Run_Err.txt");
    if(runerr){
        return ans(2, "Run_Err.txt").c_str();
    }

    int differror = system("diff Exp_Output.txt Run_Out.txt >diff.txt");
    if(differror){
        return ans(3, "diff.txt").c_str();
    }

    return "Pass";
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[40960];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    portno = atoi(argv[1]);
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 1);

    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        bzero(buffer, 40960);

        n = read(newsockfd, buffer, 40960);

        if (n < 0)
            error("ERROR reading from socket");
        
        int fd = open("./received-file.cpp", O_CREAT|O_RDONLY|O_WRONLY|O_TRUNC, S_IRWXU);
        write(fd, buffer, n);
        close(fd);

        const char* result = check("received-file.cpp");

        //sleep(argv[4]);
        int len = strlen(result);
        n = write(newsockfd, result, len);
        if (n < 0)
            error("ERROR writing to socket");
        close(newsockfd);
    }

    return 0;
}

