#include <stdio.h>
#include <string.h>
#include <util.h>

int str_endwith(char *src, int src_len, char *dst, int dst_len){
    if(src_len < dst_len)
        return 0;
    if(strncmp(&src[src_len - dst_len], dst, dst_len) != 0)
        return 0;
    return 1;
}

int parse_reqline(char *buf,int *buf_num){
    int bitrate;
    int seqnum;
    int fragnum;

    buf[*buf_num] = '\0'; // we're assuming the request line length is less than 4096
    if( strstr(buf, "big_buck_bunny.f4m") ){
        sprintf(buf, "GET /vod/big_buck_bunny_nolist.f4m HTTP/1.1\r\n");
        *buf_num = strlen(buf);
    }
    else if(sscanf(buf, "GET /vod/%dSeg%d-Frag%d", &bitrate, &seqnum, &fragnum) == 3){
        // update bitrate to desired bitrate
        bitrate = 100;
        sprintf(buf, "GET /vod/%dSeg%d-Frag%d HTTP/1.1\r\n", bitrate, seqnum, fragnum);
        *buf_num = strlen(buf);
    }
    return 0;
}
