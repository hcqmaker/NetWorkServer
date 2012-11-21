#ifndef __SMSERVER_H__
#define __SMSERVER_H__


/************************************************************************/
/** @file smserver.h

1. I need a threadPool to save all thread
2. I need a queue to 


*/
/************************************************************************/


#include <sys/types.h>
#include <sys/stat.h>


#include <stdio.h>
#include <stdlib.h>

#include <event2/event-config.h>
#include <event2/event.h>
#include <event2/bufferevent.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>


#include "work_queue.h"


#if defined(_MSC_VER) && defined(WIN32)

#pragma comment(lib, "libevent.lib")
#pragma comment(lib, "libevent_core.lib")
#pragma comment(lib, "libevent_extras.lib")
// use pthread for win32
#pragma comment(lib, "pthreadVC2.lib")

#endif

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

static void signalhandler(int signal);

static int setnonbolck(int fd);

static void closeClient(client_t *client);
static void coloseAndFreeClient(client_t *client);

//
void buffered_on_read(struct bufferevent *bev, void *arg);
void buffered_on_write(struct bufferevent *bev, void *arg);

void buffered_on_error(struct bufferevent *bev, short what, void *arg);

static void server_job_function(struct job *job);
//void on_accept(int fd, short ev, void *arg);
static void signal_cb(evutil_socket_t sig, short events, void *user_data);
static void on_accept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data);

int runServer(int port, int numthread);
void killSever(void);

#if defined(WIN32) && defined(_MSC_VER)
void close(SOCKET fd);
#endif



#endif // __SMSERVER_H__