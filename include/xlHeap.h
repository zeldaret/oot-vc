#ifndef _XLHEAP_H
#define _XLHEAP_H

#include "types.h"

s32 xlHeapTake(void **ptr, size_t size);
void xlHeapFree(void *ptr);

#endif