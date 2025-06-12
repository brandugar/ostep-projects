#include <stdio.h>
#include "request.h"
#include "io_helper.h"
#include "thread_pool.h"
#include <getopt.h>

char default_root[] = ".";
request_buffer_t conn_buffer;

double get_seconds()
{
	struct timeval t;
	int rc = gettimeofday(&t, NULL);
	assert(rc == 0);
	return (double)((double)t.tv_sec + (double)t.tv_usec / 1e6);
}

void *worker_thread(void *arg)
{
	while (1)
	{
		int connfd = buffer_remove(&conn_buffer);
		double time1 = get_seconds();
		while ((get_seconds() - time1) < 3)
		{
			sleep(1);
		}
		request_handle(connfd);
		close_or_die(connfd);
	}
	return NULL;
}
//
// ./wserver [-d <basedir>] [-p <portnum>]
//
int main(int argc, char *argv[])
{
	int c;
	char *root_dir = default_root;
	int port = 10000;
	int num_threads = 4;
	int buffer_size = 16;

	while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
		switch (c)
		{
		case 'd':
			root_dir = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 't':
			num_threads = atoi(optarg);
			break;
		case 'b':
			buffer_size = atoi(optarg);
			break;
		default:
			fprintf(stderr, "usage: %s [-d basedir] [-p port] [-t threads] [-b buffer]\n", argv[0]);
			exit(1);
		}

	chdir_or_die(root_dir);
	buffer_init(&conn_buffer, buffer_size);

	pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
	for (int i = 0; i < num_threads; i++)
	{
		pthread_create(&threads[i], NULL, worker_thread, NULL);
	}

	int listen_fd = open_listen_fd_or_die(port);
	while (1)
	{
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *)&client_addr, (socklen_t *)&client_len);
		buffer_add(&conn_buffer, conn_fd);
	}

	return 0;
}