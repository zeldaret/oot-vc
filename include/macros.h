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

#define ROUND_UP(x, align) (((x) + (align) - 1) & (-(align)))
#define ROUND_UP_PTR(x, align) ((void*)((((u32)(x)) + (align) - 1) & (~((align) - 1))))
#define ROUND_DOWN(x, align) ((x) & (-(align)))
#define ROUND_DOWN_PTR(x, align) ((void*)(((u32)(x)) & (~((align) - 1))))
#define MEMCLR(x) __memclr((x), sizeof(*(x)))

#ifndef __INTELLISENSE__
#define NO_INLINE __attribute__((never_inline))
#define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define NO_INLINE
#define ATTRIBUTE_UNUSED
#endif

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

#define DECLTYPE(x) __decltype__(x)

#define NW4R_VERSION(major_, minor_) ((major_) << 8 | (minor_))

// Sort of like C++11's range for, except now it's a macro for NW4R iterators
#define NW4R_RANGE_FOR(it_, list_) \
    for (DECLTYPE((list_).GetBeginIter())(it_) = (list_).GetBeginIter(); (it_) != (list_).GetEndIter(); ++(it_))

/* This macro specifically is for the for loops which declare an
 * Iterator currIt = it++; in the body, so that it does not get incremented
 * twice.
 */
#define NW4R_RANGE_FOR_NO_AUTO_INC(it_, list_) \
    for (DECLTYPE((list_).GetBeginIter())(it_) = (list_).GetBeginIter(); (it_) != (list_).GetEndIter();)

#define NW4HBM_VERSION NW4R_VERSION
#define NW4HBM_RANGE_FOR NW4R_RANGE_FOR
#define NW4HBM_RANGE_FOR_NO_AUTO_INC NW4R_RANGE_FOR_NO_AUTO_INC

#define NW4HBM_ASSERT_PTR(ptr, line)                                                                             \
    {                                                                                                            \
        bool bVar3 = (((u32)(ptr) & 0xFF000000) == 0x80000000 || ((u32)(ptr) & 0xFF800000) == 0x81000000) ||     \
                     !(((u32)(ptr) & 0xF8000000) != 0x90000000) || !(((u32)(ptr) & 0xFF000000) != 0xC0000000) || \
                     !(((u32)(ptr) & 0xFF800000) != 0xC1000000) || !(((u32)(ptr) & 0xF8000000) != 0xD0000000) || \
                     !(((u32)(ptr) & 0xFFFFC000) != 0xE0000000);                                                 \
                                                                                                                 \
        if (!bVar3) {                                                                                            \
            fn_8010CB20(__FILE__, line, "NW4HBM:Pointer Error\n" #ptr "(=%p) is not valid pointer.", ptr);       \
        }                                                                                                        \
    }

#define NW4HBM_ASSERT_PTR2(ptr, file, line)                                                                      \
    {                                                                                                            \
        bool bVar3 = (((u32)(ptr) & 0xFF000000) == 0x80000000 || ((u32)(ptr) & 0xFF800000) == 0x81000000) ||     \
                     !(((u32)(ptr) & 0xF8000000) != 0x90000000) || !(((u32)(ptr) & 0xFF000000) != 0xC0000000) || \
                     !(((u32)(ptr) & 0xFF800000) != 0xC1000000) || !(((u32)(ptr) & 0xF8000000) != 0xD0000000) || \
                     !(((u32)(ptr) & 0xFFFFC000) != 0xE0000000);                                                 \
                                                                                                                 \
        if (!bVar3) {                                                                                            \
            fn_8010CB20(file, line, "NW4HBM:Pointer Error\n" #ptr "(=%p) is not valid pointer.", ptr);           \
        }                                                                                                        \
    }

#define NW4HBM_ASSERT_PTR3(ptr, file, line)                                                             \
    {                                                                                                   \
        if ((((u32)(ptr) & 0xFF000000) == 0x80000000 || ((u32)(ptr) & 0xFF800000) == 0x81000000) ||     \
            !(((u32)(ptr) & 0xF8000000) != 0x90000000) || !(((u32)(ptr) & 0xFF000000) != 0xC0000000) || \
            !(((u32)(ptr) & 0xFF800000) != 0xC1000000) || !(((u32)(ptr) & 0xF8000000) != 0xD0000000) || \
            !(((u32)(ptr) & 0xFFFFC000) != 0xE0000000)) {                                               \
                                                                                                        \
            fn_8010CB20(file, line, "NW4HBM:Pointer Error\n" #ptr "(=%p) is not valid pointer.", ptr);  \
        }                                                                                               \
    }

#define NW4HBM_PANIC(cond, line, ...)                 \
    {                                                 \
        if ((cond)) {                                 \
            fn_8010CB20(__FILE__, line, __VA_ARGS__); \
        }                                             \
    }

#define NW4HBM_PANIC2(cond, line, ...)                \
    {                                                 \
        if (!(cond)) {                                \
            fn_8010CB20(__FILE__, line, __VA_ARGS__); \
        }                                             \
    }

#define NW4HBM_PANIC3(cond, line, ...)                \
    {                                                 \
        bool b = false;                               \
                                                      \
        if ((cond)) {                                 \
            b = true;                                 \
        }                                             \
                                                      \
        if (!b) {                                     \
            fn_8010CB20(__FILE__, line, __VA_ARGS__); \
        }                                             \
    }

#define NW4HBM_PANIC4(cond, file, line, ...)      \
    {                                             \
        if ((cond)) {                             \
            fn_8010CB20(file, line, __VA_ARGS__); \
        }                                         \
    }

#define NW4HBM_ASSERT(cond, line) NW4HBM_PANIC(cond, line, "NW4HBM:Failed assertion " #cond)

#define NW4HBM_ASSERT2(cond, line) NW4HBM_PANIC(!(cond), line, "NW4HBM:Failed assertion " #cond)

#define NW4HBM_ASSERT3(cond, file, line) NW4HBM_PANIC4(!(cond), file, line, "NW4HBM:Failed assertion " #cond)

#define NW4HBM_ASSERT_ALIGN32(var, line) \
    NW4HBM_PANIC((u32)var & 0x1F, line,  \
                 "NW4HBM:Alignment Error(0x%x)\n" #var " must be aligned to 32 bytes boundary.", var)

#define NW4HBM_ASSERT_ALIGN2(var, file, line) \
    NW4HBM_PANIC4((u32)var & 0x1, file, line, \
                  "NW4HBM:Alignment Error(0x%x)\n" #var " must be aligned to 2 bytes boundary.", var)

#define NW4HBM_ASSERT_PTR_NULL(ptr, line) NW4HBM_PANIC(ptr == NULL, line, "NW4HBM:Pointer must not be NULL (" #ptr ")")

#define FONT_TYPE_NNGCTEXTURE 1
#define GLYPH_INDEX_NOT_FOUND 0xFFFF

#endif

#endif
