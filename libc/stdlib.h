#ifndef _STDLIB_H
#define _STDLIB_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "alloc.h"
#include "arith.h"
#include "mbstring.h"
#include "rand.h"
#include "strtold.h"
#include "strtoul.h"

typedef int (*_compare_function)(const void*, const void*);
void qsort(void*, size_t, size_t, _compare_function);

// For functions that return 0 on a success and -1 on failure
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1
#endif

#ifdef __cplusplus
};
#endif

#endif
