#include "llist.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

typedef void *(*listener_t)(void *args);

typedef struct event_t
{
    void *event;
    void *return_val;
    void *arg;
    int fd;
    listener_t listener;
} event_t;

typedef struct _listener_entry
{
    event_t *event;
    listener_t listener;
    int once;
} _listener_entry;

typedef struct _event_entry
{
    int id;
    LinkedList *listeners;
} _event_entry;

struct _listener_args_s
{
    void *args;
    void *event;
    pthread_mutex_t mutex;
} _listener_args_default = {.mutex = PTHREAD_MUTEX_INITIALIZER};
typedef struct _listener_args_s _listener_args;

struct _listener_op_args_s
{
    event_t *event;
    int once;
    listener_t listener;
    pthread_mutex_t mutex;
} _listener_op_args_default = {.mutex = PTHREAD_MUTEX_INITIALIZER};
typedef struct _listener_op_args_s _listener_op_args;

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
    run_args_t *args;
    int listeners_count;

} emitter_t;

typedef struct emitter_config_t
{
    int max_listeners;
} emitter_config_t;

typedef struct find_event_ctx_t
{
    int event;
} find_event_ctx_t;

typedef struct listener_remove_ctx_t
{
    listener_t listener;
    LinkedList *listeners;
    run_args_t *args;

} listener_remove_ctx_t;

typedef struct listener_emit_ctx_t
{
    void *arg;
    LinkedList *listeners;
    run_args_t *args;
} listener_emit_ctx_t;

run_args_t *_init_run_args(int max_listeners)
{
    run_args_t *args = malloc(sizeof(run_args_t));
    pipe(args->fd_read);
    pipe(args->fd_write);
    args->listener_args = malloc(sizeof(_listener_args));
    args->append_listener_args = malloc(sizeof(_listener_op_args));
    args->remove_listener_args = malloc(sizeof(_listener_op_args));
    args->listener_args->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    args->append_listener_args->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    args->remove_listener_args->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    args->listeners_count = 0;
    args->max_listeners = max_listeners;
    return args;
}

void _foreach_listener_destroy(int pos, _listener_entry *entry, void *context)
{
    if (entry)
    {
	if(entry->event)
	{
	    free(entry->event);
	}
        free(entry);
    }
}

void _foreach_event_destroy(int pos, _event_entry *entry, void *context)
{
    if (entry)
    {
        llist_foreach(entry->listeners, &_foreach_listener_destroy, NULL);
        llist_destroy(entry->listeners);
        free(entry);
    }
}

int _find_event(int pos, _event_entry *entry, find_event_ctx_t *context)
{
    return entry && context->event == entry->id ? 1 : 0;
}

void _foreach_listener_remove(int pos, _listener_entry *entry, listener_remove_ctx_t *context)
{
    if (entry && context->listener == entry->event->listener)
    {
	if(entry->event)
	{
	    free(entry->event);
	}
        free(entry);
        llist_delete_at(context->listeners, pos);
        context->args->listeners_count -= 1;
    }
}

void _foreach_listener_emit(int pos, _listener_entry *entry, listener_emit_ctx_t *context)
{
    if (entry)
    {
        pthread_t id;
        entry->event->arg = context->arg;
        pthread_create(&id, NULL, entry->listener, entry->event);
        pthread_detach(id);
        if (entry->once)
        {
            listener_remove_ctx_t *listener_remove_context = malloc(sizeof(listener_remove_ctx_t));
            listener_remove_context->listener = entry->listener;
            listener_remove_context->listeners = context->listeners;
            listener_remove_context->args = context->args;
            _foreach_listener_remove(pos, entry, listener_remove_context);
            free(listener_remove_context);
        }
    }
}

void _append_listener(LinkedList *events, run_args_t *arg)	{
    int event = (int)arg->append_listener_args->event->event;
    _event_entry *event_entry;
    find_event_ctx_t *ctx = malloc(sizeof(find_event_ctx_t));
    ctx->event = event;
    event_entry = llist_find(events, &_find_event, ctx);
    free(ctx);
    if (event_entry)
    {
        LinkedList *listeners = event_entry->listeners;
        int size = llist_size(listeners);
        _listener_entry *listener_entry = malloc(sizeof(_listener_entry));
	listener_entry->event = arg->append_listener_args->event;
        listener_entry->listener = arg->append_listener_args->listener;
        listener_entry->once = arg->append_listener_args->once;
        llist_set_at(listeners, size, listener_entry);
    }
    else
    {
        int size = llist_size(events);
        _event_entry *new_event_entry = malloc(sizeof(_event_entry));
        _listener_entry *listener_entry = malloc(sizeof(_listener_entry));
	listener_entry->event = arg->append_listener_args->event;
        listener_entry->listener = arg->append_listener_args->listener;
        listener_entry->once = arg->append_listener_args->once;
        new_event_entry->id = event;
        new_event_entry->listeners = llist_init();
        llist_set_at(new_event_entry->listeners, 0, listener_entry);
        llist_set_at(events, size, new_event_entry);
    }
    arg->listeners_count += 1;
}

void _remove_listener(LinkedList *events, run_args_t *arg)
{
    int event = (int)arg->remove_listener_args->event->event;
    _event_entry *event_entry;
    free(arg->remove_listener_args->event);
    find_event_ctx_t *ctx = malloc(sizeof(find_event_ctx_t));
    ctx->event = event;
    event_entry = llist_find(events, &_find_event, ctx);
    free(ctx);
    if (event_entry)
    {
        LinkedList *listeners = event_entry->listeners;
        listener_remove_ctx_t *listener_remove_context = malloc(sizeof(listener_remove_ctx_t));
        listener_remove_context->listener = arg->remove_listener_args->listener;
        listener_remove_context->listeners = listeners;
        listener_remove_context->args = arg;
        llist_foreach(listeners, &_foreach_listener_remove, listener_remove_context);
        free(listener_remove_context);
    }
}

void _emit(LinkedList *events, run_args_t *arg)
{
    find_event_ctx_t *find_event_context = malloc(sizeof(find_event_ctx_t));
    find_event_context->event = (int)arg->listener_args->event;
    _event_entry *event_entry = llist_find(events, &_find_event, find_event_context);
    free(find_event_context);
    if (event_entry)
    {
        listener_emit_ctx_t *listener_emit_context = malloc(sizeof(listener_emit_ctx_t));
        listener_emit_context->arg = arg->listener_args->args;
        listener_emit_context->listeners = event_entry->listeners;
        listener_emit_context->args = arg;
        llist_foreach(event_entry->listeners, &_foreach_listener_emit, listener_emit_context);
        free(listener_emit_context);
    }
}

void *on_callback(event_t *event)
{
    event->listener(event->arg);
    return NULL;
}

int _on(emitter_t *emitter, event_t *event, int once)
{
    int locked = pthread_mutex_lock(&emitter->args->append_listener_args->mutex);
    emitter->args->append_listener_args->event = event;
    emitter->args->append_listener_args->listener = &on_callback;
    emitter->args->append_listener_args->once = once;
    char signal[1] = "a";
    char buffer[1];
    write(emitter->args->fd_read[1], signal, sizeof(char));
    read(emitter->args->fd_write[0], buffer, sizeof(char));
    int result = buffer[0] == 'o' ? 1 : 0;
    int unlocked = pthread_mutex_unlock(&emitter->args->append_listener_args->mutex);
    return result;
}

void *_run(run_args_t *arg)
{
    int running = 1;
    char buffer[1];
    char res[1];
    res[0] = 'o';
    LinkedList *events = llist_init();
    while (running)
    {
        read(arg->fd_read[0], buffer, sizeof(char));
        switch (buffer[0])
        {
        /**
         * @brief add a listener
         *
         */
        case 'a':
            if (arg->listeners_count < arg->max_listeners)
            {
                _append_listener(events, arg);
            }
            else
            {
                res[0] = 'n';
            }
            break;
        /**
         * @brief remove a listener
         *
         */
        case 'r':
            _remove_listener(events, arg);
            break;
        /**
         * @brief emit event
         *
         */
        case 'e':
            _emit(events, arg);
            break;
        case 's':
            close(arg->fd_read[0]);
            llist_foreach(events, &_foreach_event_destroy, NULL);
            llist_destroy(events);
            running = 0;
            break;
        default:
            break;
        }
        write(arg->fd_write[1], res, sizeof(char));
        res[0] = 'o';
    }
    close(arg->fd_write[1]);
    return NULL;
}

emitter_t *create_emitter(emitter_config_t *config)
{
    emitter_t *emitter = malloc(sizeof(emitter_t));
    emitter->listeners_count = 0;
    pthread_t id;
    run_args_t *args = _init_run_args(config->max_listeners);
    emitter->args = args;
    pthread_create(&id, NULL, &_run, args);
    emitter->id = id;
    return emitter;
}

int destroy_emitter(emitter_t *emitter)
{
    char signal[1] = "s";
    char res[1];
    write(emitter->args->fd_read[1], signal, sizeof(char));
    close(emitter->args->fd_read[1]);
    read(emitter->args->fd_write[0], res, sizeof(char));
    close(emitter->args->fd_write[0]);
    if (emitter->args->listener_args)
    {
        pthread_mutex_destroy(&emitter->args->listener_args->mutex);
        free(emitter->args->listener_args);
    }
    if (emitter->args->append_listener_args)
    {
        pthread_mutex_destroy(&emitter->args->append_listener_args->mutex);
        free(emitter->args->append_listener_args);
    }
    if (emitter->args->remove_listener_args)
    {
        pthread_mutex_destroy(&emitter->args->remove_listener_args->mutex);
        free(emitter->args->remove_listener_args);
    }
    pthread_join(emitter->id, NULL);
    free(emitter->args);
    free(emitter);
    return 0;
}

int on(emitter_t *emitter, void *event, listener_t listener)
{
    event_t *event_s = malloc(sizeof(event_t));
    event_s->listener = listener;
    event_s->event = event;
    return _on(emitter, event_s, 0);
}

int once(emitter_t *emitter, void *event, listener_t listener)
{
    event_t *event_s = malloc(sizeof(event_t));
    event_s->listener = listener;
    event_s->event = event;
    return _on(emitter, event_s, 1);
}

void off(emitter_t *emitter, void *event, listener_t listener)
{
    pthread_mutex_lock(&emitter->args->remove_listener_args->mutex);
    event_t *event_s = malloc(sizeof(event_t));
    event_s->event = event;
    emitter->args->remove_listener_args->event = event_s;
    emitter->args->remove_listener_args->listener = listener;
    char signal[1] = "r";
    char res[1];
    write(emitter->args->fd_read[1], signal, sizeof(char));
    read(emitter->args->fd_write[0], res, sizeof(char));
    pthread_mutex_unlock(&emitter->args->listener_args->mutex);
}

void emit(emitter_t *emitter, void *event, void *data)
{
    pthread_mutex_lock(&emitter->args->listener_args->mutex);
    emitter->args->listener_args->event = event;
    emitter->args->listener_args->args = data;
    char signal[1] = "e";
    char res[1];
    write(emitter->args->fd_read[1], signal, sizeof(char));
    read(emitter->args->fd_write[0], res, sizeof(char));
    pthread_mutex_unlock(&emitter->args->listener_args->mutex);
}

void *take_callback(event_t *args)
{
    printf("take callback executed, fd : %d", args->fd);
    args->return_val = args->arg;
    char signal[1] = "r";
    write(args->fd, signal, sizeof(char));
    return NULL;

}

void *take(emitter_t *emitter, void *event)
{
    pthread_mutex_lock(&emitter->args->append_listener_args->mutex);
    int fd[2];
    void *return_val;
    pipe(fd);
    event_t *event_s = malloc(sizeof(event_t));
    event_s->fd = fd[1];
    event_s->return_val = return_val;
    event_s->event = event;
    emitter->args->append_listener_args->event = event_s;
    emitter->args->append_listener_args->listener = &take_callback;
    emitter->args->append_listener_args->once = 1;
    char signal[1] = "a";
    char res[1];
    write(emitter->args->fd_read[1], signal, sizeof(char));
    read(emitter->args->fd_write[0], res, sizeof(char));
    int result = res[0] == 'o' ? 1 : 0;
    pthread_mutex_unlock(&emitter->args->append_listener_args->mutex);
    if(result) {
        char buff[1];
	printf("listening for answer ...");
        read(fd[0], buff, sizeof(char));
	printf("got answer ! ");
        close(fd[0]);
        close(fd[1]);
        return return_val;
    } else {
        close(fd[0]);
        close(fd[1]);
        return NULL;
    }
}
