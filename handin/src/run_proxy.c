#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <proxy.h>
#include <run_proxy.h>
#include <client.h>
#include <server.h>
#include <util.h>
#include <logger.h>

extern Proxy proxy;

void run_proxy() {
    int nfds;
    fd_set readfds;

    proxy.maxfd = MAX(proxy.listenfd, proxy.server.fd);
    proxy.client.fd = 0;
    init_client(&proxy.client);
    init_server(&proxy.server);
    
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(proxy.listenfd, &readfds);
        if(proxy.server.fd){
            FD_SET(proxy.server.fd, &readfds);
        }
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
                proxy.client.addrlen = sizeof(proxy.client.addr);
                proxy.client.fd = accept(proxy.listenfd, (struct sockaddr *) &proxy.client.addr,
                                         &proxy.client.addrlen);

                proxy.maxfd = MAX(proxy.maxfd, proxy.client.fd);
                proxy.client.state = REQ_LINE;
                proxy.client.buf_num = 0;

                if(proxy.server.fd){
                    close(proxy.server.fd);
                    proxy.maxfd = MAX(proxy.listenfd, proxy.client.fd);
                    proxy_reconnect_server();
                }
            }

            if( proxy.client.fd && FD_ISSET(proxy.client.fd, &readfds) ){
                handle_client();
            }
            if( proxy.server.fd && FD_ISSET(proxy.server.fd, &readfds) ){
                handle_server();
                if(proxy.server.closed){
                    logger(LOG_DEBUG, "server connection close");
                    close(proxy.server.fd);
                    close(proxy.client.fd); // release client fd
                    proxy.client.fd = 0;
                    proxy.server.fd = 0;
                    proxy.maxfd = proxy.listenfd;
                    // we can't do anything without the server
                    proxy_reconnect_server();
                }
            }
        }
    }
}
