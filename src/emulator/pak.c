#include "emulator/pak.h"
#include "emulator/cpu.h"
#include "emulator/store.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlHeap.h"

_XL_OBJECTTYPE gClassPak = {
    "MEMORY-PAK",
    sizeof(Pak),
    NULL,
    (EventFunc)pakEvent,
};

static bool pakPut8(Pak* pPak, u32 nAddress, s8* pData) { return true; }

static bool pakPut16(Pak* pPak, u32 nAddress, s16* pData) { return true; }

static bool pakPut32(Pak* pPak, u32 nAddress, s32* pData) { return true; }

static bool pakPut64(Pak* pPak, u32 nAddress, s64* pData) { return true; }

static bool pakGet8(Pak* pPak, u32 nAddress, s8* pData) { return true; }

static bool pakGet16(Pak* pPak, u32 nAddress, s16* pData) { return true; }

static bool pakGet32(Pak* pPak, u32 nAddress, s32* pData) { return true; }

static bool pakGet64(Pak* pPak, u32 nAddress, s64* pData) { return true; }

static bool pakGetBlock(Pak* pPak, CpuBlock* pBlock) {
    if (pBlock->pfUnknown != NULL) {
        if (!pBlock->pfUnknown(pBlock, 1)) {
            return false;
        }
    }

    return true;
}

bool fn_80044708(Pak* pPak, s32 arg2, u32 nUnknown, void* pBuffer) {
    SystemRomType eTypeROM;
    s32 nOffset;

    nOffset = (nUnknown & 0xFFFF) << 5;
    eTypeROM = gpSystem->eTypeROM;

    switch (eTypeROM) {
        case NKTJ:
        case NKTE:
        case NKTP:
            fn_80061B88(pPak->pStore, pBuffer, nOffset + 0x200, 0x20);
            break;
        default:
            break;
    }

    return true;
}

bool fn_8004477C(Pak* pPak, s32 arg2, u32 nUnknown, void* pBuffer) {
    SystemRomType eTypeROM;
    s32 nOffset;

    if (nUnknown == 0x400) {
        return true;
    }

    eTypeROM = gpSystem->eTypeROM;
    nOffset = (nUnknown & 0xFFFF) << 5;

    switch (eTypeROM) {
        case NKTJ:
        case NKTE:
        case NKTP:
            fn_80061BC0(pPak->pStore, pBuffer, nOffset + 0x200, 0x20);
            break;
        default:
            break;
    }

    return true;
}

static inline bool pakEvent_UnknownInline(Pak* pPak, void* pArgument) {
    s32 var_r6;

    if (pPak->pStore != NULL && !storeFreeObject((void**)&pPak->pStore)) {
        return false;
    }

    if ((s32)pArgument < 0x8000) {
        var_r6 = 0x8000;
    } else {
        var_r6 = 0x10000;
    }

    pPak->unk_00 = var_r6;

    if (!fn_80061770((void**)&pPak->pStore, "PAK", gpSystem->eTypeROM, (void*)var_r6)) {
        return false;
    }

    return true;
}

bool pakEvent(Pak* pPak, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            xlHeapTake((void**)&pPak->pRAM, 0x8000);
            if (!pakEvent_UnknownInline(pPak, pArgument)) {
                return false;
            }
            break;
        case 3:
            xlHeapFree((void**)&pPak->pRAM);
            break;
        case 0x1002:
            if (!cpuSetGetBlock(SYSTEM_CPU(gpSystem), (CpuDevice*)pArgument, (GetBlockFunc)pakGetBlock)) {
                return false;
            }
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), (CpuDevice*)pArgument, (Put8Func)pakPut8, (Put16Func)pakPut16,
                                 (Put32Func)pakPut32, (Put64Func)pakPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), (CpuDevice*)pArgument, (Get8Func)pakGet8, (Get16Func)pakGet16,
                                 (Get32Func)pakGet32, (Get64Func)pakGet64)) {
                return false;
            }
        case 0:
        case 1:
        case 0x1003:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return false;
    }

    return true;
}
