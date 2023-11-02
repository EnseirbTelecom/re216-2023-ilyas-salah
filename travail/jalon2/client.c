#include "common.h"
#include "communicate.h"

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
        printf("enter a server address and port\n");
        exit(EXIT_FAILURE);
    }
    //socket config
    int socket_fd = handle_connect(argv[1],argv[2]);
    printf("Connected to server \n");

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
    
    char buffer[MSGLEN];
    struct message msgstruct;

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
            
            int  received = receive_msg(fds[0].fd,&msgstruct,buffer);
            
            if(received>0)
            {
            printf("[Server] : %s\n ",buffer);
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
            
            memset(&msgstruct, 0, sizeof(struct message));
            memset(buffer,0,MSGLEN);

            ssize_t  received = read(fds[1].fd,buffer,MSGLEN);
            
            if(received>=0)
            {
                if(strcmp(buffer,"/quit\n")==0)
                {
                    run = 0;
                    break;
                }
                msgstruct.pld_len = strlen(buffer);
                msgstruct.type = NICKNAME_LIST;
                strncpy(msgstruct.infos,"toto\0",6); 
                strcpy(msgstruct.nick_sender,"tit1\0");
                
                int sent = send_msg(fds[0].fd,&msgstruct,buffer);
                if (sent == -1)
                {
                    printf("error while sending\n");
                    break;
                }
                printf("message sent!!\n");
            }   
        }
    }
    //close socket
    close(fds[0].fd);

    exit(EXIT_SUCCESS);
    
    return 0;
}