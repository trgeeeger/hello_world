#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<thread>
#include<unistd.h>

#include"pthreadpool.h"

using namespace std;

void add(void* arg){
    int num=*((int*)arg);
    printf("thread %ld is working, number = %d\n",pthread_self(), num);
    sleep(1);
}

int main()
{
    threadpool*pool=threadpool_create(3,10,100);
    for(int i=1;i<=100;i++){
        int *num=(int*)malloc(sizeof(int));
        *num=i+100;
        task_add(pool,add,num);
    }
    sleep(30);
    threadpool_destroy(pool);
    return 0;
}