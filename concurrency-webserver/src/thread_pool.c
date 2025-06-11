#include "thread_pool.h"
#include <stdlib.h>
#include <assert.h>

void buffer_init(request_buffer_t *buffer, int capacity)
{
    buffer->buf = malloc(sizeof(int) * capacity);
    buffer->capacity = capacity;
    buffer->count = 0;
    buffer->head = 0;
    buffer->tail = 0;
    pthread_mutex_init(&buffer->lock, NULL);
    pthread_cond_init(&buffer->empty, NULL);
    pthread_cond_init(&buffer->full, NULL);
}

void buffer_add(request_buffer_t *buffer, int item)
{
    pthread_mutex_lock(&buffer->lock);
    while (buffer->count == buffer->capacity)
        pthread_cond_wait(&buffer->full, &buffer->lock);

    buffer->buf[buffer->tail] = item;
    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->count++;

    pthread_cond_signal(&buffer->empty);
    pthread_mutex_unlock(&buffer->lock);
}

int buffer_remove(request_buffer_t *buffer)
{
    pthread_mutex_lock(&buffer->lock);
    while (buffer->count == 0)
        pthread_cond_wait(&buffer->empty, &buffer->lock);

    int item = buffer->buf[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->count--;

    pthread_cond_signal(&buffer->full);
    pthread_mutex_unlock(&buffer->lock);
    return item;
}
