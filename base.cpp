
#ifndef __BASE_H_HH
#define __BASE_H_HH
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include <errno.h>
#include <stdio.h>
#include<stdlib.h>
#include<stdio.h>

int open_clientfd(char *hostname,  char *port){
    int clientfd;
    struct addrinfo hints, *listp,*p;
    //获取潜在的服务器地址
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; //打开连接
    hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG; //强制使用端口
    int i = getaddrinfo(hostname, port, &hints, &listp);

    //遍历列表拿到成功连接的地址
    for(p = listp; p; p = p->ai_next){
        if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)<0)) continue; //创建一个socket描述符
        if(connect(clientfd, p->ai_addr, p->ai_addrlen)!=-1){
            break; //成功
        }
        close(clientfd);
    }
    //清除
    freeaddrinfo(listp);
    if(!p){
        return -1;
    }else{
        return clientfd;
    }
}

int open_listenfd(char *port){
    struct addrinfo hints, *listp,*p;
    int listenfd, optval;

    //获取潜在的服务器地址
    memset(&hints,0,sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    getaddrinfo(NULL, port, &hints, &listp);

    for(p = listp;p;p=p->ai_next){
        //创建一个socket描述符
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0) continue; //socket failed
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval, sizeof(int));

        if(bind(listenfd, p->ai_addr, p->ai_addrlen)==0){
            break;
        }
        close(listenfd);
    }
    //clean up
    freeaddrinfo(listp);
    if(!p){
        return -1;
    }
    if(listen(listenfd, 100)<0){
        close(listenfd);
        return -1;
    }
    return listenfd;
}

#define RIO_BUFSIZE 8192
typedef struct{
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
} rio_t;

void rio_readinitb(rio_t *rp, int fd){
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n){
    int cnt;
    while(rp->rio_cnt<=0){
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0){
            if(errno!=EINTR) return -1;
        }
        else if (rp->rio_cnt ==0 ) return 0; //eof
        else rp->rio_bufptr = rp->rio_buf;
    }

    cnt = n;
    if(rp->rio_cnt < n){
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}
ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;
    for(n = 1; n<maxlen; n++){
        if((rc = rio_read(rp, &c,1))==1){
            *bufp++ = c;
            if(c=='\n'){
                n++;
                break;
            }
        }else if(rc==0){
                if(n==1){
                    return 0;
                }else{
                    break;
                }
        }else{
            return -1;
        }
    }
    *bufp = 0;
    return n-1;

}

ssize_t rio_readnb(rio_t *rp, char *usrbuf, size_t n){
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft>0){
        if((nread = rio_read(rp, bufp, nleft))<0){
            return -1;
        }else if(nread==0){
            break;
        }
        nleft -= nread;
        bufp+=nread;
    }
    return n - nleft;
}


ssize_t rio_readn(int fd, char *usrbuf, size_t n){
    size_t nleft = n;
    size_t nread;
    char *bufp = usrbuf;
    while(nleft>0){
        if((nread = read(fd, bufp, nleft))<0){
            if(errno == EINTR){
                nread = 0; // 被中断
            }else{
                return -1;
            }
        }else if(nread == 0){
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}

ssize_t rio_writen(int fd, char *usrbuf, size_t n){
    size_t nleft = n;
    size_t nwritten;
    char *bufp = usrbuf;

    while(nleft>0){
        if((nwritten = write(fd, bufp, nleft))<=0){
            if(errno == EINTR){
                nwritten = 0; // 被中断
            }else{
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}


#endif

