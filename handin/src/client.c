#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <client.h>
#include <util.h>
#include <proxy.h>

#define BUF_SIZE 8192

extern Proxy proxy;

int init_client(Client *cli){
    cli->state = REQ_LINE;
    cli->fd = 0;
    return 0;
}

int handle_client(){
    int n;
    char buf[BUF_SIZE];
    n = recv(proxy.client.fd, buf, BUF_SIZE, 0);
    send(proxy.server.fd, buf, n, 0);
  return 0;
}
