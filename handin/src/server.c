#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <server.h>
#include <proxy.h>

#define BUF_SIZE 8192

extern Proxy proxy;

int init_server(Server *srv){
    return 0;
}

int handle_server(){
    int n;
    char buf[BUF_SIZE];
    n = recv(proxy.server.fd, buf, BUF_SIZE, 0);
    send(proxy.client.fd, buf, n, 0);
    return 0;
}
