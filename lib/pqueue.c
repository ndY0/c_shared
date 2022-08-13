#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include "event-emitter.h"

char event_available[] = "available";
char event_enqueued[] = "enqueued";
char event_drain[] = "drain";

typedef void *(*job_callback)(void *args);

typedef struct pqueue_job_t
{
    double id;
    int answer;
    void *args;
    job_callback callback;
} pqueue_job_t;

typedef struct pqueue_runner_t
{
    int state;
    pthread_t id;
} pqueue_runner_t;

typedef struct pqueue_t
{
    queue_t *pending;
    emitter_t *emitter;
    int runner_count;
    pqueue_runner_t *runners[];
} pqueue_t;

typedef struct pqueue_args_t
{
    int max_size;
    int num_threads;
} pqueue_args_t;

typedef struct run_job_args_t
{
    pqueue_t *queue;
    emitter_t *emitter;
    pqueue_job_t *job;
    pqueue_runner_t *runner;
} run_job_args_t;

typedef struct pqueue_await_publish_args_t
{
    pqueue_t *pqueue;
    pqueue_job_t *payload;
} pqueue_await_publish_args_t;

typedef struct pqueue_await_await_args_t
{
    pqueue_t *pqueue;
    pqueue_job_t *payload;
    void *response;
} pqueue_await_await_args_t;

void *_run_job(run_job_args_t *args)
{
    job_callback callback = args->job->callback;
    void *result = callback(args->job->args);
    if(args->job->answer)
    {
        emit(args->emitter, &args->job->id, result);
    }
    free(args->job);
    args->runner->state = 1;
    emit(args->emitter, event_available, args->queue);
    free(args);
    return NULL;
}

void *_run_pqueue(pqueue_t *queue)
{
    int found = 0;
    int i = 0;
    for (i = 0; i < queue->runner_count; i++)
    {
        if (!found && queue->runners[i]->state)
        {
            found = 1;
            break;
        }
    }
    if (found)
    {
        pqueue_job_t *job = queue_shift(queue->pending);
        if(job)
        {
            emit(queue->emitter, event_drain, NULL);
            queue->runners[i]->state = 0;
            run_job_args_t *args = malloc(sizeof(run_job_args_t));
            args->queue = queue;
            args->emitter = queue->emitter;
            args->job = job;
            args->runner = queue->runners[i];
            pthread_t runner_id;
            pthread_create(&runner_id, NULL, &_run_job, args);
            pthread_detach(runner_id);
        }
    }

    return NULL;
}

pqueue_t *create_pqueue(pqueue_args_t *args)
{
    srand(time(NULL));
    pqueue_t *queue = malloc(sizeof(pqueue_t) + args->num_threads * sizeof(pqueue_runner_t *));
    queue->emitter = create_emitter(EMITTER_DEFAULT_CONFIG);
    queue->runner_count = args->num_threads;
    queue_args_t *queue_create_args = malloc(sizeof(queue_args_t));
    queue_create_args->max_size = args->max_size;
    queue->pending = create_queue(queue_create_args);
    free(queue_create_args);
    int runner_index = 0;
    for (runner_index = 0; runner_index < args->num_threads; runner_index++)
    {
        pqueue_runner_t *runner = malloc(sizeof(pqueue_runner_t));
        runner->state = 1;
        queue->runners[runner_index] = runner;
    }
    on(queue->emitter, event_enqueued, &_run_pqueue);
    on(queue->emitter, event_available, &_run_pqueue);
    return queue;
}
pqueue_job_t *create_pqueue_job(job_callback callback, void *args)
{
    double id = round(rand() / RAND_MAX * pow((double)10, (double)37));
    pqueue_job_t *job = malloc(sizeof(pqueue_job_t));
    job->args = args;
    job->callback = callback;
    job->id = id;
    job->answer = 0;
    return job;
}
void pqueue_push(pqueue_t *pqueue, pqueue_job_t *payload)
{
    int pushed = 0;
    pushed = queue_push(pqueue->pending, payload);
    if(!pushed)
    {
        do
        {
            take(pqueue->emitter, event_drain);
            pushed = queue_push(pqueue->pending, payload);
        } while (!pushed);
    }
    emit(pqueue->emitter, event_enqueued, pqueue);
}

void *_await(pqueue_await_await_args_t *arg)
{
    arg->response = take(arg->pqueue->emitter, &arg->payload->id);
    return NULL;
};

void *pqueue_await(pqueue_t *pqueue, pqueue_job_t *payload)
{
    void * response;
    pthread_t await_thread_id;
    pqueue_await_await_args_t *await_args = malloc(sizeof(pqueue_await_await_args_t));
    payload->answer = 1;
    await_args->pqueue = pqueue;
    await_args->payload = payload;
    pthread_create(&await_thread_id, NULL, &_await, await_args);
    pqueue_push(pqueue, payload);
    pthread_join(await_thread_id, NULL);
    response = await_args->response;
    free(await_args);
    return response;
}

void pqueue_await_batch(pqueue_t *pqueue, pqueue_job_t *payload[], void *responses[], int batch_size)
{
    pthread_t await_threads_id[batch_size];
    pqueue_await_await_args_t *await_args[batch_size];
    int i = 0;
    for (i = 0; i < batch_size; i++)
    {
        payload[i]->answer = 1;
        pqueue_await_await_args_t *await_arg = malloc(sizeof(pqueue_await_await_args_t));
        await_arg->pqueue = pqueue;
        await_arg->payload = payload[i];
        await_args[i] = await_arg;
        pthread_create(&await_threads_id[i], NULL, &_await, await_arg);
        pqueue_push(pqueue, payload[i]);
    }
    for (i = 0; i < batch_size; i++)
    {
        pthread_join(await_threads_id[i], NULL);
        if(responses)
        {
            responses[i] = await_args[i]->response;
        }
        free(await_args[i]);
    }
    
}
void destroy_pqueue(pqueue_t *pqueue)
{
    off(pqueue->emitter, event_enqueued, &_run_pqueue);
    off(pqueue->emitter, event_available, &_run_pqueue);
    destroy_emitter(pqueue->emitter);
    pqueue_job_t *job = queue_shift(pqueue->pending);
    while (job)
    {
        if(job) {
            free(job);
        }
        job = queue_shift(pqueue->pending);
    }
    destroy_queue(pqueue->pending);
    int i = 0;
    for (i = 0; i < pqueue->runner_count; i++)
    {
        free(pqueue->runners[i]);
    }
    free(pqueue);
}