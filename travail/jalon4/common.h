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
#define PORT 12345
#define IP "127.0.0.1"