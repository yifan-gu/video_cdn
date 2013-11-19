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

    proxy.maxfd = MAX(proxy.listenfd, proxy.connfd);

    while(1) {
        FD_SET(proxy.listenfd, &readfds);
        FD_SET(proxy.connfd, &readfds);
        if(proxy.clientfd){
            FD_SET(proxy.clientfd, &readfds);
        }
        // select
        nfds = select(
                   proxy.maxfd + 1,
                   &readfds,
                   NULL, NULL, NULL );
        if(nfds > 0) {
            if(FD_ISSET(proxy.listenfd, &readfds)){
                proxy.clientfd = accept(proxy.listenfd, NULL, 0);
                proxy.maxfd = MAX(proxy.maxfd, proxy.clientfd);
            }

            if(FD_ISSET(proxy.clientfd, &readfds)){
                handle_client();
            }
            if(FD_ISSET(proxy.connfd, &readfds)){
                handle_server();
            }
        }
    }
}
