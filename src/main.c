#include "tcp.h"

#include <string.h>

#define TOPIC_LEN (7)
#define BUFFER_LEN (128+4) /* 2 bytes for length */

void parse_topic(char *);
void handle_publish(int sock);

char topic[TOPIC_LEN+1];
char buffer[BUFFER_LEN+1];

int main(int argc, char *argv[])
{
	/* Check arguments */
	if (argc != 4) {
		printf("Usage: %s <ip> <port> <topic>\n", argv[0]);
		return ERR;
	}

	/* Connect to bridge server */
	int sock = tcp_connect(argv[1], (unsigned short) atoi(argv[2]));
	if (sock < 0) {
		fprintf(stderr, "Failed to connect\n");
		return ERR;
	}
	printf("Connected to bridge server...");

	/* Set topic to the specified size 7 according to protocol */
	parse_topic(argv[3]);
	printf("Subscribing to \"%s\"...\n", topic);

	/* Send the subscribe request */
	memset(buffer, 0, BUFFER_LEN);
	sprintf(buffer, "S%s", topic);
	if (send(sock, buffer, strlen(buffer), 0) <= 0) {
		fprintf(stderr, "Failed to send subscribe request");
		close(sock);
		return 0;
	}

	/* Print out the response */
	memset(buffer, 0, BUFFER_LEN);
	ssize_t ret;
	if ((ret = recv(sock, buffer, 1, 0)) <= 0) {
		fprintf(stderr, "Failed to receive response for subscribe request");
		close(sock);
		return 0;
	}
	buffer[ret] = '\0';
	printf("Recieved response : %s\n", buffer);

	/* Server internal error */
	if (buffer[0] == 'F') {
		close(sock);
		return 0;
	}

	/* Wait for publish content*/
	for (;;) {
		handle_publish(sock);
	}

	/* Clean-up */
	close(sock);
	return 0;	
}

void parse_topic(char * raw_topic)
{
	size_t given_len;

	if ((given_len = strlen(raw_topic)) >= TOPIC_LEN) {
		strncpy(topic, raw_topic, TOPIC_LEN);
		return;
	}

	strncpy(topic, raw_topic, given_len);
	
	/* Pad it with spaces */
	for (int i = given_len; i < TOPIC_LEN; i++) {
		topic[i] = ' ';
	}
	topic[TOPIC_LEN] = '\0';
}

void handle_publish(int sock)
{
	/* Handle heartbeat message */
	char resp;
	if (recv(sock, &resp, sizeof(resp), 0) <= 0) {
		perror("recv(heartbeat)");
		return;
	}

	/* Reply to heartbeat */
	if (send(sock, &resp, sizeof(resp), 0) <= 0) {
		perror("send(heartbeat)");
		return;
	}
	printf("Heartbeat handled...\n");

	/* TODO: */
	ssize_t ret;
	while ((ret = recv(sock, buffer, BUFFER_LEN, 0)) > 0) {
		buffer[ret] = '\0';
		printf("%s", buffer);
	}
}
