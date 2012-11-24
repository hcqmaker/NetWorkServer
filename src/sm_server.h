#ifndef __SMSERVER_H__
#define __SMSERVER_H__


/************************************************************************/
/** @file smserver.h

1. I need a threadPool to save all thread
2. I need a queue to 


*/
/************************************************************************/

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include<arpa/inet.h>
# endif
#include <sys/socket.h>
#else
#include <WinSock2.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include <event2/bufferevent_struct.h>

#include "work_queue.h"


#define  errorOut(...) {\
	fprintf(stderr, "%s:%d: %s:\t", __FILE__, __LINE__, __FUNCTION__); \
	fprintf(stderr, __VA_ARGS__); \
};

struct client 
{
	int fd;
	struct event_base *evbase;
	struct bufferevent *evbuff;
	struct evbuffer *output_buff;
};

typedef client client_t;

static struct event_base *evbase_accept;
static work_queue_t workqueue;

static void closeClient(client_t *client);
static void coloseAndFreeClient(client_t *client);

static void signal_cb(evutil_socket_t sig, short events, void *user_data);
static void server_job_function(struct job *job);

// server 
void buffered_on_read(struct bufferevent *bev, void *arg);
void buffered_on_write(struct bufferevent *bev, void *arg);
void buffered_on_event(struct bufferevent *bev, short what, void *arg);

void on_accept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data);

int runServer(int port, int numthread);
void killSever(void);

#if defined(WIN32) && defined(_MSC_VER)
void close(SOCKET fd);
#endif



#endif // __SMSERVER_H__