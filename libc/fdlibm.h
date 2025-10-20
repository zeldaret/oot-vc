#ifndef _FDLIBM_H
#define _FDLIBM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "errno.h"
#include "limits.h"

/**
 * For including FDLIBM declarations without also including its many macros.
 */

double __ieee754_acos(double);
double __ieee754_fmod(double, double);
double __ieee754_log(double);
double __ieee754_log10(double);
double __ieee754_pow(double, double);
double __ieee754_sqrt(double);
double __ieee754_atan2(double, double);
double __ieee754_asin(double);
double __ieee754_exp(double);
int __ieee754_rem_pio2(double, double*);

double __kernel_sin(double, double, int);
double __kernel_cos(double, double);
double __kernel_tan(double, double, int);

double nan(const char* arg);
double atan(double x);
double ceil(double x);
double copysign(double x, double y);
double cos(double x);
double floor(double x);
double frexp(double x, int* eptr);
double ldexp(double value, int exp);
double modf(double x, double* iptr);
double sin(double x);
double tan(double x);
double acos(double x);
double asin(double x);
double atan2(double y, double x);
double fmod(double x, double y);
double pow(double x, double y);
double sqrt(double x);
double log(double x);
float log10f(float x);

// In reality, these are "weak" functions which all have C++ names (except scalbn).
// We fake it by defining them as strong C functions instead.
float sin__Ff(float x);
float cos__Ff(float x);
double scalbn(double x, int n);
double fabs__Fd(double x);
float fabsf__Ff(float x);

#ifdef __cplusplus
};
#endif

#endif
