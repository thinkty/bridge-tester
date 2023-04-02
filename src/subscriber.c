#include "tcp.h"

#include <string.h>

#define TOPIC_LEN (7)
#define SERVER_PF_SIZE (2)   /* Publish format size is 2 bytes */
#define SERVER_PF_DATA (128) /* Publish format data is 128 bytes max */
#define SERVER_PF_END "\r\n\r\n"

void parse_topic(char *);
int handle_publish(int sock);
int handle_publish_message(int sock);

char topic[TOPIC_LEN+1];
char buffer[SERVER_PF_SIZE+SERVER_PF_DATA+1];

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
	printf("Connected to bridge server...\n");

	/* Set topic to the specified size 7 according to protocol */
	parse_topic(argv[3]);
	printf("Subscribing to \"%s\"...\n", topic);

	/* Send the subscribe request */
	memset(buffer, 0, SERVER_PF_SIZE+SERVER_PF_DATA+1);
	sprintf(buffer, "S%s", topic);
	if (send(sock, buffer, strlen(buffer), 0) <= 0) {
		fprintf(stderr, "Failed to send subscribe request\n");
		close(sock);
		return 0;
	}

	/* Print out the response */
	memset(buffer, 0, SERVER_PF_SIZE+SERVER_PF_DATA+1);
	ssize_t ret;
	if ((ret = recv(sock, buffer, 1, 0)) <= 0) {
		fprintf(stderr, "Failed to receive response for subscribe request\n");
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

	/* Wait for publish content and handle it */
	while (handle_publish(sock) != ERR) {}

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

int handle_publish(int sock)
{
	/* Handle heartbeat message */
	char resp;
	if (recv(sock, &resp, sizeof(resp), 0) <= 0) {
		perror("recv(heartbeat)");
		return ERR;
	}

	/* Reply to heartbeat */
	if (send(sock, &resp, sizeof(resp), 0) <= 0) {
		perror("send(heartbeat)");
		return ERR;
	}
	printf("Heartbeat handled...\n");

	/* Handle publish messages until \r\n\r\n message is received or error */
	if (handle_publish_message(sock) != OK) {
		return ERR;
	}

	printf("End of stream...\n");
	fflush(stdout);
	return OK;
}

int handle_publish_message(int sock)
{
	ssize_t ret;

	for (;;) {
		/* First two bytes indicate size */
		memset(buffer, 0, SERVER_PF_SIZE+SERVER_PF_DATA+1);
		if ((ret = recv(sock, buffer, SERVER_PF_SIZE, 0)) <= 0) {
			fprintf(stderr, "Error while receiving size of publish message\n");
			return ERR;
		}

		uint16_t size = ntohs(buffer[0] << 8 | buffer[1]);
		printf("\n[%u]\n", size);
		uint16_t received = 0;

		/* Make sure to read until the specified size */
		while (received < size) {
			if ((ret = recv(sock, &buffer[received], size-received, 0)) <= 0) {
				fprintf(stderr, "Error while receiving size of publish message\n");
				return ERR;
			}

			for (int i = 0; i < ret; i++) {
				printf("%c", buffer[i+received]);
			}

			received += ret;
			fflush(stdout);
		}
		if (received != size) {
			printf("(%u != %u) ", received, size);
		}
		buffer[size] = '\0';

		/* End of stream */
		if (strcmp(buffer, SERVER_PF_END) == 0) {
			break;
		}
		fflush(stdout);
	}

	if (ret < 0) {
		return ERR;
	}
	return OK;
}
