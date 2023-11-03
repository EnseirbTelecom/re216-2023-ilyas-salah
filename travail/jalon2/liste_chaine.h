#include "common.h"
#include "msg_struct.h"
// Structure pour stocker struct sockaddr
struct SockAddrNode {
    struct sockaddr_in addr;
    int fd;
    char nickname[NICK_LEN];
    struct SockAddrNode* next;
};

// Fonction pour créer un nouveau nœud avec struct sockaddr
struct SockAddrNode* createSockAddrNode(struct sockaddr_in addr,int fd) {
    struct SockAddrNode* newNode = (struct SockAddrNode*)malloc(sizeof(struct SockAddrNode));
    if (newNode == NULL) {
        fprintf(stderr, "Erreur : Échec de l'allocation mémoire\n");
        exit(1);

    }
    newNode->addr = addr;
    newNode->next = NULL;
    newNode->fd = fd;
    memset(newNode->nickname,'\0',NICK_LEN);
    return newNode;
}

// Fonction pour ajouter un nœud à la fin de la liste chaînée
void appendSockAddrNode(struct SockAddrNode** head, struct sockaddr_in addr,int fd) {
    struct SockAddrNode* newNode = createSockAddrNode(addr,fd);
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



// Fonction pour libérer la mémoire de la liste chaînée
void freeSockAddrList(struct SockAddrNode* head) {
    struct SockAddrNode* current = head;
    while (current != NULL) {
        struct SockAddrNode* temp = current;
        current = current->next;
        free(temp);
    }
}
