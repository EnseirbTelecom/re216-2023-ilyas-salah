#include "common.h"

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
    }
    
    int socket_fd = handle_connect(argv[1],argv[2]);
    printf("Connected to server \n");

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
        if (fds[0].revents & POLLIN ) // check if data to read in socket from server
        {
            char buffer[MSGLEN];
            memset(buffer,0,MSGLEN);
            
            ssize_t  received = recv(fds[0].fd,buffer,MSGLEN,0);
            
            if(received>0)
            {
            printf("Response from server: %s\n ",buffer);
            }

        }

        if (fds[1].revents & POLLIN)
        {
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
                ssize_t sent = send(fds[0].fd,buffer,MSGLEN,0);
                if (sent == -1)
                {
                    perror("send!");
                    break;
                }
                printf("%li octets sent!!\n",sent);
            }   
        }
    }

    close(fds[0].fd);

    
    exit(EXIT_SUCCESS);
    
    return 0;
}
