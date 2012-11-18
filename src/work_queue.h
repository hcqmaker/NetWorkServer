/*
-----------------------------------------------------------------------------
This source file is part of smserver
    (https://code.google.com/p/smserver/)

from: hcqmaker
-----------------------------------------------------------------------------
*/

#ifndef __WORK_QUEUE_H__
#define __WORK_QUEUE_H__


// in window you can use pthread for win
// download from ftp://sourceware.org/pub/pthreads-win32

#include <pthread.h>

struct worker 
{
	pthread_t thread;
	int terminate;
	struct work_queue *workqueue;
	struct worker *prev;
	struct worker *next;
};

struct job 
{
	void (*job_function)(struct job *job);
	void *user_data;
	struct job *prev;
	struct job *next;
};

struct work_queue 
{
	struct worker *workers;
	struct job *waiting_jobs;
	pthread_mutex_t jobs_mutex;
	pthread_cond_t jobs_cond;
};



typedef struct worker worker_t;
typedef struct job job_t;
typedef struct work_queue work_queue_t;

int work_queue_init(work_queue_t *workqueue, int numWorkers);
void work_queue_shutdown(work_queue_t *workqueue);
void work_queue_add_job(work_queue_t *workqueue, job_t *job);

#endif // __WORK_QUEUE_H__
