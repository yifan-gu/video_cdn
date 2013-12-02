#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "logger.h"
#include "dns_util.h"

int make_question(dns_message_t *m, const char *node) {
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

    /* split by "." */
    while ((tok = strtok_r(str, ".", &saveptr)) != NULL) {
        len = (char)strlen(tok);
        qptr->qname[qptr->qname_len] = len; // copy len
        strncpy(qptr->qname + qptr->qname_len+1, tok, len); //copy str
        qptr->qname_len += (len+1);
        
        str = NULL;
    }
    qptr->qname_len++; // tail 0
    
    m->length = sizeof(dns_header_t) + qptr->qname_len // sum up the message length
        + sizeof(qptr->qtype) + sizeof(qptr->qclass);

    return 0;
}

static void decode_header(dns_message_t *m, void *buf) {
    memcpy(&m->header, buf, sizeof(m->header));
    /* id */
    m->header.id = ntohs(m->header.id);
    /* flag */
    m->header.flag = ntohs(m->header.flag);
    /* qdcount */
    m->header.qdcount = ntohs(m->header.qdcount);
    /* ancount */
    m->header.ancount = ntohs(m->header.ancount);

    return;
}

static void decode_question(dns_message_t *m, void *buf, void *offset) {
    m->question.qname_len = m->length - sizeof(m->header)
        - sizeof(m->question.qtype) - sizeof(m->question.qclass);
    
    /* decode names */
    memcpy(&m->question.qname, offset, m->question.qname_len);
    offset += m->question.qname_len;

    /* decode qtype */
    memcpy(&m->question.qtype, offset, sizeof(m->question.qtype));
    m->question.qtype = ntohs(m->question.qtype);
    offset += sizeof(m->question.qtype);

    /* decode qclass */
    memcpy(&m->question.qclass, offset, sizeof(m->question.qclass));
    m->question.qclass = ntohs(m->question.qclass);

    return;
}

static void decode_answer(dns_message_t *m, void *buf, void *offset) {
    m->answer.name_len = m->length - sizeof(m->header)
        - sizeof(m->answer.type) - sizeof(m->answer.class) - sizeof(m->answer.ttl)
        - sizeof(m->answer.rdlength) - sizeof(m->answer.rdata);

    /* decode names */
    memcpy(&m->answer.name, offset, m->answer.name_len);
    offset += m->answer.name_len;

    /* decode type */
    memcpy(&m->answer.type, offset, sizeof(m->answer.type));
    m->answer.type = ntohs(m->answer.type);
    offset += sizeof(m->answer.type);

    /* decode class */
    memcpy(&m->answer.class, offset, sizeof(m->answer.class));
    m->answer.class = ntohs(m->answer.class);
    offset += sizeof(m->answer.class);

    /* decode ttl */
    memcpy(&m->answer.ttl, offset, sizeof(m->answer.ttl));
    m->answer.ttl = ntohs(m->answer.ttl);
    offset += sizeof(m->answer.ttl);

    /* decode rdlength */
    memcpy(&m->answer.rdlength, offset, sizeof(m->answer.rdlength));
    m->answer.rdlength = ntohl(m->answer.rdlength);
    offset += sizeof(m->answer.rdlength);

    /* decode rdata */
    memcpy(&m->answer.rdata, offset, sizeof(m->answer.rdata));
    
    return;
}

static void encode_header(dns_message_t *m, void *buf) {
    /* id */
    m->header.id = htons(m->header.id);
    /* flag */
    m->header.flag = htons(m->header.flag);
    /* qdcount */
    m->header.qdcount = htons(m->header.qdcount);
    /* ancount */
    m->header.ancount = htons(m->header.ancount);

    memcpy(buf, &m->header, sizeof(m->header));
    return;
}

static void encode_question(dns_message_t *m, void *buf, void *offset) {
    /* encode qname */
    offset = memcpy(offset, m->question.qname, m->question.qname_len)
        + m->question.qname_len;

    /* encode qtype */
    m->question.qtype = htons(m->question.qtype);
    offset = memcpy(offset, &m->question.qtype, sizeof(m->question.qtype))
        + sizeof(m->question.qtype);

    /* encode qclass */
    m->question.qclass = htons(m->question.qclass);
    memcpy(offset, &m->question.qclass, sizeof(m->question.qclass));
    
    return;
}

static void encode_answer(dns_message_t *m, void *buf, void *offset) {
    /* encode name */
    offset = memcpy(offset, m->answer.name, m->answer.name_len)
        + m->answer.name_len;

    /* encode type */
    m->answer.type = htons(m->answer.type);
    offset = memcpy(offset, &m->answer.type, sizeof(m->answer.type))
        + sizeof(m->answer.type);
    
    /* encode class */
    m->answer.class = htons(m->answer.class);
    offset = memcpy(offset, &m->answer.class, sizeof(m->answer.class))
        + sizeof(m->answer.class);
    
    /* ttl */
    m->answer.ttl = htonl(m->answer.ttl);
    offset = memcpy(offset, &m->answer.ttl, sizeof(m->answer.ttl))
        + sizeof(m->answer.ttl);

    /* rdlength */
    m->answer.rdlength = ntohs(m->answer.rdlength);
    offset = memcpy(offset, &m->answer.rdlength, sizeof(m->answer.rdlength))
        + sizeof(m->answer.rdlength);

    /* rdata */
    memcpy(offset, &m->answer.rdata, sizeof(m->answer.rdata));
    
    return;
}

int make_answer(dns_message_t *m, dns_message_t *q,
                char rcode, const char *answer) {
    dns_answer_t *aptr;
    dns_question_t *qptr;

    /* init header */
    memset(&m->header, 0, sizeof(dns_header_t));
    m->type = DNS_TYPE_ANSWER;
    m->header.id = q->header.id;
    SETFLAG(m->header.flag, 1, 0);
    SETFLAG(m->header.flag, 1, 5);
    SETFLAG(m->header.flag, rcode, 15);
    m->header.ancount = 1;

    /* make answer */
    aptr = &m->answer;
    qptr = &q->question;
    aptr->name_len = qptr->qname_len; // store length
    if (aptr->name_len > DNS_NAME_LEN) {
        logger(LOG_WARN, "name length too long %d", aptr->name_len);
        return -1;
    }
    memcpy(aptr->name, qptr->qname, aptr->name_len); // copy qname
    aptr->type = 1; // deal with other fields
    aptr->class = 1;
    aptr->rdlength = sizeof(aptr->rdata);
    if (inet_aton(answer, &aptr->rdata) < 0) { /* get ip address */
        logger(LOG_WARN, "inet_aton() failed");
        return -1;
    }

    /* update length */
    m->length = sizeof(dns_header_t) + aptr->name_len // sum up the message length
        + sizeof(aptr->type) + sizeof(aptr->class)
        + sizeof(aptr->ttl) + sizeof(aptr->rdlength) + aptr->rdlength;
    
    return 0;
}

ssize_t encode_message(dns_message_t *m, void *buf) {
    void *offset;
    dns_message_t mm;

    memcpy(&mm, m, sizeof(mm));
    
    /* deal with header */
    encode_header(&mm, buf);
    offset = memcpy(buf, &mm.header, sizeof(mm.header))
        + sizeof(mm.header);

    /* deal with question or answer */
    if (DNS_TYPE_QUESTION == mm.type) {
        encode_question(&mm, buf, offset);
    } else { // type == DNS_TYPE_ANSWER
        encode_answer(&mm, buf, offset);
        //encode_answer
    }

    return mm.length;
}

int decode_message(dns_message_t *m, void *buf, ssize_t len) {
    void *offset;

    // TODO exam the len
    memset(m, 0, sizeof(*m));
    m->length = len;

    /* deal with header */
    decode_header(m, buf);
    if (m->header.qdcount > 1 || m->header.ancount > 1) {
        logger(LOG_WARN, "multiple query/answers");
        return -1;
    }
    
    /* deal with body */
    offset = buf + sizeof(m->header);
    
    m->type = GETFLAG(m->header.flag, 0, 1); // get type
    if (DNS_TYPE_QUESTION == m->type) {
        decode_question(m, buf, offset);
    } else { // answer
        decode_answer(m, buf, offset);
    }
    
    return 0;
}

int exam_answer(dns_message_t *qm, dns_message_t *am) {
    if (qm->header.id != am->header.id) {
        logger(LOG_WARN, "answer id not correct, q: %d, a: %d", qm->header.id, am->header.id);
        return -1;
    }

    int rcode = GETFLAG(am->header.flag, 15, 4);
    if (rcode != 0) {
        logger(LOG_WARN, "RCODE not zero, RCODE = %d", rcode);
        return -1;
    }

    return 0;
}

void interprete_qname(char *name, char *res, int len) {
    char cnt;
    int i, j;

    i = 0;
    while (1) {
        cnt = name[i];
        if (0 == cnt) {
            i--;
            res[i] = 0;
            break;
        }
        
        for (j = 0; j < cnt; j++) {
            res[i+j] = name[(i+1)+j];
        }
        
        res[i+j] = '.';
        i += cnt + 1;
        
    }

    return;
}

int dump_dns_message(dns_message_t *m) {
    fprintf(stderr, "-----------------\n");
    fprintf(stderr, "|type: %9d|\n", m->type);
    fprintf(stderr, "|length: %7ld|\n", m->length);
    fprintf(stderr, "|header |\n");
    fprintf(stderr, "| id: %9x|\n", m->header.id);
    fprintf(stderr, "| AA: %9d|\n", GETFLAG(m->header.flag, 5, 1));
    fprintf(stderr, "| qdcount: %4d|\n", m->header.qdcount);
    fprintf(stderr, "| ancount: %4d|\n", m->header.ancount);

    if (m->type == DNS_TYPE_QUESTION) {
        fprintf(stderr, "|question |\n");
        fprintf(stderr, "| qname_len: %2d|\n", m->question.qname_len);
        fprintf(stderr, "| qname: %s|\n", m->question.qname);
        fprintf(stderr, "| qtype: %6d|\n", m->question.qtype);
        fprintf(stderr, "| qclass: %5d|\n", m->question.qclass);
    } else if (m->type == DNS_TYPE_ANSWER) {
        fprintf(stderr, "|answer |\n");
        fprintf(stderr, "| name_len: %2d|\n", m->answer.name_len);
        fprintf(stderr, "| name: %s|\n", m->answer.name);
        fprintf(stderr, "| type: %6d|\n", m->answer.type);
        fprintf(stderr, "| class: %5d|\n", m->answer.class);
        fprintf(stderr, "| ttl: %7d|\n", m->answer.ttl);
        fprintf(stderr, "| rdlength: %d\n", m->answer.rdlength);
        fprintf(stderr, "| rdata: %s\n", inet_ntoa(m->answer.rdata));
    }

    fprintf(stderr, "-----------------\n");

    return 0;
}
