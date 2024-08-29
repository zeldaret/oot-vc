#ifndef _SRAM_H
#define _SRAM_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Sram {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 unk_04;
    /* 0x08 */ struct Store* pStore;
} Sram; // size = 0xC

bool sramEvent(Sram* pSram, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassSram;

#ifdef __cplusplus
}
#endif

#endif
