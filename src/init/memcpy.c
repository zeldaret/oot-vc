#include "types.h"

void *memcpy(void *dst, void *src, size_t len) {
    u8 *s;
    u8 *d;

    if(src >= dst){
        for(s = (u8 *)src - 1, d = (u8*)dst - 1, len++; --len;) {
            *++d = *++s;
        }
    } else {
        for(s = (u8 *)src + len, d = (u8*)dst + len, len++; --len;) {
            *--d = *--s;
        }
    }
    return dst;
}
