#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <proxy.h>
#include <run_proxy.h>

extern Proxy proxy;

void run_proxy() {
    int n;
    int nfds;
    fd_set readfds;
    char buf[4096];

    proxy.maxfd = proxy.connfd;
    if(proxy.listenfd > proxy.maxfd)
        proxy.maxfd = proxy.listenfd;

    while(1) {
        FD_SET(proxy.listenfd, &readfds);
        FD_SET(proxy.connfd, &readfds);
        if(proxy.clientfd)
            FD_SET(proxy.clientfd, &readfds);
        // select
        nfds = select(
                  proxy.maxfd + 1,
                   &readfds,
                   NULL, NULL, NULL );
        if(nfds > 0) {
            if(FD_ISSET(proxy.listenfd, &readfds)){
                if(proxy.clientfd == 0){
                    proxy.clientfd = accept(proxy.listenfd, NULL, 0);
                    if(proxy.clientfd > proxy.maxfd)
                        proxy.maxfd = proxy.clientfd;
                }
            }
            if(FD_ISSET(proxy.connfd, &readfds)){
                n = read(proxy.connfd, buf, 4096);
                write(proxy.clientfd, buf, n);
            }
            if(FD_ISSET(proxy.clientfd, &readfds)){
                n = read(proxy.clientfd, buf, 4096);
                write(proxy.connfd, buf, n);
            }
        }
    }
}
