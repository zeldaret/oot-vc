#include "types.h"

void func_80167898(void *p, s32 fill, size_t len);

void *TRK_memset(void *p, s32 fill, size_t len) {
    func_80167898(p, fill, len);
    return p;
}

void *TRK_memcpy(void *dst, void *src, size_t len) {
    u8 *s;
    u8 *d;

    for(s = (u8*)src - 1, d = (u8*)dst - 1, len++; --len;) {
        *++d = *++s;
    }
}
