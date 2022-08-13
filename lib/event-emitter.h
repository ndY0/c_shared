#include <pthread.h>
#define MAX_LISTENERS 10000
#define __EMITTER_DEFAULT_CONFIG_INITIALIZER() ({                   \
        emitter_config_t *config = malloc(sizeof(emitter_config_t));\
        config->max_listeners = MAX_LISTENERS;                      \
        config;                                                     \
    })

#define EMITTER_DEFAULT_CONFIG __EMITTER_DEFAULT_CONFIG_INITIALIZER()

typedef void *(*listener_t)(void *args);

typedef struct _listener_args
{
    void *args;
    void *event;
    pthread_mutex_t mutex;
} _listener_args;

typedef struct _listener_op_args
{
    void *event;
    int once;
    listener_t listener;
    pthread_mutex_t mutex;
} _listener_op_args;

typedef struct run_args_t
{
    int fd_read[2];
    int fd_write[2];
    _listener_args *listener_args;
    _listener_op_args *append_listener_args;
    _listener_op_args *remove_listener_args;
    int listeners_count;
    int max_listeners;
} run_args_t;

typedef struct emitter_t
{
    pthread_t id;
    // run_args_t *args;

} emitter_t;

typedef struct emitter_config_t
{
    int max_listeners;
} emitter_config_t;

emitter_t *create_emitter(emitter_config_t *config);
int destroy_emitter(emitter_t *emitter);
void *take(emitter_t *emitter, void *event);
int on(emitter_t *emitter, void *event, listener_t listener);
int once(emitter_t *emitter, void *event, listener_t listener);
void off(emitter_t *emitter, void *event, listener_t listener);
void emit(emitter_t *emitter, void *event, void *data);