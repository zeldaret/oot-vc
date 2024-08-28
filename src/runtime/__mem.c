#include "types.h"

void* memcpy(void* dst, void* src, size_t len) {
    u8* s;
    u8* d;

    if (src >= dst) {
        for (s = (u8*)src - 1, d = (u8*)dst - 1, len++; --len;) {
            *++d = *++s;
        }
    } else {
        for (s = (u8*)src + len, d = (u8*)dst + len, len++; --len;) {
            *--d = *--s;
        }
    }
    return dst;
}

void __fill_mem(void* ptr, u32 fill, size_t len) {
    u32 i;
    u32* w_ptr;
    u8* c_ptr;
    u32 val;

    val = (u8)fill;
    c_ptr = (u8*)ptr - 1;
    if (len >= 32) {
        i = ~(u32)c_ptr % 4;
        if (i != 0) {
            len -= i;
            do {
                *++c_ptr = val;
            } while (--i);
        }

        if (val != 0) {
            val |= (val << 0x18) | (val << 0x10) | (val << 0x8);
        }

        i = len / 32;
        w_ptr = (u32*)(c_ptr - 3);
        if (i != 0) {
            do {
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
            } while (--i);
        }

        i = (len / 4) % 8;
        if (i != 0) {
            do {
                *++w_ptr = val;
            } while (--i);
        }

        c_ptr = (u8*)w_ptr + 3;
        len %= 4;
    }

    if (len != 0) {
        do {
            *++c_ptr = val;
        } while (--len);
    }
}

void* memset(void* ptr, s32 fill, size_t len) {
    __fill_mem(ptr, fill, len);
    return ptr;
}
