#include "types.h"

void __fill_mem(void *ptr, s32 fill, size_t len);

void *memset(void *ptr, s32 fill, size_t len) {
    __fill_mem(ptr, fill, len);
    return ptr;
}