#ifndef _UTIL_H
#define _UTIL_H

#define MAX(a, b) ((a) < (b)? (b): (a) )
#define MIN(a, b) ((a) > (b)? (b): (a) )
// integer only
#define SWAP(a, b) {(a) ^= (b); (b) ^= (a); (a) ^= (b);}

int str_endwith(char *src, int src_len, char *dst, int dst_len);
int parse_reqline(char *buf,int *buf_num);
unsigned long get_timestamp_now(void);
int change_connection(char *buf, int recvlen);
#endif // for #ifndef _UTIL_H
