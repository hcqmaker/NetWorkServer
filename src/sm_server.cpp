#include "sm_server.h"

#define MAX_BUFF 4096

//----------------------------------------------------------------
//
void signalhandler(int signal)
{
	fprintf(stdout, "Receive singnal %d: %s. shutting down. \n", signal, strsignal(signal));
	killSever();
}

//----------------------------------------------------------------
//
int setnonbolck(int fd)
{
	int flags = fcntl(fd, F_GETFL);

	if (flags < 0) 
		return flags;

	flags |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, flags) < 0)
		return -1;
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

	while (bev->input->off > 0)
	{
		nbytes = (bev->input->off > MAX_BUFF) ? MAX_BUFF : bev->input->off;
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

//----------------------------------------------------------------
//
void on_accept(int fd, sort ev, void *arg)
{

}

//----------------------------------------------------------------
//

void runServer(void)
{
	int listenfd;
	struct sockaddr_in listen_addr;
	struct event ev_accept;
	int reuseaddr_on;

	sigset_t sigset;
	sigemptyset(&sigset);

	struct sigaction siginfo = {
		.sa_handler = signalhandler,
		.sa_mask = sigset,
		.sa_flags = SA_RESTART,
	};

	sigaction(SIGINT, &siginfo, NULL);
	sigaction(SIGTERM, &siginfo, NULL);



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
	sprintf(stdout, "Stopping workers. \n");
	work_queue_shutdown(&workqueue);
}

//----------------------------------------------------------------
//