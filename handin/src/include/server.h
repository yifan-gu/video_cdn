#ifndef _SERVER_H
#define _SERVER_H

#define SRVBUF_SIZE 8192

enum server_st {
    SRV_ST_STLINE,
    SRV_ST_HEADER,
    SRV_ST_BODY,
    SRV_ST_FINISH
};
    
typedef struct _Server{
    enum server_st state;
    int fd;

    char buf[SRVBUF_SIZE];
    int closed;
    //int buf_num;
} Server;


int init_server(Server *);
int handle_server();

#endif // for #ifndef _SERVER_H
