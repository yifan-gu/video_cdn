#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <proxy.h>
#include <mydns.h>
#include <logger.h>

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

static int make_question(dns_message_t *m, const char *node) {
    char *tok, *str;
    char *saveptr = NULL;
    char len;
    char buf[DNS_NAME_LEN];
    dns_question_t *qptr;
    
    /* init header */
    memset(&m->header, 0, sizeof(dns_header_t));
    m->type = DNS_TYPE_QUESTION;
    m->header.id = random() % (1 << 16); // randomly choose one id
    m->header.qdcount = 1;

    /* init question */
    qptr = &m->question;
    memset(qptr, 0, sizeof(dns_question_t));
    qptr->qtype = 1; // request for A record
    qptr->qclass = 1; // request for IP address

    /* make qname */
    if (strlen(node) > DNS_NAME_LEN) {
        logger(LOG_WARN, "name too long");
        return -1;
    }
    strncpy(buf, node, strlen(node));
    str = buf;
    //printf("str %s\n", buf);
    /* save labels */
    while ((tok = strtok_r(str, ".", &saveptr)) != NULL) {
        //printf("str %s\n", tok);
        len = (char)strlen(tok);
        qptr->qname[qptr->qname_len] = len; // copy len
        strncpy(qptr->qname + qptr->qname_len+1, tok, len); //copy str
        qptr->qname_len += (len+1);
        
        str = NULL;
    }
    qptr->qname_len++;
    // qptr->qname[qptr->qname_len] = 0;
    m->length = sizeof(dns_header_t) + qptr->qname_len
        + sizeof(qptr->qtype) + sizeof(qptr->qclass);

    return 0;
}

static size_t encode_message(dns_message_t *m, void *buf) {
    char *offset;
    dns_message_t mm;

    memcpy(&mm, m, sizeof(mm));
    
    /* deal with header */
    mm.header.id = htons(mm.header.id);
    mm.header.flag = htons(mm.header.flag);
    mm.header.qdcount = htons(mm.header.qdcount);
    mm.header.ancount = htons(mm.header.ancount);
    offset = memcpy(buf, &mm.header, sizeof(mm.header)) + sizeof(mm.header);

    /* deal with question or answer */
    if (mm.type == DNS_TYPE_QUESTION) {
        offset = memcpy(offset, &mm.question.qname, mm.question.qname_len) + mm.question.qname_len;
        
        mm.question.qtype = htons(mm.question.qtype);
        offset = memcpy(offset, &mm.question.qtype, sizeof(mm.question.qtype)) + sizeof(mm.question.qtype);
        
        mm.question.qclass = htons(mm.question.qclass);
        offset = memcpy(offset, &mm.question.qclass, sizeof(mm.question.qclass)) + sizeof(mm.question.qclass);
    } else { // type == DNS_TYPE_ANSWER

    }

    return mm.length;
}

static int decode_message(dns_message_t *m, void *buf, ssize_t len) {
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

    printf("fd %d\n", dns.sock);
    printf("dns %s:%d\n", inet_ntoa(dns.addr.sin_addr), ntohs(dns.addr.sin_port));
    if ((ret = sendto(dns.sock, buf, ret, 0,
                      (struct sockaddr *)&dns.addr,
                      sizeof(dns.addr))) < 0) {
        printf("ret %ld\n", ret);
        logger(LOG_WARN, "sendto() failed");
        return -1;
    }

    FD_ZERO(&fdset);
    FD_SET(dns.sock, &fdset);
    tv.tv_sec = DNS_TIMEOUT;
    tv.tv_usec = 0;
    printf("select\n");
    
    if ((ret = select(dns.sock+1, &fdset, NULL, NULL, &tv)) < 0) {
        logger(LOG_WARN, "select() failed");
        return -1;
    } else if (0 == ret) {
        logger(LOG_WARN, "select() timeout");
        return -1;
    } else { // select succeeded
        if (FD_ISSET(dns.sock, &fdset)) { // socket ISSET
            if ((ret = recvfrom(dns.sock, buf, UDP_LEN, 0, NULL, NULL)) < 0) {
                logger(LOG_WARN, "recvfrom() failed");
                return -1;
            } else if (0 == ret) {
                logger(LOG_WARN, "recvfrom() received 0 bytes, peer shutdown");
                return -1;
            } else { // recvfrom succeeded
                if (decode_message(&m, buf, ret) < 0) {
                    logger(LOG_WARN, "decode_message() failed");
                    return -1;
                }

                // do resove
            }
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

int dump_dns_message(dns_message_t *m, int type) {
    fprintf(stderr, "-----------------\n");
    fprintf(stderr, "|type: %9d|\n", m->type);
    fprintf(stderr, "|length: %7ld|\n", m->length);
    fprintf(stderr, "|header         |\n");
    fprintf(stderr, "|  id: %9x|\n", m->header.id);
    fprintf(stderr, "|  AA: %9d|\n", m->header.flag >> 10);
    fprintf(stderr, "|  qdcount: %4d|\n", m->header.qdcount);
    fprintf(stderr, "|  ancount: %4d|\n", m->header.ancount);

    if (type == DNS_TYPE_QUESTION) {
        fprintf(stderr, "|question       |\n");
        fprintf(stderr, "|  qname_len: %2d|\n", m->question.qname_len);
        fprintf(stderr, "|  qname: %s|\n", m->question.qname);
        fprintf(stderr, "|  qtype: %6d|\n", m->question.qtype);
        fprintf(stderr, "|  qclass: %5d|\n", m->question.qclass);
    } else if (type == DNS_TYPE_ANSWER) {
        fprintf(stderr, "|answer       |\n");
    }

    fprintf(stderr, "-----------------\n");

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

    return 0;
}
#endif
