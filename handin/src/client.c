#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <client.h>
#include <util.h>
#include <proxy.h>
#include <logger.h>


extern Proxy proxy;

int init_client(Client *cli) {
    cli->state = REQ_LINE;
    cli->buf_num = 0;
    return 0;
}

int handle_client() {
    int n;

    switch(proxy.client.state) {
    case REQ_LINE:
        n = recv( proxy.client.fd,
                  & proxy.client.buf [proxy.client.buf_num],
                  1,
                  0);
        if(n > 0){
            proxy.client.buf_num ++;
            if( str_endwith(proxy.client.buf, proxy.client.buf_num, "\r\n", 2) ){
                //logger(LOG_DEBUG, "Before parse: %s", proxy.client.buf);
                parse_reqline(proxy.client.buf, &proxy.client.buf_num);
                //logger(LOG_DEBUG, "After parse:  %s", proxy.client.buf);

                proxy.client.state = REQ_DONE;
                send(proxy.server.fd, proxy.client.buf, proxy.client.buf_num, 0);
                proxy.client.buf_num = 0;
            }
        }
        /*else if(n == 0){*/
            /*// close connection*/
            /*logger(LOG_DEBUG, "close connection");*/
            /*close(proxy.client.fd); // release client fd*/
            /*proxy.client.fd = 0;*/
            /*proxy.maxfd = MAX(proxy.listenfd, proxy.server.fd);*/
        /*}*/
        break;

        // header
    case REQ_DONE:
        n = recv( proxy.client.fd,
                  & proxy.client.buf [proxy.client.buf_num],
                  1,
                  0);
        if(n > 0){
            proxy.client.buf_num ++;
            if( str_endwith(proxy.client.buf, proxy.client.buf_num, "\r\n", 2) ){
                proxy.client.buf[proxy.client.buf_num] = '\0';
                if(proxy.client.get_chunk &&
                  strstr(proxy.client.buf, "Connection")){
                    sprintf(proxy.client.buf, "Connection: close\r\n");
                    proxy.client.buf_num = strlen(proxy.client.buf);
                }
                send(proxy.server.fd, proxy.client.buf, proxy.client.buf_num, 0);
                if(proxy.client.buf_num == 2){
                    // finished sending one http request
                    proxy.client.state = REQ_LINE;
                    if(proxy.client.get_chunk){
                        proxy.ts = get_timestamp_now(); // update timestamp
                    }
                }
                proxy.client.buf_num = 0;
            }
        }
        /*else if(n == 0){*/
            /*// close connection*/
            /*logger(LOG_DEBUG, "close connection");*/
            /*close(proxy.client.fd); // release client fd*/
            /*proxy.client.fd = 0;*/
            /*proxy.maxfd = MAX(proxy.listenfd, proxy.server.fd);*/
        /*}*/
        break;
    }
    return 0;
}
