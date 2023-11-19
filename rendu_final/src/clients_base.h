#ifndef CLIENT_BASE_H
#define CLIENT_BASE_H

#include "common.h"

// Structure pour stocker struct sockaddr
struct SockAddrNode {
    struct sockaddr_in addr;
    int fd;
    char nickname[NICK_LEN];
    struct SockAddrNode* next;
    char time[64];
    char salon[CHANEL_LEN];
};

// Fonction pour créer un nouveau nœud avec struct sockaddr
struct SockAddrNode* createSockAddrNode(struct sockaddr_in addr,int fd,char *time) {
    struct SockAddrNode* newNode = (struct SockAddrNode*)malloc(sizeof(struct SockAddrNode));
    if (newNode == NULL) {
        fprintf(stderr, "Erreur : Échec de l'allocation mémoire\n");
        exit(1);

    }
    newNode->addr = addr;
    newNode->next = NULL;
    newNode->fd = fd;
    strcpy(newNode->time,time);
    memset(newNode->nickname,'\0',NICK_LEN);
    memset(newNode->salon,'\0',CHANEL_LEN);
    return newNode;
}

// Fonction pour ajouter un nœud à la fin de la liste chaînée
void appendSockAddrNode(struct SockAddrNode** head, struct sockaddr_in addr,int fd,char *time) {
    struct SockAddrNode* newNode = createSockAddrNode(addr,fd,time);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    struct SockAddrNode* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

// Fonction pour afficher la liste chaînée de struct sockaddr
void displaySockAddrList(struct SockAddrNode* head) {
    struct SockAddrNode* current = head;
    if (current != NULL)
    {
        current = current->next;
    }
    
    while (current != NULL) {
        printf("Port: %d\n", ntohs(current->addr.sin_port));
        printf("Adresse IP: %s\n", inet_ntoa(current->addr.sin_addr));
        printf("file descriptor: %i and nickname: %s\n",current->fd,current->nickname);
        printf("\n");
        current = current->next;
    }
}

//supprimer le client qui est deconnectes 
void deleteSockNode_from_fd(struct SockAddrNode* head, int fd) {
    struct SockAddrNode* current = head;

    if (current != NULL && current->fd == fd) {
        // If the first node has the target fd, delete it
        struct SockAddrNode* next = current->next;
        free(current);
        head = next;
        return;
    }

    while (current != NULL && current->next != NULL) {
        if (current->next->fd == fd) {
            // If the next node has the target fd, delete it
            struct SockAddrNode* next = current->next->next;
            free(current->next);
            current->next = next;
            return;
        }
        current = current->next;
    }
}

struct SockAddrNode* get_client_node(struct SockAddrNode* head,int fd)
{
    struct SockAddrNode *current = head;
    while(current != NULL) {
        if (current->fd == fd)
        {
            return current;
        }
        current = current->next;
        
    }
    return NULL;
}


int verify_nickname(int fd,char *nickname,struct SockAddrNode *head) // 3 code erreur, -2 si tres long -1 si nickname contient des caratere speciaux, 0 si nickname deja atribbue ,
{
    char *n_copy = nickname;
    while (*n_copy) {
        if (!isalnum(*n_copy)) {
            printf(RED"contains special character"RESET"\n");
            return -1; 
        }
        n_copy++;
    }
    if (n_copy - nickname >= NICK_LEN)
    {
        printf(RED"too long!"RESET"\n");
        return -2;
    }
    
    struct SockAddrNode* current = head;
    while (current != NULL)
    {
        if (strcmp(current->nickname,nickname) == 0 && current->fd != fd)
        {
            printf(RED"username already taken %s"RESET"\n",current->nickname);
            return 0;
        }
        current = current->next; 
    }
    return 1;
}

int store_nickname(char *nickname,struct SockAddrNode *head,int fd)
{
    struct SockAddrNode* current = head;
    while (current != NULL)
    {
        if (current->fd == fd)
        {
            strcpy(current->nickname,nickname);
            printf("username of fd %i set to: %s\n",current->fd,current->nickname);
            return 1;
        }
        current = current->next; 
    }
    return -1;  
}

int all_users_name(char *buff ,struct SockAddrNode *head )
{
    struct SockAddrNode* current = head;
    current = current->next;

    while (current != NULL)
    {
        if (strcmp(current->nickname,"\0") != 0)
        {
            strcat(buff,"\t\t\t - ");
            strcat(buff,current->nickname);
            strcat(buff,"\n");
        }
        current = current->next; 
    }
    return 1;
}



int get_info_about_user(char *nickname,char *buff,struct SockAddrNode *head)
{
    struct SockAddrNode* current = head;
    current = current->next;

    while (current != NULL)
    {
        if (strcmp(current->nickname,nickname) == 0)
        {
            strcat(buff,MAGENTA);
            strcat(buff,current->nickname);
            strcat(buff,RESET);
            strcat(buff," connected since ");
            strcat(buff,MAGENTA);
            strcat(buff,current->time);
            strcat(buff,RESET);
            strcat(buff," with IP address ");
            strcat(buff,MAGENTA);
            strcat(buff,inet_ntoa(current->addr.sin_addr));
            strcat(buff,RESET);
            strcat(buff," and port number ");
            strcat(buff,MAGENTA);
            char port[6] = {0};
            sprintf(port, "%d", ntohs(current->addr.sin_port));
            strcat(buff,port);
            strcat(buff,RESET);
            //strcat(buff,"\n");
            return 1;
        }
        current = current->next; 
    }
    return -1;
}

int fd_from_username(struct SockAddrNode* head,char *nickname){
    struct SockAddrNode* current = head;
    //current = current->next;

    while (current != NULL)
    {
        if (strcmp(current->nickname,nickname) == 0)
        {
            return current->fd;
        }
        current = current->next; 
    }
    return -1;
}


// Fonction pour libérer la mémoire de la liste chaînée
void freeSockAddrList(struct SockAddrNode* head) {
    struct SockAddrNode* current = head;
    while (current != NULL) {
        struct SockAddrNode* temp = current;
        current = current->next;
        free(temp);
    }
}

#endif