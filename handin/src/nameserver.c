#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "dns_util.h"
#include <nameserver.h>
#include <logger.h>

NameServer ns;

static int init_server(NameServer *ns) {
    if ((ns->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        logger(LOG_WARN, "socket() failed");
        return -1;
    }

    if (bind(ns->sock, (struct sockaddr *) &ns->addr, sizeof(ns->addr)) < 0) {
        logger(LOG_WARN, "bind() failed");
        return -1;
    }
    
    return 0;
}

int main(int argc, char const* argv[])
{
    ssize_t ret;
    char rcode, buf[UDP_LEN];
    char *answer;
    struct sockaddr_in cli_addr;
    socklen_t addrlen;

    dns_message_t qm, am;
    
    parse_argument(argc, argv);
    
    init_log(NULL);

#ifdef TESTING
    char input[1024];
    while(scanf("%s", input) != EOF){
        char *output = get_server(input);
        printf("%s - %s\n", input,
               (output)? output: "None" );
    }
#endif

    if (init_server(&ns) < 0) {
        return -1;
    }
    
    // main loop, it's a blocking version for simplicity
    while (1) {
        addrlen = sizeof(cli_addr);
        if ((ret = recvfrom(ns.sock, buf, UDP_LEN, 0,
                            (struct sockaddr *) &cli_addr, &addrlen)) < 0) {
            logger(LOG_WARN, "recvfrom() failed");
            continue;
        }

        if (0 == ret) {
            logger(LOG_WARN, "recvfrom() receives 0 bytes, peer shutdown");
            continue;
        }

        if (decode_message(&qm, buf, ret) == 0) { // recvfrom and decode succeeded
            // lookup
            answer = "128.1.34.33";
            rcode = 0; // TODO choose rcode

            // make answer
            if (make_answer(&am, &qm, rcode, answer) < 0) {
                continue;
            }
            //dump_dns_message(&am);
            // encode
            if ((ret = encode_message(&am, buf)) < 0) {
                continue;
            }

            // sendto
            if (sendto(ns.sock, buf, ret, 0,
                       (struct sockaddr *) &cli_addr,
                       addrlen) < 0) {
                continue;
            }
        }
    }

    return 0;
}

int parse_argument(int argc, const char *argv[]) {
    int i,j,k;
    ns.rr = -1;

    k = -1;
    for (i = 1; i < argc; i++) {
        if(strcmp("-r", argv[i]) == 0) {
            ns.rr = 0;
            k = i;
            break;
        }
    }

    j = 0;
    for (i = 1; i < argc; i++) {
        if(i == k) continue;
        switch (j) {
        case 0: //log

            break;
        case 1: //ip
            inet_aton(argv[i], & ns.addr.sin_addr);
            break;
        case 2: // port
            ns.addr.sin_port = htons(atoi(argv[i]));
            ns.addr.sin_family = AF_INET;
            break;
        case 3: // servers
            init_nodemap(& ns.nmap);
            if(parse_servers(argv[i]) < 0) {
                return -1;
            }
            if(ns.rr != -1)
                return 0;
            break;
        case 4: // lsa
            if(parse_lsa(argv[i]) < 0) {
                return -1;
            }
            break;
        default:
            break;
        }
        j ++;
    }

    return 0;
}

int parse_servers(const char *filename) {
    FILE *fp;
    char name[MAX_NODENAME_LEN + 1];
    int slen;

    fp = fopen(filename, "r");
    if(! fp) {
        logger(LOG_ERROR, "Can't open server file: %s", filename);
        return -1;
    }

    while( fgets(name, MAX_NODENAME_LEN + 1, fp) ) {
        slen = strlen(name);
        if(name[slen-1] != '\n') {
            logger(LOG_ERROR, "Server name length exceeds predefined length" );
            fclose(fp);
            return -1;
        }
        name[slen-1] = '\0';
        markAsServer(& ns.nmap ,
                     createNode(&ns.nmap, name) );
    }
    fclose(fp);
    return 0;
}


#define MAX_LINELEN 1024

int parse_lsa(const char *filename) {
    FILE *fp;
    char line[MAX_LINELEN];
    int slen;
    char *p;
    char *name;
    int v;

    fp = fopen(filename, "r");
    while( fgets(line, MAX_LINELEN, fp) ) {
        slen = strlen(line);
        if(line[slen-1] != '\n') {
            logger(LOG_ERROR, "LSA line length exceeds predefined length: %s", line);
            fclose(fp);
            return -1;
        }
        line[slen-1] = '\0';

        p = strtok( line, " ");
        name = p;

        p = strtok( NULL, " ");
        v = atoi(p);

        p = strtok( NULL, " ");
        if(update_adj( &ns.nmap,
                       getNodeOrCreateIt(&ns.nmap, name),
                       v, p) < 0) {
            fclose(fp);
            logger(LOG_ERROR, "Update adjacent nodes failed!");
            return -1;
        }
    }

#ifdef TESTING
    print_graph();
#endif

    fclose(fp);
    return 0;
}

void print_graph(){
    NodeMap *nmap = & ns.nmap;

    int i, j;

    printf("------- NETWORK TOPOLOGY ------------\n");
    for (i = 0; i < nmap->node_num; i++) {
        printf("%s %s: ", nmap->nodes[i].name,
               (nmap->nodes[i].is_server)? "(server)":"" );
        for (j = 0; j < nmap->nodes[i].adj.count; j++) {
            printf("%s ", nmap->nodes[(nmap->nodes[i].adj.node_pos[j])].name );
        }
        printf("\n");
    }
    printf("------- NETWORK TOPOLOGY END --------\n");
}

const char *get_server(char *client){
    const char *res;

    // Geographic distance
    if(ns.rr == -1){
        return find_closest_server(&ns.nmap, client);
    }

    // round robin
    if(ns.nmap.node_num == 0)
        return NULL;

    res = (const char*) ns.nmap.nodes[ns.rr].name ;
    ns.rr = (ns.rr + 1) % ns.nmap.node_num;
    return res;
}


