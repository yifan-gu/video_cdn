#ifndef _PROXY_H
#define _PROXY_H

#define MAX_CONN 1024
#define BITRATE_MAXNUM 1024

#include <client.h>
#include <server.h>

typedef struct _Proxy {
  float alpha;

  int listenfd;
  int maxfd;

  int bps[BITRATE_MAXNUM];
  int bps_len;

  Client client;
  Server server;

} Proxy;

int proxy_conn_server(const char *local_ip, const char * server_ip);
int proxy_start_listen(const char *port);

#endif // for #ifndef _PROXY_H
