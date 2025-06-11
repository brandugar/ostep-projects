#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>

typedef struct
{
    int *buf;
    int capacity;
    int count;
    int head;
    int tail;
    pthread_mutex_t lock;
    pthread_cond_t empty;
    pthread_cond_t full;
} request_buffer_t;

void buffer_init(request_buffer_t *buffer, int capacity);
void buffer_add(request_buffer_t *buffer, int item);
int buffer_remove(request_buffer_t *buffer);

#endif
