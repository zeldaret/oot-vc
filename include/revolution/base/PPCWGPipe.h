#ifndef _RVL_SDK_PPCWGPIPE_H
#define _RVL_SDK_PPCWGPIPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "revolution/types.h"

typedef union uPPCWGPipe {
    /* 0x00 */ u8 u8;
    /* 0x01 */ u16 u16;
    /* 0x04 */ u32 u32;
    /* 0x08 */ u64 u64;
    /* 0x10 */ s8 s8;
    /* 0x11 */ s16 s16;
    /* 0x14 */ s32 s32;
    /* 0x18 */ s64 s64;
    /* 0x20 */ f32 f32;
    /* 0x24 */ f64 f64;
} PPCWGPipe; // size = 0x28

#ifdef __cplusplus
}
#endif

#endif
