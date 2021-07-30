#include "factory.h"
int tranFile(int newFd,char *filepath)
{
    train_t t;
    memset(&t,0,sizeof(train_t));
    off_t len;//已下载文件大小
    recv(newFd,&len,sizeof(len),0);
    int fd=open(filepath,O_RDWR);
    ERROR_CHECK(fd,-1,"tranFileopen");
    printf("已存在%ld字节\n",len);
    struct stat buf;
    fstat(fd,&buf);
    t.dataLen=sizeof(buf.st_size);
    off_t restsize=buf.st_size-len;
    printf("buf.st_size=%ld,restsize=%ld\n",buf.st_size,restsize);
    memcpy(t.buf,&buf.st_size,t.dataLen);
    int ret=send(newFd,&t,8+t.dataLen,0);
    ERROR_CHECK(ret,-1,"send");
    struct timeval start,end;
    gettimeofday(&start,NULL);
    char *pMap=(char *)mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);
    send(newFd,pMap+len,restsize,0);
    printf("send restsize=%ld\n",restsize);
    gettimeofday(&end,NULL);
    printf("use time=%ld\n",(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
    close(fd);
    return 0;
}

