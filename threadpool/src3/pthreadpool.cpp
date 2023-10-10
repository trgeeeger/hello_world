#include"pthreadpool.h"
#include<iostream>
#include<cstdio>
#include<thread>
#include<unistd.h>
#include<cstdlib>
#include<bits/stdc++.h>
using namespace std;

const int ADDTHREADNUM=2;
void* worker(void* arg);
void* manager(void* arg);
struct Task{
    void (*function)(void *arg);
    void *arg;
};

struct threadpool{
    Task* task;
    int task_capicity;//任务容量
    int task_now;     //当前任务数
    int queue_front;  //队头
    int queue_back;   //队尾

    int max_thread_num;   //最大线程数
    int min_thread_num;   //最小线程数
    int busy_thread_num;  //忙的线程数
    int alive_thread_num; //存活线程数
    int destroy_thread_num;//要销毁的线程数

    int shutdown;//0表示存在 1表示需要销毁

    pthread_t manager_thread;//管理者线程id
    pthread_t* worker_thread;//工作线程id
    pthread_mutex_t mutex_pthreadpool;//线程池锁
    pthread_mutex_t mutex_pthread_busy;//忙的线程锁
    pthread_cond_t not_full;            //判断任务是否满了
    pthread_cond_t not_empty;            //判断任务是否空了
};

threadpool* threadpool_create(int min,int max,int task_num){
    threadpool* pool=(threadpool*)malloc(sizeof(threadpool));
    do{
        if(pool==NULL){
            printf("线程池创建失败\n");
            break;
        }
        //////////////////初始化线程相关数据

        pool->worker_thread=(pthread_t*)malloc(sizeof(pthread_t)*max);
        if(pool->worker_thread==NULL){
            printf("工作线程创建失败\n");
            break;
        }
        memset(pool->worker_thread,0,sizeof(pthread_t)*max);
        pool->max_thread_num=max;
        pool->min_thread_num=min;
        pool->busy_thread_num=0;
        pool->alive_thread_num=min;
        pool->destroy_thread_num=0;
        pool->shutdown=0;
        //////////////////初始化任务相关数据
        pool->task=(Task*)malloc(sizeof(Task)*task_num);
        pool->task_capicity=task_num;
        pool->task_now=0;
        pool->queue_back=0;
        pool->queue_front=0;

        if(pthread_mutex_init(&pool->mutex_pthread_busy,NULL)!=0||
            pthread_mutex_init(&pool->mutex_pthreadpool,NULL)!=0||
            pthread_cond_init(&pool->not_full,NULL)!=0||
            pthread_cond_init(&pool->not_empty,NULL)!=0){
                printf("线程池锁或者条件变量创建失败");
                break;
        }
        ///创建管理者线程
        pthread_create(&pool->manager_thread,NULL,manager,pool);
        ///创建工作线程
        for(int i=0;i<pool->min_thread_num;i++){
            pthread_create(&pool->worker_thread[i],NULL,worker,pool);
            printf("创建线程：%ld\n",pool->worker_thread[i]);
        }
        return pool;
    }while(0);
    if(pool&&pool->worker_thread)free(pool->worker_thread);
    if(pool&&pool->task)free(pool->task);
    if(pool)free(pool);
    return NULL;
}

void task_add(threadpool* pool,void (*function)(void *arg),void* arg){
    pthread_mutex_lock(&pool->mutex_pthreadpool);
    while(pool->task_now==pool->task_capicity&&!pool->shutdown){
        printf("任务数量过大 不能加入任务\n");
        pthread_cond_wait(&pool->not_full,&pool->mutex_pthreadpool);
    }
    if(pool->shutdown){
        pthread_mutex_unlock(&pool->mutex_pthreadpool);
        return;
    }
    pool->task[pool->queue_back].function=function;
    pool->task[pool->queue_back].arg=arg;
    pool->task_now++;
    pool->queue_back=(pool->queue_back+1)%pool->task_capicity;
    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->mutex_pthreadpool);
}

int threadpool_destroy(threadpool* pool){
    if(pool==NULL)return -1;
    pool->shutdown=1;
    ////回收管理者线程
    pthread_join(pool->manager_thread,NULL);
    ///回收消费者线程
    for(int i=0;i<pool->alive_thread_num;i++){
        pthread_cond_signal(&pool->not_empty);
    }
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
    pthread_mutex_destroy(&pool->mutex_pthread_busy);
    pthread_mutex_destroy(&pool->mutex_pthreadpool);
    if(pool->task)free(pool->task);
    if(pool->worker_thread)free(pool->worker_thread);
    if(pool)free(pool);
    pool=NULL;
    return 0;
}

int busy_thread_num(threadpool* pool){
    pthread_mutex_lock(&pool->mutex_pthread_busy);
    int busy_thread_num=pool->busy_thread_num;
    pthread_mutex_unlock(&pool->mutex_pthread_busy);
    return busy_thread_num;
}

int alive_thread_num(threadpool* pool){
    pthread_mutex_unlock(&pool->mutex_pthreadpool);
    int alive_thread_num=pool->alive_thread_num;
    pthread_mutex_unlock(&pool->mutex_pthreadpool);
    return alive_thread_num;
}

void* worker(void* arg){
    threadpool* pool=(threadpool*)arg;
    while(1){
        pthread_mutex_lock(&pool->mutex_pthreadpool);
        while(pool->task_now==0&&!pool->shutdown){
            pthread_cond_wait(&pool->not_empty,&pool->mutex_pthreadpool);
            if(pool->destroy_thread_num>0){
                pool->destroy_thread_num--;
                if(pool->alive_thread_num>pool->min_thread_num){
                    pool->alive_thread_num--;
                    pthread_mutex_unlock(&pool->mutex_pthreadpool);
                    thread_exit(pool);
                }
            }
        }
        if(pool->shutdown){
            pthread_mutex_unlock(&pool->mutex_pthreadpool);
            thread_exit(pool);
        }
        Task task;
        task.arg=pool->task[pool->queue_front].arg;
        task.function=pool->task[pool->queue_front].function;
        pool->queue_front=(pool->queue_front+1)%pool->task_capicity;
        pool->task_now--;
        pthread_cond_signal(&pool->not_full);
        pthread_mutex_unlock(&pool->mutex_pthreadpool);
        printf("thread %ld 开始工作\n",pthread_self());
        pthread_mutex_lock(&pool->mutex_pthread_busy);
        pool->busy_thread_num++;
        pthread_mutex_unlock(&pool->mutex_pthread_busy);
        cout<<2<<endl;
        task.function(task.arg);
        free(task.arg);
        task.arg=NULL;
        printf("thread %ld 结束工作\n",pthread_self());
        pthread_mutex_lock(&pool->mutex_pthread_busy);
        pool->busy_thread_num--;
        pthread_mutex_unlock(&pool->mutex_pthread_busy);

    }
    return NULL;
}

void* manager(void *arg){
    threadpool* pool=(threadpool*)arg;
    while(!pool->shutdown){
        sleep(3);
        pthread_mutex_lock(&pool->mutex_pthreadpool);
        int busy_thread_num=pool->busy_thread_num;
        int alive_thread_num=pool->alive_thread_num;
        int max_thread_num=pool->max_thread_num;
        int min_thread_num=pool->min_thread_num;
        int task_now=pool->task_now;
        pthread_mutex_unlock(&pool->mutex_pthreadpool);
        ///增加线程
        if(task_now>alive_thread_num-busy_thread_num&&alive_thread_num<max_thread_num){
            pthread_mutex_lock(&pool->mutex_pthreadpool);
            int count=0;
            for(int i=0;i<max_thread_num&&count<ADDTHREADNUM&&pool->alive_thread_num<max_thread_num;i++){
                if(pool->worker_thread[i]==0){
                    pthread_create(&pool->worker_thread[i],NULL,worker,pool);
                    pool->alive_thread_num++;
                    count++;
                }
            }
            pthread_mutex_unlock(&pool->mutex_pthreadpool);
        }
        ///减少线程
        if(busy_thread_num*2<alive_thread_num&&alive_thread_num>min_thread_num){
            int count=0;
            pthread_mutex_lock(&pool->mutex_pthreadpool);
            pool->destroy_thread_num=ADDTHREADNUM;
            pthread_mutex_unlock(&pool->mutex_pthreadpool);
            for(int i=0;i<ADDTHREADNUM;i++){
                pthread_cond_signal(&pool->not_empty);
            }
        }
    }
    return NULL;
}

void thread_exit(threadpool* pool){
    pthread_t tid=pthread_self();
    for(int i=0;i<pool->max_thread_num;i++){
        if(pool->worker_thread[i]==tid){
            pool->worker_thread[i]=0;
            printf("thread %ld 线程退出\n",tid);
            break;
        }
    }
    pthread_exit(NULL);
}
