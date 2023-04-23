#include "tcp.h"

#include <string.h>
#include <sys/time.h>

#define TOPIC_LEN (7)
#define SERVER_PF_DATA (128) /* Publish format data is 128 bytes max */

void parse_topic(char *);

char topic[TOPIC_LEN+1];
char buffer[SERVER_PF_DATA+1];

int main(int argc, char *argv[])
{
	/* Check arguments */
	if (argc != 5) {
		printf("Usage: %s <ip> <port> <topic> <file>\n", argv[0]);
        printf("\tex: %s 192.168.4.22 55555 update ./image\n\n", argv[0]);
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
	printf("Publishing to \"%s\"...\n", topic);

	/* Send the publish request */
	memset(buffer, 0, SERVER_PF_DATA+1);
	sprintf(buffer, "P%s", topic);
	if (send(sock, buffer, strlen(buffer), 0) <= 0) {
		fprintf(stderr, "Failed to send publish request\n");
		close(sock);
		return 0;
	}
	memset(buffer, 0, SERVER_PF_DATA+1);

    /* Open the given file */
    FILE * fp = fopen(argv[4], "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open input file %s\n", argv[4]);
        close(sock);
        return 0;
    }

	struct timeval time;
	gettimeofday(&time, NULL);
	int64_t s1 = (int64_t)(time.tv_sec);

    size_t total_sent = 0;
    while (!feof(fp)) {
        memset(buffer, 0, SERVER_PF_DATA+1);

        /* Read from given file */
        size_t size = fread(buffer, 1, SERVER_PF_DATA, fp);
		if (size == 0 && feof(fp)) {
			fclose(fp);
			close(sock);
			break;
		}

        /* Send to the server */
        size_t sent = 0;
        while (sent < size) {
            size_t ret_send = send(sock, &buffer[sent], size-sent, 0);
            if (ret_send < 0) {
                fprintf(stderr, "Failed to send file\n");
                fclose(fp);
                close(sock);
                return 0;
            }

            sent += ret_send;
            total_sent += ret_send;
        }

		/* Receive response from the server */
		char resp;
		if (recv(sock, &resp, sizeof(resp) , 0) <= 0 || resp != 'O') {
			fprintf(stderr, "Error while receiving response for publish block\n");
			fclose(fp);
			close(sock);
			return 0;
		}
    }

	gettimeofday(&time, NULL);
	int64_t s2 = (int64_t)(time.tv_sec);

    printf("Successfully sent %ld bytes in %ld seconds...\n", s2-s1, total_sent);
    fflush(stdout);

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
