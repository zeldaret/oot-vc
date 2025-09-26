#ifndef _ALLOC_H
#define _ALLOC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t) throw();
void free(void*) throw();

#ifdef __cplusplus
}
#endif

#endif
