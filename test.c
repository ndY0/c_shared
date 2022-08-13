#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "./lib/event-emitter.h"

#undef MAX_LISTENERS
#define MAX_LISTENERS 2

typedef struct test_arg
{
    int test;
} test_arg;

typedef struct test_take_arg
{
    emitter_t *emitter;
    void* event;
    int running;
} test_take_arg;

void *say_hello(void *arg)
{
    sleep(1);
    printf("in worker func !\n");
    return NULL;
}

void *say_hello2(void *arg)
{
    sleep(2);
    printf("in worker func type 2 !\n");
    return NULL;
}

void *test_take(test_take_arg *arg)
{
    while(arg->running)
    {
        test_arg *args = take(arg->emitter, arg->event);
        printf("in test_take, received arg : %d\n", args ? args->test : 0);
    }
}

int main(int argc, char *argv[])
{
    emitter_t *emitter = create_emitter(EMITTER_DEFAULT_CONFIG);

    char event[] = "event";
    char event2[] = "event2";

    on(emitter, event, &say_hello);
    on(emitter, event, &say_hello2);
    on(emitter, event, &say_hello);
    on(emitter, event2, &say_hello2);
    on(emitter, event2, &say_hello);
    on(emitter, event2, &say_hello2);

    off(emitter, event, &say_hello);
    off(emitter, event2, &say_hello2);

    pthread_t test_take_id;
    test_take_arg * test_take_args = malloc(sizeof(test_take_arg));
    test_take_args->emitter = emitter;
    test_take_args->event = event;
    test_take_args->running = 1;
    pthread_create(&test_take_id, NULL, &test_take, test_take_args);
    sleep(1);

    on(emitter, event, &say_hello);
    on(emitter, event, &say_hello2);
    on(emitter, event, &say_hello);

    test_arg *arg = malloc(sizeof(test_arg));
    arg->test = 10;
    test_arg *arg2 = malloc(sizeof(test_arg));
    arg2->test = 8;

    emit(emitter, event, arg);
    emit(emitter, event, arg2);
    test_take_args->running = 0;
    sleep(1);
    emit(emitter, event2, arg);

    sleep(6);
    printf("my emitter pid : %lu\n", emitter->id);
    free(test_take_args);
    free(arg);
    free(arg2);
    pthread_join(test_take_id, NULL);
    destroy_emitter(emitter);

    return 0;
}