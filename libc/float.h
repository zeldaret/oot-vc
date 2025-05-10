#ifndef _FLOAT_H
#define _FLOAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define FLT_MAX 3.402823466e+38f
#define FLT_EPSILON 1.192092896e-07f
#define FLT_MIN 1.175494351e-38f

#define DBL_EPSILON 1.1920929e-07

#define LDBL_MANT_DIG 53
#define LDBL_DIG 15
#define LDBL_MIN_EXP (-1021)
#define LDBL_MIN_10_EXP (-308)
#define LDBL_MAX_EXP 1024
#define LDBL_MAX_10_EXP 308

#define LDBL_MAX 0x1.fffffffffffffP1023L
#define LDBL_EPSILON 0x1.0000000000000P-52L
#define LDBL_MIN 0x1.0000000000000P-1022L

#ifdef __cplusplus
}
#endif

#endif
