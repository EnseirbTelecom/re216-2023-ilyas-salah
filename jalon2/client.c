#include "common.h"
#include "msg_struct.h"


char ** split_sentence(char * str, int TAILLE_MAX_OF_WORDS){
    char *token = strtok(str, " ");
    char ** words = (char ** )malloc(sizeof(char*)*TAILLE_MAX_OF_WORDS); // Tokenize the string
    //strcpy(words[0],token);
    int i = 0;
    while (token != NULL) {
        words[i] = token;
        token = strtok(NULL, " ");
        i++;
    }    
    words[i] = NULL;
    return words;
    free(words);
}
int get_length(char** words) {
    int length = 0;
    while (words[length] != NULL) {
        length++;
    }
    return length;
}
char * concatenate(char ** words, int index){
    char * result = (char*)malloc(sizeof(char*) * (get_length(words)-1));
    result[0] = '\0';
    for(int i = index; i < get_length(words); i++)
    {
        strcat(result,words[i]);
        if (i < get_length(words) - 1)
        {
                strcat(result, " ");
        }
    }
    return result;

}

struct message what_to_send(char * buff, char nickname[NICK_LEN]){
    struct message msg;
    memset(&msg, 0, sizeof(struct message));
    msg.pld_len = strlen(buff);
    strcpy(msg.nick_sender,nickname);
    if(buff[0]!='/')
    {
        msg.type = ECHO_SEND;
    }
    else
    {
        char **words_from_buffer = split_sentence(buff,20);
        if(strcmp(words_from_buffer[0],"/nick") == 0)
        {
            msg.type = NICKNAME_NEW;
            strcpy(msg.nick_sender,words_from_buffer[1]);
            printf("%s\n",words_from_buffer[1]);
            strcpy(msg.infos,words_from_buffer[1]);

        }
        else if(strcmp(words_from_buffer[0],"/whois") == 0)
        {
            msg.type = NICKNAME_INFOS;
            strcpy(msg.infos,words_from_buffer[1]);
        }
        else if(strcmp(words_from_buffer[0],"/who") == 0)
        {
            msg.type = NICKNAME_LIST;
        }
        else if(strcmp(words_from_buffer[0],"/msgall") == 0)
        {
            msg.type = BROADCAST_SEND;
            strcpy(msg.infos,concatenate(words_from_buffer,1));
            printf("Sending broadcastly : %s\n",concatenate(words_from_buffer,1));
        }
        else if(strcmp(words_from_buffer[0],"/msg") == 0)
        {
            msg.type = UNICAST_SEND;
            strcpy(msg.infos,concatenate(words_from_buffer,2));
            printf("Sending unicastly to %s the message : %s\n",words_from_buffer[1],concatenate(words_from_buffer,2));

        }
        
        
    }
    return msg;
}

void echo_client(int sockfd) {
	struct message msgstruct;
	char buff[MSGLEN];
    char nickname[NICK_LEN];
	int n;
	while (1) {
		// Cleaning memory
		memset(&msgstruct, 0, sizeof(struct message));
		memset(buff, 0, MSGLEN);
		// Getting message from client
		printf("Message: ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n') {} // trailing '\n' will be sent
		// Filling structure
        msgstruct = what_to_send(buff,nickname);
		msgstruct.pld_len = strlen(buff);
		//strncpy(msgstruct.nick_sender, "Toto", 4);
		//msgstruct.type = ECHO_SEND;
		//strncpy(msgstruct.infos, "\0", 1);
		// Sending structure
		if (send(sockfd, &msgstruct, sizeof(msgstruct), 0) <= 0) {
			break;
		}
		// Sending message (ECHO)
		if (send(sockfd, buff, msgstruct.pld_len, 0) <= 0) {
			break;
		}
		printf("Message sent!\n");
		// Cleaning memory
		memset(&msgstruct, 0, sizeof(struct message));
		memset(buff, 0, MSGLEN);
		// Receiving structure
		if (recv(sockfd, &msgstruct, sizeof(struct message), 0) <= 0) {
			break;
		}
		// Receiving message
		if (recv(sockfd, buff, msgstruct.pld_len, 0) <= 0) {
			break;
		}
		printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s\n", msgstruct.pld_len, msgstruct.nick_sender, msg_type_str[msgstruct.type], msgstruct.infos);
		printf("Received: %s", buff);
	}
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

    int fd;
    fd = handle_connect(argv[1],argv[2]);
    echo_client(fd);
    close(fd);



    //socket config
    /*int socket_fd = handle_connect(argv[1],argv[2]);
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

        if (fds[0].revents & POLLIN ) // check if data to read in socket
        {
            struct message mssg;
            char buffer[MSGLEN];
            memset(buffer,0,MSGLEN);
            
            ssize_t  received = recv(fds[0].fd,&mssg,sizeof(struct message),0);
            
            if(received>0)
            {
                printf("[ %s ] : %s\n ",mssg.nick_sender,buffer);
            }
            if (received <= 0)
            {
                printf("No connexion. Server closed :'(\n");
                run = 0;
                break;
            }
            

        }

        if (fds[1].revents & POLLIN) // check if data to read from standard input, if so : send to the socket
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
                char nickname[NICK_LEN];
			    msg = what_to_send(buffer,nickname);
                ssize_t sent = send(fds[0].fd,&msg,sizeof(struct message),0);
                if (sent == -1)
                {
                    perror("send!");
                    break;
                }
                printf("message sent!!\n");
            }   
        }
    }
    //close socket
    close(fds[0].fd);

    exit(EXIT_SUCCESS);*/
    exit(EXIT_SUCCESS);
}