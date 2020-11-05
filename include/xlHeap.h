#ifndef _XLHEAP_H
#define _XLHEAP_H

#include "types.h"

s32 xlHeapTake(void **ptr, size_t size);
s32 xlHeapFree(void **ptr);
s32 xlHeapFill32(u32 *src, s32 len, u32 fill);

#endif
