#ifndef CHANNELS_BASE_H
#define CHANNELS_BASE_H

#include "clients_base.h"

// Structure pour stocker struct sockaddr
struct channel_node {
    char name[CHANEL_LEN];
    struct channel_node* next;
    struct SockAddrNode *clients[MAX_CLIENT_CHANNEL];
    int clients_left;
};
// Fonction pour afficher la liste chaînée de struct sockaddr
void display_channels(struct channel_node* head) {
    printf("All channels: \n");
    struct channel_node* current = head;
    
    while (current != NULL) {
        printf("channel name: %s || ", current->name);
        printf("clients available in channel %i: ",current->clients_left);
        for (int i = 0; i < MAX_CLIENT_CHANNEL; i++)
        {
            if (current->clients[i] != NULL)
            {
                printf("- %s\t",current->clients[i]->nickname);
            }
        }
        printf("\n");
        current = current->next;
    }
}

// Fonction pour créer un nouveau nœud avec struct sockaddr
struct channel_node* create_channel_node(struct SockAddrNode *client,char *name) {
    struct channel_node *newNode = (struct channel_node*)malloc(sizeof(struct channel_node));
    if (newNode == NULL) {
        fprintf(stderr, "Erreur : Échec de l'allocation mémoire\n");
        exit(1);

    }
    memset(newNode,0,sizeof(struct channel_node));
    memset(newNode->name,'\0',CHANEL_LEN);
    strcpy(newNode->name,name);

    for (int i = 0; i < MAX_CLIENT_CHANNEL; i++)
    {
        newNode->clients[i] = NULL;
    } 
    newNode->clients[0] = client;
    strcpy(newNode->clients[0]->salon,name);
    newNode->clients_left = 1;
    newNode->next = NULL;

    return newNode;
}

// Fonction pour ajouter un nœud à la fin de la liste chaînée
void append_channel_node(struct channel_node** head,struct SockAddrNode *client,char *name) {
    struct channel_node* newNode = create_channel_node(client,name);
    if (*head == NULL) {
        *head = newNode;

        return;
    }
    struct channel_node* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
    
}



//supprimer le client qui est deconnectes 
int quit_channel_node(struct channel_node** head, int fd_client , char *name)
{
    // 1 if channel deleted , 2 if client deleted from channel , 0
    struct channel_node* current = *head;

    if (current != NULL && strcmp(current->name,name) == 0 ) {
        
        printf("the first 1\n");
        for (int i = 0; i < MAX_CLIENT_CHANNEL; i++)
        {
            printf("the first2 %i\n",i);
            if (current->clients[i] != NULL && current->clients[i]->fd == fd_client)
            {
                strcpy(current->clients[i]->salon,"");
                printf("the first2\n");
                if (current->clients_left == 1)
                {
                    printf("the first\n");
                    struct channel_node * next = current->next;
                    free(current);
                    *head = next;
                    
                    display_channels(*head);
                    return 1;
                }
                printf("the first4\n");
                current->clients[i] = NULL;
                current->clients_left--;
                return 2;
            }
        }  
    }

    while (current != NULL && current->next != NULL) {
        if (strcmp(current->next->name,name) == 0) {
            // If the next node has the target fd, delete it
            
            for (int i = 0; i < MAX_CLIENT_CHANNEL; i++)
            {
                if (current->next->clients[i] != NULL && current->next->clients[i]->fd == fd_client)
                {
                    strcpy(current->next->clients[i]->salon,"");
                    if (current->next->clients_left <= 1)
                    {
                        struct channel_node* next = current->next->next;
                        free(current->next);
                        current->next = next;
                        printf("not the first \n");
                        return 1;
                    }
                    current->next->clients[i] = NULL;
                    current->next->clients_left--;
                    return 2;
                }
            }  
        }
        current = current->next;
    }
    return 0;
}
int verify_channel_name(int fd,char *channel_name,struct channel_node *head) // 3 code erreur, -2 si tres long -1 si channel_name contient des caratere speciaux, 0 si channel_name deja atribbue ,
{
    char *n_copy = channel_name;
    while (*n_copy) {
        if (!isalnum(*n_copy)) {
            printf(RED"contains special character"RESET"\n");
            return -1; 
        }
        n_copy++;
    }
    if (n_copy - channel_name >= CHANEL_LEN)
    {
        printf("too long!\n");
        return -2;
    }
    
    struct channel_node* current = head;
    while (current != NULL)
    {
        if (strcmp(current->name,channel_name) == 0)
        {
            printf(RED"Channel %s already exists"RESET" \n",current->name);
            return 0;
        }
        current = current->next; 
    }
    return 1;
}





int all_channels_name(char *buff ,struct channel_node *head )
{
    struct channel_node* current = head;
    if (current == NULL)
    {
        strcpy(buff,RED"0 channels available to join right now"RESET"\n");
        return 0;
    }
    

    while (current != NULL)
    {
        if (strcmp(current->name,"\0") != 0)
        {
            strcat(buff,"\t\t\t - ");
            strcat(buff,current->name);
            strcat(buff,"\n");
        }
        current = current->next; 
    }
    return 1;
}

int channel_exist(struct channel_node *head,char *channel_name)
{
    // 1 if channel exists , -1 if not , useful before joining a channel
    struct channel_node *current = head;
    while (current != NULL)
    {
        if (strcmp(current->name,channel_name) == 0)
        {
            return 1;
        }
        current = current->next;
        
    }
    return -1;
}

int join_channel(struct channel_node *head,char *channel_name,struct SockAddrNode *client)
{
    // 1 for success , -2 for number max of users in channel reached , -1 no such channel name.
    struct channel_node *current = head;
    while (current != NULL)
    {
        if (strcmp(current->name,channel_name) == 0)
        {
            for (int i = 0; i < MAX_CLIENT_CHANNEL; i++)
            {
                if (current->clients[i] == NULL)
                {

                    current->clients[i] = client;
                    current->clients_left ++;
                    strcpy(current->clients[i]->salon,channel_name);
                    return 1;
                }
                
            }
            return -2;
        }
        current = current->next;
        
    }
    return -1;
    
}

// Fonction pour libérer la mémoire de la liste chaînée
void freeChannelList(struct channel_node* head) {
    struct channel_node* current = head;
    while (current != NULL) {
        struct channel_node* temp = current;
        current = current->next;
        free(temp);
    }
}

#endif
