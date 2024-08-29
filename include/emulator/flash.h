#ifndef _FLASH_H
#define _FLASH_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Flash {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ struct Store* pStore;
    /* 0x08 */ s32 flashStatus;
    /* 0x0C */ s32 flashCommand;
    /* 0x10 */ s32 nOffsetRAM;
    /* 0x14 */ s32 unk_14;
    /* 0x18 */ s32 unk_18;
} Flash; // size = 0x1C

bool fn_80045260(Flash* pFLASH, s32 arg1, void* arg2);
bool fn_800452B0(Flash* pFLASH, s32 arg1, void* arg2);
bool fn_80045300(Flash* pFLASH, s32* arg1);
bool flashEvent(Flash* pFLASH, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassFlash;

#ifdef __cplusplus
}
#endif

#endif
