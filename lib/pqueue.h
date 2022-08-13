#include <pthread.h>

#define PQUEUE_MAX_SIZE 10000
#define PQUEUE_NUM_THREAD 10000
#define __PQUEUE_DEFAULT_CONFIG_INITIALIZER() ({                \
        pqueue_args_t *config = malloc(sizeof(pqueue_args_t));  \
        config->max_size = PQUEUE_MAX_SIZE;                     \
        config->num_threads = PQUEUE_NUM_THREAD;                \
        config;                                                 \
    })
#define PQUEUE_DEFAULT_CONFIG __PQUEUE_DEFAULT_CONFIG_INITIALIZER()

typedef void *(*job_callback)(void *args);

typedef struct pqueue_job_t
{
    double id;
} pqueue_job_t;

typedef struct pqueue_t
{
    pthread_t id;
} pqueue_t;

typedef struct pqueue_args_t
{
    int max_size;
    int num_threads;
} pqueue_args_t;

pqueue_t *create_pqueue(pqueue_args_t *args);
pqueue_job_t *create_pqueue_job(job_callback callback, void *args);
void pqueue_push(pqueue_t *pqueue, pqueue_job_t *payload);
void *pqueue_await(pqueue_t *pqueue, pqueue_job_t *payload);
void pqueue_await_batch(pqueue_t *pqueue, pqueue_job_t *payload[], void *responses[], int batch_size);
void destroy_pqueue(pqueue_t *pqueue);
