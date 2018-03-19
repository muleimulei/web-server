
#ifndef _WEB_H_HH
#define _WEB_H_HH
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "base.cpp"
#define MAXLINE 1000
#define MAXBUF 1000

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
    char buf[MAXLINE], body[MAXLINE];
    // 构建相应头
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body,"%s<body bgcolor='#fff'>\r\n", body);
    sprintf(body,"%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    // print Http response
    sprintf(buf, "HTTP/1.0 %s %s \r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));

    sprintf(buf, "Content-Type: text/html\n\r");
    rio_writen(fd, buf, strlen(buf));

    sprintf(buf, "Content-Type: %ld \r\n\r\n", strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));

}

void read_requesthdrs(rio_t &rio){
    char buf[MAXLINE];
    rio_readlineb(rio, buf, MAXLINE);
    while(strcmp(buf, "\r\n")){
        rio_readlineb(rio, buf, MAXLINE);
        printf("%s\n", buf);
    }
    return;
}

void parse_uri(char *uri, char *filename, char *cgiargs){
    char *ptr;
    if(!strstr(uri, "cgi-bin")){ // static content
        strcpy(cgiargs, "");
        strcpy(filename, "./");
        strcat(filename, uri);

        if(uri[strlen(uri)-1]=='/'){
            strcat(filename, "home.html");
        }
        return 1;
    }else {
        ptr = index(uri, '?');
        if(ptr){
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }else{
            strcpy(cgiargs, "");
        }
        strcpy(filename, "./");
        strcat(filename, uri);
        return 0;
    }
}

void get_filetype(char *filename, char *filetype){
    if(strstr(filename, ".html")){
        strcpy(filetype, "text/html");
    }else if(strstr(filename, ".gif")){
        strcpy(filetype, "image/gif");
    }else if(strstr(filename, ".png")){
        strcpy(filetype, "image/png");
    }
}

void serve_static(int fd, char *filename, int filesize){
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    // send response headers to client
    get_filetype(filename, filetype);
}


void doit(int fd){
    int is_static;
    struct stat sbuf;
    rio_t rio;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    char filename[MAXLINE], cgiargs[MAXLINE];
    //读取请求头
    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s\n", buf);

    sscanf(buf, "%s %s %s", method, uri, version);
    if(strcasecmp(method, "GET")){
        clienterror(fd, method, "501", "Not implemented", "Webserver does not implement this method");
    }

    read_requesthdrs(&rio);

    // parse url
    is_static = parse_uri(uri, filename, cgiargs);
    if(stat(filename, &sbuf)<0){
        clienterror(fd, filename, "404", "Not Found", "Tiny couldn't find this file");
        return;
    }
    if(is_static){
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode) ){
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }

}

#endif
