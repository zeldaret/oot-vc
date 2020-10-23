#include "types.h"

void __fill_mem(void *ptr, u32 fill, size_t len) {
    u32 i;
    u32 *w_ptr;
    char *c_ptr;
    u32 val;

    val = (u8)fill;
    c_ptr = (char*)ptr - 1;
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
        if(i != 0){
            do{
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
                *++w_ptr = val;
            } while(--i);
        }

        i = (len / 4) % 8;
        if(i != 0){
            do {
                *++w_ptr = val;
            } while(--i);
        }

        c_ptr = (char*)w_ptr + 3;
        len %= 4;
    }

    if (len != 0) {
        do {
            *++c_ptr = val;
        } while (--len);
    }
}