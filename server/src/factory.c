#include "factory.h"
#define READ_DATA_SIZE	1024
#define MD5_SIZE		16
#define MD5_STR_LEN		(MD5_SIZE * 2)
//线程池初始化函数
int factoryInit(pFactory_t pf,int threadNum,int capacity)
{
    queInit(&pf->que,capacity);
    pthread_cond_init(&pf->cond,NULL);
    pf->pthid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));
    pf->threadNum=threadNum;
    pf->startFlag=0;
    return 0;
}
//线程清理函数
void cleanup(void *p){
    pQue_t pq=(pQue_t) p;
    pthread_mutex_unlock(&pq->mutex);
}
//生成随机数
char* Generate(){
    char *str=(char*)calloc(STR_LEN+1,sizeof(char));
    // char str[STR_LEN+1]={0};
    int i,flag;
    srand(time(NULL));
    for(i=0;i<STR_LEN;i++){
        flag=rand()%3;
        switch(flag){
        case 0:
            str[i]=rand()%26+'a';
            break;
        case 1:
            str[i]=rand()%26+'A';
            break;
        case 2:
            str[i]=rand()%10+'0';
            break;

        }
    }
    return str;
}
//子线程函数
void * threadFunc(void *p){
    pFactory_t pf=(pFactory_t)p; 
    pQue_t pq=&pf->que;
    pNode_t pCur;
    int getSuccess;
    int dataLen;
    int control_code;
    char buf[1000]={0};
    while(1){
        pthread_mutex_lock(&pq->mutex);
        pthread_cleanup_push(cleanup,pq);
        if(!pq->size){
            pthread_cond_wait(&pf->cond,&pq->mutex);
        }
        getSuccess=queGet(pq,&pCur);
        pthread_cleanup_pop(1);
        char CurPath[1024]={0};
        strcpy(CurPath,getcwd(NULL,0));
        if(!getSuccess){
            char *salt=(char*)calloc(256,sizeof(char));
            char *pwdp=(char*)calloc(256,sizeof(char));
            dataLen=control_code=0;
            train_t t;
            char userName[30]={0};
            MYSQL *conn;
            MYSQL_RES *res;
            MYSQL_ROW row;
            char *server="localhost";
            char *user="root";
            char *password="123";
            char *database="Netdisk";
            conn=mysql_init(NULL);
            if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0)){
                printf("Erro connecting to database:%s\n",mysql_error(conn));
                return NULL;
            }else{
                printf("Connected...\n");
            }
            //判断用户是注册还是登陆
            int  option=0;
            recv(pCur->newFd,&option,4,0);
            if(option==2){
                while(1){
                    memset(userName,0,sizeof(userName));
                    recv(pCur->newFd,userName,sizeof(userName),0);
                    char query[300]="select * from User where name='";
                    sprintf(query,"%s%s%s",query,userName,"'");
                    puts(query);
                    int r;
                    r=mysql_query(conn,query);
                    if(r){
                        printf("Error makeing query:%s\n",mysql_error(conn));
                        return NULL;
                    }else{
                        res=mysql_use_result(conn);
                        int i=0;
                        if((row=mysql_fetch_row(res))==NULL){
                            printf("用户名不存在\n");
                            send(pCur->newFd,&i,4,0);
                            char *s=NULL;
                            s=Generate();
                            printf("salt1:%s\n",s);
                            sprintf(salt,"%s%s%s","$6$",s,"$");
                            printf("salt1:%s\n",salt);
                            send(pCur->newFd,salt,strlen(salt),0);
                            free(s);
                            s=NULL;
                            break;
                        }else{
                            i=1;
                            printf("用户名已存在\n");
                            send(pCur->newFd,&i,4,0);
                            continue;
                        }
                    }
                }
                char insert[500]="insert into User(name,salt,pwdp)values('";
                sprintf(insert,"%s%s%s%s",insert,userName,"','",salt);
                recv(pCur->newFd,pwdp,256,0);
                sprintf(insert,"%s%s%s%s",insert,"','",pwdp,"')");
                printf("%s\n",insert);
                int r=mysql_query(conn,insert);
                if(r){
                    printf("Error making query:%s\n",mysql_error(conn));
                    return NULL;
                }else{
                    printf("insert success\n");
                }
                mysql_free_result(res);
            }else{
                while(1){
                    memset(userName,0,sizeof(userName));
                    recv(pCur->newFd,userName,sizeof(userName),0);
                    char query[300]="select * from User where name='";
                    sprintf(query,"%s%s%s",query,userName,"'");
                    puts(query);
                    int r;
                    r=mysql_query(conn,query);
                    if(r){
                        printf("Error makeing query:%s\n",mysql_error(conn));
                    }else{
                        res=mysql_use_result(conn);
                        int i=0;
                        if((row=mysql_fetch_row(res))==NULL){
                            printf("用户名不存在\n");
                            send(pCur->newFd,&i,4,0);
                            continue;
                        }else{
                            i=1;
                            printf("用户名已存在\n");
                            send(pCur->newFd,&i,4,0);
                            break;
                        }
                    }
                }
                strcpy(salt,(char*)row[2]);
                send(pCur->newFd,salt,strlen(salt),0);
                while(1){
                    int ret=recv(pCur->newFd,pwdp,256,0);
                    printf("ret=%d\n",ret);
                    printf("recv pwdw:%s\n",pwdp);
                    int i=0;
                    if(strcmp(pwdp,(char*)row[3])==0){
                        printf("登陆成功\n");
                        i=1;
                        send(pCur->newFd,&i,4,0);
                        break;
                    }else{
                        send(pCur->newFd,&i,4,0);
                        continue;
                    }
                }
                mysql_free_result(res);
            }// while(1)上面的大括号的对应
            sprintf(CurPath,"%s%s%s",CurPath,"/Diskof",userName);
            if(option==1){
                mkdir(CurPath,0666);
            }
            char logpath[40]={0};
            sprintf(logpath,"%s%s%s%s",CurPath,"/",userName,".log");
            int logfd=open(logpath,O_RDWR|O_CREAT,0666);
            char op[30]="login";
            writeLog(logfd,userName,op);
            while(1){
                dataLen=control_code=0;
                memset(buf,0,sizeof(buf));
                int ret=recv(pCur->newFd,&dataLen,4,0);
                printf("dataLen:%d\n",dataLen);
                if(ret==0){
                    break;
                }
                ret=recv(pCur->newFd,&control_code,4,0);
                printf("control:%d\n",control_code);
                if(ret==0){
                    break;
                }
                if(dataLen!=0){
                    recv(pCur->newFd,buf,dataLen,MSG_WAITALL);
                }
                printf("buf:%s\n",buf);
                //cd 指令
                if(control_code==1){
                    int len;
                    int i;
                    char op[40]="cd";
                    sprintf(op,"%s %s",op,buf);
                    writeLog(logfd,userName,op);
                    if(strcmp(buf,"..")==0){
                        len=strlen(CurPath);
                        i=len;
                        while(CurPath[i-1]!='/'){
                            CurPath[i-1]='\0';
                            i--;
                        }
                        CurPath[i-1]='\0';
                        continue;
                    }
                    int flag=0;
                    len=strlen(buf);
                    for(i=0;i<len;i++){
                        if(buf[i]=='/'){
                            flag=1;
                            break;
                        }
                    }
                    if(flag){
                        memset(CurPath,0,sizeof(CurPath));
                        strcpy(CurPath,buf);
                    }else{
                        sprintf(CurPath,"%s%s%s",CurPath,"/",buf);
                    }
                }
                //ls 指令
                if(control_code==2){
                    //操作写入日志
                    char op[40]="ls";
                    writeLog(logfd,userName,op);
                    //ls功能
                    DIR *pd;
                    printf("%s\n",CurPath);
                    pd=opendir(CurPath);
                    if(pd==NULL){
                        perror("ls:opendir");
                        return NULL;
                    }
                    struct dirent *p;
                    char st_mode[2];
                    while((p=readdir(pd))!=NULL){
                        memset(st_mode,0,sizeof(st_mode));
                        struct stat st_buf;
                        int ret;
                        char path[1024];
                        memset(path,0,sizeof(path));
                        if(strcmp(p->d_name,".")==0||strcmp(p->d_name,"..")==0){
                            continue;
                        }
                        sprintf(path,"%s%s%s",CurPath,"/",p->d_name);
                        ret=stat(path,&st_buf);
                        if(ret==-1){
                            perror("stat");
                            return NULL;
                        } 
                        modeToLetter(st_buf.st_mode,st_mode);
                        memset(&t,0,sizeof(train_t));
                        sprintf(t.buf,"%c\t%20s\t%15ldB",st_mode[0],p->d_name,st_buf.st_size);
                        t.dataLen=strlen(t.buf);
                        send(pCur->newFd,&t,t.dataLen+8,0);
                    }
                    t.dataLen=0;
                    send(pCur->newFd,&t,4,0);
                }
                //puts 指令
                if(control_code==3){
                    char op[40]="puts";
                    sprintf(op,"%s %s",op,buf);
                    writeLog(logfd,userName,op);
                    //收md5
                    memset(&t,0,sizeof(train_t));
                    recv(pCur->newFd,&t.dataLen,4,MSG_WAITALL);
                    printf("t.dataLen=%d\n",t.dataLen);
                    recv(pCur->newFd,&t.control_code,4,MSG_WAITALL);
                    printf("t.control_code=%d\n",t.control_code);
                    recv(pCur->newFd,t.buf,t.dataLen,MSG_WAITALL);
                    char md5[50]={0};
                    strcpy(md5,t.buf);
                    printf("md5=%s\n",md5);
                    char query[300]="select * from file where md5='";
                    sprintf(query,"%s%s%s",query,t.buf,"'");
                    puts(query);
                    mysql_query(conn,query);
                    res=mysql_use_result(conn);
                    int i=0;
                    char filepath[1024]={0};
                    sprintf(filepath,"%s%s%s",CurPath,"/",buf);
                    puts(filepath);
                    if((row=mysql_fetch_row(res))!=NULL){
                        i=1;
                        send(pCur->newFd,&i,4,0);
                        if(strcmp((char *)row[1],filepath)==0){
                            printf("文件已存在\n");
                            continue;
                        }
                        link((char*)row[1],filepath);
                        mysql_free_result(res);
                        char insert[300]="insert into file (filename,path,md5)values('";
                        sprintf(insert,"%s%s%s%s%s%s%s",insert,buf,"','",filepath,"','",t.buf,"')");
                        puts(insert);
                        mysql_query(conn,insert);
                        continue;
                    }else{
                        send(pCur->newFd,&i,4,0);
                        printf("send i=%d success\n",i);
                    }
                    mysql_free_result(res);
                    off_t filesize;
                    memset(&t,0,sizeof(train_t));
                    recv(pCur->newFd,&t.dataLen,4,MSG_WAITALL);
                    printf("t.dataLen=%d\n",t.dataLen);
                    recv(pCur->newFd,&t.control_code,4,MSG_WAITALL);
                    printf("t.control_code=%d\n",t.control_code);
                    recv(pCur->newFd,&filesize,t.dataLen,MSG_WAITALL);
                    printf("filesize =%ldB\n",filesize);
                    memset(&t,0,sizeof(train_t));
                    printf("open filepaht:%s\n",filepath);
                    int fd=open(filepath,O_RDWR|O_CREAT,0666);
                    if(fd==-1){
                        perror("puts:open");
                        return NULL;
                    }
                    int fds[2];
                    pipe(fds);
                    int oldsize,downloadsize=0;
                    int slicesize=filesize/100;
                    int ret;
                    while(1){
                        ret=splice(pCur->newFd,NULL,fds[1],NULL,4096,SPLICE_F_MOVE|SPLICE_F_MORE); 
                        downloadsize+=ret;
                        if(ret==0){
                            break;
                        }
                        splice(fds[0],NULL,fd,NULL,ret,SPLICE_F_MORE|SPLICE_F_MOVE);
                        if(downloadsize==filesize){
                            printf("100.00%%\n");
                            fflush(stdout);
                            break;
                        }
                        if(downloadsize-oldsize>=slicesize){
                            printf("%5.2f%%\r",(double)downloadsize/filesize*100);
                            fflush(stdout);
                            oldsize=downloadsize;
                        }
                    }
                    char insert[300]="insert into file (filename,path,md5)values('";
                    sprintf(insert,"%s%s%s%s%s%s%s",insert,buf,"','",filepath,"','",md5,"')");
                    puts(insert);
                    int r=mysql_query(conn,insert);
                    if(r){
                        printf("Error makeing query:%s\n",mysql_error(conn));
                    }
                    printf("upload success\n");
                    close(fd);
                }
                //gets 指令
                if(control_code==4){
                    char op[40]="gets";
                    sprintf(op,"%s %s",op,buf);
                    writeLog(logfd,userName,op);
                    char filepath[1024]={0};
                    sprintf(filepath,"%s%s%s",CurPath,"/",buf);
                    printf("%s\n",filepath);
                    tranFile(pCur->newFd,filepath);
                }
                //remove 指令
                if(control_code==5){
                    char op[40]="remove";
                    sprintf(op,"%s %s",op,buf);
                    writeLog(logfd,userName,op);
                    char filepath[1024]={0};
                    sprintf(filepath,"%s%s%s",CurPath,"/",buf);
                    char query[300]="delete from file where path='";
                    sprintf(query,"%s%s%s",query,filepath,"'");
                    int r=mysql_query(conn,query);
                    if(r){
                        printf("Error makeing query:%s\n",mysql_error(conn));
                    }else{
                      printf("delete success\n");
                    }
                    //remove(filepath);
                    unlink(filepath);
                }
                //pwd 指令
                if(control_code==6){
                    char op[40]="pwd";
                    writeLog(logfd,userName,op);
                    memset(&t,0,sizeof(train_t));
                    t.dataLen=strlen(CurPath);
                    t.control_code=0;
                    strcpy(t.buf,CurPath);
                    send(pCur->newFd,&t,8+t.dataLen,0);
                }
            }
            close(logfd);
            free(pCur);
        }
        pCur=NULL;
    }
    return NULL;
}
int factoryStart(pFactory_t pf){
    if(!pf->startFlag){
        for(int i=0;i<pf->threadNum;i++){
            pthread_create(pf->pthid+i,NULL,threadFunc,pf);
        }
        pf->startFlag=1;
    }
    return 0;
}

int configInit(char *path,char *addr,char *port,int *threadNum,int *capacity){
    FILE *fp=fopen(path,"r+");
    if(fp==NULL){
        perror("init_fopen");
        return 0;
    }
    fscanf(fp,"%s %s %d%d",addr,port,threadNum,capacity);
    return 0;
}

int modeToLetter(int mode,char *str){
    if(S_ISDIR(mode)){
        str[0]='d';
    } 
    if(S_ISREG(mode)){
        str[0]='-';
    }
    if(S_ISLNK(mode)){
        str[0]='l';
    }
    return 0;
}
void get_salt(char *salt,char *passwd){
    int i,j;
    for(i=0,j=0;passwd[i]&&j!=3;i++){
        if(passwd[i]=='$'){
            ++j;
        }
    }
    strncpy(salt,passwd,i-1);
}
void writeLog(int newFd,char* userName,char* op){
    lseek(newFd,0,SEEK_END);
    char loginfo[100]={0};
    time_t nowt;
    time(&nowt);
    char timestring[40]={0};
    ctime_r(&nowt,timestring);
    sprintf(loginfo,"%-15s\t%-15s\t%-25s\n",userName,op,timestring);
    write(newFd,loginfo,strlen(loginfo));
}
int Compute_file_md5(const char *file_path, char *md5_str)
{
    int i;
    int fd;
    int ret;
    unsigned char data[READ_DATA_SIZE];
    unsigned char md5_value[MD5_SIZE];
    MD5_CTX md5;

    fd = open(file_path, O_RDONLY);
    if (-1 == fd)
    {
        perror("open");
        return -1;
    }

    // init md5
    MD5Init(&md5);

    while (1)
    {
        ret = read(fd, data, READ_DATA_SIZE);
        if (-1 == ret)
        {
            perror("read");
            close(fd);
            return -1;
        }

        MD5Update(&md5, data, ret);

        if (0 == ret || ret < READ_DATA_SIZE)
        {
            break;
        }
    }

    close(fd);

    MD5Final(&md5, md5_value);

    // convert md5 value to md5 string
    for(i = 0; i < MD5_SIZE; i++)
    {
        snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
    }

    return 0;
}

