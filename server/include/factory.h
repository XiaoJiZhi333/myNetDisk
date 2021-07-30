#ifndef  _FACTORY_H_
#define  _FACTORY_H_
#include "head.h"
#include "work_que.h"
typedef struct{
    Que_t que;
    pthread_cond_t cond;
    pthread_t *pthid;
    int threadNum;
    int startFlag;
}Factory_t,*pFactory_t;
#endif
