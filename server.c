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
#include <poll.h>
#include "liste_chaine.h"

int main(int argc, char **argv) {
    struct SockAddrNode * list_clients = NULL;
    // Création de la socket

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'option SO_REUSEADDR pour réutiliser le port rapidement
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    // Remplissage de la structure sockaddr_in pour définir l'adresse sur laquelle le serveur écoutera
    struct sockaddr_in listening_addr;
    listening_addr.sin_family = AF_INET;
    listening_addr.sin_port = htons(atoi(argv[2])); // Port 8080
    inet_aton(argv[1], &listening_addr.sin_addr); // Adresse IP "0.0.0.0" (toutes les interfaces)



    // Liaison de la socket à l'adresse et au port spécifiés
    int ret = bind(listen_fd, (struct sockaddr *)&listening_addr, sizeof(listening_addr));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Passage de la socket en mode écoute
    ret = listen(listen_fd, 10); // Jusqu'à 10 clients en attente de connexion
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Initialise tableau de struct pollfd
    int max_cnx = 256;
    struct pollfd fds[max_cnx];
    memset(fds, 0, max_cnx * sizeof(struct pollfd)); // Remplir avec des zéros
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    while (1) { // The server never stops accepting

        // Call to poll to wait for events
        int active_fds = poll(fds, max_cnx, -1);

        if (active_fds == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        // Check each pollfd for activity
        for (int i = 0; i < max_cnx; i++) {
            if (fds[i].fd == listen_fd) {
                // If activity on listen_fd, accept new connections and add the new fd to the array
                if (fds[i].revents & POLLIN) {
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    
                    int new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
                    if (new_fd == -1) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    // Add the new fd to the array
                    for (int j = 0; j < max_cnx; j++) {
                        if (fds[j].fd == 0) {
                            fds[j].fd = new_fd;
                            fds[j].events = POLLIN;
                            fds[j].revents = 0;
                            break;
                        }
                    }
                appendSockAddrNode(&list_clients,client_addr);
                
                }
            } else {
                // If activity on a client fd, read and display the message
                if (fds[i].revents & POLLIN) {
                    char buf[256];
                    int to_read = sizeof(buf);
                    int received = 0;
                    while (to_read != received) {
                        int ret = read(fds[i].fd, buf + received, to_read - received);
                        if (ret == -1) {
                            perror("read");
                            exit(EXIT_FAILURE);
                        } else if (ret == 0) {
                            // Connection closed by client
                            close(fds[i].fd);
                            fds[i].fd = 0;
                            break;
                        }
                        received += ret;
                    }
                    // send msg again
                    int send_back = strlen(buf);
                    int sent = 0;
                    while(send_back != sent){
                        int ret = write(fds[i].fd,(char*)buf+sent,send_back-sent);
                        if (ret == -1) {
                            perror("write");
                            exit(EXIT_FAILURE);                        
                        }
                        sent += ret;
                    }

                    if(received > 0) {
                        buf[received] = '\0';
                        printf("New message: %s\n", buf);
                        //displaySockAddrList(list_clients);
                    }
                }
                
            }
        }
    }
    

    

    // Fermeture des sockets et sortie propre
    close(listen_fd);
    exit(EXIT_SUCCESS);
}

