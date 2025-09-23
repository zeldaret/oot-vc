#ifndef REVOLUTION_TYPES_H_
#define REVOLUTION_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdint.h"

typedef signed char s8;
typedef unsigned char u8;
typedef signed short int s16;
typedef unsigned short int u16;
typedef signed long s32;
typedef unsigned long u32;
typedef signed long long int s64;
typedef unsigned long long int u64;
typedef unsigned char u128[16];

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float f32;
typedef double f64;

typedef volatile f32 vf32;
typedef volatile f64 vf64;

#ifndef __cplusplus
typedef int bool;
#define false 0
#define true 1
#else
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#endif

#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))

typedef int UNKWORD;
typedef void UNKTYPE;
typedef void (*funcptr_t)(void);

typedef unsigned long int byte4_t;
typedef unsigned short int byte2_t;
typedef unsigned char byte1_t;
typedef byte1_t byte_t;
typedef unsigned long int register_t;
typedef unsigned char char_t;
typedef unsigned char char8_t;
typedef unsigned short char16_t;
typedef unsigned long char32_t;

#ifdef __cplusplus
}
#endif

#if !defined(__cplusplus) && __STDC_VERSION__ >= 199901L
#define RESTRICT restrict
#else
#define RESTRICT
#endif

#endif
