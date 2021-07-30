#include "factory.h"
int recvCycle(int fd,void *pstart,int len){
    char *p=(char *)pstart;
    int ret=0;
    int total=0;
    while(total<len){
        ret=recv(fd,p+total,len-total,0);
        if(ret==0){
            return -1;
        }
        total+=ret;    
        printf("%5.2f%%\r",(double)total/len*100);
        fflush(stdout);
    }
    printf("\n");
    return 0;
}
