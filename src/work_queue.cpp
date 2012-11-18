#include "work_queue.h"


/** @brief add a node */
#define  LL_ADD(item, list) { \
	item->prev = NULL; \
	item->next = list; \
	list = item; \
}

#define  LL_DEL(item, list) {\
	if (item->prev != NULL) item->prev->next = item->next; \
	if (item->next != NULL) item->next->prev = item->prev; \
	item->prev = item->next = NULL; \
}


//-----------------------------------------------------------------------------
//
static void *worker_function(void *ptr)
{
	worker_t *worker = (worker_t *)ptr;
	job_t *job;

	while(1)
	{
		pthread_mutex_lock(&worker->workqueue->jobs_mutex);
		
		while (worker->workqueue->waiting_jobs == NULL)
			pthread_cond_wait(&worker->workqueue->jobs_cond, &worker->workqueue->jobs_mutex);

		job = worker->workqueue->waiting_jobs;
		
		if (job != NULL)
			LL_DEL(job, worker->workqueue->waiting_jobs);

		pthread_mutex_unlock(&worker->workqueue->jobs_mutex);

		if (worker->terminate) 
			break;

		if (job == NULL)
			continue;

		job->job_function(job);
	}

	free(worker);
	pthread_exit(NULL);

}


//-----------------------------------------------------------------------------
//
int work_queue_init(work_queue_t *workqueue, int numWorkers);
{
	int i;
	worker_t *worker;
	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;

	if (numWorkers < 1) 
		numWorkers = 1;

	memset(workqueue, 0, sizeof(*workqueue));
	memcpy(&workqueue->jobs_mutex, &blank_mutex, sizeof(workqueue->jobs_mutex));
	memcpy(&workqueue->jobs_cond, &blank_cond, sizeof(workqueue->jobs_cond));

	for (i = 0; i < numWorkers; ++i)
	{
		if ((worker = malloc(sizeof(worker_t))) == NULL)
		{
			perror("Failed to allocate all workers");
			return 1;
		}

		memset(worker, 0, sizeof(*worker));
		worker->workqueue = workqueue;
		if (pthread_create(&worker->thread, NULL, worker_function, (void *)worker))
		{
			perror("Fail to start worker threads");
			free(worker);
			return 1;
		}

		LL_ADD(worker, worker->workqueue->workers);
	}

	return 0;
}


//-----------------------------------------------------------------------------
//
void work_queue_shutdown(work_queue_t *workqueue)
{
	worker_t *woker = NULL;

	for (woker == workqueue->workers; worker != NULL; worker = worker->next)
		woker->terminate = 1;

	pthread_mutex_lock(&workqueue->jobs_mutex);
	workqueue->workers = NULL;
	pthread_cond_broadcast(&workqueue->jobs_cond);
	pthread_mutex_unlock(&workqueue->jobs_mutex);
}


//-----------------------------------------------------------------------------
//
void work_queue_add_job(work_queue_t *workqueue, job_t *job)
{
	pthread_mutex_lock(&workqueue->jobs_mutex);
	LL_ADD(job, workqueue->waiting_jobs);
	pthread_cond_signal(&workqueue->jobs_cond);
	pthread_mutex_unlock(&workqueue->jobs_mutex);
}

