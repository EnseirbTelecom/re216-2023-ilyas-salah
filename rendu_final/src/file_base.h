#ifndef FILE_BASE_H
#define FILE_BASE_H
#include "common.h"

char* getFileName(char* filePath) {
    //find last sperator '/'
    char *fileName = strrchr(filePath, '/');
    
    // if no separator found, return the full path
    if (!fileName)
        return filePath;
    
    
    return fileName + 1;
}

void loadingBar(int total) {
    int i;
    printf("Loading: ");
    for (i = 0; i <= total; ++i) {
        printf("%c[2K", 27); 
        printf("\r["); 
        for (int j = 0; j < 50; j++) {
            if (j <= (i * 50) / total) {
                printf(YELLOW"#"RESET); 
            } else {
                printf(" ");
            }
        }
        printf("] %d%%", (i * 100) / total); 
        fflush(stdout);
        usleep(10000); // sleep for 10 milliseconds (0.01 second)
    }
    printf("\n");
}
void send_file(FILE *fp, int sockfd,char *file_name,char *nick)
{
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Invalid file pointer.\n");
        return;
    }

    char buffer[FILE_LEN];
    size_t bytesRead;

    // sending stuct msg FILE SEND
    struct message msg;
    memset(&msg,'\0',sizeof(struct message));
    strcpy(msg.infos,file_name);
    msg.type = FILE_SEND;
    msg.pld_len = FILE_LEN;
    strcpy(msg.nick_sender,nick);
    ssize_t sent_structure = send(sockfd,&msg,sizeof(struct message),0);
    if (sent_structure == -1)
    {
        perror("send");
    }
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

void write_file(int sockfd,char *filename)
{
    int n;
    FILE *fp;
    char buffer[FILE_LEN];

    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("[-]Error opening file");
        return;
    }

    // RECV STRUCT MESSAGE

    while (1)
    {
        n = recv(sockfd, buffer, FILE_LEN, 0);
        if (n <= 0)
        {
            if (n < 0)
                perror("Error receiving data");

            break;
        }

        fwrite(buffer, 1, n, fp);

        // if (n < FILE_LEN)
        //     break;
    }

    fclose(fp);
    printf("[+]File received successfully.\n");
}

void client_sender(char *file_path,char *IP,char *nick)
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
    printf("server ip set to %s",IP);
	e = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (e == -1)
	{
		perror("[-]Error in socket");
		exit(1);
	}

	printf("[+]Connected to Server.\n");

	fp = fopen(file_path, "r");
    printf("opened file path:%s\n reading %i",file_path,FILE_LEN);
	if (fp == NULL)
	{
		perror("[-]Error in reading file.");
		exit(1);
	}

	send_file(fp, sockfd,getFileName(file_path),nick);
    loadingBar(100);
	printf("[+]File data sent successfully.\n");
	printf("[+]Closing the connection.\n");
	close(sockfd);
    close(e);
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
	server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

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


    struct message mssg;
    memset(&mssg,0,sizeof(struct message));
    recv(new_sock,&mssg,sizeof(struct message),0);
    if (mssg.type == FILE_SEND)
    {
        printf("Receiving the file from %s...",mssg.nick_sender);
        write_file(new_sock,mssg.infos);
        loadingBar(100);
	    printf("[+]Data written in the file successfully.\n");
	    close(sockfd);
	    close(new_sock);
    }
	
}



#endif