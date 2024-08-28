#ifndef _PI_H
#define _PI_H

// Note: this is the `peripheral.h` file from oot-gc

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// __anon_0x83D15
typedef struct PI {
    /* 0x00 */ s32 nStatus;
    /* 0x04 */ s32 nSizePut;
    /* 0x08 */ s32 nSizeGet;
    /* 0x0C */ s32 nLatency1;
    /* 0x10 */ s32 nLatency2;
    /* 0x14 */ s32 nRelease1;
    /* 0x18 */ s32 nRelease2;
    /* 0x1C */ s32 nSizePage1;
    /* 0x20 */ s32 nSizePage2;
    /* 0x24 */ s32 nAddressRAM;
    /* 0x28 */ s32 nAddressROM;
    /* 0x2C */ s32 nWidthPulse1;
    /* 0x30 */ s32 nWidthPulse2;
    /* 0x34 */ s32 storageDevice;
    /* 0x38 */ s32 unk_38[20];
} PI; // size = 0x38

bool piPut32(PI* pPI, u32 nAddress, s32* pData);
bool piGet32(PI* pPI, u32 nAddress, s32* pData);
bool piEvent(PI* pPI, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassPI;

#ifdef __cplusplus
}
#endif

#endif
