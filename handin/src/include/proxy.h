#ifndef _PROXY_H
#define _PROXY_H

#define MAX_CONN 1024
#define BITRATE_MAXNUM 1024

#include <client.h>
#include <server.h>

#define UPDATE_TPUT(p, len)                                             \
    ((p)->alpha * (float)(len) / (time(NULL) - (p)->ts) + (1 - (p)->alpha) * (p)->tput)

typedef struct _Proxy {
    float alpha;

    int listenfd;
    int maxfd;

    int bps[BITRATE_MAXNUM];
    int bps_len;

    Client client;
    Server server;

    time_t ts;
    int tput;
} Proxy;

int proxy_conn_server(const char *local_ip, const char * server_ip);
int proxy_start_listen(const char *port);
int dump_proxy_info(Proxy *p);
#endif // for #ifndef _PROXY_H
