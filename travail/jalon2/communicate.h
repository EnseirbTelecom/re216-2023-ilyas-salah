#include "common.h"
#include "liste_chaine.h"


int receive_msg(int fd,struct message *msg,char *buff)
{
    memset(msg, 0, sizeof(struct message));
    memset(buff, 0, MSGLEN);

    // Receiving structure
    if (recv(fd, msg, sizeof(struct message), 0) <= 0) {
        printf("error while receiving struct msg\n");
        return -1;
    }

    printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s\n", msg->pld_len, msg->nick_sender, msg_type_str[msg->type], msg->infos);

    // Receiving message
    if (recv(fd,buff, msg->pld_len, 0) <= 0) {
        printf("error while receiving msg utile\n");
        return -1;
    }

    printf("Received: %s\n", buff);

    return 1;
}

int send_msg(int fd,struct message *msg,char *buff)
{

    // Sending structure (ECHO)
    printf("pld_len: %i / nick_sender: %s / type: %s / infos: %s\n", msg->pld_len, msg->nick_sender, msg_type_str[msg->type], msg->infos);

    if (send(fd, msg, sizeof(struct message),0) <= 0) {
        printf("error while sending struct msg\n");
        
        return -1;
    }
    printf("%i\n",msg->type);
    // Sending message (ECHO)
    if (send(fd, buff, msg->pld_len, 0) <= 0) {
        printf("error while sending msg utile\n");
        return -1;
    }
    printf("Message sent: %s\n",buff);
    return 1;
}

int send_echo(int fd,char *message,char *sender)
{
    struct message msg;
    memset(&msg, 0, sizeof(struct message));
    msg.pld_len = strlen(message);
    msg.type = ECHO_SEND;
    strcpy(msg.nick_sender,sender);
    strncpy(msg.infos, "\0", 1);
    int res = send_msg(fd,&msg,message);
    return res;
}



int verify_nickname(char *nickname,struct SockAddrNode *head) // 3 code erreur, 0 si tres long -1 si nickname contient des caratere speciaux, -2 si nickname deja atribbue , -
{
    char *n_copy = nickname;
    while (*n_copy) {
        if (!isalnum(*n_copy)) {
            printf("contains special character\n");
            return -1; 
        }
        n_copy++;
    }
    if (n_copy - nickname >= NICK_LEN)
    {
        printf("too long!\n");
        return 0;
    }
    
    struct SockAddrNode* current = head;
    while (current != NULL)
    {
        if (strcmp(current->nickname,nickname) == 0)
        {
            printf("username already taken %s\n",current->nickname);
            return -2;
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
    //current = current->next;

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

int broadcast(char *sender,char *message_utile,struct SockAddrNode *head )
{
    struct SockAddrNode* current = head;
    current = current->next;

    while (current != NULL)
    {
        if (strcmp(current->nickname,sender) != 0)
        {
           if(send_echo(current->fd,message_utile,sender)<=0)
            return -1; 
        }
        
        current = current->next; 
    }
    return 1;
}

int get_info_about_user(char *nickname,char *buff,struct SockAddrNode *head)
{
    struct SockAddrNode* current = head;
    //current = current->next;

    while (current != NULL)
    {
        if (strcmp(current->nickname,nickname) == 0)
        {
            strcat(buff,current->nickname);
            strcat(buff," connected with IP address ");
            strcat(buff,inet_ntoa(current->addr.sin_addr));
            strcat(buff," and port number ");
            char port[6] = {0};
            sprintf(port, "%d", ntohs(current->addr.sin_port));
            strcat(buff,port);
            strcat(buff,"\n");
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

int handle_request(int fd,struct message *msg,char *buff,struct SockAddrNode *head) 
{
    //printf("%s\n",token);
    if (msg->type == NICKNAME_NEW)
    {
        int res = verify_nickname(msg->infos,head);
        if (res == -1)
        {
            char *response = "Please try again using ONLY alphabets and numbers\n\0";
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        if (res == -2)
        {
            char *response = "Username already in use, please try another one\n\0";
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        if (res == 1)
        {
            printf("Valid nickname!\n");
            if(store_nickname(msg->infos,head,fd)<= 0)
                return -1;
            char response[MSGLEN] = "Welcome on the chat ";
            strcat(response,msg->infos);
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }  
    }
    
    if (msg->type == NICKNAME_LIST)
    {
        char response[MSGLEN] = "Online users are \n";
        all_users_name(response,head);
        if(send_echo(fd,response,"Server")<= 0)
        {
            return -1;
        }
    }

    if (msg->type == NICKNAME_INFOS)
    {
        char response[MSGLEN];
        memset(response,'\0',MSGLEN);
        int res = get_info_about_user(msg->infos,response,head);
        if (res == -1)
        {
            strcpy(response,"user does not exist :/ , type /who to see current users\n");
        }
        
        if(send_echo(fd,response,"Server")<= 0)
        {
            return -1;
        }
    }
    if (msg->type == UNICAST_SEND)
    {
        char response[MSGLEN];
        memset(response,'\0',MSGLEN);

        int fd_receiver = fd_from_username(head,msg->infos);
        printf("%s\n",buff);
        if (fd_receiver < 0)
        {
            strcpy(response,"The user you're trying to send to communicate with does not exist.\n");
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        else if(send_echo(fd_receiver,buff,msg->nick_sender)<= 0)
        {
            return -1;
        }

    }
    if (msg->type == BROADCAST_SEND)
    {
        if (broadcast(msg->nick_sender,buff,head) <= 0)
        {
            return -1;
        }
        
    }
    return 0;
    
}

