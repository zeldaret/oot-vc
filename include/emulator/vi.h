#ifndef _VI_H
#define _VI_H

// Note: this is the `video.h` file from oot-gc

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// __anon_0x75B37
typedef struct VI {
    /* 0x00 */ s32 nScan;
    /* 0x04 */ bool bBlack;
    /* 0x08 */ s32 nBurst;
    /* 0x0C */ s32 nSizeX;
    /* 0x10 */ s32 nStatus;
    /* 0x14 */ s32 nTiming;
    /* 0x18 */ s32 nAddress;
    /* 0x1C */ s32 nScanInterrupt;
    /* 0x20 */ s32 nScaleX;
    /* 0x24 */ s32 nScaleY;
    /* 0x28 */ s32 nStartH;
    /* 0x2C */ s32 nStartV;
    /* 0x30 */ s32 nSyncH;
    /* 0x34 */ s32 nSyncV;
    /* 0x38 */ s32 nSyncLeap;
} VI; // size = 0x40

bool viPut8(VI* pVI, u32 nAddress, s8* pData);
bool viPut16(VI* pVI, u32 nAddress, s16* pData);
bool viPut32(VI* pVI, u32 nAddress, s32* pData);
bool viPut64(VI* pVI, u32 nAddress, s64* pData);

bool viGet8(VI* pVI, u32 nAddress, s8* pData);
bool viGet16(VI* pVI, u32 nAddress, s16* pData);
bool viGet32(VI* pVI, u32 nAddress, s32* pData);
bool viGet64(VI* pVI, u32 nAddress, s64* pData);

bool viForceRetrace(VI* pVI);
bool viEvent(VI* pVI, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassVI;

#ifdef __cplusplus
}
#endif

#endif
