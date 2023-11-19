#ifndef HANDLE_MSG_H
#define HANDLE_MSG_H

#include "common.h"
#include "msg_struct.h"

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
    printf(GREEN"Message sent:"RESET CYAN"%s"RESET"\n",buff);
    return 1;
}

#endif