#ifndef _PROXY_H
#define _PROXY_H

typedef struct _Proxy {
  float alpha;
  int listenfd;
  int connfd;
} Proxy;

int proxy_conn_server(const char * server_ip);
int proxy_start_listen(const char *local_ip, const char *port);

#endif // for #ifndef _PROXY_H
