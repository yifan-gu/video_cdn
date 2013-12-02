#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <logger.h>
#include <proxy.h>
#include <run_proxy.h>
#include <util.h>
#include <mydns.h>

#define SERVER_PORT 8080
#define BUNNY_FILE "big_buck_bunny.f4m"


const char *GET_BUNNY = "GET /vod/big_buck_bunny.f4m HTTP/1.1\r\n"
                        "Host: 127.0.0.1\r\n"
                        "\r\n";

Proxy proxy;

static int download_bunny();

#ifndef TESTING
int main(int argc, char const* argv[])
{
    // argv:
    //  1. log
    //  2. alpha
    //  3. listen port
    //  4. fake-ip
    //  5. dns ip
    //  6. dns port
    //  7. (optional) server ip

    // system logging
    if( init_log(NULL) < 0 ) {
        printf("Failed: Can't init logging\n");
        return 0;
    }

    parse_addr(& proxy.myaddr, argv[4], 0); // 0 -- random port

    if (argc < 8) {
        if( init_mydns(argv[5], atoi(argv[6]) ) < 0){
            logger(LOG_ERROR, "Failed: Init DNS");
            return -1;
        }
        if(dns_server_info("video.cs.cmu.edu") < 0){
            logger(LOG_ERROR, "Failed: DNS query");
            return -1;
        }
        //parse_addr(& proxy.toaddr, "0.0.0.0", SERVER_PORT);
    }
    else{
        parse_addr(& proxy.toaddr, argv[7], SERVER_PORT);
    }

    //return 0; // for testing DNS

    proxy.alpha = atof(argv[2]);
    proxy.tput = proxy.avg_tput = 512;

    init_activity_log(&proxy, argv[1]);

    logger(LOG_INFO, "Connecting to video server...");
    if( proxy_conn_server() < 0) {
        return 0;
    }

    if( proxy_start_listen(argv[3]) < 0 ) {
        return -1;
    }
    logger(LOG_INFO, "Proxy starts listening on port: %s", argv[3]);

    /*if( download_bunny() < 0) {*/
        /*return -1;*/
    /*}*/

    /*if( proxy_conn_server() < 0) {*/
        /*return 0;*/
    /*}*/

    proxy.bps[0] = 10;
    proxy.bps[1] = 100;
    proxy.bps[2] = 500;
    proxy.bps[3] = 1000;
    proxy.bps_len = 4;

    run_proxy();
    return 0;
}
#endif

/*
@reference:
getline, http://man7.org/linux/man-pages/man3/getline.3.html
*/
static int parse_bitrates() {
    int read;
    FILE *fp;
    int rate;
    char *line = NULL;
    size_t len = 0;
    int count = 0;
    char *ptr;
    int i,j;

    fp = fopen(BUNNY_FILE, "r");
    while ((read = getline(&line, &len, fp)) != -1) {
        ptr = strstr(line, "bitrate=\"");
        if(ptr == NULL)
            continue;
        if(sscanf(ptr, "bitrate=\"%d", &rate) == 1) {
            proxy.bps[count] = rate;
            count++;
            if(count == BITRATE_MAXNUM) {
                break;
            }
        }
    }
    free(line);
    fclose(fp);

    if(count == 0) {
        logger(LOG_WARN, "No bit rate parsed!");
        return -1;
    }

    // sort
    for (i = 0; i < count; i++) {
        for (j = i + 1; j < count; j++) {
            if(proxy.bps[i] > proxy.bps[j]) {
                SWAP(proxy.bps[i], proxy.bps[j]);
            }
        }
        logger(LOG_INFO, "bitrate %d: %d", i+1, proxy.bps[i]);
    }

    proxy.bps_len = count;
    return 0;
}

static int download_bunny() {
    int n;
    FILE *fp;
    char buf[4096];

    fp = fopen(BUNNY_FILE, "w");
    if(!fp) {
        return -1;
    }

    send(proxy.server.fd, GET_BUNNY, strlen(GET_BUNNY), 0);

    while(1) {
        n = read(proxy.server.fd, buf, 4096);
        if(n < 0) {
            if(errno != EINTR) {
                logger(LOG_ERROR, "Failed: Can't download bunny file:\n");
                return -1;
            }
        }
        else if (n == 0) {
            break;
        }
        else {
            fwrite(buf,1, n, fp);
        }
    }
    fclose(fp);

    if(parse_bitrates() < 0) {
        return -1;
    }

    return 0;
}

int parse_addr(struct sockaddr_in *addr, const char *ip, int port){
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    inet_aton(ip, & addr->sin_addr);
    addr->sin_port = htons(port);
    return 0;
}

int proxy_conn_server() {
    int optval;

    proxy.server.fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(proxy.server.fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval) );

    if( bind(proxy.server.fd, (struct sockaddr *) &proxy.myaddr, sizeof(proxy.myaddr)) < 0) {
        logger(LOG_ERROR, "Failed: Can't bind to local ip");
        return -1;
    }

    if( connect(proxy.server.fd,
                (struct sockaddr *)(& proxy.toaddr), sizeof(proxy.toaddr)) < 0)
    {
        logger(LOG_ERROR, "Failed: Can't connect to video server");
        return -1;
    }

    logger(LOG_INFO, "Connected video server successfully");
    return 0;
}

int proxy_reconnect_server(){

    proxy.server.fd = socket(AF_INET, SOCK_STREAM, 0);
    if( bind(proxy.server.fd, (struct sockaddr *)(&proxy.myaddr), sizeof(proxy.myaddr)) < 0) {
        logger(LOG_ERROR, "Failed: reconnect can't bind to local addr");
        return -1;
    }
    if( connect(proxy.server.fd,
                (struct sockaddr *)(& proxy.toaddr), sizeof(proxy.toaddr)) < 0)
    {
        logger(LOG_ERROR, "Failed: reconnect can't reconnect to server");
        return -1;
    }

    proxy.maxfd = MAX(proxy.maxfd, proxy.server.fd);
    proxy.server.state = SRV_ST_STLINE;
    proxy.server.closed = 0;
    logger(LOG_DEBUG, "server reconnect successfully.");
    return 0;
}

#define MAX_CONN 1024

int proxy_start_listen(const char *port) {
    struct sockaddr_in addr;

    /* create socket */
    proxy.listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy.listenfd < 0) {
        logger(LOG_ERROR, "socket() failed");
        return -1;
    }

    /* set up addr */
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = INADDR_ANY;
int optval = 1;
setsockopt(proxy.listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval) );
    /*bind(proxy.listenfd, (struct sockaddr *)&addr, sizeof(addr));*/
    /*while(bind(proxy.listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0);*/
    if (bind(proxy.listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        logger(LOG_ERROR, "bind() failed");
        perror("");
        return -1;
    }

    if (listen(proxy.listenfd, MAX_CONN) < 0) {
        logger(LOG_ERROR, "listen() failed");
        return -1;
    }

    return 0;
}

/* for debugging */
int dump_proxy_info(Proxy *p) {
    fprintf(stdout, "------------|\n");
    fprintf(stdout, "| alpha: %.3f|\n", p->alpha);
    fprintf(stdout, "| ts: %ld|\n", p->ts);
    fprintf(stdout, "| tput: %f|\n", p->tput);
    fprintf(stdout, "| avg_tput: %f|\n", p->avg_tput);
    fprintf(stdout, "| server_state: %d\n", p->server.state);
    fprintf(stdout, "-------------\n");

    return 0;
}

/**
* activity log
*/
int init_activity_log(Proxy *p, const char *file) {
    p->log = fopen(file, "w+");
    if (NULL == p->log) {
        logger(LOG_ERROR, "cannot open activity log");
        return -1;
    }

    return 0;
}

int write_activity_log(Proxy *p) {
    fprintf(p->log, "%lu %f %d %d %d %s %dSeg%d-Frag%d\n",
            get_timestamp_now() / 1000,
            (float)(1.0*p->delta/1000),
            (int)(p->tput),
            (int)(p->avg_tput),
            p->bitrate,
            inet_ntoa(p->client.addr.sin_addr),
            p->bitrate, p->segnum, p->fragnum);

    fflush(p->log);

    return 0;
}
