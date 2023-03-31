#include "tcp.h"

int tcp_connect(char * ip, unsigned short port)
{
	/* Create TCP socket for IPv4 */
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket()");
		return ERR;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("connect()");
		close(sock);
		return ERR;
	}

	return sock;
}
