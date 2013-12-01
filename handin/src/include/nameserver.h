#ifndef _NAMESERVER_H
#define _NAMESERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <node_map.h>

typedef struct _NameServer {
    // round robin counter:
    // if round robin is not set, this variable will be -1;
    // otherwise it's init to 0 and incremented circularly [0 .. servers_number-1]
    int rr;

    struct sockaddr_in addr;

    NodeMap nmap;
} NameServer;

int parse_argument(int argc, const char *argv[]);
int parse_servers(const char *);
int parse_lsa(const char *);

void print_graph();

#endif // for #ifndef _NAMESERVER_H
