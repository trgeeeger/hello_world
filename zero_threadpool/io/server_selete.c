#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main()
{

    int lfd=socket(AF_INET,SOCK_STREAM,0);
    if(lfd==-1){
        perror("socket创建失败\n");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_port=htons(9990);
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_family=AF_INET;
    int cnt=bind(lfd,(struct sockaddr*)&addr,sizeof(addr));
    if(cnt==-1){
        perror("bind 创建失败\n");
        exit(-2);
    }

    cnt=listen(lfd,100);
    if(cnt==-1){
        perror("listen创建失败\n");
        exit(-3);
    }

    fd_set rsets,temp;
    FD_ZERO(&rsets);
    FD_SET(lfd,&rsets);
    int maxn=lfd;
    while(1){
        temp=rsets;
        int num=select(maxn+1,&temp,NULL,NULL,NULL);

        if(FD_ISSET(lfd,&temp)){
            struct sockaddr_in addrr;
            socklen_t len=sizeof(addrr);
            int cfd=accept(lfd,(struct sockaddr*)&addrr,&len);
            if(cfd==-1){
                perror("accept创建失败\n");
                exit(-4);
            }
            FD_SET(cfd,&rsets);
            maxn=maxn>cfd?maxn:cfd;
        }
        for(int i=lfd+1;i<maxn+1;i++){
            if(FD_ISSET(i,&temp)){
                char buf[1024];
                int cnt=recv(i,buf,sizeof(buf),0);
                if(cnt==0){
                    printf("客户端断开了连接\n");
                    FD_CLR(i,&rsets);
                    close(i);
                }else if(cnt>0){
                    write(i,buf,strlen(buf)+1);
                }else{
                    perror("出现异常\n");
                }
            }
        }
    }
    return 0;
}
