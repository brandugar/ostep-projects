#include "io_helper.h"
#include <pthread.h>

#define MAXBUF (8192)

typedef struct
{
    char *host;
    int port;
    char *filename;
    int thread_id;
} client_args;

void client_send(int fd, char *filename)
{
    char buf[MAXBUF];
    char hostname[MAXBUF];

    gethostname_or_die(hostname, MAXBUF);
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    write_or_die(fd, buf, strlen(buf));
}

void client_print(int fd)
{
    char buf[MAXBUF];
    int n;

    n = readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0))
    {
        printf("Header: %s", buf);
        n = readline_or_die(fd, buf, MAXBUF);
    }

    n = readline_or_die(fd, buf, MAXBUF);
    while (n > 0)
    {
        printf("%s", buf);
        n = readline_or_die(fd, buf, MAXBUF);
    }
}

void *client_thread(void *arg)
{
    client_args *args = (client_args *)arg;
    int fd = open_client_fd_or_die(args->host, args->port);
    printf("[Thread %d] Enviando solicitud a %s:%d\n", args->thread_id, args->host, args->port);
    client_send(fd, args->filename);
    client_print(fd);
    close_or_die(fd);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Uso: %s <host> <port> <filename> <num_threads>\n", argv[0]);
        exit(1);
    }

    char *host = argv[1];
    int port = atoi(argv[2]);
    char *filename = argv[3];
    int num_threads = atoi(argv[4]);

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    client_args *args = malloc(sizeof(client_args) * num_threads);

    for (int i = 0; i < num_threads; i++)
    {
        args[i].host = host;
        args[i].port = port;
        args[i].filename = filename;
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, client_thread, &args[i]);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
    return 0;
}
