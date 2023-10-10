#ifndef _PTHREADPOOL_H
#define _PTHREADPOOL_H

struct threadpool;

threadpool* threadpool_create(int min,int max,int task_num);//创建线程池

void task_add(threadpool* pool,void (*function)(void *arg),void* arg);//往线程池添加任务

int threadpool_destroy(threadpool* pool);//线程池销毁

int busy_thread_num(threadpool* pool);//查询忙的线程数量

int alive_thread_num(threadpool* pool);//查询所有活着的线程数量

void *manager(void* arg);//管理者线程

void *worker(void* arg);//工作者线程

void thread_exit(threadpool* pool);//线程退出

#endif