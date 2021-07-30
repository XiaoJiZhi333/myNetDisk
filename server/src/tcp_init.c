#include "head.h"
int tcpInit(int *sFd,char *addr,char *port){
    int socketFd;
    socketFd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(socketFd,-1,"socket");
    int ret;
    int reuse=1;
    setsockopt(socketFd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
    struct sockaddr_in ser;
    bzero(&ser,sizeof(ser));
    ser.sin_family=AF_INET;
    ser.sin_addr.s_addr=inet_addr(addr);
    ser.sin_port=htons(atoi(port));
    ret=bind(socketFd,(struct sockaddr *)&ser,sizeof(struct sockaddr));
    ERROR_CHECK(ret,-1,"bind");
    ret=listen(socketFd,10);
    ERROR_CHECK(ret,-1,"listen");
    *sFd=socketFd;
    return 0;
}


