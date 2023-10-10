#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<bits/stdc++.h>
#include<arpa/inet.h>

int main()
{
    //创建套接字
    int cfd=socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1){
        perror("socket\n");
        exit(0);
    }
    //connect连接
    struct sockaddr_in addr;
    addr.sin_port=htons(10000);
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);
    int ret=connect(cfd,(sockaddr*)&addr,sizeof(addr));
    if(ret==-1){
        perror("connect\n");
        exit(0);
    }
    int num=0;
    while(1){
        //发送数据
        char buf[1024];
        sprintf(buf,"你好 服务器%d\n",num++);
        write(cfd,buf,sizeof(buf));
        //接受数据
        memset(buf,0,sizeof(buf));
        int ret=read(cfd,buf,sizeof(buf));
        if(ret>0){
            printf("服务器say:%s\n",buf);
        }
        else if(ret==0){
            printf("服务器断开连接\n");
            break;
        }
        else {
            perror("read1\n");
            break;
        }
        sleep(1);
    }
    close(cfd);
    return 0;
}