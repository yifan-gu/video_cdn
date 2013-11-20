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
                //logger(LOG_DEBUG, "accept");
                if (SRV_ST_FINISH == proxy.server.state) {
                    close(proxy.client.fd); // release client fd
                    //logger(LOG_DEBUG, "close fd");
                    proxy.server.state = SRV_ST_STLINE;
                }
                
                proxy.client.fd = accept(proxy.listenfd, NULL, 0);
                proxy.maxfd = MAX(proxy.maxfd, proxy.client.fd);
            }

            if( FD_ISSET(proxy.client.fd, &readfds) ){
                proxy.ts = get_timestamp_now(); // update timestamp
                handle_client();
            }
            if( FD_ISSET(proxy.server.fd, &readfds) ){
                handle_server();
            }
        }
    }
}
