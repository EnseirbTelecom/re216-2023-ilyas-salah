#include "common.h"
#include "msg_struct.h"


void SendFile(FILE *fp, int sockfd)
{
    char data[MSGLEN];

    while (fgets(data, sizeof(data), fp) != NULL)
    {
        ssize_t bytes_sent = send(sockfd, data, strlen(data), 0);
        
        if (bytes_sent == -1)
        {
            perror("[-]Error in sending file.");
            exit(1);
        }

        printf("data sent: %s \n", data);
        memset(data, 0, sizeof(data));
    }
}

void CreateFile(int sockfd)
{
    int run = 1;
    FILE *fp;
    char *filename = "received.txt";
    char buffer[MSGLEN];

    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        perror("[-]Error in opening file.");
        return;
    }

    while (run)
    {
        ssize_t bytes = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytes <= 0)
        {
            perror("[-]Error in receiving data.");
            break;
        }
        printf("[+]Writing data into file ...\n");
        fputs(buffer, fp);
        memset(buffer, 0, sizeof(buffer)); 
    }

    fclose(fp);
    return;
}

void HandleSendingFile(char *filename)
{
	int sockfd;
	struct sockaddr_in server_addr;
	FILE * fp;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd <= 0)
	{
		perror("[-]Error in socket");
		exit(1);
	}

	printf("[+]Server socket created successfully.\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = PORT;
	server_addr.sin_addr.s_addr = inet_addr(IP);

	int fd = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (fd <= 0)
	{
		perror("[-]Error in socket");
		exit(1);
	}

	printf("[+]Connected to Server.\n");

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		perror("[-]Error in reading file.");
		exit(1);
	}

	SendFile(fp, sockfd);

	printf("[+]File data sent successfully.\n");
	printf("[+]Closing the connection.\n");
	close(sockfd);
}

void HandleReceptionFile()
{
	int sockfd, fd_accept;
	struct sockaddr_in server_addr, new_addr;
	socklen_t addr_size;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("[-]Error in socket");
		exit(1);
	}

	printf("[+]Server socket created successfully.\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = PORT;
	server_addr.sin_addr.s_addr = inet_addr(IP);

	int fd_bind = bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (fd_bind <= 0)
	{
		perror("[-]Error in bind");
		exit(1);
	}

	printf("[+]Binding successfull.\n");

	if (listen(sockfd, 10) == 0)
	{
		printf("[+]Listening....\n");
	}
	else
	{
		perror("[-]Error in listening");
		exit(1);
	}

	addr_size = sizeof(new_addr);
	fd_accept = accept(sockfd, (struct sockaddr *) &new_addr, &addr_size);
	CreateFile(fd_accept);
	printf("[+]Data written in the file successfully.\n");
	close(sockfd);
	close(fd_accept);
}



struct message messagefill(char buff[], char nickname[NICK_LEN], char salon[CHANEL_LEN], char file[MSGLEN]) {
    size_t len = strlen(buff);

    if (len > 0 && buff[len - 1] == '\n') {
        buff[len - 1] = '\0'; // Replace the newline character with a null character
    }

    struct message msg;
    memset(&msg, 0, sizeof(struct message));
    strcpy(msg.infos, "");
    strcpy(msg.nick_sender, nickname);
    msg.pld_len = strlen(buff);

    if (buff[0] != '/') {
        msg.type = ECHO_SEND;
    } else {
        char temp[MSGLEN];
        strcpy(temp, buff);
        char *parts[3];
        parts[0] = strtok(temp, " ");
        parts[1] = strtok(NULL, " \n");

        if (strcmp(parts[0], "/nick") == 0) {
            msg.type = NICKNAME_NEW;
            strcpy(nickname, parts[1]);
            strcpy(msg.infos, parts[1]);
        } else if (strncmp(temp, "/who", 5) == 0) {
            msg.type = NICKNAME_LIST;
            strcpy(msg.infos, "");
        } else if (strcmp(parts[0], "/whois") == 0) {
            msg.type = NICKNAME_INFOS;
            strcpy(msg.infos, parts[1]);
        } else if (strcmp(parts[0], "/msgall") == 0) {
            char *text = buff + strlen(parts[0]);

            //j'ai prefere envoyer que le message utile au lieu de tout le buff , plus facile a traiter au niveau du server, juste un forwarding.
            strcpy(buff,text);
            msg.pld_len = strlen(buff);

            printf("sending : %s \n", text);
            msg.type = BROADCAST_SEND;
            strcpy(msg.infos, "\0");

        } else if (strcmp(parts[0], "/msg") == 0) {
            char *text = buff + strlen(parts[0]) + strlen(parts[1]) + 1;

            //j'ai prefere envoyer que le message utile au lieu de tout le buff , plus facile a traiter au niveau du server, juste un forwarding.
            strcpy(buff,text);
            msg.pld_len = strlen(buff);

            char recipient[NICK_LEN];
            strncpy(recipient, parts[1], NICK_LEN);
            printf("sending to %s the message : %s\n", recipient, text);
            msg.type = UNICAST_SEND;
            strcpy(msg.infos, recipient);
        }else if(strcmp(parts[0], "/create") == 0){

            msg.type = MULTICAST_CREATE;
            strcpy(salon,parts[1]);
            strcpy(msg.infos,parts[1]);
            printf("creating the channel %s\n ...", salon);

        }
        else if(strcmp(parts[0], "/channel_list") == 0){
            msg.type = MULTICAST_LIST;
            strcpy(msg.infos,"");
            
        }
        else if(strcmp(parts[0], "/join") == 0){
            msg.type = MULTICAST_JOIN;
            strcpy(msg.infos,parts[1]);
			strcpy(salon,msg.infos);
            printf("joining %s ...\n", salon);
        }
        else if (strcmp(parts[0], "/send") == 0) {
            if (parts[1] != NULL) {
                msg.type = FILE_REQUEST;

                // Extract recipient and filename
                char *recipient = parts[1];
                char *filename = buff + strlen(parts[0]) + strlen(parts[1]) + 2;

                // Set message information
                strcpy(msg.infos, recipient);
                strcpy(file, filename); 
                printf("sending file request to %s for file: %s\n", recipient, filename);
            } else {
                // Handle the case where no recipient is provided
                msg.type = ECHO_SEND;
                printf("Usage: /send [recipient] [filename]\n");
            }
        }
        else if(strcmp(salon, "") != 0){
            msg.type = MULTICAST_SEND;
        }
        else if(strcmp(parts[0],"/quit") == 0){
            msg.type = MULTICAST_QUIT;
            strcpy(msg.infos,salon);
			strcpy(salon, "");
        }
        else {
            msg.type = ECHO_SEND;
        }
    }
    return msg;
}


/*void set_nickname(int socket, char *nick_name, char* salon, char * file) {
    char buffer[NICK_LEN];
    struct message msg;
    char repeat = 1;

    while (repeat) {
        if (repeat) {
            printf("Choose a nickname. Ex: /nick salah\n");
            repeat = 0; // Pour éviter de répéter le message
        }

        fgets(buffer, NICK_LEN, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Supprimer le caractère de nouvelle ligne

        msg = messagefill(buffer, nick_name,salon,file);

        if (msg.type == NICKNAME_NEW ) {
            // Mettre à jour nickname avec le nouveau nom
            free(nick_name); // Libérer l'ancien espace mémoire
            nick_name = (char *)malloc(NICK_LEN); // Allouer un nouvel espace mémoire
            strcpy(nick_name, msg.infos); // Copier le nouveau nom

            if (send(socket, &msg, sizeof(msg), 0) <= 0) {
                perror("send");
                break;
            }
            if (send(socket, buffer, msg.pld_len, 0) <= 0) {
                perror("send");
                break;
            }
        } else {
            printf("Invalid nickname. Please try again.\n");
        }
    }
}
*/



int handle_connect(char *server_address,char *server_port) {
	struct addrinfo hints, *result, *rp;
	int sfd;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(server_address, server_port, &hints, &result) != 0) {
		perror("getaddrinfo()");
		exit(EXIT_FAILURE);
	}
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
		if (sfd == -1) {
			continue;
		}
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}
		close(sfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	return sfd;
}


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Try : ./client <@IP_client> <Numero_Port> \n");
        exit(EXIT_FAILURE);
    }

    int socket_fd = handle_connect(argv[1],argv[2]);
    char * nick = (char *)malloc(NICK_LEN);
    char * salon = (char *)malloc(CHANEL_LEN);
    char * filename = (char *)malloc(MSGLEN);


    int type;
    
    printf("Connecting to server ... done! \n");


    //config of 2 pollfd , one for standard input and other for socket
    struct pollfd fds[2];
    memset(fds,'\0',sizeof(struct pollfd)*2);
    
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;
    fds[1].revents = 0;

    int active_fds = -1;
    int run = 1;

    //timeout , client disconnect after 5min of none activity
    int timeout = 5*60*1000;

    printf("Entering the chat:\n");
    while (run)
    {
        active_fds = poll(fds,2,timeout);
        if (active_fds < 0)
        {
            perror("poll");
            run = 0;
        }
        if (active_fds == 0)
        {
            printf("Time Out\n");
            run = 0;
            break;
        }

        if (fds[0].revents & POLLIN ) 
        {
            struct message mssg;
            char buffer[MSGLEN];
            memset(buffer,0,MSGLEN);
            memset(&mssg,0,sizeof(struct message));
            
            ssize_t received_structure = recv(fds[0].fd,&mssg,sizeof(struct message),0);
            ssize_t received_message = recv(fds[0].fd,buffer,mssg.pld_len,0);
            
            if(received_message > 0 || received_structure > 0)
            {
                if (mssg.type == FILE_ACCEPT)
                {
                    printf("ghansendiw\n");
                    strcpy(filename, "send.txt");
                    HandleSendingFile(filename);
                    printf("Sending done \n");
                    type = mssg.type;
                }
                //strcpy(nick, mssg.nick_sender);
                if(strcmp(salon,"") != 0)
                {
                    // a determiner comment le client va voir les messages sur le salon 
                    printf("channel : %s [ %s ] : %s \n", salon ,mssg.nick_sender,buffer);
                }
                else
                {
                    printf("[ %s ] : %s\n ",mssg.nick_sender,buffer);
                }
                
                
            }
            else if (received_structure <= 0 && received_message <= 0)
            {
                printf("No connexion. Server closed :'(\n");
                run = 0;
                break;
            }

        }

        if (fds[1].revents & POLLIN) 
        {
            struct message msg;
            char buffer[MSGLEN];
            memset(buffer,0,MSGLEN);

            ssize_t  received = read(fds[1].fd,buffer,MSGLEN);
            
            if(received>0)
            {
                if(strcmp(buffer,"/quit\n")==0)
                {
                    run = 0;
                    break;
                }

			    msg = messagefill(buffer,nick,salon,filename);
                if(type = FILE_REQUEST)
                {
                    printf("4444444444");
                    type = 0;
                    if (strcmp(buffer, "Y\n") == 0)
                    {
                        msg.type = FILE_ACCEPT;
                        strcpy(msg.infos, nick);
                        printf("yay\n");
                    }
                    else if(strcmp(buffer, "N\n") == 0)
                    {
                        msg.type = FILE_REJECT;
                        strcpy(msg.infos, nick);
                    }
                    if (send(fds[1].fd, &msg, sizeof(struct message), 0) <= 0)
						{
							continue;
						}

					if (send(fds[1].fd, buffer, msg.pld_len, 0) <= 0)
						{
							continue;
						}

					if (msg.type == FILE_ACCEPT)
						{
							//start the p2p connection 
                            printf("ghanacceptiw\n");
							HandleReceptionFile();
							printf("client_receiver done \n");
						}
                }
                else
                {
                    ssize_t sent_structure = send(fds[0].fd,&msg,sizeof(struct message),0);
                    ssize_t sent_message = send(fds[0].fd,buffer,msg.pld_len,0);
                    printf("send");
                    if (sent_structure == -1)
                    {
                        perror("send");
                        break;
                    }
                    if(sent_message<=0)
                    {
                        perror("send");
                        break;
                }
                }

                //printf("message sent!! %s\n",msg.nick_sender);
            }   
        }
    }
    //freeing
    free(salon);
    free(nick);
    free(filename);
    
    //close socket
    close(fds[0].fd);

    exit(EXIT_SUCCESS);
    }

