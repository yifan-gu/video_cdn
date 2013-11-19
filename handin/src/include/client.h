#ifndef _CLIENT_H
#define _CLIENT_H

enum CLI_STATE {
    REQ_LINE,
    REQ_DONE
};

typedef struct _Client {
    enum CLI_STATE state;
    int fd;
} Client;

int init_client(Client *cli);
int handle_client();

#endif // for #ifndef _CLIENT_H
