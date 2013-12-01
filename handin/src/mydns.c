#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <mydns.h>
#include <logger.h>
#include <proxy.h>
#include "dns_util.h"

#ifdef TESTING
Proxy proxy;
#else
extern Proxy proxy;
#endif

dns_t dns;

int init_mydns(const char *dns_ip, unsigned int dns_port) {
    memset(&dns, 0, sizeof(dns_t));
    strncpy(dns.dns_ip, dns_ip, IP_LEN);
    dns.dns_port = dns_port;

    if ((dns.sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0) {
        logger(LOG_ERROR, "socket() failed");
        return -1;
    }

    if (bind(dns.sock, (struct sockaddr *) &proxy.myaddr, sizeof(proxy.myaddr)) < 0 ) {
        logger(LOG_ERROR, "bind() failed");
        return -1;
    }
    
    memset(&dns.addr, 0, sizeof(dns.addr));
    dns.addr.sin_family = AF_INET;
    dns.addr.sin_port = htons(dns_port);
    if (inet_aton(dns_ip, &dns.addr.sin_addr) < 0) {
        logger(LOG_ERROR, "inet_aton() failed");
        return -1;
    }
    
    return 0;
}

int resolve(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res) {

    char buf[UDP_LEN];
    dns_message_t qm, am;
    size_t ret;
    fd_set fdset;
    struct timeval tv;
    struct addrinfo *new_res;
    struct sockaddr_in * sa_in;

    if ((ret = make_question(&qm, node)) < 0) {
        return -1;
    }

    if ((ret = encode_message(&qm, buf)) < 0) {
        return -1;
    }

    if ((ret = sendto(dns.sock, buf, ret, 0,
                      (struct sockaddr *)&dns.addr,
                      sizeof(dns.addr))) < 0) {
        logger(LOG_WARN, "sendto() failed");
        return -1;
    }

    FD_ZERO(&fdset);
    FD_SET(dns.sock, &fdset);
    tv.tv_sec = DNS_TIMEOUT;
    tv.tv_usec = 0;
    
    if ((ret = select(dns.sock+1, &fdset, NULL, NULL, &tv)) < 0) {
        logger(LOG_WARN, "select() failed");
        return -1;
    }

    if (0 == ret) {
        logger(LOG_WARN, "select() timeout");
        return -1;
    }

    if(FD_ISSET(dns.sock, &fdset)) { // select succeeded, socket ISSET
        if ((ret = recvfrom(dns.sock, buf, UDP_LEN, 0, NULL, NULL)) < 0) {
            logger(LOG_WARN, "recvfrom() failed");
            return -1;
        }

        if (0 == ret) {
            logger(LOG_WARN, "recvfrom() received 0 bytes, peer shutdown");
            return -1;
        }
        
        if (decode_message(&am, buf, ret) == 0
            && exam_answer(&qm, &am) == 0) { // decode succeeded

            new_res = NULL;
            new_res = (struct addrinfo *)calloc(1, sizeof(struct addrinfo));
            if (NULL == *res) {
                logger(LOG_WARN, "malloc() failed");
                goto FAILED;
            }
            
            /* fill in the addrinfo struct, ignoring hints */
            new_res->ai_flags = 0;
            new_res->ai_family = AF_INET;
            new_res->ai_socktype = SOCK_STREAM;
            new_res->ai_protocol = 0;
            new_res->ai_addrlen = sizeof(struct sockaddr_in);
            new_res->ai_addr = (struct sockaddr *)malloc(sizeof(struct sockaddr_in));
            if (NULL == new_res->ai_addr) {
                logger(LOG_WARN, "malloc() failed");
                goto FAILED;
            }

            new_res->ai_canonname = (char *)calloc(1, strlen(node) + 1);
            if (NULL == new_res->ai_canonname) {
                logger(LOG_WARN, "malloc() failed");
                goto FAILED;
            }
            memcpy(new_res->ai_canonname, node, strlen(node));

            new_res->ai_next = NULL;

            /* fill in the sockaddr struct */
            sa_in = (struct sockaddr_in *)new_res->ai_addr;
            sa_in->sin_family = AF_INET;
            sa_in->sin_port = htons(atoi(service));
            memcpy(&sa_in->sin_addr, &am.answer.rdata, sizeof(am.answer.rdata));

            (*res) = new_res;
        }
    }

    return 0;
    
FAILED:
    if (new_res != NULL) {
        if (new_res->ai_addr != NULL) {
            free(new_res->ai_addr);
        }

        if (new_res->ai_canonname != NULL) {
            free(new_res->ai_canonname);
        }

        free(new_res);
    }

    return -1;
}

void freeaddr(struct addrinfo *res) {
    free(res->ai_addr);
    freeaddrinfo(res);

    return;
}

int dns_server_info(const char *server_ip) {
    struct addrinfo *result;
    int res;
    //int sfd;

    res = resolve(server_ip, "8080", NULL, &result);
    if (res != 0) {
        logger(LOG_ERROR, "Failed: Can't resolve domain name: (%s)\n", server_ip);
        return -1;
    }

    if (result == NULL) {               /* No address succeeded */
        logger(LOG_ERROR, "Failed: Can't connect to any server resolved by DNS\n");
        return -1;
    }
    
#ifndef TESTING
    proxy.toaddr.sin_addr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr;
#endif

    freeaddr(result); /* No longer needed */
    return 0;
}

#ifdef TESTING
int main(int argc, char *argv[])
{
    int ret;
    struct addrinfo *res;

    //dns_message_t m;
    init_log(NULL);
    
    //ret = make_question(&m, "video.cmu.edu");
    //printf("ret: %d\n", ret);
    
    //dump_dns_message(&m, DNS_TYPE_QUESTION);
    
    //printf("myaddr %s:%d\n", inet_ntoa(proxy.myaddr.sin_addr), ntohs(proxy.myaddr.sin_port));
    parse_addr(& proxy.myaddr, "127.0.0.1", 0); // 0 -- random port
    init_mydns("127.0.0.1", 53);
    ret = resolve("video.cmu.edu", "8080", NULL, &res);
    printf("ret %d\n", ret);

    printf("addr %s:%d\n", inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr),
           ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port));
    printf("%p %s\n", res->ai_addr, res->ai_canonname);

    freeaddr(res);
    
    return 0;
}
#endif
