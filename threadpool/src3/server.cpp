#include<iostream>
#include<unistd.h>
#include<cstdio>
#include<cstdlib>
#include<bits/stdc++.h>
#include<arpa/inet.h>
#include<thread>

#include"pthreadpool.h"

using namespace std;

void working(void* arg);
void connect(void* arg);
struct conninfo
{
    int fd;
    threadpool* pool;
};
struct pinfo
{
    sockaddr_in addr;
    int cfd;
};
int main()
{
    //创建套接字
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1){
        perror("socket\n");
        exit(0);
    }
    //绑定 ip和端口
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(10000);
    addr.sin_addr.s_addr=INADDR_ANY;
    int ret=bind(sfd,(sockaddr*)&addr,sizeof(addr));
    if(ret==-1){
        perror("bind\n");
        exit(0);
    }
    //设置监听
    ret=listen(sfd,128);
    if(ret==-1){
        perror("listen\n");
        exit(0);
    }
    //accept连接
    threadpool* pool=threadpool_create(3,8,100);
    conninfo* info_pp=(conninfo*)malloc(sizeof(conninfo));
    
    info_pp->fd=sfd;
    info_pp->pool=pool;
    cout<<1<<endl;
    task_add(pool,connect,info_pp);
    cout<<1<<endl;
    pthread_exit(NULL);
    return 0;
}
void connect(void* arg){
    conninfo* info_pp=(conninfo*)arg;
    socklen_t len=sizeof(struct sockaddr_in);
    while(1){
        pinfo* info_p=(pinfo*)malloc(sizeof(pinfo));
        int cfd=accept(info_pp->fd,(struct sockaddr*)&info_p->addr,&len);
        if(cfd==-1){
            perror("accept\n");
            break;
        }
        info_p->cfd=cfd;
        task_add(info_pp->pool,working,info_p);
    }
    close(info_pp->fd);
}
void working(void* arg){
    struct pinfo* info_p=(struct pinfo*)arg;
    char ip[24];
    printf("客户端ip:%s 客户端端口:%d\n",inet_ntop(info_p->cfd,&info_p->addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(info_p->addr.sin_port));
    while(1){
        char buf[1024];
        memset(buf,0,sizeof(buf));
        int len=read(info_p->cfd,buf,sizeof(buf));
        if(len>0){
            printf("客户端say:%s\n",buf);
            write(info_p->cfd,buf,len);
        }
        else if(len==0){
            printf("客户端断开连接\n");
            break;
        }
        else if(len<0){
            perror("read\n");
            break;
        }
    }
    close(info_p->cfd);
}