#include<iostream>
#include<cstring>
#include<cstdio>
#include<cstdlib>
using namespace std;

#define MEM_PAGE_SIZE 0x1000

struct mempool_t{
    int block_size;//单个块的大小
    int free_count;//块都数量
    void* mem;     //块的起始地址
    void* ptr;     //当前空闲的块的首地址
};

int memp_init(mempool_t* mp,size_t block_size){
    if(!mp)return -1;
    memset(mp,0,sizeof(mempool_t));

    mp->block_size=block_size;
    mp->free_count=MEM_PAGE_SIZE/block_size;

    mp->mem=malloc(MEM_PAGE_SIZE);
    if(!mp->mem)return -1;

    mp->ptr=mp->mem;
    //char *ptr=(char*)mp->ptr;
    char *ptr2=(char*)mp->ptr;
    for(int i=0;i<mp->free_count;i++){
        *(char**)ptr2=ptr2+block_size;
        ptr2+=block_size;
    }
    *(char**)ptr2=NULL;
    return 0;
}

void* _malloc(mempool_t *mp,size_t size){
    if(!mp||mp->free_count==0)return NULL;
    if(mp->block_size<size)return NULL;

    void* ptr1=mp->ptr;
    printf("mp->ptr---%p   ptr1---%p\n",mp->ptr,ptr1);
    mp->ptr=*(char**)ptr1;
    printf("mp->ptr---%p   pre1---%p\n   ",mp->ptr,*(char**)ptr1);
    mp->free_count--;

    return ptr1;
}

void _free(mempool_t *mp,void *ptr){
    if(!mp)return ;
    *(char**)ptr=(char*)mp->ptr;
    mp->ptr=ptr;
    mp->free_count++;
    printf("free\n");
}

//#define malloc(mempool_t *mp,size_t size)   _malloc(mempool_t* mp,size_t size)

int main()
{
    mempool_t mp;
    memp_init(&mp,32);
    void* p1=_malloc(&mp,5);
    printf("p1-> %p\n",p1);
    void* p2=_malloc(&mp,10);
    printf("p2-> %p\n",p2);
    void* p3=_malloc(&mp,15);
    printf("p3-> %p\n",p3);
    void* p4=_malloc(&mp,20);
    printf("p4-> %p\n",p4);

    _free(&mp,p1);
    printf("ptr->%p\n",mp.ptr);
    _free(&mp,p3);
    printf("ptr->%p\n",mp.ptr);

    void* p5=_malloc(&mp,25);
    printf("p5-> %p\n",p5);
    void* p6=_malloc(&mp,30);
    printf("p6-> %p\n",p6);
    return 0;
}