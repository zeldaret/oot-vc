#ifndef _RVL_SDK_OS_RTC_H
#define _RVL_SDK_OS_RTC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSSram {
    /* 0x0 */ u16 checksum;
    /* 0x2 */ u16 invchecksum;
    /* 0x4 */ u32 ead0;
    /* 0x8 */ u32 ead1;
    /* 0xC */ u32 counterBias;
    /* 0x10 */ u8 dispOfsH;
    /* 0x11 */ u8 ntd;
    /* 0x12 */ u8 lang;
    /* 0x13 */ u8 flags;
} OSSram;

typedef struct OSSramEx {
    char UNK_0x0[0x1C];
    /* 0x1C */ u16 wirelessPadId[4];
    char UNK_0x38[0x3C - 0x38];
    /* 0x3C */ u16 gbs;
    char UNK_0x3E[0x40 - 0x3E];
} OSSramEx;

void __OSInitSram(void);
OSSramEx* __OSLockSramEx(void);
bool __OSUnlockSramEx(bool save);
bool __OSSyncSram(void);
bool __OSReadROM(void* dst, s32 size, const void* src);
u16 OSGetWirelessID(s32 pad);
void OSSetWirelessID(s32 pad, u16 id);
u16 OSGetGbsMode(void);
void OSSetGbsMode(u16 gbs);
bool __OSGetRTCFlags(u32* out);
bool __OSClearRTCFlags(void);

#ifdef __cplusplus
}
#endif

#endif
