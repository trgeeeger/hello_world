#include"pthreadpool.h"
#include<iostream>
#include<thread>
#include<unistd.h>
#include<bits/stdc++.h>
struct Task//任务体
{
    void (*function)(void *arg);
    void *arg;
};
const int N=2;
struct ThreadPool       //定义线程池结构体
{
    Task* taskQ;
    int queueCapacity;  //任务容量
    int queueSize;      //当前任务个数
    int queueFront;     //队头
    int queueRear;       //队尾

    pthread_t managerID;//管理者线程id
    pthread_t *threadIDs;   //工作线程id
    int minNum;         //最小线程数
    int maxNum;         //最大线程数
    int liveNum;        //存活线程个数
    int busyNum;        //忙的线程个数
    int exitNum;        //销毁线程个数

    pthread_mutex_t mutexPool;//锁整个线程池
    pthread_mutex_t mutexBusy;//锁忙碌的线程
    pthread_cond_t notFull;//判断线程是不是满了
    pthread_cond_t notEmpty;//判断线程是不是为空
    int shutdown;           //是否关闭线程池 1为是 0为否
};


ThreadPool* threadPoolCreate(int min,int max,int queueSize)
{
    ThreadPool *pool=(ThreadPool*)malloc(sizeof(ThreadPool));//创建线程池
    do{
        if(pool==NULL){
            printf("pool线程池创建失败\n");
        }
        pool->threadIDs=(pthread_t*)malloc(sizeof(pthread_t)*max);//创建工作线程id
        if(pool->threadIDs==NULL){
            printf("工作线程创建失败\n");
        }
        memset(pool->threadIDs,0,sizeof(pthread_t)*max);///初始化为0
        ////////////////初始化线程变量
        pool->maxNum=max;
        pool->minNum=min;
        pool->liveNum=min;
        pool->busyNum=0;
        pool->exitNum=0;
        ////////////////互斥锁以及条件变量初始化
    if(pthread_mutex_init(&pool->mutexPool,NULL)!=0||
        pthread_mutex_init(&pool->mutexBusy,NULL)!=0||
        pthread_cond_init(&pool->notFull,NULL)!=0||
        pthread_cond_init(&pool->notEmpty,NULL)!=0){
        printf("初始化互斥锁或条件变量失败\n");                    
    }
    ////////////////初始化任务队列
    pool->taskQ=(Task*)malloc(sizeof(Task)*queueSize);
    pool->queueCapacity=queueSize;
    pool->queueFront=0;
    pool->queueRear=0;
    pool->queueSize=0;
    pool->shutdown=0;
    ////////////////创建管理者线程
    pthread_create(&pool->managerID,NULL,manager,pool);
    ////////////////创建工作线程
    for(int i=0;i<pool->minNum;i++){
        pthread_create(&pool->threadIDs[i],NULL,worker,pool);
    }
    return pool;
    }while(0);
    ///////////////释放堆内存
    if(pool&&pool->threadIDs)free(pool->threadIDs);
    if(pool&&pool->taskQ)free(pool->taskQ);
    if(pool)free(pool);
    return NULL;
}

int threadPoolDestroy(ThreadPool* pool)
{
    if(pool==NULL)return -1;
    pool->shutdown=1;
    //////////////回收管理者线程
    pthread_join(pool->managerID,NULL);
    //////////////唤醒阻塞的消费者线程 在消费者线程函数里会自动销毁
    for(int i=0;i<pool->liveNum;i++){
        pthread_cond_signal(&pool->notEmpty);
    }
    if(pool->taskQ)free(pool->taskQ);
    if(pool->threadIDs)free(pool->threadIDs);
    ///////////////////////互斥锁以及条件变量销毁
    pthread_mutex_destroy(&pool->mutexBusy);
    pthread_mutex_destroy(&pool->mutexPool);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);
    free(pool);
    pool=NULL;
    return 0;
}

void threadpoolAdd(ThreadPool*pool,void (*func)(void *arg),void *arg)
{
    pthread_mutex_lock(&pool->mutexPool);
    if(pool->queueSize==pool->queueCapacity&&!pool->shutdown){
        pthread_cond_wait(&pool->notFull,&pool->mutexPool);
    }
    if(pool->shutdown){
        pthread_mutex_unlock(&pool->mutexPool);
        return;
    }
    pool->taskQ[pool->queueRear].function=func;
    pool->taskQ[pool->queueRear].arg=arg;
    pool->queueRear=(pool->queueRear+1)%pool->queueCapacity;
    pool->queueSize++;
    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->mutexPool);
}

int Busythread(ThreadPool* pool)
{
    pthread_mutex_lock(&pool->mutexBusy);
    int BusyThreadNum=pool->busyNum;
    pthread_mutex_unlock(&pool->mutexBusy);
    return BusyThreadNum;
}
int Alivethread(ThreadPool* pool)
{
    pthread_mutex_lock(&pool->mutexPool);
    int AliveThreadNum=pool->liveNum;
    pthread_mutex_unlock(&pool->mutexPool);
    return AliveThreadNum;
}
void *worker(void *arg)
{
    ThreadPool *pool=(ThreadPool*)arg;
    while(1){
        pthread_mutex_lock(&pool->mutexPool);
        while(pool->queueSize==0&&!pool->shutdown){
            pthread_cond_wait(&pool->notEmpty,&pool->mutexPool);
            if(pool->exitNum>0){
                pool->exitNum--;
                if(pool->liveNum>pool->minNum){
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    ThreadExit(pool);
                }
            }
        }
        if(pool->shutdown){
            pthread_mutex_unlock(&pool->mutexPool);
            ThreadExit(pool);
        }
        Task task;
        task.function=pool->taskQ[pool->queueFront].function;
        task.arg=pool->taskQ[pool->queueFront].arg;
        pool->queueFront=(pool->queueFront+1)%pool->queueCapacity;
        pool->queueSize--;
        pthread_cond_signal(&pool->notFull);
        pthread_mutex_unlock(&pool->mutexPool);
        printf("thread %ld start working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexBusy);
        task.function(task.arg);
        free(task.arg);
        task.arg=NULL;
        printf("thread %ld end working...\n", pthread_self());
        pthread_mutex_lock(&pool->mutexBusy);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexBusy);
    }
    return NULL;
}

void *manager(void* arg)
{
    ThreadPool* pool=(ThreadPool*)arg;
    while(!pool->shutdown){
        sleep(3);
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize=pool->queueSize;
        int threadAlive=pool->liveNum;
        int threadBusy=pool->busyNum;
        pthread_mutex_unlock(&pool->mutexPool);

        if(queueSize>threadAlive&&threadAlive<pool->maxNum){
            pthread_mutex_lock(&pool->mutexPool);
            int count=0;
            for(int i=0;i<pool->maxNum&&count<N&&threadAlive<pool->maxNum;i++){
                if(pool->threadIDs[i]==0){
                    count++;
                    pthread_create(&pool->threadIDs[i],NULL,worker,pool);
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }
        if(pool->busyNum*2<pool->liveNum&&pool->liveNum>pool->minNum){
            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum=N;
            pthread_mutex_unlock(&pool->mutexPool);
            for(int i=0;i<N;i++){
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return NULL;
}
void ThreadExit(ThreadPool* pool)
{
    pthread_t tid=pthread_self();
    for(int i=0;i<pool->maxNum;i++){
        if(pool->threadIDs[i]==tid){
            pool->threadIDs[i]=0;
            printf("threadExit() called, %ld exiting...\n", tid);
            break;
        }
    }
    pthread_exit(NULL);
}
