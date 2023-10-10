#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/epoll.h>
int main()
{
    int lfd=socket(AF_INET,SOCK_STREAM,0);
    if(lfd==-1){
        perror("socket创建失败\n");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(9990);
    int len=sizeof(addr);
    int cnt=bind(lfd,(struct sockaddr*)&addr,len);
    if(len==-1){
        perror("bind绑定失败\n");
        exit(-2);
    }

    cnt=listen(lfd,100);
    if(cnt==-1){
        perror("listen创建失败\n");
        exit(-3);
    }

    int efd=epoll_create(1);

    struct epoll_event ev;
    ev.data.fd=lfd;
    ev.events=EPOLLIN;
    cnt=epoll_ctl(efd,EPOLL_CTL_ADD,lfd,&ev);
    if(cnt==-1){
        perror("cpoll_ctl创建失败\n");
        exit(-4);
    }
    struct epoll_event evs[1024];
    int size=sizeof(evs);
    while(1){
        int num=epoll_wait(efd,evs,size,-1);
        for(int i=0;i<num;i++){
            int cdfd=evs[i].data.fd;
            if(cdfd==lfd){
                int cfd=accept(lfd,NULL,NULL);
                ev.data.fd=cfd;
                ev.events=EPOLLIN;
                cnt=epoll_ctl(efd,EPOLL_CTL_ADD,cfd,&ev);
                if(cnt==-1){
                    perror("cpoll_ctl创建失败\n");
                    exit(-4);
                }
            }else{
                char buf[5];
                memset(buf,0,sizeof(buf));
                int cnt=recv(cdfd,buf,sizeof(buf),0);
                if(cnt==0){
                    printf("客户端断开连接\n");
                    epoll_ctl(efd,EPOLL_CTL_DEL,cdfd,NULL);
                    close(cdfd);
                }else if(cnt>0){
                    printf("客户端say : %s\n",buf);
                    send(cdfd,buf,cnt,0);
                }else{
                    perror("异常\n");
                    exit(0);
                }
            }
        }
    }

    return 0;
}