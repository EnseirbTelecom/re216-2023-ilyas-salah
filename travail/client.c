#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("needs 2 args\n");
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[2]);
    printf("le port %i\n",port);
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if (fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    inet_aton(argv[1],&addr.sin_addr);

    int ret = connect(fd,(struct sockaddr *)&addr,sizeof(addr));
    if (ret == -1)
    {
        perror("connect");
        //exit(EXIT_FAILURE);
    }
    else {printf("connected\n");
    }

    char msg[256];
    printf("chat:");
    scanf("%s",msg);

    //printf("server: %s\n with len %li",,strlen(buf));
    int next_msg_size = strlen(msg);

    int sent = 0;
    while (sent != sizeof(int)){
        int ret = write(fd,(char *)&next_msg_size+sent,sizeof(int));
        if (ret == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        sent += ret;
    }
    int to_send = next_msg_size;
    sent = 0;
    while (to_send != sent){
        int ret = write(fd,(char *)msg+sent,to_send);
        if (ret == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        sent += ret;
    }
    printf("sent!!");
    // struct sockaddr_in addr_local_socket;
    // int len = sizeof(addr_local_socket);
    // getsockname(fd,(struct socketaddr_in *)&addr_local_socket,&len);
    // u_int16_t local_port = ntohs(addr_local_socket.sin_port);
    // printf("local socket port %u\n",local_port);

    exit(EXIT_SUCCESS);
    
    return 0;
}