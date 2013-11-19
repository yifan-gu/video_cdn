#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <client.h>
#include <util.h>
#include <proxy.h>


extern Proxy proxy;

int init_client(Client *cli) {
    cli->state = REQ_LINE;
    cli->fd = 0;
    cli->buf_num = 0;
    return 0;
}

int handle_client() {
    int n;

    n = recv(proxy.client.fd, proxy.client.buf, CLIBUF_SIZE, 0);
    send(proxy.server.fd, proxy.client.buf, n, 0);

    switch(proxy.client.state) {
    case REQ_LINE:
        n = recv( proxy.client.fd,
                  &proxy.client.buf[proxy.client.buf_num],
                  1,
                  0);
        if(n > 0){
            proxy.client.buf_num ++;
            if( str_endwith(proxy.client.buf, proxy.client.buf_num, "\r\n", 2) ){
            }
        }
        break;
    case REQ_DONE:
        break;
    }
    return 0;
}
