#ifndef __TCP_H
#define __TCP_H

#include <arpa/inet.h>  /* sockaddr_in, htons(), htonl() */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* socket(), bind(), listen(), accept() */
#include <unistd.h>     /* read(), write() */

#define OK  (0)
#define ERR (-1)

#define SOCK_BUFFER_SIZE  (100)

int tcp_connect(char * ip, unsigned short port);


#endif

