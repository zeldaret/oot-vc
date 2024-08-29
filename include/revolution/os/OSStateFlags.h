#ifndef _RVL_SDK_OS_STATE_FLAGS_H
#define _RVL_SDK_OS_STATE_FLAGS_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSStateFlags {
    /* 0x0 */ u32 checksum;
    u8 BYTE_0x4;
    /* 0x5 */ u8 BYTE_0x5;
    /* 0x6 */ u8 discState;
    u8 BYTE_0x7;
    u32 WORD_0x8;
    u32 WORD_0xC;
    u32 WORD_0x10;
    u32 WORD_0x14;
    u32 WORD_0x18;
    u32 WORD_0x1C;
} OSStateFlags;

bool __OSWriteStateFlags(const OSStateFlags* state);
bool __OSReadStateFlags(OSStateFlags* state);

#ifdef __cplusplus
}
#endif

#endif
