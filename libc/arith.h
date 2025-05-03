#ifndef _MSL_COMMON_ARITH_H
#define _MSL_COMMON_ARITH_H

#include "revolution/types.h"

typedef struct {
    int quot; /* quotient */
    int rem; /* remainder */
} div_t;

int abs(int __x);
long labs(long __x);
div_t div(int __numer, int __denom);
int __msl_add(int* __x, int __y);
int __msl_ladd(long* __x, long __y);
int __msl_mul(int* __x, int __y);
div_t __msl_div(int __x, int __y);
int __msl_mod(int __x, int __y);

#endif
