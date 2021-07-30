#include "factory.h"
int exitFds[2];
void sigExitFunc(int signum){
    printf("%d is coming\n",signum);
    write(exitFds[1],&signum,1);     
}
int epollAdd(int epfd,int fd){
    struct epoll_event event;
    event.events=EPOLLIN;
    event.data.fd=fd;
    int ret;
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
    ERROR_CHECK(ret,-1,"epoll_ctl");
    return 0;
}
int main(int argc,char *argv[])
{
    char addr[30];
    char port[30];
    int threadNum,capacity;
    if(argc!=2){
        printf("./server config\n");
        return -1;
    }
    configInit(argv[1],addr,port,&threadNum,&capacity);
    pipe(exitFds);
    //退出机制 子进程负责初始化线程池
    while(fork()){
        signal(SIGUSR1,sigExitFunc);
        int status;
        wait(&status);
        if(WIFEXITED(status)){
            printf("exit success\n");
            exit(0);
        }
    }
    close(exitFds[1]);
    Factory_t threadMainData;
    factoryInit(&threadMainData,threadNum,capacity);
    factoryStart(&threadMainData);
    int socketFd,newFd; 
    tcpInit(&socketFd,addr,port);
    pQue_t pq=&threadMainData.que;
    pNode_t pNew;
    int epfd=epoll_create(1);
    struct epoll_event evs[2];
    epollAdd(epfd,socketFd);
    epollAdd(epfd,exitFds[0]);
    int readyFdCount;
    while(1){
        readyFdCount=epoll_wait(epfd,evs,2,-1);
        for(int i=0;i<readyFdCount;i++){
            if(evs[i].data.fd==socketFd){
                newFd=accept(socketFd,NULL,NULL);
                pNew=(pNode_t)calloc(1,sizeof(Node_t));
                pNew->newFd=newFd;
                pthread_mutex_lock(&pq->mutex);
                queInsert(pq,pNew);
                pthread_mutex_unlock(&pq->mutex);
                pthread_cond_signal(&threadMainData.cond);
            }
            if(evs[i].data.fd==exitFds[0]){
                for(int j=0;j<threadNum;j++){
                    pthread_cancel(threadMainData.pthid[j]);
                }
                for(int j=0;j<threadNum;j++){
                    pthread_join(threadMainData.pthid[j],NULL);
                }
                exit(0);
            }
        }
    }
    return 0;
}

