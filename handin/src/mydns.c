#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <proxy.h>
#include <mydns.h>
#include <logger.h>

extern Proxy proxy;

int init_mydns(const char *dns_ip, unsigned int dns_port) {
    return 0;
}

int resolve(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res) {
    return 0;
}

int dns_server_info(const char *server_ip) {

    struct addrinfo *result;
    int res;
    int sfd;

    res = resolve(server_ip, "8080", NULL, &result);
    if (res != 0) {
        logger(LOG_ERROR, "Failed: Can't resolve Domain name: (%s)\n", server_ip);
        return -1;
    }

    if (result == NULL) {               /* No address succeeded */
        logger(LOG_ERROR, "Failed: Can't connect to any server resolved by DNS\n");
        return -1;
    }
    proxy.toaddr.sin_addr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr;

    freeaddrinfo(result);           /* No longer needed */
    return 0;
}
