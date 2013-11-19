#ifndef _SERVER_H
#define _SERVER_H

#define SRVBUF_SIZE 8192

typedef struct _Server{
    int fd;

    char buf[SRVBUF_SIZE];
    int buf_num;
} Server;


int init_server(Server *);
int handle_server();

#endif // for #ifndef _SERVER_H
