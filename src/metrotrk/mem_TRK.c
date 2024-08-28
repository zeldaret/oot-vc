#include "types.h"

void TRK_fill_mem(void* p, s32 fill, size_t len);

void* TRK_memset(void* p, s32 fill, size_t len) {
    TRK_fill_mem(p, fill, len);
    return p;
}

void* TRK_memcpy(void* dst, void* src, size_t len) {
    u8* s;
    u8* d;

    for (s = (u8*)src - 1, d = (u8*)dst - 1, len++; --len;) {
        *++d = *++s;
    }
}
