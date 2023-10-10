#include<iostream>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

const int N=50;
int num=0;
/*

/////////////////互斥锁
pthread_mutex_t mutex;//创建互斥锁

void* funA_num(void *arg)
{
    for(int i=1;i<=N;i++){
        pthread_mutex_lock(&mutex);
        int res=num;
        res++;
        num=res;
        usleep(5);
        pthread_mutex_unlock(&mutex);
        printf("threadA id=%lu num=%d\n",pthread_self(),num);
    }
    return NULL;
}

void *funB_num(void *arg)
{
    for(int i=1;i<=N;i++){
        pthread_mutex_lock(&mutex);
        int res=num;
        res++;
        usleep(10);
        num=res;
        pthread_mutex_unlock(&mutex);
        printf("threadB id=%lu num=%d\n",pthread_self(),num);
    }    
    return NULL;
}

int main(int argv,char *argc[])
{
    pthread_mutex_init(&mutex,NULL);//初始化互斥锁
    pthread_t p1,p2;
    //创建两个线程
    pthread_create(&p1,NULL,funA_num,NULL);
    pthread_create(&p2,NULL,funB_num,NULL);

    //阻塞。回收线程
    pthread_join(p1,NULL);
    pthread_join(p2,NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}

*/

/*
/////////////////读写锁
pthread_rwlock_t rwlock;

void *writeNum(void *arv)
{
    while(1){
        pthread_rwlock_wrlock(&rwlock);
        int res=num;
        res++;
        num=res;
        printf("写操作完成 num=%d tid=%ld\n",num,pthread_self());
        pthread_rwlock_unlock(&rwlock);
        usleep(rand()%100);
    }
    return NULL;
}
void *readNum(void *arv)
{
    while(1){
        pthread_rwlock_rdlock(&rwlock);
        printf("全局变量num=%d pid=%ld\n",num,pthread_self());
        pthread_rwlock_unlock(&rwlock);
        usleep(rand()%100);
    }
}

int main()
{
    pthread_rwlock_init(&rwlock,NULL);

    pthread_t wrid[3];
    pthread_t rdid[5];

    for(int i=1;i<=3;i++){
        pthread_create(&wrid[i],NULL,writeNum,NULL);
    }
    for(int i=1;i<=5;i++){
        pthread_create(&rdid[i],NULL,readNum,NULL);
    }

    for(int i=1;i<=3;i++){
        pthread_join(wrid[i],NULL);
    }
    for(int i=1;i<=5;i++){
        pthread_join(rdid[i],NULL);
    }

    pthread_rwlock_destroy(&rwlock);
    return 0;
}
*/

/////////////条件变量

/*
struct Node
{
    int num;
    struct Node* next;
};
struct Node* head=NULL;

pthread_cond_t cond;
pthread_mutex_t mutex;

void *produce(void *arv)
{
    while(1){
        pthread_mutex_lock(&mutex);
        Node* pnew=(Node*)malloc(sizeof(Node));
        pnew->num=rand()%1000;
        pnew->next=head;
        head=pnew;
        printf("++produce  num=%d tid=%ld\n",pnew->num,pthread_self());
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&cond);
        sleep(rand()%3);
    }
    return NULL;
}

void *consume(void *arv)
{
    while(1){
        pthread_mutex_lock(&mutex);
        while(head==NULL){
            pthread_cond_wait(&cond,&mutex);
        }
        Node *pnew=head;
        printf("--consume num=%d tid=%ld\n",pnew->num,pthread_self());
        head=pnew->next;
        free(pnew);
        pthread_mutex_unlock(&mutex);

        sleep(rand()%3);
    }
    return NULL;
}

int main()
{
    pthread_cond_init(&cond,NULL);
    pthread_mutex_init(&mutex,NULL);

    pthread_t ptid[5];
    pthread_t ctid[5];
    for(int i=1;i<=5;i++){
        pthread_create(&ptid[i],NULL,produce,NULL); 
    }
    for(int i=1;i<=5;i++){
        pthread_create(&ctid[i],NULL,consume,NULL);
    }

    for(int i=1;i<=5;i++){
        pthread_join(ptid[i],NULL);
        pthread_join(ctid[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
*/


/////////////信号量
struct Node
{
    int num;
    struct Node* next;
};
struct Node* head=NULL;

sem_t psem;
sem_t csem;
pthread_mutex_t mutex;

void *produce(void *arv)
{
    while(1){
        sem_wait(&psem);
        pthread_mutex_lock(&mutex);
        Node* pnew=(Node*)malloc(sizeof(Node));
        pnew->num=rand()%1000;
        pnew->next=head;
        head=pnew;
        printf("++produce  num=%d tid=%ld\n",pnew->num,pthread_self());
        pthread_mutex_unlock(&mutex);
        sem_post(&csem);
        sleep(rand()%3);
    }
    return NULL;
}

void *consume(void *arv)
{
    while(1){
        sem_wait(&csem);
        pthread_mutex_lock(&mutex);
        Node *pnew=head;
        printf("--consume num=%d tid=%ld\n",pnew->num,pthread_self());
        head=pnew->next;
        free(pnew);
        pthread_mutex_unlock(&mutex);
        sem_post(&psem);
        sleep(rand()%3);
    }
    return NULL;
}

int main()
{
    sem_init(&psem,0,5);
    sem_init(&csem,0,0);
    pthread_mutex_init(&mutex,NULL);

    pthread_t ptid[5];
    pthread_t ctid[5];
    for(int i=1;i<=5;i++){
        pthread_create(&ptid[i],NULL,produce,NULL); 
    }
    for(int i=1;i<=5;i++){
        pthread_create(&ctid[i],NULL,consume,NULL);
    }

    for(int i=1;i<=5;i++){
        pthread_join(ptid[i],NULL);
        pthread_join(ctid[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&psem);
    sem_destroy(&csem);
    return 0;
}
