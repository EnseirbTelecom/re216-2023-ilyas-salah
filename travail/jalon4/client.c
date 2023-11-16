#include "common.h"
#include "msg_struct.h"

struct message messagefill(char buff[], char nickname[NICK_LEN], char salon[CHANEL_LEN], char file[FILE_LEN]) {
    size_t len = strlen(buff);
/*

    if (len > 0 && buff[len - 1] == '\n') {
        buff[len - 1] = '\0'; // Replace the newline character with a null character
    }
*/


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
                // Extract recipient and filename
                char *recipient = parts[1];
                char *filename = buff + strlen(parts[0]) + strlen(parts[1]) + 2;

                // Set message information
                strcpy(msg.infos, recipient);
                strcpy(filename, file); 
                printf("sending file request to %s for file: %s\n", recipient, filename);
				msg.type = FILE_REQUEST;

                // cpy filename into buffer 

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
			strcpy(salon, "");
        }
        else {
            msg.type = ECHO_SEND;
        }
    }
    return msg;
}



void send_file(FILE *fp, int sockfd)
{
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Invalid file pointer.\n");
        return;
    }

    char buffer[MSGLEN];
    size_t bytesRead;

    // sending stuct msg FILE SEND
    // msg.info "nom fichier"

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        ssize_t sentBytes = send(sockfd, buffer, bytesRead, 0);

        if (sentBytes == -1)
        {
            perror("[-]Error in sending file.");
            exit(1);
        }
        else if (sentBytes < bytesRead)
        {
            fprintf(stderr, "Warning: not all bytes sent. Expected %zu, sent %zd.\n", bytesRead, sentBytes);
        }

        printf("Bytes sent: %zu\n", bytesRead);
        memset(buffer, 0, sizeof(buffer));
    }

    if (ferror(fp))
    {
        perror("Error reading file");
    }
}


void write_file(int sockfd)
{
    int n;
    FILE *fp;
    char *filename = "file_received";
    char buffer[MSGLEN];

    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("[-]Error opening file");
        return;
    }

    // RECV STRUCT MESSAGE

    while (1)
    {
        n = recv(sockfd, buffer, MSGLEN, 0);
        if (n <= 0)
        {
            if (n < 0)
                perror("Error receiving data");

            break;
        }

        fwrite(buffer, 1, n, fp);

        if (n < MSGLEN)
            break;
    }

    fclose(fp);
    printf("[+]File received successfully.\n");
}



void client_sender(char *filename)
{
	int e;

	int sockfd;
	struct sockaddr_in server_addr;
	FILE * fp;

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

	e = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (e == -1)
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

	send_file(fp, sockfd);

	printf("[+]File data sent successfully.\n");
	printf("[+]Closing the connection.\n");
	close(sockfd);
}

void client_receiver()
{
	int e;
	int sockfd, new_sock;
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

	e = bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (e < 0)
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
	new_sock = accept(sockfd, (struct sockaddr *) &new_addr, &addr_size);
	write_file(new_sock);
	printf("[+]Data written in the file successfully.\n");
	close(sockfd);
	close(new_sock);
}


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
    char * file = (char *)malloc(FILE_LEN);
    char * sender = (char *)malloc(NICK_LEN);
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
            
            if(received_message>0)
            {
                if (mssg.type == FILE_ACCEPT)
                {
                    strcpy(file, "/home/lairgi/Bureau/10.png");
                    printf("%s",file);
                    client_sender(file);
                    printf("client_sender done \n");
                }

                if(mssg.type == FILE_SEND)
                {
                    printf("ghanaaceptiw");
                    client_receiver();

                }
                type = mssg.type;
                strcpy(sender, mssg.nick_sender);
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

                if (type == FILE_REQUEST)
				// a client requested to send a file to this client
				{
					type = 0;
					if (!(strcmp(buffer, "Y\n") == 0 || strcmp(buffer, "N\n") == 0))
					{
						printf("respond with only Y/N \n");
					}
					else
					{
						if (strcmp(buffer, "Y\n") == 0)
						{
							msg.type = FILE_ACCEPT;
							strcpy(msg.infos, sender);
						}
						else
						{
							msg.type = FILE_REJECT;
							strcpy(msg.infos, sender);
						}

						if (send(fds[0].fd, &msg, sizeof(msg), 0) <= 0)
						{
							continue;
						}

						if (send(fds[0].fd, buffer, msg.pld_len, 0) <= 0)
						{
							continue;
						}

						if (msg.type == FILE_ACCEPT)
						{
							//start the p2p connection 
							client_receiver();
							printf("client_receiver done \n");
						}
					}
				}
                
			    msg = messagefill(buffer,nick,salon,file);
                ssize_t sent_structure = send(fds[0].fd,&msg,sizeof(struct message),0);
                ssize_t sent_message = send(fds[0].fd,buffer,msg.pld_len,0);
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
                //printf("message sent!! %s\n",msg.nick_sender);
            }   
        }
    }
    //freeing
    free(salon);
    free(nick);
    free(sender);
    free(file);
    
    //close socket
    close(fds[0].fd);

    exit(EXIT_SUCCESS);
    }

