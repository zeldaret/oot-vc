#ifndef _FLASH_H
#define _FLASH_H

#include "emulator/xlObject.h"
#include "macros.h"
#include "revolution/types.h"
#include "versions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Flash {
    /*  OoT - SM64 - MK64 */
    /* 0x00 - 0x00 - 0x00 */ u32 nFlashSize;
#if IS_SM64 || IS_MK64
    /*  N/A - 0x04 - 0x04 */ void* unk_04;
#endif
    /* 0x04 - 0x08 - 0x08 */ struct Store* pStore;
    /* 0x08 - 0x0C - 0x0C */ s32 flashStatus;
    /* 0x0C - 0x10 - 0x10 */ s32 flashCommand;
#if IS_OOT
    /* 0x10 -  N/A - N/A */ s32 nOffsetRAM;
    /* 0x14 -  N/A - N/A */ s32 unk_14;
    /* 0x18 -  N/A - N/A */ s32 unk_18;
#endif
} Flash; // size = 0x1C - 0x14 - 0x14

bool fn_80045260(Flash* pFLASH, s32 arg1, void* arg2);
bool fn_800452B0(Flash* pFLASH, s32 arg1, void* arg2);
bool flashGetSize(Flash* pFLASH, u32* pnFlashSize);
bool flashEvent(Flash* pFLASH, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassFlash;

#ifdef __cplusplus
}
#endif

#endif
