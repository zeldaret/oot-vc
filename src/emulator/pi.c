#include "emulator/pi.h"
#include "emulator/cpu.h"
#include "emulator/flash.h"
#include "emulator/sram.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"

// Note: peripheral.c in oot-gc

static inline bool piDMA_Complete_UnknownInline(PI* pPI, CpuBlock** pBlock) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (*pBlock == (CpuBlock*)(pPI->unk_38 + (i * 5))) {
            pPI->storageDevice &= ~(1 << i);
            pPI->nStatus &= ~3;
            xlObjectEvent(gpSystem, 0x1000, (void*)9);
            return true;
        }
    }

    return false;
}

bool piDMA_Complete(CpuBlock* pBlock, bool bUnknown) {
    if (bUnknown == true) {
        if (!piDMA_Complete_UnknownInline(SYSTEM_PI(gpSystem), &pBlock)) {
            return false;
        }
    } else {
#if VERSION >= MK64_J
        return false;
#endif
    }

    return true;
}

static inline bool piGetNewBlock(PI* pPI, CpuBlock** ppBlock) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (!(pPI->storageDevice & (1 << i))) {
            pPI->storageDevice |= (1 << i);
            pPI->nStatus |= 3;
            *ppBlock = (CpuBlock*)(pPI->unk_38 + (i * 5));
            return true;
        }
    }

    *ppBlock = NULL;
    return false;
}

bool piPut8(PI* pPI, u32 nAddress, s8* pData) { return false; }

bool piPut16(PI* pPI, u32 nAddress, s16* pData) { return false; }

bool piPut32(PI* pPI, u32 nAddress, s32* pData) {
    CpuBlock* pBlock;
    CpuBlock* pNewBlock;
    s32 bFlag = true;
    SystemObjectType storageDevice;
    s32 nAddressROM;
    s32 nSize;

    switch (nAddress & 0x3F) {
        case 0x00:
#if VERSION < MK64_J 
            pPI->nAddressRAM = *pData & 0xFFFFFF;
#else
            pPI->nAddressRAM = *pData & 0x7FFFF8;
#endif
            break;
        case 0x04:
#if VERSION < MK64_J 
            pPI->nAddressROM = *pData;
#else
            pPI->nAddressROM = *pData & ~0x1;
#endif
            break;
        case 0x08:
            pPI->nSizeGet = *pData & 0xFFFFFF;

#if VERSION >= SM64_E
            nSize = (pPI->nSizeGet + 1) & ~1;
#endif

            if (!piGetNewBlock(pPI, &pNewBlock)) {
                return false;
            }

#if VERSION < MK64_J 
            pNewBlock->nSize = pPI->nSizeGet + 1;
#else
            pNewBlock->nSize = nSize;
#endif

            pNewBlock->pfUnknown = piDMA_Complete;
            pNewBlock->nAddress0 = pPI->nAddressRAM;
            pNewBlock->nAddress1 = pPI->nAddressROM;

            if (!fn_8000A6A4(gpSystem, pNewBlock)) {
                return false;
            }
            break;
        case 0x0C:
            pPI->nSizePut = *pData & 0xFFFFFF;

#if VERSION >= SM64_E
            nSize = (pPI->nSizePut + 1) & ~1;
#endif

            if (!piGetNewBlock(pPI, &pNewBlock)) {
                return false;
            }

#if VERSION < MK64_J 
            pNewBlock->nSize = pPI->nSizePut + 1;
#else
            pNewBlock->nSize = nSize;
#endif

            pNewBlock->pfUnknown = piDMA_Complete;
            pNewBlock->nAddress0 = pPI->nAddressROM;
            pNewBlock->nAddress1 = pPI->nAddressRAM;

            if (!fn_8000A6A4(gpSystem, pNewBlock)) {
                return false;
            }
            break;
        case 0x10:
            if (*pData & 1) {
                pPI->nStatus = 0;
            }
            if (*pData & 2) {
                if (!xlObjectEvent(gpSystem, 0x1001, (void*)9)) {}
            }
            break;
        case 0x14:
            pPI->nLatency1 = *pData & 0xFF;
            break;
        case 0x18:
            pPI->nWidthPulse1 = *pData & 0xFF;
            break;
        case 0x1C:
            pPI->nSizePage1 = *pData & 0xF;
            break;
        case 0x20:
            pPI->nRelease1 = *pData & 1;
            break;
        case 0x24:
            pPI->nLatency2 = *pData & 0xFF;
            break;
        case 0x28:
            pPI->nWidthPulse2 = *pData & 0xFF;
            break;
        case 0x2C:
            pPI->nSizePage2 = *pData & 0xF;
            break;
        case 0x30:
            pPI->nRelease2 = *pData & 1;
            break;
        default:
            return false;
    }

    return true;
}

bool piPut64(PI* pPI, u32 nAddress, s64* pData) { return false; }

bool piGet8(PI* pPI, u32 nAddress, s8* pData) { return false; }

bool piGet16(PI* pPI, u32 nAddress, s16* pData) { return false; }

bool piGet32(PI* pPI, u32 nAddress, s32* pData) {
    switch (nAddress & 0x3F) {
        case 0x00:
            *pData = pPI->nAddressRAM;
            break;
        case 0x04:
            *pData = pPI->nAddressROM;
            break;
        case 0x08:
            *pData = pPI->nSizeGet;
            break;
        case 0x0C:
            *pData = pPI->nSizePut;
            break;
        case 0x10:
            *pData = pPI->nStatus & 7;
            break;
        case 0x14:
            *pData = pPI->nLatency1 & 0xFF;
            break;
        case 0x18:
            *pData = pPI->nWidthPulse1 & 0xFF;
            break;
        case 0x1C:
            *pData = pPI->nSizePage1 & 0xF;
            break;
        case 0x20:
            *pData = pPI->nRelease1 & 1;
            break;
        case 0x24:
            *pData = pPI->nLatency2 & 0xFF;
            break;
        case 0x28:
            *pData = pPI->nWidthPulse2 & 0xFF;
            break;
        case 0x2C:
            *pData = pPI->nSizePage2 & 0xF;
            break;
        case 0x30:
            *pData = pPI->nRelease2 & 1;
            break;
        default:
            return false;
    }

    return true;
}

bool piGet64(PI* pPI, u32 nAddress, s64* pData) { return false; }

bool piEvent(PI* pPI, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            pPI->nStatus = 0;
            pPI->storageDevice = SOT_CPU;
            break;
        case 0x1002:
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)&piPut8, (Put16Func)&piPut16,
                                 (Put32Func)&piPut32, (Put64Func)&piPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)&piGet8, (Get16Func)&piGet16,
                                 (Get32Func)&piGet32, (Get64Func)&piGet64)) {
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

_XL_OBJECTTYPE gClassPI = {
    "PI",
    sizeof(PI),
    NULL,
    (EventFunc)piEvent,
};
