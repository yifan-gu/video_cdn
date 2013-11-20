#ifndef _UTIL_H
#define _UTIL_H

#define MAX(a, b) ((a) < (b)? (b): (a) )
#define MIN(a, b) ((a) > (b)? (b): (a) )
// integer only
#define SWAP(a, b) {(a) ^= (b); (b) ^= (a); (a) ^= (b);}

int str_endwith(char *src, int src_len, char *dst, int dst_len);
int parse_reqline(char *buf,int *buf_num);

#endif // for #ifndef _UTIL_H
