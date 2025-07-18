#ifndef COMMONS_H
#define COMMONS_H
// server code
#include <arpa/inet.h>
#include <dispatch/dispatch.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>


#define MAX_CLIENTS 10
#define MAX_NAME_LEN 16

int read_n_string(char **line_ptr, int maxlen);
void print_peer_info(int fd);

#endif
