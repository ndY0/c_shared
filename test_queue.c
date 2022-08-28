#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "./lib/queue.h"
#include "./lib/pqueue.h"

typedef struct test
{
    int val;
} test;

int job_done = 0;


void *worker2(void * args)
{
    usleep(207930);
    printf("waited in worker 2 !\n");
    if(args) {
        printf("jobs done : %d\n", ((test *)args)->val);
    } else {
        job_done += 1;
        printf("jobs done : %d\n", job_done);
    }
    return  args;
}

void *worker1(void * args)
{
    usleep(230841);
    printf("waited in worker 1 !\n");
    if(args) {
        printf("jobs done : %d\n", ((test *)args)->val);
    } else {
        job_done += 1;
        printf("jobs done : %d\n", job_done);
    }
    return args;
}

int main(int argc, char *argv[])
{
    // queue_args_t *args = malloc(sizeof(queue_args_t));
    // args->max_size = 100;
    // queue_t *queue = create_queue(args);
    // test *values[200];
    // for (int i = 0; i < 200; i++)
    // {
    //     test *val = malloc(sizeof(test));
    //     val->val = i;
    //     values[i] = val;
    // }
    // for (int i = 0; i < 200; i++)
    // {
    //     queue_push(queue, values[i]);
    // }
    // for (int i = 0; i < 200; i++)
    // {
    //     test *retrieved = queue_shift(queue);
    //     if(retrieved)
    //     {
    //         printf("retrieved : %d\n", retrieved->val);
    //         free(retrieved);
    //     }
    // }
    // destroy_queue(queue);

    pqueue_args_t *config = malloc(sizeof(pqueue_args_t));
    config->max_size = 100;
    config->num_threads = 10;
    pqueue_t *pqueue = create_pqueue(config);
    free(config);
    int i = 0;
    // test fire and forget
    for (i = 0; i < 100; i++)
    {
         usleep(10000);
         pqueue_job_t *job1 = create_pqueue_job(&worker1, NULL);
         pqueue_job_t *job2 = create_pqueue_job(&worker2, NULL);
         printf("pushed jobs : %d\n", 2 * (i + 1));
         pqueue_push(pqueue, job1);
         pqueue_push(pqueue, job2);
     }
     sleep(5);

     // test await
     // int i = 0;
     for (i = 0; i < 100; i++)
     {
         usleep(10000);
         test *arg1 = malloc(sizeof(test));
         arg1->val = 2 * (i) + 1;
         test *arg2 = malloc(sizeof(test));
         arg2->val = 2 * (i + 1);
         pqueue_job_t *job1 = create_pqueue_job(&worker1, arg1);
         pqueue_job_t *job2 = create_pqueue_job(&worker2, arg2);
         printf("pushed jobs : %d\n", 2 * (i + 1));
         test *returned_arg1 = (test *)pqueue_await(pqueue, job1);
         test *returned_arg2 = (test *)pqueue_await(pqueue, job2);
	 printf("return 1 addr : %p\n", returned_arg1);
	 printf("return 2 addr : %p\n", returned_arg2);
	 printf("return 2 val : %d\n", returned_arg2->val);
	 printf("return 1 val : %d\n", returned_arg1->val);
         if(returned_arg1)
         {
             printf("returned arg1 : %d\n", returned_arg1->val);
             free(returned_arg1);
         }
         if(returned_arg2)
         {
             printf("returned arg2 : %d\n", returned_arg2->val);
             free(returned_arg2);
         }
     }
     sleep(5);

    //test await batch
    // int i = 0;
    int batch_size = 10;
    test *args[batch_size];
    pqueue_job_t *jobs[batch_size];
    for (i = 0; i < 100; i++)
    {
        // /!\ WARNING : bug, if allow waiting, no task => crash ?
        usleep(200000);
        int j = 0;
        for (j = 0; j < batch_size; j++)
        {
            test *arg = malloc(sizeof(test));
            arg->val = batch_size * (i) + j + 1;
            args[j] = arg;
            pqueue_job_t *job;
            if(j % 2) {
                job = create_pqueue_job(&worker2, arg);    
            } else {
                job = create_pqueue_job(&worker1, arg);
            }
            jobs[j] = job;
        }
        printf("pushed jobs : %d\n", batch_size * (i + 1));
        test *answers[batch_size];
        pqueue_await_batch(pqueue, jobs, answers, batch_size);
        for (j = 0; j < batch_size; j++)
        {
            if(answers[j])
            {
                printf("returned arg at index %d : %d\n", j, answers[j]->val);
                free(answers[j]);
            }
        }
    }
    sleep(5);

    destroy_pqueue(pqueue);

    sleep(1);
    return 0;
}
