#include "factory.h"
#define READ_DATA_SIZE	1024
#define MD5_SIZE		16
#define MD5_STR_LEN		(MD5_SIZE * 2)
int socketFd;
int recvCycle(int,void*,int);
void handle(int signum){
    printf("exit\n");
    close(socketFd);
    exit(1);
}

int Compute_file_md5(const char *file_path, char *md5_str);

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
int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);
    socketFd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(socketFd,-1,"socket");
    struct sockaddr_in ser;
    bzero(&ser,sizeof(ser));
    ser.sin_family=AF_INET;
    ser.sin_addr.s_addr=inet_addr(argv[1]);
    ser.sin_port=htons(atoi(argv[2]));
    int ret;
    ret=connect(socketFd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
    ERROR_CHECK(ret,-1,"connect");
    train_t t;
    char order[1024];
    int  dataLen=0;
    int  control_code=0;
    //char buf[1000]={0};
    char s1[30],s2[30];
    char *salt=(char*)calloc(256,sizeof(char));
    char *pwdp=(char*)calloc(256,sizeof(char));
    signal(SIGINT,handle);
    while(1){
        printf("1.网盘登陆\n");
        printf("2.用户注册\n");
        printf("请输入1或2\n");
        int option;
        scanf("%d",&option);
        //rewind(stdin);
        getchar();
        //fflush(stdin);
        if(option==1){
            send(socketFd,&option,4,0);
            char username[30]={0};
            char *passwd;
            while(1){
                printf("请输入用户名:");
                if(fgets(username,sizeof(username),stdin)!=NULL){
                    //memset(&t,0,sizeof(t));
                    username[strlen(username)-1]='\0';
                    // t.dataLen=strlen(username);
                    //strcpy(t.buf,username);
                    send(socketFd,username,strlen(username),0);
                    int i=0;
                    recv(socketFd,&i,4,0);
                    if(i){
                        break;
                    }else{
                        continue;
                    }
                }
            }
            recv(socketFd,salt,256,0);
            while(1){
                passwd=getpass("请输入密码:");
                printf("%s\n",passwd);
                pwdp=crypt(passwd,salt);
                int ret=send(socketFd,pwdp,strlen(pwdp),0);
                printf("passwd:%s\n",pwdp);
                printf("send:%d\n",ret);
                int i=0;
                recv(socketFd,&i,4,0);
                if(i){
                    printf("登陆成功\n");
                    break;
                }else{
                    continue;
                }
            }
            break;
        }else if(option==2){
            send(socketFd,&option,4,0);
            char username[30]={0};
            char *passwd;
            while(1){
                printf("请输入用户名:");
                if(fgets(username,sizeof(username),stdin)!=NULL){
                    //memset(&t,0,sizeof(t));
                    username[strlen(username)-1]='\0';
                    // t.dataLen=strlen(username);
                    //strcpy(t.buf,username);
                    send(socketFd,username,strlen(username),0);
                    int i=0;
                    recv(socketFd,&i,4,0);
                    if(i){
                        printf("用户名已存在\n");
                    }else{
                        recv(socketFd,salt,256,0);
                        printf("recv salt:%s\n",salt);
                        break;
                    }
                }
            }
            while(1){
                passwd=getpass("请输入密码:");
                if(passwd!=NULL){
                    break;
                }
            }
            while(1){
                char *passwd2=getpass("请再次输入密码:");
                if(strcmp(passwd,passwd2)==0){
                    break;
                }
            }
            pwdp=crypt(passwd,salt);
            send(socketFd,pwdp,strlen(pwdp),0);
            printf("注册成功\n");
            break;
        }else{
            continue;
        }
    }
    while(1){
        memset(order,0,sizeof(order));
        memset(s1,0,sizeof(s1));
        memset(s2,0,sizeof(s2));
        printf("请输入命令：");
        fgets(order,sizeof(order),stdin);
        ret=sscanf(order,"%s %s",s1,s2);
        char md5_str[MD5_STR_LEN + 1];
        if(strcmp(s1,"cd")==0){
            if(strlen(s2)==0){
                continue;
            }
            memset(&t,0,sizeof(train_t));
            t.dataLen=strlen(s2);
            t.control_code=1;
            strcpy(t.buf,s2);
            send(socketFd,&t,8+t.dataLen,0);
        }
        if(strcmp(s1,"ls")==0){
            memset(&t,0,sizeof(train_t));
            t.dataLen=strlen(s2);
            t.control_code=2;
            send(socketFd,&t,8+t.dataLen,0);
            memset(&t,0,sizeof(train_t));
            while(1){
                recv(socketFd,&t.dataLen,4,MSG_WAITALL);
                if(t.dataLen==0){
                    break;
                }
                recv(socketFd,&t.control_code,4,MSG_WAITALL);
                recv(socketFd,t.buf,t.dataLen,MSG_WAITALL);
                printf("%s\n",t.buf);
            }
        }
        if(strcmp(s1,"puts")==0){
            memset(md5_str,0,sizeof(md5_str));
            //发送文件名
            memset(&t,0,sizeof(train_t));
            Compute_file_md5(s2,md5_str);
            t.dataLen=strlen(s2);
            t.control_code=3;
            strcpy(t.buf,s2);
            send(socketFd,&t,8+t.dataLen,0);
            //发送MD5
            memset(&t,0,sizeof(train_t));
            t.dataLen=strlen(md5_str);
            strcpy(t.buf,md5_str);
            printf("%s\n",t.buf);
            printf("%s\n",md5_str);
            send(socketFd,&t,8+t.dataLen,0);
            int i=0;
            recv(socketFd,&i,4,0);
            //服务器端没有文件就发送
            if(i==0){
                tranFile(socketFd,s2);
            }
        }
        if(strcmp(s1,"gets")==0){
            memset(&t,0,sizeof(train_t));
            t.dataLen=strlen(s2);
            t.control_code=4;
            strcpy(t.buf,s2);
            send(socketFd,&t,8+t.dataLen,0);
            //搜索当前文件夹下是否已经下载文件
            char CurPath[40]={0};
            strcpy(CurPath,getcwd(NULL,0));
            DIR *pd;
            pd=opendir(CurPath);
            if(pd==NULL){
                perror("opendir");
                return 0;
            }
            struct dirent *p;
            struct stat st_buf;
            memset(&st_buf,0,sizeof(struct stat));
            int flag=0;
            int oldsize=0,downloadsize=0;
            while((p=readdir(pd))!=NULL){
                int ret;
                if(strcmp(p->d_name,s2)==0){
                    ret=stat(p->d_name,&st_buf);
                    flag=1;
                    break;
                    ERROR_CHECK(ret,-1,"stat");
                }
            }
            if(flag){
                printf("已存在\n");
                oldsize=downloadsize=st_buf.st_size;
                printf("客户端，已存在文件的大小为：%ld\n",st_buf.st_size);
                send(socketFd,&st_buf.st_size,sizeof(off_t),0);
            }else{
                off_t i=0;
                printf("不存在\n");
                send(socketFd,&i,sizeof(i),0);
            }
            off_t filesize;
            recv(socketFd,&dataLen,4,MSG_WAITALL);
            recv(socketFd,&control_code,4,MSG_WAITALL);
            recv(socketFd,&filesize,sizeof(off_t),MSG_WAITALL);
            printf("filesize =%ldB\n",filesize);
            int fd;
            fd=open(s2,O_RDWR|O_CREAT,0666);
            if(st_buf.st_size){
                lseek(fd,st_buf.st_size,SEEK_SET);
            }
            ERROR_CHECK(fd,-1,"open");
            int fds[2];
            pipe(fds);
            int slicesize=filesize/100;
            flag=1;
            if(downloadsize==filesize){
                flag=0;
            }
            printf("before while:downloadsize %d\n",downloadsize);
            while(flag){
                ret=splice(socketFd,NULL,fds[1],NULL,4096,SPLICE_F_MOVE|SPLICE_F_MORE); 
                if(ret==0){
                    break;
                }
                splice(fds[0],NULL,fd,NULL,ret,SPLICE_F_MORE|SPLICE_F_MOVE);
                downloadsize+=ret;
                /* printf("downloadsize %d\n",downloadsize); */
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
            close(fd);
            close(fds[0]);
            close(fds[1]);
            printf("close success\n");
            fflush(stdout);
        }
        if(strcmp(s1,"remove")==0){
            memset(&t,0,sizeof(train_t));
            t.dataLen=strlen(s2);
            t.control_code=5;
            strcpy(t.buf,s2);
            send(socketFd,&t,8+t.dataLen,0);
        }
        if(strcmp(s1,"pwd")==0){
            memset(&t,0,sizeof(train_t));
            t.dataLen=0;
            t.control_code=6;
            send(socketFd,&t,8+t.dataLen,0);
            recv(socketFd,&t.dataLen,4,MSG_WAITALL);
            recv(socketFd,&t.control_code,4,MSG_WAITALL);
            recv(socketFd,t.buf,t.dataLen,MSG_WAITALL);
            printf("%s\n",t.buf);
        }
    }
    close(socketFd);
    return 0;
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

