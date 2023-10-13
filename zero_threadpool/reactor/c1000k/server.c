#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_LENGTH	128
#define EVENTS_LENGTH	128

#define PORT_COUNT		100
#define ITEM_LENGTH		1024


// listenfd, clientfd
struct sock_item { // conn_item每个fd都分配一个sock_item对象

	int fd; // clientfd

	char *rbuffer;
	int rlength; //

	char *wbuffer;
	int wlength;
	
	int event;//事件

	void (*recv_cb)(int fd, char *buffer, int length);
	void (*send_cb)(int fd, char *buffer, int length);
	void (*accept_cb)(int fd, char *buffer, int length);

};
//每个eventblock对象都相当于一个块，块与块之间是通过next指针联系在一起，相当于一个链表关系
struct eventblock {
	struct sock_item *items; //这个对象相当于一个socket_item结构体指针数组，每一个指针数组里面有1024个元素
	struct eventblock *next;
};

struct reactor {
	int epfd; //epoll
	int blkcnt;

	struct eventblock *evblk; //当前块的地址
};

int reactor_resize(struct reactor *r) { // new eventblock 创建reactor内的块

	if (r == NULL) return -1;	

	struct eventblock *blk = r->evblk;
    
	while (blk != NULL && blk->next != NULL) {//这个while循环的目的是找到当前需要存放fd的块的地址
		blk = blk->next;
	}

	struct sock_item* item = (struct sock_item*)malloc(ITEM_LENGTH * sizeof(struct sock_item));
	if (item == NULL) return -4;
	memset(item, 0, ITEM_LENGTH * sizeof(struct sock_item));

	printf("-------------\n");
	struct eventblock *block = malloc(sizeof(struct eventblock));
	if (block == NULL) {
		free(item);
		return -5;
	}
	memset(block, 0, sizeof(struct eventblock));

	block->items = item;
	block->next = NULL;

	if (blk == NULL) {
		r->evblk = block;
	} else {
		blk->next = block;
	}
	r->blkcnt ++;

	return 0;
}


struct sock_item* reactor_lookup(struct reactor *r, int sockfd) {//这个函数的目的是找出对应套接字在reactor对象里面实际地址

	if (r == NULL) return NULL;
	//if (r->evblk == NULL) return NULL;
	if (sockfd <= 0) return NULL;

	printf("reactor_lookup --> %d\n", r->blkcnt); //64
	int blkidx = sockfd / ITEM_LENGTH;//计算出需要存放的是第几个快
	while (blkidx >= r->blkcnt) {//将已存在的快和需要存放块之间所有的块都malloc出来
		reactor_resize(r);
	}

	int i = 0;
	struct eventblock *blk = r->evblk;//
	while (i ++ < blkidx && blk != NULL) {//这一小段代码的目的是找出存放的块里面的要存放的地址
		blk = blk->next;
	}

	return  &blk->items[sockfd % ITEM_LENGTH];
}

// thread --> fd
void *routine(void *arg) {

	int clientfd = *(int *)arg;

	while (1) {
		
		unsigned char buffer[BUFFER_LENGTH] = {0};
		int ret = recv(clientfd, buffer, BUFFER_LENGTH, 0);
		if (ret == 0) {
			close(clientfd);
			break;
			
		}
		printf("buffer : %s, ret: %d\n", buffer, ret);

		ret = send(clientfd, buffer, ret, 0); // 

	}

}


int init_server(short port) {//初始化服务器套接字然后绑定设置监听

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);  // 
	if (listenfd == -1) return -1;
// listenfd
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if (-1 == bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) {
		return -2;
	}

#if 1 // nonblock将监听套件设置成非阻塞的
	int flag = fcntl(listenfd, F_GETFL, 0);
	flag |= O_NONBLOCK;
	fcntl(listenfd, F_SETFL, flag);
#endif

	listen(listenfd, 10);

	return listenfd;
}


int is_listenfd(int *fds, int connfd) {//该函数的作用是查找listenfd是否在监听的套接字之内，如果是的话就代表有客户端想要建立连接

	int i = 0;
	for (i = 0;i < PORT_COUNT;i ++) {
		if (fds[i] == connfd) {
			return 1;
		}
	}

	return 0;
}

// socket --> 
// bash --> execve("./server", "");
// 
// 0, 1, 2
// stdin, stdout, stderr
int main() {
// block
///  **********
	struct reactor *r = (struct reactor*)calloc(1, sizeof(struct reactor));//单例模式 先声明一个reactor的对象，后续都对于这一个对象进行操作
	if (r == NULL) {
		return -3;
	}
	//memset();

	r->epfd = epoll_create(1);
	struct epoll_event ev, events[EVENTS_LENGTH];

 	
	int sockfds[PORT_COUNT] = {0};
	int i = 0;
	for (i = 0;i < PORT_COUNT;i ++) {           //由于fd是由五元组确定的（源IP，目的IP，源端口，目的端口，协议）
		sockfds[i] = init_server(9999 + i);     //由于此时源IP和目的IP只有一个，协议使用的是TCP，源端口只有65535-1024个
                                                //要实现百万并发的话，当客户端很少时，只能改变目的端口的数量
	#if 0

		文件描述符（File Descriptor，通常用 fd 表示）的性质与五元组并无直接关系。文件描述符是操作系统内部用于标识已打开文件、
		套接字或其他 I/O 资源的整数值，它是操作系统的一个抽象表示。每个文件描述符唯一标识一个已打开的资源，但它通常不包含关于
		资源的具体信息，比如资源的来源、目标、端口等等。

		五元组（Five-Tuple）通常用于网络通信，特别是在网络层和传输层协议中。五元组包括源 IP 地址、目标 IP 地址、源端口号、目标端口号和协议类型，
		用于唯一标识网络连接。五元组用于区分不同的网络数据流，例如 TCP 连接或 UDP 连接。

		因此，文件描述符和五元组是两个不同的概念，它们在不同的上下文中使用。如果您需要理解文件描述符的性质，
		可以将其视为操作系统用于管理 I/O 资源的标识符，而不需要与网络连接的五元组相混淆。

		虽然文件描述符（File Descriptor）和五元组（Five-Tuple）是不同的概念，但它们在某些情况下可以有联系，特别是在网络编程中：

		网络套接字描述符：在网络编程中，通常会使用套接字（Socket）来进行网络通信。每个套接字也被分配一个文件描述符，这个文件描述符可以用来标识
		和管理套接字。当您使用 socket() 函数创建套接字时，操作系统会为套接字分配一个文件描述符。这个文件描述符用于进行套接字的读取、写入等操作。

		网络连接管理：五元组通常用于唯一标识网络连接，例如源 IP 地址、目标 IP 地址、源端口号、目标端口号和协议类型。当您建立网络连接（如 TCP 连接）时，
		连接的五元组信息会被记录和使用。这与文件描述符没有直接关系，但您可以通过文件描述符来管理和操作网络连接，例如使用 read() 和 write() 来处理数据传输。

		多路复用（Multiplexing）：在某些情况下，您可能会使用多路复用技术来同时管理多个文件描述符，包括网络套接字。多路复用技术允许您在一个事件循环中同时
		监听多个文件描述符的活动，并响应相应的事件，例如套接字上有可读数据到达。这种情况下，五元组信息可能用于确定哪个套接字产生了事件，以便在事件处理程序
		中正确处理它。
	#endif
		ev.events = EPOLLIN;
		ev.data.fd = sockfds[i]; //

		epoll_ctl(r->epfd, EPOLL_CTL_ADD, sockfds[i], &ev);
	}
	/// ************** //

	 // 
	

	while (1) { // 7 * 24 

		int nready = epoll_wait(r->epfd, events, EVENTS_LENGTH, -1); // -1, ms 
		//printf("------- %d\n", nready);
		
		int i = 0;
		for (i = 0;i < nready;i ++) {
			int clientfd = events[i].data.fd;
			
			if (is_listenfd(sockfds, clientfd)) { // accept

				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				int connfd = accept(clientfd, (struct sockaddr*)&client, &len);
				if (connfd == -1) break;
				
				if (connfd % 1000 == 999) {
				
					printf("accept: %d\n", connfd);

				}

#if 1 // nonblock
				int flag = fcntl(connfd, F_GETFL, 0);
				flag |= O_NONBLOCK;
				fcntl(connfd, F_SETFL, flag);
#endif
				//将accept的fd加入到epoll事件里面
				ev.events = EPOLLIN;
				ev.data.fd = connfd;
				epoll_ctl(r->epfd, EPOLL_CTL_ADD, connfd, &ev);
#if 0
				r->items[connfd].fd = connfd;
			
				r->items[connfd].rbuffer = calloc(1, BUFFER_LENGTH);
				r->items[connfd].rlength = 0;
				
				r->items[connfd].wbuffer = calloc(1, BUFFER_LENGTH);
				r->items[connfd].wlength = 0;

				r->items[connfd].event = EPOLLIN;
#else

				struct sock_item *item = reactor_lookup(r, connfd);//找出要存放的块里面具体的地址
				item->fd = connfd;
				item->rbuffer = calloc(1, BUFFER_LENGTH);
				item->rlength = 0;

				item->wbuffer = calloc(1, BUFFER_LENGTH);
				item->wlength = 0;
#endif				
			} else if (events[i].events & EPOLLIN) { //clientfd读事件

				//char rbuffer[BUFFER_LENGTH] = {0};

				struct sock_item *item = reactor_lookup(r, clientfd);

				char *rbuffer = item->rbuffer;
				char *wbuffer = item->wbuffer;
				
				int n = recv(clientfd, rbuffer, BUFFER_LENGTH, 0);
				if (n > 0) {//如果收到数据之后，将数据打印出来，然后将数据复制到写操作的字符串上，将epoll事件里面原来的读事件修改为写事件
					//rbuffer[n] = '\0';

					printf("recv: %s, n: %d\n", rbuffer, n);

					memcpy(wbuffer, rbuffer, BUFFER_LENGTH);

					ev.events = EPOLLOUT;
					ev.data.fd = clientfd;

					epoll_ctl(r->epfd, EPOLL_CTL_MOD, clientfd, &ev);
					
				} else if (n == 0) {

					free(rbuffer);
					free(wbuffer);
					
					item->fd = 0;

					close(clientfd);
				}
				
			} else if (events[i].events & EPOLLOUT) {//写操作

				
				struct sock_item *item = reactor_lookup(r, clientfd);//找出clientfd的具体地址
				char *wbuffer = item->wbuffer;
				
				int sent = send(clientfd, wbuffer, BUFFER_LENGTH, 0); //
				printf("sent: %d\n", sent);

				ev.events = EPOLLIN;
				ev.data.fd = clientfd;

				epoll_ctl(r->epfd, EPOLL_CTL_MOD, clientfd, &ev);//将epoll事件修改为写事件
				
				
			}

		}

	}
	

}




