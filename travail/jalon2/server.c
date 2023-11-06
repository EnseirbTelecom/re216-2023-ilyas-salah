#include "common.h"
#include "communicate.h"



int main(int argc, char **argv) {
    struct SockAddrNode * list_clients = NULL;

    if (argc < 2)
    {
        printf("specify a port number\n");
        exit(EXIT_FAILURE);
    }

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
    listening_addr.sin_port = htons(atoi(argv[1])); // Port 8080
    inet_aton("0.0.0.0", &listening_addr.sin_addr); // Adresse IP "0.0.0.0" (toutes les interfaces)

    appendSockAddrNode(&list_clients,listening_addr,listen_fd,"0");
    strcpy(list_clients->nickname,"Server");



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
        printf("Waiting ...\n");
        // Call to poll to wait for events
        int active_fds = poll(fds, max_cnx, -1);

        if (active_fds == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        // Check each pollfd for activity
        for (int i = 0; i < max_cnx; i++) {
            if (fds[i].fd == listen_fd && (fds[i].revents & POLLIN)) 
            {
                // If activity on listen_fd, accept new connections and add the new fd to the array
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                
                int new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
                if (new_fd == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                // Add the new fd to the array
                printf("New incoming connection : %d\n", new_fd);
                for (int j = 0; j < max_cnx; j++) {
                    if (fds[j].fd == 0) {
                        fds[j].fd = new_fd;
                        fds[j].events = POLLIN;
                        fds[j].revents = 0;
                        break;
                    }
                }
                time_t rawtime;
                struct tm * timeinfo;

                time ( &rawtime );
                timeinfo = localtime ( &rawtime );
                appendSockAddrNode(&list_clients,client_addr,new_fd,asctime (timeinfo));  
                send_echo(new_fd,"please login with /nick <your pseudo>\n","Server");
            } 
            
            else if (fds[i].revents & POLLIN) 
            {
                int close_conn = 0;
                struct message msgstruct;
                char buff[MSGLEN];
                if (receive_msg(fds[i].fd,&msgstruct,buff)<= 0)
                {
                    close_conn = 1;
                }
                if (!close_conn)
                {
                    handle_request(fds[i].fd,&msgstruct,buff,list_clients);
                    printf("sortie de request\n");
                }
                // if (!close_conn && send_msg(fds[i].fd,&msgstruct,buff) <= 0)
                // {
                //     close_conn = 1;
                // }

                if (close_conn)
                {
                    printf("Connection of client %i closed :/ \n",fds[i].fd);
                    deleteSockNode_from_fd(list_clients,fds[i].fd);

                    printf("-----------------\n");
                    printf("Clients left: \n\n");
                    displaySockAddrList(list_clients);
                    printf("-----------------\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;

                }
            }      
        }
    }
    // free list_clients
    freeSockAddrList(list_clients);
    // Fermeture des sockets et sortie propre
    for (int i = 0; i < max_cnx; i++)
    {
        if(fds[i].fd >= 0)
        close(fds[i].fd);
    }
    exit(EXIT_SUCCESS);
}


