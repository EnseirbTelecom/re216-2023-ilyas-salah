#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Structure pour stocker struct sockaddr
struct SockAddrNode {
    struct sockaddr_in addr;
    struct SockAddrNode* next;
};

// Fonction pour créer un nouveau nœud avec struct sockaddr
struct SockAddrNode* createSockAddrNode(struct sockaddr_in addr) {
    struct SockAddrNode* newNode = (struct SockAddrNode*)malloc(sizeof(struct SockAddrNode));
    if (newNode == NULL) {
        fprintf(stderr, "Erreur : Échec de l'allocation mémoire\n");
        exit(1);
    }
    newNode->addr = addr;
    newNode->next = NULL;
    return newNode;
}

// Fonction pour ajouter un nœud à la fin de la liste chaînée
void appendSockAddrNode(struct SockAddrNode** head, struct sockaddr_in addr) {
    struct SockAddrNode* newNode = createSockAddrNode(addr);
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
    while (current != NULL) {
        printf("Port: %d\n", ntohs(current->addr.sin_port));
        printf("Adresse IP: %s\n", inet_ntoa(current->addr.sin_addr));
        printf("\n");
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
/*
int main() {
    struct SockAddrNode* addrList = NULL;

    // Ajouter des adresses struct sockaddr à la liste chaînée
    struct sockaddr_in addr1, addr2;
    memset(&addr1, 0, sizeof(addr1));
    memset(&addr2, 0, sizeof(addr2));

    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(8080);
    inet_pton(AF_INET, "192.168.1.1", &addr1.sin_addr);

    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(9090);
    inet_pton(AF_INET, "10.0.0.1", &addr2.sin_addr);

    appendSockAddrNode(&addrList, addr1);
    appendSockAddrNode(&addrList, addr2);

    // Afficher la liste chaînée
    printf("Liste d'adresses struct sockaddr :\n");
    displaySockAddrList(addrList);

    // Libérer la mémoire
    freeSockAddrList(addrList);

    return 0;
}
*/