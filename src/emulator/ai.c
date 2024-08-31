#include "emulator/ai.h"
#include "emulator/ram.h"
#include "emulator/rsp.h"
#include "emulator/soundRVL.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlPostRVL.h"

// Note: audio.c in oot-gc

bool aiPut8(AI* pAI, u32 nAddress, s8* pData) { return false; }

bool aiPut16(AI* pAI, u32 nAddress, s16* pData) { return false; }

bool aiPut32(AI* pAI, u32 nAddress, s32* pData) {
    void* pBuffer;

    switch (nAddress & 0x1F) {
        case 0x0:
            if ((pAI->nAddress = (*pData & 0xFFFFFF)) != 0) {
                if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pBuffer, pAI->nAddress, NULL)) {
                    return false;
                }
                if (pAI->bEnable) {
                    soundSetAddress(SYSTEM_SOUND(gpSystem), pBuffer);
                }
                break;
            }
            break;
        case 0x4:
            pAI->nSize = *pData & 0x3FFF8;
            if ((pAI->nControl != 0) && pAI->bEnable) {
                soundSetLength(SYSTEM_SOUND(gpSystem), pAI->nSize);
            }
            break;
        case 0x8:
            pAI->nControl = *pData & 1;
            break;
        case 0xC:
            xlObjectEvent(gpSystem, 0x1001, (void*)7);
            break;
        case 0x10:
            pAI->nRateDAC = *pData & 0x3FFF;
            if (pAI->bEnable) {
                soundSetDACRate(SYSTEM_SOUND(gpSystem), pAI->nRateDAC);
            }
            break;
        case 0x14:
            pAI->nRateBit = *pData & 0xF;
            break;
        default:
            return false;
    }

    return true;
}

bool aiPut64(AI* pAI, u32 nAddress, s64* pData) { return false; }

bool aiGet8(AI* pAI, u32 nAddress, s8* pData) { return false; }

bool aiGet16(AI* pAI, u32 nAddress, s16* pData) { return false; }

bool aiGet32(AI* pAI, u32 nAddress, s32* pData) {
    s32 sp8;

    switch (nAddress & 0x1F) {
        case 0:
            *pData = pAI->nAddress;
            break;
        case 4:
            if (!soundGetDMABuffer(SYSTEM_SOUND(gpSystem), (u32*)pData)) {
                *pData = pAI->nSize;
            }
            break;
        case 8:
            *pData = pAI->nControl;
            break;
        case 12:
            if (!soundGetDMABuffer(SYSTEM_SOUND(gpSystem), (u32*)&sp8)) {
                return false;
            }
            *pData = sp8 ? 0x40000000 : 0;
            break;
        case 16:
            *pData = pAI->nRateDAC;
            break;
        case 20:
            *pData = pAI->nRateBit;
            break;
        default:
            return false;
    }

    return true;
}

bool aiGet64(AI* pAI, u32 nAddress, s64* pData) { return false; }

bool aiEnable(AI* pAI, bool bEnable) {
    pAI->bEnable = bEnable ? true : false;

    if (!rspEnableABI(SYSTEM_RSP(gpSystem), pAI->bEnable)) {
        return false;
    }

    return true;
}

bool aiEvent(AI* pAI, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            pAI->nSize = 0;
            pAI->nAddress = 0;
            pAI->nRateBit = 0;
            pAI->nRateDAC = 0;
            pAI->nControl = 1;
            pAI->bEnable = true;
            break;
        case 0x1002:
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)aiPut8, (Put16Func)aiPut16,
                                 (Put32Func)aiPut32, (Put64Func)aiPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)aiGet8, (Put16Func)aiGet16,
                                 (Put32Func)aiGet32, (Put64Func)aiGet64)) {
                return false;
            }
            break;
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

_XL_OBJECTTYPE gClassAI = {
    "AI",
    sizeof(AI),
    NULL,
    (EventFunc)aiEvent,
};
