#include "types.h"

void *memcpy(void *dst, void *src, size_t len) {
    char *s;
    char *d;

    if(src >= dst){
        for(s = (char *)src - 1, d = (char*)dst - 1, len++; --len;) {
            *++d = *++s;
        }
    } else {
        for(s = (char *)src + len, d = (char*)dst + len, len++; --len;) {
            *--d = *--s;
        }
    }
    return dst;
}