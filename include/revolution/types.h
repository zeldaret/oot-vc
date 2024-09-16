#ifndef REVOLUTION_TYPES_H_
#define REVOLUTION_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char s8;
typedef unsigned char u8;
typedef signed short int s16;
typedef unsigned short int u16;
typedef signed long s32;
typedef unsigned long u32;
typedef signed long long int s64;
typedef unsigned long long int u64;

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
#endif

#define false 0
#define true 1

#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))

#ifndef NULL
#define NULL (void*)0
#endif

// some conditions don't match if it's using `(void*)0`
#ifndef null
#define null 0
#endif

typedef int UNKWORD;
typedef void UNKTYPE;
typedef void (*funcptr_t)(void);

#ifdef __cplusplus
}
#endif

#endif
