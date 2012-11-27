#include "sm_server.h"

#define  MAX_SIZE 4096

/**
这里的做法是根据原来的方式进行其中可能在新的机制中会出问题
方案1：
	使用原始的源代码方式进行
	在libevent的 on_accept, bufferevent_on_read, bufferevent_on_read, bufferevent_on_error,
	中对接受的数据进行处理，这里使用work_queue来进行

方案2：


*/


#define  WAY_TYPE 1

#define WAY_TYPE_1 1
#define WAY_TYPE_2 2


#if WAY_TYPE == WAY_TYPE_1
//---------------------------------------------------------------------
//
static void closeClient(client_t *client)
{
	if (client != NULL)
	{
		if (client->fd >= 0)
			client->fd = -1;
	}
}

//---------------------------------------------------------------------
//
static void coloseAndFreeClient(client_t *client)
{
	if (client != NULL)
	{
		// 这里要注意的是libevent在关闭的时候会自动去清理他需要清理的部分
		// 所以
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

//---------------------------------------------------------------------
//

static void signal_cb(evutil_socket_t sig, short events, void *user_data)
{

	killSever();
	struct event_base *base = (event_base *)user_data;
	struct timeval delay = { 2, 0};
	printf("Caught an interrupt signal; exiting cleanly in two secondes \n");
	event_base_loopexit(base, &delay);
}

//---------------------------------------------------------------------
// 这里是一个死循环其实就是一个线程
static void server_job_function(struct job *job)
{
	client_t *client = (client_t *)job->user_data;
	event_base_dispatch(client->evbase);
	coloseAndFreeClient(client);
	free(job);
}

//---------------------------------------------------------------------
//

// server 
void buffered_on_read(struct bufferevent *bev, void *arg)
{
	client_t *client = (client_t *)arg;
	char data[MAX_SIZE];
	int nbytes;

	while (1)
	{
		int lenth = evbuffer_get_length(bev->input);
		if (lenth <= 0)
			break;

		nbytes = (lenth > MAX_SIZE) ? MAX_SIZE:lenth;
		evbuffer_remove(bev->input, data, nbytes);
		evbuffer_add(client->output_buff, data, nbytes);
	}

	if (bufferevent_write_buffer(bev, client->output_buff))
	{
		fprintf(stderr, "error sending to client on fd %d  \n", client->fd);
		closeClient(client);
	}
}

//---------------------------------------------------------------------
//
void buffered_on_write(struct bufferevent *bev, void *arg)
{

}

//---------------------------------------------------------------------
//
void buffered_on_event(struct bufferevent *bev, short events, void *arg)
{

	if (events & BEV_EVENT_EOF) {
		fprintf(stdout, "Connection closed in event: %d  \n", events);
	} else if (events & BEV_EVENT_ERROR) {
		printf("Got an error on the connection: %s\n",
		    strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	//bufferevent_free(bev);

	closeClient((client_t *)arg);
}

//---------------------------------------------------------------------
//

void on_accept(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	fprintf(stdout, "something connect fd:%d  \n", fd);
	struct sockaddr_in client_addr;
	int client_len = sizeof(client_addr);
	work_queue_t *workqueue = (work_queue_t *)user_data;
	client_t *client;
	job_t *job;

	if (evutil_make_socket_nonblocking(fd) < 0)
	{
		fprintf(stderr, "failed to set client socket to non-blocking :%d  \n", fd);
		evutil_make_socket_closeonexec(fd);
		return;
	}

	if ((client = (client_t *)malloc(sizeof(*client))) == NULL)
	{
		fprintf(stderr, "fail to allocate memory for client state :%d  \n", fd);
		evutil_make_socket_closeonexec(fd);
		return;
	}

	memset(client, 0, sizeof(*client));
	client->fd = fd;

	if ((client->output_buff = evbuffer_new()) == NULL)
	{
		fprintf(stderr, "client output buffer allocation failed \n");
		coloseAndFreeClient(client);
		return;
	}

	if ((client->evbase = event_base_new()) == NULL)
	{
		fprintf(stderr, "client event_base creation failed \n");
		coloseAndFreeClient(client);
		return;
	}

	if ((client->evbuff = bufferevent_socket_new(client->evbase, fd, BEV_OPT_CLOSE_ON_FREE)) == NULL)
	{
		fprintf(stderr, "client bufferevent creation failed fd:%d\n", fd);
		event_base_loopbreak(client->evbase);
		coloseAndFreeClient(client);
		return;
	}

	bufferevent_setcb(client->evbuff, buffered_on_read, buffered_on_write, buffered_on_event, client);
	bufferevent_base_set(client->evbase, client->evbuff);
	bufferevent_enable(client->evbuff, EV_READ);
	//bufferevent_enable(client->evbuff, EV_WRITE);


	if ((job = (job_t *)malloc(sizeof(*job))) == NULL)
	{
		fprintf(stderr, "failed to allocate memory for job state fd:%d\n", fd);
		coloseAndFreeClient(client);
		return;
	}

	job->job_function = server_job_function;
	job->user_data = client;

	work_queue_add_job(workqueue, job);
}

//---------------------------------------------------------------------
//

int runServer(int port, int numthread)
{

	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;
	struct sockaddr_in sin;

#ifdef WIN32
	WSAData wsa_data;
	WSAStartup(0x0201,&wsa_data);
#endif

	base = event_base_new();
	if (!base)
	{
		fprintf(stderr, "could not initializ libevent \n");
		return 1;
	}


	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	if (work_queue_init(&workqueue, numthread))
	{
		fprintf(stderr, "Failed to create work queue \n");
		work_queue_shutdown(&workqueue);
		return 1;
	}
	
	listener = evconnlistener_new_bind(base, on_accept, (void*)&workqueue,
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr *)&sin, sizeof(sin));

	if (!listener)
	{
		fprintf(stderr, "could not initializ libevent \n");
		return 1;
	}


	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
	if (!signal_event || event_add(signal_event, NULL) < 0)
	{
		fprintf(stderr, "could not created/add a signal event \n");
		return 1;
	}

	event_base_dispatch(base);

	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

#ifdef WIN32
	WSACleanup();
#endif
	printf("done \n");
	return 0;
}

//---------------------------------------------------------------------
//
void killSever(void)
{
	fprintf(stdout, "Stopping socket listener event loop.\n");
	if (event_base_loopexit(evbase_accept, NULL)) {
		perror("Error shutting down server");
	}
	fprintf(stdout, "Stopping workers.\n");
	work_queue_shutdown(&workqueue);
}

#endif // 1