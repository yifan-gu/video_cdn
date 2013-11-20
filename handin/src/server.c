#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "server.h"
#include "proxy.h"
#include "logger.h"

extern Proxy proxy;

/**
 * now, only parse Content-Length,
 * do not modify Connetcion
 */
static int parse_headers(char *begin) {
    char *hdr = "Content-Length:";
    char *ptr;
    int len = 0;
    
    if ((ptr = strstr(begin, hdr)) != NULL) {
        len = atoi(ptr+strlen(hdr));
        proxy.tput = UPDATE_TPUT(&proxy, len);
    }

    return len;
}

int init_server(Server *srv){
    return 0;
}

/**
 * read response from server,
 * extract the content-length part
 */
int handle_server(){
    int n;
    char *ptr;
    static int content_len;
    static int already_len = 0;

    Server *s = &proxy.server;
    Client *c = &proxy.client;
    
    n = recv(s->fd, s->buf, SRVBUF_SIZE, 0);
    
    if (n == 0) {
        logger(LOG_DEBUG, "connection close");
        return n;
    }

    if (n < 0) {
        logger(LOG_ERROR, "recv() failed");
        perror("");
        return n;
    }
    
    switch (s->state) {
    case SRV_ST_STLINE:
        if ((ptr = strstr(s->buf, "\r\n")) != NULL) {
            s->state = SRV_ST_HEADER;
        } else {
            break;
        }
        
    case SRV_ST_HEADER:
        content_len = parse_headers(ptr+strlen("\r\n"));
        if ((ptr = strstr(ptr+strlen("\r\n"), "\r\n\r\n")) != NULL) {
            s->state = SRV_ST_BODY;
        } else {
            break;
        }

    case SRV_ST_BODY:
        already_len += ((s->buf + n) - (ptr+strlen("\r\n\r\n")));
        logger(LOG_DEBUG, "already len: %d",already_len);
        if (already_len >= content_len) {
            s->state = SRV_ST_STLINE;
        }
    }
        
    if (send(c->fd, s->buf, n, 0) < 0) {
        logger(LOG_ERROR, "send() failed");
        return -1;
    };


    return 0;
}
