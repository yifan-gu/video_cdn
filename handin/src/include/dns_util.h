#ifndef _DNS_UTIL_H
#define _DNS_UTIL_H

#include "mydns.h"

/**
 * make question
 *
 * @param m, ptr to DNS question struct
 * @param node, the domain name
 *
 * @return 0 on success, -1 if fails
 */
int make_question(dns_message_t *m, const char *node);

/**
 * make answer
 *
 * @param m, ptr to DNS answer message struct
 * @param q, ptr to DNS question message struct
 * @param rcode, rcode
 * @param answer, the answer (IP address in string)
 *
 * @return 0 on success, -1 if fails
 */
int make_answer(dns_message_t *m, dns_message_t *q, char rcode, char *answer);

/**
 * encode the DNS message into a UDP buffer
 *
 * @param m, the DNS message struct
 * @param buf, the empty buffer
 *
 * @return the size of result buffer on success, -1 if fails
 */
ssize_t encode_message(dns_message_t *m, void *buf);

/**
 * decode the DNS message from a UDP buffer
 *
 * @param m, the DNS message struct
 * @param buf, the UDP buffer
 * @param len, the length of the UDP buffer
 *
 * @return 0 on success, -1 if fails
 */
int decode_message(dns_message_t *m, void *buf, ssize_t len);

/**
 * a debugging helper
 */
int dump_dns_message(dns_message_t *m);

#endif // _DNS_UTIL_H
