#include "common.h"
#include "msg_struct.h"

struct message messagefill(char buff[], char nickname[NICK_LEN], char salon[CHANEL_LEN]) {
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
            //printf("sending to %s the message : %s\n", recipient, text);
            msg.type = UNICAST_SEND;
            strcpy(msg.infos, recipient);
        }else if(strcmp(parts[0], "/create") == 0){

            msg.type = MULTICAST_CREATE;
            //strcpy(salon,parts[1]);
            strcpy(msg.infos,parts[1]);

        }
        else if(strcmp(parts[0], "/channel_list") == 0){
            msg.type = MULTICAST_LIST;
            strcpy(msg.infos,"");
            
        }
        else if(strcmp(parts[0], "/join") == 0){
            msg.type = MULTICAST_JOIN;
            strcpy(msg.infos,parts[1]);
			//strcpy(salon,msg.infos);
            //printf("joining %s ...\n", salon);
        }
        else if(strcmp(parts[0], "/send") == 0){
            msg.type = FILE_REQUEST;
        }
        else if(strcmp(parts[0],"/quit") == 0 && strcmp(salon, "") != 0){
            msg.type = MULTICAST_QUIT;
			strcpy(msg.infos,salon);
        }
        else if(strcmp(salon, "") != 0){
            msg.type = MULTICAST_SEND;
            strcpy(msg.infos,salon);
        }
        else {
            msg.type = ECHO_SEND;
        }
    }
    return msg;
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
    memset(salon,'\0',CHANEL_LEN);
    
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
                if (mssg.type == MULTICAST_QUIT)
                {
                    strcpy(salon,"");
                }
                if (mssg.type == MULTICAST_JOIN)
                {
                    strcpy(salon,mssg.infos);
                }
                
                if(strcmp(salon,"") != 0)
                {
                    // a determiner comment le client va voir les messages sur le salon 
                    printf("[[ %s ]] [ %s ] : %s \n", salon ,mssg.nick_sender,buffer);
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
                if(strcmp(buffer,"/quit\n")==0 && strcmp(salon,"") == 0)
                {
                    run = 0;
                    break;
                }
                
			    msg = messagefill(buffer,nick,salon);
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
    
    //close socket
    close(fds[0].fd);

    exit(EXIT_SUCCESS);
    }

