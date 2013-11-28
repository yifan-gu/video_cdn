/*
 * Notice:
 * This dns implementation only supports only IP address request
 */
#ifndef _MYDNS_H
#define _MYDNS_H

#include <netdb.h>
#include <inttypes.h>

#define IP_LEN 64
#define DNS_NAME_LEN 255

#define SETFLAG(flag, value, offset) ((flag) |= ((value) << (15 - (offset))))

/**
 * Dns struct
 */
typedef struct dns_s {
    char dns_ip[IP_LEN];
    unsigned int dns_port;
} dns_t;

/**
 * dns header struct
 */
typedef struct dns_header_s {
    uint16_t id;
    uint16_t flag;
    uint16_t qdcount;
    uint16_t ancount;
} dns_header_t;

/**
 * dns question struct
 */
typedef struct dns_question_s {
    char qname[DNS_NAME_LEN];
    uint16_t qtype;
    uint16_t qclass;
} dns_question_t;

/**
 * dns resource record struct
 */
typedef struct dns_answer_s {
    char name[DNS_NAME_LEN];
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint32_t rdata; // assuming only one type of rdata(class A) is requested for simplicity
} dns_answer_t;


/**
 * Dns message struct
 */
typedef struct dns_message_s {
    dns_header_t header;
    union {
        dns_question_t question;
        dns_answer_t answer; // actually, we will only have one answer
    } content;
} dns_message_t;

/**
 * Initialize your client DNS library with the IP address and port number of
 * your DNS server.
 *
 * @param  dns_ip  The IP address of the DNS server.
 * @param  dns_port  The port number of the DNS server.
 *
 * @return 0 on success, -1 otherwise
 */
int init_mydns(const char *dns_ip, unsigned int dns_port);

/**
 * Resolve a DNS name using your custom DNS server.
 *
 * Whenever your proxy needs to open a connection to a web server, it calls
 * resolve() as follows:
 *
 * struct addrinfo *result;
 * int rc = getaddrinfo("video.cs.cmu.edu", "8080", null, &result);
 * if (rc != 0) {
 *     // handle error
 * }
 * // connect to address in result
 * free(result);
 *
 *
 * @param  node  The hostname to resolve.
 * @param  service  The desired port number as a string.
 * @param  hints  Should be null. resolve() ignores this parameter.
 * @param  res  The result. resolve() should allocate a struct addrinfo, which
 * the caller is responsible for freeing.
 *
 * @return 0 on success, -1 otherwise
 */

int resolve(const char *node, const char *service, 
            const struct addrinfo *hints, struct addrinfo **res);

int dns_server_info(const char *server_ip);

int init_question(dns_question_t *q);

#endif // for #ifndef _MYDNS_H
