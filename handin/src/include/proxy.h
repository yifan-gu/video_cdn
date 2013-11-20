#ifndef _PROXY_H
#define _PROXY_H

#include <netinet/in.h>
#include <client.h>
#include <server.h>
#include <time.h>

#define BITRATE_MAXNUM 1024

typedef struct _Proxy {
    float alpha;
    float avg_tput;
    float tput;

    int listenfd;
    int maxfd;

    int bps[BITRATE_MAXNUM];
    int bps_len;

    Client client;
    struct sockaddr_in myaddr;
    struct sockaddr_in toaddr;
    Server server;

    unsigned long ts;
    FILE *fp;
} Proxy;

int proxy_conn_server(const char *local_ip, const char * server_ip);
int proxy_reconnect_server();
int proxy_start_listen(const char *port);
int dump_proxy_info(Proxy *p);
#endif // for #ifndef _PROXY_H
