#include "common.h"
#include "channels.h"


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

int send_channel_quit(int fd,char *message,char *sender)
{
    struct message msg;
    memset(&msg, 0, sizeof(struct message));
    msg.pld_len = strlen(message);
    msg.type = MULTICAST_QUIT;
    strcpy(msg.nick_sender,sender);
    strncpy(msg.infos, "\0", 1);
    int res = send_msg(fd,&msg,message);
    return res;
}

int send_channel_joined(int fd,char *message,char *sender,char *salon)
{
    struct message msg;
    memset(&msg, 0, sizeof(struct message));
    msg.pld_len = strlen(message);
    msg.type = MULTICAST_JOIN;
    strcpy(msg.nick_sender,sender);
    strcpy(msg.infos, salon);
    int res = send_msg(fd,&msg,message);
    return res;
}

int send_file_request(int fd,char *message,char *sender)
{
    struct message msg;
    memset(&msg, 0, sizeof(struct message));
    msg.pld_len = strlen(message);
    msg.type = FILE_REQUEST;
    strcpy(msg.nick_sender,sender);
    strncpy(msg.infos, "\0", 1);
    int res = send_msg(fd,&msg,message);
    return res;
}

int send_file_accept_reject(int fd,char *message,char *sender,int option)
{
    struct message msg;
    memset(&msg, 0, sizeof(struct message));
    msg.pld_len = strlen(message);
    if (option == 1)
    {
        msg.type = FILE_ACCEPT;
    }
    if (option == 0)
    {
        msg.type = FILE_REJECT;
    }
    strcpy(msg.nick_sender,sender);
    strncpy(msg.infos, "\0", 1);
    int res = send_msg(fd,&msg,message);
    return res;
}

int verify_nickname(int fd,char *nickname,struct SockAddrNode *head) // 3 code erreur, -2 si tres long -1 si nickname contient des caratere speciaux, 0 si nickname deja atribbue ,
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
        return -2;
    }
    
    struct SockAddrNode* current = head;
    while (current != NULL)
    {
        if (strcmp(current->nickname,nickname) == 0 && current->fd != fd)
        {
            printf("username already taken %s\n",current->nickname);
            return 0;
        }
        current = current->next; 
    }
    return 1;
}

int verify_channel_name(int fd,char *channel_name,struct channel_node *head) // 3 code erreur, -2 si tres long -1 si channel_name contient des caratere speciaux, 0 si channel_name deja atribbue ,
{
    char *n_copy = channel_name;
    while (*n_copy) {
        if (!isalnum(*n_copy)) {
            printf("contains special character\n");
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
            printf("Channel %s already exists \n",current->name);
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
    current = current->next;

    while (current != NULL)
    {
        if (strcmp(current->nickname,nickname) == 0)
        {
            strcat(buff,current->nickname);
            strcat(buff," since ");
            strcat(buff,current->time);
            strcat(buff," connected with IP address ");
            strcat(buff,inet_ntoa(current->addr.sin_addr));
            strcat(buff," and port number ");
            char port[6] = {0};
            sprintf(port, "%d", ntohs(current->addr.sin_port));
            strcat(buff,port);
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

int send_in_channel(char *sender,char *message_utile,struct channel_node* channels,char *channel_name,int fd) {
    struct channel_node* current = channels;
    //-1 if salon name does not exist, 1 if sucess , -2 if send_echo failed
    while (current != NULL)
    {
        if (strcmp(current->name,channel_name) == 0)
        {
           for (int i = 0; i < MAX_CLIENT_CHANNEL; i++)
           {
                if (current->clients[i] != NULL && current->clients[i]->fd != fd )
                {
                    if(send_echo(current->clients[i]->fd,message_utile,sender)<=0)
                        return -2;
                }
                
           }
           return 1;
           
        }
        
        current = current->next; 
    }
    return -1;
}

int all_channels_name(char *buff ,struct channel_node *head )
{
    struct channel_node* current = head;
    if (current == NULL)
    {
        strcpy(buff,"0 channels available to join right now\n");
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


int handle_request(int fd,struct message *msg,char *buff,struct SockAddrNode *head,struct channel_node **channels) 
{
    //printf("%s\n",token);
    if ((strcmp(msg->nick_sender,"") == 0 || verify_nickname(fd,msg->nick_sender,head) <= 0 )&& msg->type != NICKNAME_NEW)
    {
        if (send_echo(fd,"please login with /nick <your pseudo>\n","Server"))
        {
            return -1;
        }
        return 0;
    }
    
    if (msg->type == NICKNAME_NEW)
    {
        int res = verify_nickname(fd,msg->infos,head);
        if (res == -1)
        {
            char *response = "Please try again using ONLY alphabets and numbers\n\0";
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        if (res == 0)
        {
            char *response = "Username already in use, please try another one\n\0";
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        if (res == -2)
        {
            char *response = "Pfff Username too long!!! please make it short\n\0";
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
    if (msg->type == MULTICAST_CREATE)
    {
        
        int res = verify_channel_name(fd,msg->infos,*channels);
        if (res == -1)
        {
            char *response = "Please try again using ONLY alphabets and numbers\n\0";
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        if (res == 0)
        {
            char *response = "Channel already exists, please try another one name or join the existing channel using /join <channel_name>\n\0";
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        if (res == -2)
        {
            char *response = "Pfff Channel_name too long!!! please make it short\n\0";
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        if (res == 1)
        {
            struct SockAddrNode *client = get_client_node(head,fd);
            if (strcmp(client->salon,"") != 0)
            {
                quit_channel_node(channels,fd,client->salon);
            }
            
            printf("client name:%s\n",client->nickname);
            append_channel_node(channels,client,msg->infos);
            display_channels(*channels);
            printf("channel created successfully\n");

            char response[MSGLEN] = "You have created channel ";
            strcat(response,msg->infos);

            if(send_channel_joined(fd,response,"Server",msg->infos)<= 0)
            {
                return -1;
            } 
        }
        
    }
    if (msg->type == MULTICAST_QUIT)
    {
        printf("quit\n");
        int res = quit_channel_node(channels,fd,msg->infos);
        display_channels(*channels);
        // 1 if channel deleted , 2 if client deleted from channel
        if (res == 1)
        {
            char response[MSGLEN] = "You were the last user in this channel, ";
            strcat(response,msg->infos);
            strcat(response," has been destroyed");
            send_channel_quit(fd,response,"Server");
        }
        if (res == 2) {
            char response1[MSGLEN] = "You have left channel ";
            strcat(response1,msg->infos);
            send_channel_quit(fd,response1,"Server");
            char response2[MSGLEN];
            strcpy(response2,msg->nick_sender);
            strcat(response2," has left ");
            strcat(response2,msg->infos);
            send_in_channel("Server",response2,*channels,msg->infos,fd);
        }
        
    }
    if (msg->type == MULTICAST_SEND)
    {
        int res = send_in_channel(msg->nick_sender,buff,*channels,msg->infos,fd);
        if (res == -1)
        {
            if(send_echo(fd,"Try to join a channel first! type /channel_list to see available channels or type /create <channel_name> to create your own\n","Server")<= 0)
            {
                return -1;
            }
        }
        
    }
    if (msg->type == MULTICAST_LIST)
    {
        char response[MSGLEN] = "all channels available are \n";
        all_channels_name(response,*channels);
        if(send_echo(fd,response,"Server")<= 0)
        {
            return -1;
        }
    }
    if (msg->type == MULTICAST_JOIN)
    {
        int does_exists = channel_exist(*channels,msg->infos);
        
        struct SockAddrNode *client = get_client_node(head,fd);

        if (does_exists == 1)
        {
            char salon_name[CHANEL_LEN];
            strcpy(salon_name,client->salon);
            int res = quit_channel_node(channels,fd,client->salon);
            // 1 if channel deleted , 2 if client deleted from channel
            if (res == 1)
            {
                char response[MSGLEN] = "You were the last user in this channel, ";
                strcat(response,salon_name);
                strcat(response," has been destroyed");
                send_channel_quit(fd,response,"Server");
            }
            if (res == 2) {
                char response1[MSGLEN] = "You have left channel ";
                strcat(response1,salon_name);
                send_channel_quit(fd,response1,"Server");
                char response2[MSGLEN];
                strcpy(response2,msg->nick_sender);
                strcat(response2," has left ");
                strcat(response2,salon_name);
                send_in_channel("Server",response2,*channels,salon_name,fd);
            }
        }
        int res = join_channel(*channels,msg->infos,client);
        // 1 for success , -2 for number max of users in channel reached , -1 no such channel name.
        if (res == -2) {
            char reponse[MSGLEN] = "The Channel you are trying to join is Full!";
            send_echo(fd,reponse,"Server");
        }
        if (res == -1) {
            char reponse[MSGLEN] = "The channel you're trying to join does not exist , please try /channel_list to see available channels";
            send_echo(fd,reponse,"Server");
        }
        if (res == 1)
        {
            char response1[MSGLEN] = "You have joined channel ";
            strcat(response1,msg->infos);
            send_channel_joined(fd,response1,"Server",msg->infos);
            char response2[MSGLEN];
            strcpy(response2,msg->nick_sender);
            strcat(response2," has joined ");
            strcat(response2,msg->infos);
            send_in_channel("Server",response2,*channels,msg->infos,fd);

        }
        display_channels(*channels);

    }
    if (msg->type == FILE_REQUEST)
    {
        char response[MSGLEN];
        memset(response,'\0',MSGLEN);

        int fd_receiver = fd_from_username(head,msg->infos);
        printf("%s\n",buff);
        if (fd_receiver < 0)
        {
            strcpy(response,"The user you're trying to communicate with does not exist.\n");
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        else if(send_file_request(fd_receiver,buff,msg->nick_sender)<= 0)
        {
            return -1;
        }
        
    }
    if (msg->type == FILE_ACCEPT)
    {
        int fd_receiver = fd_from_username(head,msg->infos);
        struct SockAddrNode *client = get_client_node(head,fd);
        send_file_accept_reject(fd_receiver,inet_ntoa((*client).addr.sin_addr),msg->nick_sender,1);
    }
    if (msg->type == FILE_REJECT)
    {
        char response[MSGLEN];
        memset(response,'\0',MSGLEN);
        int fd_receiver = fd_from_username(head,msg->infos);
        if (fd_receiver < 0)
        {
            strcpy(response,"The user you're trying to communicate with does not exist.\n");
            if(send_echo(fd,response,"Server")<= 0)
            {
                return -1;
            }
        }
        else {
           send_file_accept_reject(fd_receiver,"---",msg->nick_sender,0); 
        }
        
    }
    

    return 0;
    
}

