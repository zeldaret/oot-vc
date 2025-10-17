#ifndef _ALLOC_H
#define _ALLOC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t) __attribute__((nothrow));
void free(void*) __attribute__((nothrow));

#ifdef __cplusplus
}
#endif

#endif
