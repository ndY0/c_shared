#include <stdlib.h>

typedef struct queue_t {
    int size;
    int max_size;
    void *list[];
} queue_t;

typedef struct queue_args_t
{
    int max_size;
} queue_args_t;

queue_t *create_queue(queue_args_t *args)
{
    queue_t *queue = malloc(sizeof(queue_t) + args->max_size * sizeof(void *));
    queue->size = 0;
    queue->max_size = args->max_size;
    return queue;
}
void *queue_shift(queue_t *queue)
{
    if(queue->size > 0)
    {
        void *elem = queue->list[0];
        int i = 1;
        for (i = 1; i < queue->size; i++)
        {
            queue->list[i - 1] = queue->list[i];
        }
        queue->size -= 1;
        return elem;
    }
    return NULL;
}
int queue_push(queue_t *queue, void *elem)
{   
    if(queue->max_size > queue->size)
    {
        queue->list[queue->size] = elem;
        queue->size += 1;
        return 1;
    }
    return 0;
}
void destroy_queue(queue_t *queue)
{
    free(queue);
}