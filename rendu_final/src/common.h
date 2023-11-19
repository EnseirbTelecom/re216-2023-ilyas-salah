#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <poll.h>
#include <time.h>
#include "msg_struct.h"

#define MSGLEN 1024
#define CHANEL_LEN 15
#define MAX_CLIENT_CHANNEL 100
#define FILE_NAME_LEN 1024
#define FILE_LEN 2*1024*1024
#define PORT 12345

#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"


#endif /* COMMON_H */



