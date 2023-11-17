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

#define MSGLEN 1024
#define CHANEL_LEN 15
#define MAX_CLIENT_CHANNEL 100
#define FILE_NAME_LEN 1024
#define FILE_LEN 2*1024*1024
#define PORT 12345