#ifndef _MACROS_H
#define _MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

// The VERSION macro will be set to one of these version numbers.
#define OOT_J 1
#define OOT_U 2
#define OOT_E 3

#define ALIGN_PREV(X, N) ((X) & ~((N) - 1))
#define ALIGN_NEXT(X, N) ALIGN_PREV(((X) + (N) - 1), N)
#define ALIGN(X, N) ((X + N) & ~N)

#define ARRAY_COUNT(arr) (s32)(sizeof(arr) / sizeof(arr[0]))
#define ARRAY_COUNTU(arr) (u32)(sizeof(arr) / sizeof(arr[0]))

#define OFFSETOF(p, field) ((u8*)&(p)->field - (u8*)(p))

#define MB(x) (x * 1024 * 1024)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define SQ(x) ((x) * (x))
#define CLAMP(x, l, h) (((x) > (h)) ? (h) : (((x) < (l)) ? (l) : (x)))

#define ROUND_UP(x, align) (((x) + ((align) - 1)) & (~((align) - 1)))
#define ROUND_UP_PTR(x, align) ((void*)((((u32)(x)) + ((align) - 1)) & (~((align) - 1))))
#define ROUND_DOWN(x, align) ((x) & (~((align) - 1)))
#define ROUND_DOWN_PTR(x, align) ((void*)(((u32)(x)) & (~((align) - 1))))
#define MEMCLR(x) __memclr((x), sizeof(*(x)))

#ifndef __INTELLISENSE__
#define NO_INLINE __attribute__((never_inline))
#define ATTRIBUTE_UNUSED __attribute__((unused))
#define ATTRIBUTE_FALLTHROUGH __attribute__((fallthrough))

#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(x) __attribute__((aligned(x)))
#endif
#else
#define NO_INLINE
#define ATTRIBUTE_UNUSED
#define ATTRIBUTE_FALLTHROUGH

#ifndef ATTRIBUTE_ALIGN
#define ATTRIBUTE_ALIGN(x)
#endif
#endif

#ifndef alignas
#define alignas ATTRIBUTE_ALIGN
#endif

#ifndef static_assert
#define static_assert __static_assert
#endif

#define ATTRIBUTE_MAYBE_UNUSED ATTRIBUTE_UNUSED /* only used in e.g. asserts */

#define __CONCAT(x, y) x##y
#define CONCAT(x, y) __CONCAT(x, y)

// Force unused data to be exported
#ifndef NON_MATCHING
#define FORCE_ACTIVE(module, ...)                     \
    void fake_function(char, ...);                    \
    void CONCAT(##module##_unused_L, __LINE__)(void); \
    void CONCAT(##module##_unused_L, __LINE__)(void) { fake_function(0, __VA_ARGS__); }
#else
#define FORCE_ACTIVE(module, ...)
#endif

// Adds a stack variable in an inline function, which can be used to pad the
// stack after other functions have been inlined
inline void padStack(void) { int pad = 0; }
#define PAD_STACK() padStack()

#ifdef __MWERKS__
#define AT_ADDRESS(xyz) : (xyz)
#define DECL_SECTION(x) __declspec(section x)
#define EXPORT __declspec(export)
#define WEAK __declspec(weak)
#define ASM asm
#else
#define AT_ADDRESS(xyz)
#define DECL_SECTION(x)
#define EXPORT
#define WEAK
#define ASM
#endif

#define INIT DECL_SECTION(".init")
#define CTORS DECL_SECTION(".ctors")
#define DTORS DECL_SECTION(".dtors")

#define BOOLIFY_TRUE_TERNARY_TYPE(type_, exp_) ((exp_) ? (type_)1 : (type_)0)
#define BOOLIFY_TRUE_TERNARY(exp_) BOOLIFY_TRUE_TERNARY_TYPE(int, exp_)

#define BOOLIFY_FALSE_TERNARY_TYPE(type_, exp_) ((exp_) ? (type_)0 : (type_)1)
#define BOOLIFY_FALSE_TERNARY(exp_) BOOLIFY_FALSE_TERNARY_TYPE(int, exp_)

#define BOOLIFY_TERNARY_TYPE BOOLIFY_TRUE_TERNARY_TYPE
#define BOOLIFY_TERNARY BOOLIFY_TRUE_TERNARY

#define CONST_FLOAT_PI 3.1415926f

#define DEG_TO_RAD_MULT_CONSTANT (CONST_FLOAT_PI / 180.0f)
#define RAD_TO_DEG_MULT_CONSTANT (180.0f / CONST_FLOAT_PI)

#define DEG_TO_RAD(x) ((x) * DEG_TO_RAD_MULT_CONSTANT)
#define RAD_TO_DEG(x) ((x) * RAD_TO_DEG_MULT_CONSTANT)

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#endif

#endif
