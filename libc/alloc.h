#ifndef _ALLOC_H
#define _ALLOC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t);
void free(void*);

#ifdef __cplusplus
}
#endif

#endif
