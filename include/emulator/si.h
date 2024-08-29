#ifndef _SI_H
#define _SI_H

// Note: serial.h in oot-gc

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SI {
    /* 0x00 */ s32 nAddress;
} SI; // size = 0x04

bool siPut32(SI* pSI, u32 nAddress, s32* pData);
bool siGet32(SI* pSI, u32 nAddress, s32* pData);
bool siEvent(SI* pSI, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassSI;

#ifdef __cplusplus
}
#endif

#endif
