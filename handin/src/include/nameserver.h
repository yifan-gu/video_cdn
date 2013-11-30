#ifndef _NAMESERVER_H
#define _NAMESERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

typedef struct _NameServer {
    int rr;
    struct sockaddr_in addr;
} NameServer;

int parse_argument(int argc, const char *argv[]);

#endif // for #ifndef _NAMESERVER_H
