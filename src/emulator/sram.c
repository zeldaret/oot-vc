#include "emulator/sram.h"
#include "emulator/cpu.h"
#include "emulator/ram.h"
#include "emulator/store.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"

static bool sramPut8(Sram* pSRAM, u32 nAddress, s8* pData) {
    fn_80061BC0(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s8));
    return true;
}

static bool sramPut16(Sram* pSRAM, u32 nAddress, s16* pData) {
    fn_80061BC0(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s16));
    return true;
}

static bool sramPut32(Sram* pSRAM, u32 nAddress, s32* pData) {
    fn_80061BC0(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s32));
    return true;
}

static bool sramPut64(Sram* pSRAM, u32 nAddress, s64* pData) {
    fn_80061BC0(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s64));
    return true;
}

static bool sramGet8(Sram* pSRAM, u32 nAddress, s8* pData) {
    fn_80061B88(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s8));
    return true;
}

static bool sramGet16(Sram* pSRAM, u32 nAddress, s16* pData) {
    fn_80061B88(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s16));
    return true;
}

static bool sramGet32(Sram* pSRAM, u32 nAddress, s32* pData) {
    fn_80061B88(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s32));
    return true;
}

static bool sramGet64(Sram* pSRAM, u32 nAddress, s64* pData) {
    fn_80061B88(pSRAM->pStore, (u8*)pData, nAddress & 0x7FFF, sizeof(s64));
    return true;
}

static bool sramGetBlock(Sram* pSRAM, CpuBlock* pBlock) {
    void* pRAM;

    if (pBlock->nAddress1 < 0x04000000) {
        if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pRAM, pBlock->nAddress1, &pBlock->nSize)) {
            return false;
        }

        fn_80061B88(pSRAM->pStore, pRAM, pBlock->nAddress0 & 0x7FFF, pBlock->nSize);
    } else {
        if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pRAM, pBlock->nAddress0, &pBlock->nSize)) {
            return false;
        }

        fn_80061BC0(pSRAM->pStore, pRAM, pBlock->nAddress1 & 0x7FFF, pBlock->nSize);
    }

    if (pBlock->pfUnknown != NULL && !pBlock->pfUnknown(pBlock, 1)) {
        return false;
    }

    return true;
}

static inline bool sramEvent_UnknownInline(Sram* pSRAM, void* pArgument) {
    if (pSRAM->pStore != NULL && !storeFreeObject((void**)&pSRAM->pStore)) {
        return false;
    }

    pSRAM->unk_00 = 0x8000;

    if (!fn_80061770((void**)&pSRAM->pStore, "RAM", gpSystem->eTypeROM, pArgument)) {
        return false;
    }

    return true;
}

bool sramEvent(Sram* pSRAM, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            if (!sramEvent_UnknownInline(pSRAM, pArgument)) {
                return false;
            }
            break;
        case 0x1002:
            if (!cpuSetGetBlock(SYSTEM_CPU(gpSystem), (CpuDevice*)pArgument, (GetBlockFunc)sramGetBlock)) {
                return false;
            }
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)sramPut8, (Put16Func)sramPut16,
                                 (Put32Func)sramPut32, (Put64Func)sramPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)sramGet8, (Get16Func)sramGet16,
                                 (Get32Func)sramGet32, (Get64Func)sramGet64)) {
                return false;
            }
            break;
        case 0:
        case 1:
        case 3:
        case 0x1003:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return false;
    }

    return true;
}

_XL_OBJECTTYPE gClassSram = {
    "SRAM",
    sizeof(Sram),
    NULL,
    (EventFunc)sramEvent,
};
