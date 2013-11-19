#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <server.h>
#include <proxy.h>


extern Proxy proxy;

int init_server(Server *srv){
    srv->buf_num = 0;
    return 0;
}

int handle_server(){
    int n;
    n = recv(proxy.server.fd, proxy.server.buf, SRVBUF_SIZE, 0);
    send(proxy.client.fd, proxy.server.buf, n, 0);
    return 0;
}
