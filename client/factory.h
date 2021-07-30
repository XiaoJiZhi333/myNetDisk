#ifndef  _FACTORY_H_
#define  _FACTORY_H_
#include "head.h"
#include "work_que.h"
#include "md5.h"
#define STR_LEN 10
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
int modeToLetter(int,char *);
char *generateStr();
#endif
