#define QUEUE_MAX_SIZE 10000
#define __QUEUE_DEFAULT_CONFIG_INITIALIZER() ({             \
        queue_args_t *config = malloc(sizeof(queue_args_t));\
        config->max_size = QUEUE_MAX_SIZE;                  \
        config;                                             \
    })

#define QUEUE_DEFAULT_CONFIG __QUEUE_DEFAULT_CONFIG_INITIALIZER()

typedef struct queue_t {
} queue_t;

typedef struct queue_args_t
{
    int max_size;
} queue_args_t;

queue_t *create_queue(queue_args_t *args);
void *queue_shift(queue_t *queue);
int queue_push(queue_t *queue, void *elem);
void destroy_queue(queue_t *queue);