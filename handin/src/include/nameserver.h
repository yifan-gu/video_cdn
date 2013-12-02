#ifndef _NAMESERVER_H
#define _NAMESERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <node_map.h>

#define SERVER_NAME "video.cs.cmu.edu"
typedef struct _NameServer {
    // round robin counter:
    // if round robin is not set, this variable will be -1;
    // otherwise it's init to 0 and incremented circularly [0 .. servers_number-1]
    int rr;

    FILE *log;
    struct sockaddr_in addr;
    int sock;
    NodeMap nmap;
} NameServer;

int parse_argument(int argc, const char *argv[]);
int parse_servers(const char *);
int parse_lsa(const char *);

void print_graph();

const char *get_server(char *client);

/**
 * activity log
 *
 * @return 0 on success, -1 if fails
 */
int init_activity_log(NameServer *ns, const char *file);

/**
 * print log
 */
void write_activity_log(NameServer *ns,
                        struct sockaddr_in *cli_addr,
                        const char *qname,
                        const char *answer);

#endif // for #ifndef _NAMESERVER_H
