#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <logger.h>
#include <proxy.h>

#define SERVER_PORT 8080

Proxy proxy;

int main(int argc, char const* argv[])
{
    if( init_log(NULL) < 0 ) {
        printf("Failed: Can't init logging\n");
        return 0;
    }

    proxy.alpha = atof(argv[2]);

    logger(LOG_INFO, "Connecting to video server...");
    if( proxy_conn_server(argv[4], argv[7]) < 0) {
        return 0;
    }

    logger(LOG_INFO, "Proxy starts listening");
    if( proxy_start_listen(argv[3]) < 0 ) {
        return 0;
    }

    return 0;
}

int proxy_conn_server(const char *local_ip, const char * server_ip) {
    struct sockaddr_in myaddr, toaddr;

    proxy.connfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    inet_aton(local_ip, &myaddr.sin_addr);
    myaddr.sin_port = htons(0); // random port

    if( bind(proxy.connfd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0) {
        logger(LOG_ERROR, "Failed: Can't bind to local ip: %s",
               local_ip);
        return -1;
    }

    inet_aton(server_ip, &toaddr.sin_addr);
    toaddr.sin_port = htons(SERVER_PORT);
    toaddr.sin_family = AF_INET;
    if( connect(proxy.connfd, (struct sockaddr *)&toaddr, sizeof(toaddr)) < 0)
    {
        logger(LOG_ERROR, "Failed: Can't connect to server: %s:%d",
               server_ip, SERVER_PORT);
        return -1;
    }

    logger(LOG_INFO, "Connected video server successfully: %s:%d",
           server_ip, SERVER_PORT);
    return 0;
}

int proxy_start_listen(const char *port) {
    return 1;
}
