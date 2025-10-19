#include "emulator/si.h"
#include "emulator/cpu.h"
#include "emulator/pif.h"
#include "emulator/ram.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"

bool siPut8(SI* pSI, u32 nAddress, s8* pData) { return false; }

bool siPut16(SI* pSI, u32 nAddress, s16* pData) { return false; }

bool siPut32(SI* pSI, u32 nAddress, s32* pData) {
    u32 nSize;
    void* aData;

    nAddress &= 0x1F;

    switch (nAddress) {
        case 0x00:
            pSI->nAddress = *pData & 0xFFFFFF;
            break;
        case 0x04:
            nSize = 0x40;

            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &aData, pSI->nAddress, &nSize)) {
                return false;
            }

            if (!pifGetData(SYSTEM_PIF(gpSystem), aData)) {
                return false;
            }

            xlObjectEvent(gpSystem, 0x1000, (void*)6);
            break;
        case 0x10:
            nSize = 0x40;

            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &aData, pSI->nAddress, &nSize)) {
                return false;
            }

            if (!pifSetData(SYSTEM_PIF(gpSystem), aData)) {
                return false;
            }

            xlObjectEvent(gpSystem, 0x1000, (void*)6);
            break;
        case 0x18:
            xlObjectEvent(gpSystem, 0x1001, (void*)6);
            break;
        default:
            return false;
    }

    return true;
}

bool siPut64(SI* pSI, u32 nAddress, s64* pData) { return false; }

bool siGet8(SI* pSI, u32 nAddress, s8* pData) { return false; }

bool siGet16(SI* pSI, u32 nAddress, s16* pData) { return false; }

bool siGet32(SI* pSI, u32 nAddress, s32* pData) {
    nAddress &= 0x1F;

    switch (nAddress) {
        case 0x00:
            *pData = pSI->nAddress;
            break;
        case 0x04:
            *pData = 0;
            break;
        case 0x10:
            *pData = 0;
            break;
        case 0x18:
#if IS_SM64
            *pData = 0;
#else
            *pData = systemExceptionPending(gpSystem, SIT_SI) ? 4096 : 0;
#endif
            break;
        default:
            return false;
    }

    return true;
}

bool siGet64(SI* pSI, u32 nAddress, s64* pData) { return false; }

bool siEvent(SI* pSI, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            break;
        case 0x1002:
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)siPut8, (Put16Func)siPut16,
                                 (Put32Func)siPut32, (Put64Func)siPut64)) {
                return false;
            }

            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)siGet8, (Get16Func)siGet16,
                                 (Get32Func)siGet32, (Get64Func)siGet64)) {
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

_XL_OBJECTTYPE gClassSI = {
    "SI",
    sizeof(SI),
    NULL,
    (EventFunc)siEvent,
};
