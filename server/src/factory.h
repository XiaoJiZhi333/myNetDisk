#ifndef  _FACTORY_H_
#define  _FACTORY_H_
#include "head.h"
#include "work_que.h"
#include "md5.h"
typedef struct{
    Que_t que;
    pthread_cond_t cond;
    pthread_t *pthid;
    int threadNum;
    int startFlag;
}Factory_t,*pFactory_t;
int factoryInit(pFactory_t,int,int);
int factoryStart(pFactory_t);
int tcpInit(int *,char *,char *);
int configInit(char*,char*,char*,int*,int*);
int recvCycle(int,void *,int);
int modeToLetter(int,char *);
void get_salt(char *,char *);
void writeLog(int,char*,char*);
int Compute_file_md5(const char *file_path, char *md5_str);
#endif
