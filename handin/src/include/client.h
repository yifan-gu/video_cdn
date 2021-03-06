#ifndef _CLIENT_H
#define _CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CLIBUF_SIZE 8192

enum CLI_STATE {
    REQ_LINE,
    REQ_DONE
};

typedef struct _Client {
    enum CLI_STATE state;
    int fd;
    struct sockaddr_in addr;
    socklen_t addrlen;

    char buf[CLIBUF_SIZE];
    int buf_num;

    int get_chunk;
} Client;

int init_client(Client *cli);
int handle_client();

#endif // for #ifndef _CLIENT_H
