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
                if (SRV_ST_FINISH == proxy.server.state) {
                    if(proxy.client.fd){
                        close(proxy.client.fd); // release client fd
                    }
                    proxy.server.state = SRV_ST_STLINE;
                }

                proxy.client.fd = accept(proxy.listenfd, (struct sockaddr *) &proxy.client.addr,
                                         &proxy.client.addrlen);
                proxy.maxfd = MAX(proxy.maxfd, proxy.client.fd);
                proxy.client.state = REQ_LINE;
                proxy.client.buf_num = 0;
            }

            if( FD_ISSET(proxy.client.fd, &readfds) ){
                handle_client();
            }
            if( FD_ISSET(proxy.server.fd, &readfds) ){
                handle_server();
                if(proxy.server.closed){
                    // we can't do anything without the server
                    while(proxy_reconnect_server() < 0){
                        /*sleep(5);*/
                    }
                    logger(LOG_DEBUG, "server reconnect successfully.");
                }
            }
        }
    }
}
