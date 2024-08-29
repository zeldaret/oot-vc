#ifndef _MI_H
#define _MI_H

// Note: mips.h in oot-gc

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MIInterruptType {
    MIT_NONE = -1,
    MIT_SP = 0,
    MIT_SI = 1,
    MIT_AI = 2,
    MIT_VI = 3,
    MIT_PI = 4,
    MIT_DP = 5,
} MIInterruptType;

typedef struct MI {
    /* 0x0 */ s32 nInterrupt;
    /* 0x4 */ s32 nMask;
    /* 0x8 */ s32 nMode;
} MI; // size = 0xC

bool miSetInterrupt(MI* pMI, MIInterruptType eType);
bool miResetInterrupt(MI* pMI, MIInterruptType eType);

bool miPut8(MI* pMI, u32 nAddress, s8* pData);
bool miPut16(MI* pMI, u32 nAddress, s16* pData);
bool miPut32(MI* pMI, u32 nAddress, s32* pData);
bool miPut64(MI* pMI, u32 nAddress, s64* pData);

bool miGet8(MI* pMI, u32 nAddress, s8* pData);
bool miGet16(MI* pMI, u32 nAddress, s16* pData);
bool miGet32(MI* pMI, u32 nAddress, s32* pData);
bool miGet64(MI* pMI, u32 nAddress, s64* pData);

bool miEvent(MI* pMI, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassMI;

#ifdef __cplusplus
}
#endif

#endif
