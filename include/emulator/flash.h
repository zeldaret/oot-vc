#ifndef _FLASH_H
#define _FLASH_H

#include "emulator/xlObject.h"
#include "revolution/types.h"
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Flash {
    /* MK64, Others */
    /* 0x00 0x00 */ u32 unk_00;
#if VERSION == MK64_U
    /* 0x04  N/A */ void* unk_04;
#endif
    /* 0x08 0x04 */ struct Store* pStore;
    /* 0x0C 0x08 */ s32 flashStatus;
    /* 0x10 0x0C */ s32 flashCommand;
#if VERSION != MK64_U
    /*  N/A 0x10 */ s32 nOffsetRAM;
    /*  N/A 0x14 */ s32 unk_14;
    /*  N/A 0x18 */ s32 unk_18;
#endif
} Flash; // size = 0x14, 0x1C

bool fn_80045260(Flash* pFLASH, s32 arg1, void* arg2);
bool fn_800452B0(Flash* pFLASH, s32 arg1, void* arg2);
bool fn_80045300(Flash* pFLASH, u32* arg1);
bool flashEvent(Flash* pFLASH, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassFlash;

#ifdef __cplusplus
}
#endif

#endif
