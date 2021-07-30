#ifndef _HEAD_H
#define _HEAD_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include<time.h>
#include<unistd.h>
#include<pwd.h>
#include<grp.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<signal.h>
#include<sys/time.h>
#include<pthread.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<sys/uio.h>
#define ARGS_CHECK(argc,num) {if(argc!=num){printf("erro args\n");return -1;}}
#define ERROR_CHECK(ret,retVal,funcName) {if(ret==retVal){perror(funcName);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName) {if(ret!=0){printf("%s:%s\n",funcName,strerror(ret));return -1;}}
#define FILENAME "file"
typedef struct{
    pid_t pid;
    int fds;
    short busy;
}ProcessData_t,*pProcessData_t;
typedef struct{
    int dataLen;
    char buf[1000];
}train_t;
int makeChild(pProcessData_t,int);
int childHandle(int );
int tcpInit(int *,char *,char *);
int sendFd(int,int);
int recvFd(int,int*);
int tranFile(int);
#endif
