#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include <nameserver.h>

NameServer ns;

int main(int argc, char const* argv[])
{
    parse_argument(argc, argv);
    return 0;
}

int parse_argument(int argc, const char *argv[]) {
    int i,j,k;
    ns.rr = 0;

    for (i = 1; i <= argc; i++) {
        if(strcmp("-r", argv[i]) == 0) {
            ns.rr = 1;
            k = i;
            break;
        }
    }

    j = 1;
    for (i = 1; i <= argc; i++) {
        if(i == k) continue;
        switch (j) {
        case 1: //log

            break;
        case 2: //ip
            inet_aton(argv[i], & ns.addr.sin_addr);
            break;
        case 3: // port
            ns.addr.sin_port = htons(atoi(argv[i]));
            ns.addr.sin_family = AF_INET;
            break;
        case 4: // servers

            if(ns.rr)
                return 0;
            break;
        case 5: // lsa

            break;
        default:
            break;
        }
        j ++;
    }

    return 0;

>>>>>>> 3aa6b14979a9b0ca85fa3ab265531a73b427356f
}
