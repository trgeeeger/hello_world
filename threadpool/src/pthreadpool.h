#ifndef _PTHREADPOOL_H
#define _PTHREADPOOL_H

struct ThreadPool;

ThreadPool* threadPoolCreate(int min,int max,int queueSize);

int threadPoolDestroy(ThreadPool* pool);

void threadpoolAdd(ThreadPool*pool,void (*func)(void *arg),void *arg);

int Busythread(ThreadPool* pool);

int Alivethread(ThreadPool* pool);

void *worker(void *arg);

void *manager(void* arg);

void ThreadExit(ThreadPool* pool);
#endif