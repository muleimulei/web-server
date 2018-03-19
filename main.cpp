#include <iostream>
#include "base.cpp"
using namespace std;
#include "web.h"

#define MAXLINE 1000
int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    sockaddr_storage clientaddr;

    //检查命令行参数
    if(argc!=2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    listenfd = open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, clientlen);
        getnameinfo((struct sockaddr *)&clientaddr, clientlen,hostname, MAXLINE, port, MAXLINE, 0);
        printf("ACCEPT connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        close(connfd);
    }
    return 0;
}

