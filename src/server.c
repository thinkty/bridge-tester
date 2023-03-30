#include "tcp.h"

void * handle(void * csock);

int main(int argc, char *argv[])
{
	/* Check arguments */
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		return ERR;
	}

	/* Setup TCP IPv4 port */
	int sock = tcp_listen((unsigned short) atoi(argv[1]));
	if (sock < 0) {
		fprintf(stderr, "Failed to setup port\n");
		return ERR;
	}

	/* Accept incoming connections and handle it in new thread */
	for (;;) {
		int * csock = malloc(sizeof(int));
		if (tcp_accept(sock, csock) != OK) {
			fprintf(stderr, "Failed to accept new connection\n");
			close(sock);
			return ERR;
		}

		/* Handle client in new thread */
		pthread_t thread;
		if (pthread_create(&thread, NULL, handle, csock) || pthread_detach(thread)) {
			perror("pthread()");
			close(sock);
			close(*csock);
			return ERR;
		}
	}

	/* Clean-up */
	close(sock);
	return 0;	
}

/**
 * Function to run in a thread to handle commands from client.
 * Currently just echoing.
 */
void * handle(void * csock)
{
	int sock = *((int *) csock);
	free((int *) csock);

	char buff[SOCK_BUFFER_SIZE] = {0};
	ssize_t ret;

	for (;;) {

		/* Unable to read from client socket */
		if ((ret = recv(sock, buff, SOCK_BUFFER_SIZE, 0)) < 0) {
			perror("recv()");
			close(sock);
			return NULL;
		}

		/* EOF */
		if (ret == 0) {
			break;
		}

		buff[ret] = '\0';
		printf("< %s", buff);

		/* Echo */
		if (send(sock, buff, ret, 0) < 0) {
			perror("send()");
			close(sock);
			return NULL;
		}
	}

	/* Clean-up */
	fprintf(stdout, "EOF\n");
	close(sock);
	return NULL;
}

