#ifndef _SERVER_H
#define _SERVER_H

typedef struct _Server{
    int fd;
} Server;


int init_server(Server *);
int handle_server();

#endif // for #ifndef _SERVER_H
