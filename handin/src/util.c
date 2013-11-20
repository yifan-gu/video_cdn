#include <stdio.h>
#include <string.h>
#include <util.h>
#include <proxy.h>
#include <sys/time.h>

#include "logger.h"

extern Proxy proxy;

unsigned long get_timestamp_now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000); // convert to milliseconds
}

int str_endwith(char *src, int src_len, char *dst, int dst_len){
    if(src_len < dst_len)
        return 0;
    if(strncmp(&src[src_len - dst_len], dst, dst_len) != 0)
        return 0;
    return 1;
}

int get_bitrate(){
    int i;
    for (i = proxy.bps_len - 1; i > 0; i -- ) {
        if(proxy.bps[i]*1.5 < proxy.avg_tput)
            break;
    }
    return proxy.bps[i];
}

int parse_reqline(char *buf,int *buf_num){
    int bitrate;
    int segnum;
    int fragnum;

    buf[*buf_num] = '\0'; // we're assuming the request line length is less than 4096
    if( strstr(buf, "big_buck_bunny.f4m") ){
        sprintf(buf, "GET /vod/big_buck_bunny_nolist.f4m HTTP/1.1\r\n");
        *buf_num = strlen(buf);
    }
    else if(sscanf(buf, "GET /vod/%dSeg%d-Frag%d", &bitrate, &segnum, &fragnum) == 3){
        // update bitrate to desired bitrate
        proxy.bitrate = get_bitrate();
        proxy.segnum = segnum;
        proxy.fragnum = fragnum;
        proxy.client.get_chunk = 1;
        sprintf(buf, "GET /vod/%dSeg%d-Frag%d HTTP/1.1\r\n", proxy.bitrate, segnum, fragnum);
        *buf_num = strlen(buf);
    }
    return 0;
}

/**
 * change keep-alive to close if found.
 * @param buf, the recv buffer
 * @param recvlen, the recvlen
 * @return the modified recvlen, because strlen("Close") is shorter...
 */
int change_connection(char *buf, int recvlen) {
    const char *connection = "Connection:";
    const char *keepalive = " Keep-Alive\r\n"; // some times, they have white space after colon
    const char *conclose = " Close\r\n";

    char *ptr, *tail;

    tail = strstr(buf, "\r\n\r\n");
    
    if ((ptr = strstr(buf, connection)) != NULL) {
        if (NULL == tail || ptr < tail) { // make sure the "Connection:" is in the header
            
            ptr += strlen(connection);
            if ((ptr - buf + strlen(keepalive)) > recvlen) {
                logger(LOG_WARN, "Header size too long %d", ptr-buf);
            } else {
                strncpy(ptr, conclose, strlen(conclose));
                memmove(ptr + strlen(conclose),
                        ptr + strlen(keepalive),
                        recvlen - (ptr + strlen(keepalive) - buf));

                // logger(LOG_DEBUG, "recvlen %d, after %d", recvlen, recvlen - (strlen(keepalive) - strlen(conclose)));
                return recvlen - (strlen(keepalive) - strlen(conclose));
            }
        }
    }
    
    return recvlen;
}
