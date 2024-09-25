#ifndef _RVL_SDK_OS_FAST_CAST_H
#define _RVL_SDK_OS_FAST_CAST_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS_GQR_F32 0x0000
#define OS_GQR_U8 0x0004
#define OS_GQR_U16 0x0005
#define OS_GQR_S8 0x0006
#define OS_GQR_S16 0x0007

#define OS_FASTCAST_U8 2
#define OS_FASTCAST_U16 3
#define OS_FASTCAST_S8 4
#define OS_FASTCAST_S16 5

static inline void OSInitFastCast(void) {
#ifdef __MWERKS__ // clang-format off
    asm {
        li r3, 4
        oris r3, r3, 4
        mtspr 0x392, r3

        li r3, 5
        oris r3, r3, 5
        mtspr 0x393, r3
        
        li r3, 6
        oris r3, r3, 6
        mtspr 0x394, r3
        
        li r3, 7
        oris r3, r3, 7
        mtspr 0x395, r3
    }
#endif // clang-format on
}

static inline void OSSetGQR6(register u32 type, register u32 scale) {
    register u32 val = ((scale << 8 | type) << 16) | ((scale << 8) | type);

#ifdef __MWERKS__ // clang-format off
    asm {
        mtspr 0x396, val
    }
#endif // clang-format on
}

static inline void OSSetGQR7(register u32 type, register u32 scale) {
    register u32 val = ((scale << 8 | type) << 16) | ((scale << 8) | type);

#ifdef __MWERKS__ // clang-format off
    asm {
        mtspr 0x397, val
    }
#endif // clang-format on
}

static inline f32 __OSu16tof32(register const u16* arg) {
    register f32 ret;

#ifdef __MWERKS__ // clang-format off
    asm {
        psq_l ret, 0(arg), 1, 3
    }
#endif // clang-format on

    return ret;
}

static inline void OSu16tof32(const u16* in, f32* out) { *out = __OSu16tof32(in); }

static inline u16 __OSf32tou16(register f32 arg) {
    f32 a;
    register f32* ptr = &a;
    u16 r;

#ifdef __MWERKS__ // clang-format off
    asm {
        psq_st arg, 0(ptr), 1, 3
    }
#endif // clang-format on

    r = *(u16*)ptr;
    return r;
}

static inline void OSf32tou16(const f32* in, u16* out) { *out = __OSf32tou16(*in); }

static inline f32 __OSs16tof32(register const s16* arg) {
    register f32 ret;

#ifdef __MWERKS__ // clang-format off
    asm {
        psq_l ret, 0(arg), 1, 5
    }
#endif // clang-format on

    return ret;
}

static inline void OSs16tof32(const s16* in, f32* out) { *out = __OSs16tof32(in); }

static inline s16 __OSf32tos16(register f32 arg) {
    f32 a;
    register f32* ptr = &a;
    s16 r;

#ifdef __MWERKS__ // clang-format off
    asm {
        psq_st arg, 0(ptr), 1, 5
    }
#endif // clang-format on

    r = *(s16*)ptr;
    return r;
}

static inline void OSf32tos16(const f32* in, s16* out) { *out = __OSf32tos16(*in); }

static inline u8 __OSf32tou8(register f32 arg) {
    f32 a;
    register f32* ptr = &a;
    u8 r;

#ifdef __MWERKS__
    // clang-format off
    asm {
        psq_st arg, 0(ptr), 1, OS_FASTCAST_U8
    }
    // clang-format on
    r = *(u8*)ptr;
#else
    r = (u8)arg;
#endif

    return r;
}

static inline void OSf32tou8(f32* in, u8* out) { *out = __OSf32tou8(*in); }

static inline f32 __OSu8tof32(register u8* arg) {
    register f32 ret;

#ifdef __MWERKS__
    // clang-format off
    asm {
        psq_l ret, 0(arg), 1, OS_FASTCAST_U8
    }
    // clang-format on
#else
    ret = (f32)*arg;
#endif

    return ret;
}

static inline void OSu8tof32(u8* in, f32* out) { *out = __OSu8tof32(in); }

#ifdef __cplusplus
}
#endif

#endif
