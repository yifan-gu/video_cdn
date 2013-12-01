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
    dns_message_t m;
    size_t ret;
    fd_set fdset;
    struct timeval tv;

    if ((ret = make_question(&m, node)) < 0) {
        return -1;
    }

    if ((ret = encode_message(&m, buf)) < 0) {
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

        if (decode_message(&m, buf, ret) == 0) { // recvfrom and decode succeeded
            
            // do resove
        }
    }

    return 0;
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
    
    freeaddrinfo(result);           /* No longer needed */
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
    //printf("ret %d\n", ret);

    return 0;
}
#endif
