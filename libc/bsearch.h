#ifndef _BSEARCH_H
#define _BSEARCH_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));

#ifdef __cplusplus
};
#endif

#endif
