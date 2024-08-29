#include "emulator/mi.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"

// Note: this is the `mi.h` file from oot-gc

bool miSetInterrupt(MI* pMI, MIInterruptType eType) {
    s32 nInterrupt = pMI->nInterrupt;

    switch (eType) {
        case MIT_SP:
            if (pMI->nMask & 0x01) {
                pMI->nInterrupt = nInterrupt | 0x01;
            }
            break;
        case MIT_SI:
            if (pMI->nMask & 0x02) {
                pMI->nInterrupt = nInterrupt | 0x02;
            }
            break;
        case MIT_AI:
            if (pMI->nMask & 0x04) {
                pMI->nInterrupt = nInterrupt | 0x04;
            }
            break;
        case MIT_VI:
            if (pMI->nMask & 0x08) {
                pMI->nInterrupt = nInterrupt | 0x08;
            }
            break;
        case MIT_PI:
            if (pMI->nMask & 0x10) {
                pMI->nInterrupt = nInterrupt | 0x10;
            }
            break;
        case MIT_DP:
            if (pMI->nMask & 0x20) {
                pMI->nInterrupt = nInterrupt | 0x20;
            }
            break;
        default:
            return false;
    }

    if (nInterrupt != pMI->nInterrupt) {
        return true;
    }

    return false;
}

bool miResetInterrupt(MI* pMI, MIInterruptType eType) {
    s32 nInterrupt = pMI->nInterrupt;

    switch (eType) {
        case MIT_SP:
            pMI->nInterrupt = nInterrupt & ~0x01;
            break;
        case MIT_SI:
            pMI->nInterrupt = nInterrupt & ~0x02;
            break;
        case MIT_AI:
            pMI->nInterrupt = nInterrupt & ~0x04;
            break;
        case MIT_VI:
            pMI->nInterrupt = nInterrupt & ~0x08;
            break;
        case MIT_PI:
            pMI->nInterrupt = nInterrupt & ~0x10;
            break;
        case MIT_DP:
            pMI->nInterrupt = nInterrupt & ~0x20;
            break;
        default:
            return false;
    }

    if (nInterrupt != pMI->nInterrupt) {
        return true;
    }

    return false;
}

bool miPut8(MI* pMI, u32 nAddress, s8* pData) { return false; }

bool miPut16(MI* pMI, u32 nAddress, s16* pData) { return false; }

bool miPut32(MI* pMI, u32 nAddress, s32* pData) {
    s32 nData;

    switch (nAddress & 0xF) {
        case 0x0:
            nData = *pData & 0xFFF;
            pMI->nMode = (pMI->nMode & ~0x7F) | (*pData & 0x7F);

            if (nData & 0x80) {
                pMI->nMode &= ~0x80;
            }
            if (nData & 0x100) {
                pMI->nMode |= 0x80;
            }
            if (nData & 0x200) {
                pMI->nMode &= ~0x100;
            }
            if (nData & 0x400) {
                pMI->nMode |= 0x100;
            }
            if (nData & 0x800) {
                xlObjectEvent(gpSystem, 0x1001, (void*)0xA);
            }
            if (nData & 0x1000) {
                pMI->nMode &= ~0x200;
            }
            if (nData & 0x2000) {
                pMI->nMode |= 0x200;
            }
            break;
        case 0xC:
            nData = *pData & 0xFFF;

            if (nData & 1) {
                pMI->nMask &= ~1;
            }
            if (nData & 2) {
                pMI->nMask |= 1;
            }
            if (nData & 4) {
                pMI->nMask &= ~2;
            }
            if (nData & 8) {
                pMI->nMask |= 2;
            }
            if (nData & 0x10) {
                pMI->nMask &= ~4;
            }
            if (nData & 0x20) {
                pMI->nMask |= 4;
            }
            if (nData & 0x40) {
                pMI->nMask &= ~8;
            }
            if (nData & 0x80) {
                pMI->nMask |= 8;
            }
            if (nData & 0x100) {
                pMI->nMask &= ~0x10;
            }
            if (nData & 0x200) {
                pMI->nMask |= 0x10;
            }
            if (nData & 0x400) {
                pMI->nMask &= ~0x20;
            }
            if (nData & 0x800) {
                pMI->nMask |= 0x20;
            }
            break;
        case 0x4:
        case 0x8:
            break;
        default:
            return false;
    }

    return true;
}

bool miPut64(MI* pMI, u32 nAddress, s64* pData) { return false; }

bool miGet8(MI* pMI, u32 nAddress, s8* pData) { return false; }

bool miGet16(MI* pMI, u32 nAddress, s16* pData) { return false; }

bool miGet32(MI* pMI, u32 nAddress, s32* pData) {
    switch (nAddress & 0xF) {
        case 0x0:
            *pData = pMI->nMode;
            break;
        case 0x4:
            *pData = 0x02020102;
            break;
        case 0x8:
            *pData = pMI->nInterrupt;
            break;
        case 0xC:
            *pData = pMI->nMask;
            break;
        default:
            return false;
    }

    return true;
}

bool miGet64(MI* pMI, u32 nAddress, s64* pData) { return false; }

bool miEvent(MI* pMI, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            pMI->nMode = 0;
            pMI->nMask = 0;
            pMI->nInterrupt = 0;
            break;
        case 0x1002:
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)miPut8, (Put16Func)miPut16,
                                 (Put32Func)miPut32, (Put64Func)miPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)miGet8, (Get16Func)miGet16,
                                 (Get32Func)miGet32, (Get64Func)miGet64)) {
                return false;
            }
        case 0:
        case 1:
        case 3:
            break;
        case 0x1003:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return false;
    }

    return true;
}

_XL_OBJECTTYPE gClassMI = {
    "MI",
    sizeof(MI),
    NULL,
    (EventFunc)miEvent,
};
