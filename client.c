#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
int main(int argc, char ** argv){
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int)); // lier le socket a un traffic lié dans un port
    /*setup du client*/
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_aton("127.0.0.1",&addr.sin_addr);
    ////////////////////
    /*Connect to server*/
    int ret = connect(fd,(struct sockaddr*)&addr,sizeof(addr));
    if (ret == -1){
        perror("connect");
        exit(EXIT_FAILURE);
    }
    /*what to send*/
    char car[256];
    char * str = "This is 1st client\n";
    strcpy(car,str);
    //////////
    int to_send = strlen(str); // on doit specifier la taille de what we send
    int sent = 0;
    /*script of how to send*/
    while(to_send!=sent){
        int ret = write(fd,(char*)car + sent,to_send - sent); // socket, whattosend, sa taille
        if(ret == -1){
            perror("write");
            exit(EXIT_FAILURE);
        }
        sent += ret;
    }
    int read_fd = socket(AF_INET,SOCK_STREAM,0);
    char buf[256];
    int to_read = 11; // Nombre de caractères à lire
    int received = 0; // Nombre de caractères reçus jusqu'à présent
    while (to_read != received) {
        int ret = read(fd, (char *)buf + received, to_read - received);
        if (ret == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        received += ret;
    }

    // Affichage du message reçu
    printf("Nouveau message : %s\n", buf);
    ///////////////
    exit(EXIT_SUCCESS);
}