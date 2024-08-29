#ifndef _RVL_SDK_DVD_ERROR_H
#define _RVL_SDK_DVD_ERROR_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*DVDErrorCallback)(s32 result, s32 arg1);

#define DVD_ERROR_CMD_MAX 5

typedef struct DVDErrorInfo {
    /* 0x0 */ char game[4];
    /* 0x4 */ u8 disk;
    /* 0x5 */ u8 version;
    /* 0x8 */ u32 error;
    /* 0xC */ s32 sec;
    /* 0x10 */ u32 disr;
    /* 0x14 */ u32 dicr;
    /* 0x18 */ u32 next;

    struct {
        /* 0x1C */ u32 command;
        /* 0x20 */ u32 param1;
        /* 0x24 */ u32 param2;
        /* 0x28 */ u32 intType;
        /* 0x2C */ u32 tick;
    } info[DVD_ERROR_CMD_MAX];
} DVDErrorInfo;

extern DVDErrorInfo __ErrorInfo;

void __DVDStoreErrorCode(u32 error, DVDErrorCallback callback);

#ifdef __cplusplus
}
#endif

#endif
