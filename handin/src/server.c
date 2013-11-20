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
#include "util.h"

extern Proxy proxy;

/**
 * update the throughput
 */
static int update_tput(Proxy *p, int len) {
    unsigned int delta = get_timestamp_now() - p->ts;
    if (0 == delta) { // avoid inf
        delta = 1;
    }

    //TODO bps or Bps??
    p->tput = (float)len / ((float)delta / 1000); // from milliseconds to seconds
    printf("p->tput %f, delta %d len %d\n", p->tput, delta, len);
    p->avg_tput = p->alpha * p->tput + (1 - p->alpha) * p->avg_tput;

    return 0;
}

/**
 * parse Content-Length,
 * modify Connection
 */
static int parse_length(char *begin) {
    const char *content_len = "Content-Length:";
    
    char *ptr;
    int len = 0;
    
    if ((ptr = strstr(begin, content_len)) != NULL) {
        len = atoi(ptr+strlen(content_len));
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
    char *st_tail, *hdr_tail;
    static int content_len;
    static int already_len = 0;

    Server *s = &proxy.server;
    Client *c = &proxy.client;

    n = recv(s->fd, s->buf, SRVBUF_SIZE, 0);
    
    //dump_proxy_info(&proxy);
    //logger(LOG_DEBUG, "already len: %d, content len: %d", already_len, content_len);
    //logger(LOG_DEBUG, "fd: %d %d %d", proxy.listenfd, s->fd, c->fd);
    if (n == 0) {
        logger(LOG_DEBUG, "connection close");
        s->close = 1;
        return n;
    }

    if (n < 0) {
        logger(LOG_ERROR, "recv() failed");
        return n;
    }
    
    switch (s->state) {
    case SRV_ST_STLINE:
        if ((st_tail = strstr(s->buf, "\r\n")) != NULL) {
            s->state = SRV_ST_HEADER;
        } else {
            break;
        }
        
    case SRV_ST_HEADER:
        n = change_connection(s->buf, n);
        content_len = parse_length(st_tail+strlen("\r\n"));
        if ((hdr_tail = strstr(st_tail+strlen("\r\n"), "\r\n\r\n")) != NULL) {
            //debug
            //memset(tmp, 0, 8192);
            //strncpy(tmp, st_tail+strlen("\r\n"), hdr_tail - st_tail - strlen("\r\n"));
            //logger(LOG_DEBUG, "hdder %s", tmp);
            //debug end
            s->state = SRV_ST_BODY;
        } else {
            //logger(LOG_DEBUG, "edder %s", s->buf);
            break;
        }

    case SRV_ST_BODY:
        if (content_len == 0) {
            logger(LOG_WARN, "no Content-Length found");
            s->state = SRV_ST_STLINE;
            break;
        }
        
        if (0 == already_len) {
            already_len = ((s->buf + n) - (hdr_tail+strlen("\r\n\r\n")));
        } else {
            already_len += n;
        }
        
        if (already_len == content_len) {
            update_tput(&proxy, content_len);
            s->state = SRV_ST_FINISH;
            already_len = 0;
        }
    default:
        break;
    }
        
    if ((n = send(c->fd, s->buf, n, 0) < 0)) {
        logger(LOG_ERROR, "send() failed");
        return -1;
    };

    //logger(LOG_DEBUG, "%s", s->buf);
    return n;
}
