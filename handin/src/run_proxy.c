#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <proxy.h>
#include <run_proxy.h>
#include <client.h>
#include <server.h>
#include <util.h>

extern Proxy proxy;

void run_proxy() {
    int nfds;
    fd_set readfds;

    proxy.maxfd = MAX(proxy.listenfd, proxy.server.fd);
    init_client(&proxy.client);
    init_server(&proxy.server);

    while(1) {
        FD_SET(proxy.listenfd, &readfds);
        FD_SET(proxy.server.fd, &readfds);
        if(proxy.client.fd){
            FD_SET(proxy.client.fd, &readfds);
        }
        // select
        nfds = select(
                   proxy.maxfd + 1,
                   &readfds,
                   NULL, NULL, NULL );
        if(nfds > 0) {
            if(FD_ISSET(proxy.listenfd, &readfds)){
                proxy.client.fd = accept(proxy.listenfd, NULL, 0);
                proxy.maxfd = MAX(proxy.maxfd, proxy.client.fd);
            }

            if(FD_ISSET(proxy.client.fd, &readfds)){
                handle_client();
            }
            if(FD_ISSET(proxy.server.fd, &readfds)){
                handle_server();
            }
        }
    }
}
