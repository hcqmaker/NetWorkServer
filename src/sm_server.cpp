#include "sm_server.h"
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>

#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#define MAX_BUFF 4096
#define  CONNECTION_BACKLOG 100
//----------------------------------------------------------------
//
void signalhandler(int signal)
{
	//fprintf(stdout, "Receive singnal %d: %s. shutting down. \n", signal, strsignal(signal));
	fprintf(stdout, "Receive singnal %d: . shutting down. \n", signal);
	killSever();
}

//----------------------------------------------------------------
//
int setnonbolck(int fd)
{
#if defined(__GUNC__)

	int flags = fcntl(fd, F_GETFL);

	if (flags < 0) 
		return flags;

	flags |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, flags) < 0)
		return -1;
#else
	unsigned long ul = 1;
	int ret = ioctlsocket(fd, FIONBIO, (unsigned long*)&ul);
	if (ret == SOCKET_ERROR)
	{
		return -1;
	}
#endif
}

//----------------------------------------------------------------
//
void closeClient(client_t *client)
{
	if (client != NULL)
	{
		if (client->fd >= 0)
			client->fd = -1;
	}
}

//----------------------------------------------------------------
//
void coloseAndFreeClient(client_t *client)
{
	if (client != NULL)
	{
		closeClient(client);
		if (client->evbuff != NULL)
		{
			bufferevent_free(client->evbuff);
			client->evbuff = NULL;
		}

		if (client->evbase != NULL)
		{
			event_base_free(client->evbase);
			client->evbase = NULL;
		}

		if (client->output_buff != NULL)
		{
			evbuffer_free(client->output_buff);
			client->output_buff = NULL;
		}
		free(client);
	}
}

//----------------------------------------------------------------
//
void buffered_on_read(struct bufferevent *bev, void *arg)
{
	client_t *client = (client_t *)arg;
	char data[MAX_BUFF];
	int nbytes;

	
	while (1)
	{
		int length = evbuffer_get_length(bev->input);
		if (length <= 0)
			break;

		nbytes = (length > MAX_BUFF) ? MAX_BUFF : length;
		evbuffer_remove(bev->input, data, nbytes);
		evbuffer_add(client->output_buff, data, nbytes);
	}

	if (bufferevent_write_buffer(bev, client->output_buff))
	{
		errorOut("Error sending to client on fd %d", client->fd);
		closeClient(client);
	}
}

//----------------------------------------------------------------
//
void buffered_on_write(struct bufferevent *bev, void *arg)
{

}

//----------------------------------------------------------------
//

void buffered_on_error(struct bufferevent *bev, short what, void *arg)
{
	errorOut("error happen in fd:  %", what);
	closeClient((client_t*) arg);
}

//----------------------------------------------------------------
//

void server_job_function(struct job *job)
{
	client_t *client = (client_t *)job->user_data;

	event_base_dispatch(client->evbase);
	coloseAndFreeClient(client);
	free(job);
}

static int verbose = 0;
static void logfn(int is_warn, const char *msg) {
	if (!is_warn && !verbose)
		return;
	fprintf(stderr, "%s: %s\n", is_warn?"WARN":"INFO", msg);
}

//----------------------------------------------------------------
//
void on_accept(int fd, short ev, void *arg)
{
	//int client_fd;
	struct sockaddr_in ca;
	int client_len = sizeof(ca);
	work_queue_t *workqueue = (work_queue_t *)arg;

}

//----------------------------------------------------------------
//

int runServer(int port, int numthread)
{
	int listenfd;
	struct sockaddr_in listen_addr;
	struct event ev_accept;
	int reuseaddr_on;

	/*
	sigset_t sigset;
	sigemptyset(&sigset);

	struct sigaction siginfo = {
		.sa_handler = signalhandler,
		.sa_mask = sigset,
		.sa_flags = SA_RESTART,
	};

	sigaction(SIGINT, &siginfo, NULL);
	sigaction(SIGTERM, &siginfo, NULL);

	
	
#ifdef WIN32
	{
		WSADATA WSAData;
		WSAStartup(0x101, &WSAData);
	}
#endif

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
	{
		errorOut("listen failed");
		return 1;
	}

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = INADDR_ANY;
	listen_addr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0)
	{
		errorOut("bind failed");
		return 1;
	}

	if (listen(listenfd, CONNECTION_BACKLOG) < 0)
	{
		errorOut("bind failed");
		return 1;
	}
	
	reuseaddr_on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseaddr_on, sizeof(reuseaddr_on));

	if (setnonbolck(listenfd) < 0)
	{
		errorOut("bind failed");
		return 1;
	}

	if ((evbase_accept = event_base_new()) == NULL)
	{
		perror("Unable to create socket accept event base");
		close(listenfd);
		return 1;
	}

	if (work_queue_init(&workqueue, numthread))
	{
		perror("Fail to create work queue");
		close(listenfd);
		work_queue_shutdown(&workqueue);
		return 1;
	}

	event_set(&ev_accept, listenfd, EV_READ | EV_PERSIST, on_accept, (void *)&workqueue);
	event_base_set(evbase_accept, &ev_accept);
	event_add(&ev_accept, NULL);

	printf("Server shutdown .\n");
	return 0;
	*/


	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;
	struct sockaddr_in sin;

#ifdef WIN32
	{
		WSADATA WSAData;
		WSAStartup(0x101, &WSAData);
	}
#endif

	base = event_base_new();
	if (!base)
	{
		fprintf(stderr, "Could not initialize libevent !\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (work_queue_init(&workqueue, numthread))
	{
		perror("Fail to create work queue");
		close(listenfd);
		work_queue_shutdown(&workqueue);
		return 1;
	}


	listener = evconnlistener_new_bind(base, on_accept, (void *)&workqueue, 
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin, sizeof(sin));
	if (listener)
	{
		fprintf(stderr, "Could not create a listener !\n");
		return 1;
	}

	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)&workqueue);
	if (!signal_event || event_add(signal_event, NULL) < 0)
	{
		fprintf(stderr, "Could not create/add signal event !\n");
		return 1;
	}

	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	printf("done \n");
	return 0;
}

#if defined(WIN32) && defined(_MSC_VER)
void close(SOCKET fd)
{
	closesocket(fd);
}
#endif

static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	//fprintf(stdout, "Received signal %d: %s.  Shutting down.\n", signal, strsignal(signal));
	fprintf(stdout, "Received signal %d: .  Shutting down.\n", sig);
	killSever();
}

static void on_accept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	int client_fd;
	struct sockaddr_in ca;
	int client_len = sizeof(ca);
	work_queue_t *workqueue = (work_queue_t *)user_data;
}


//----------------------------------------------------------------
//
void killSever(void)
{
	fprintf(stdout, "Stopping socket listener event loop. \n");
	if (event_base_loopexit(evbase_accept, NULL))
	{
		perror("Error shutting down server");
	}
	
	printf("Stopping workers. \n");
	//sprintf(stdout, "Stopping workers. \n");
	work_queue_shutdown(&workqueue);
}
