#include "emulator/eeprom.h"
#include "emulator/cpu.h"
#include "emulator/store.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlHeap.h"

_XL_OBJECTTYPE gClassEEPROM = {
    "MEMORY-PAK",
    sizeof(EEPROM),
    NULL,
    (EventFunc)eepromEvent,
};

static bool eepromPut8(EEPROM* pEEPROM, u32 nAddress, s8* pData);
static bool eepromPut16(EEPROM* pEEPROM, u32 nAddress, s16* pData);
static bool eepromPut32(EEPROM* pEEPROM, u32 nAddress, s32* pData);
static bool eepromPut64(EEPROM* pEEPROM, u32 nAddress, s64* pData);
static bool eepromGet8(EEPROM* pEEPROM, u32 nAddress, s8* pData);
static bool eepromGet16(EEPROM* pEEPROM, u32 nAddress, s16* pData);
static bool eepromGet32(EEPROM* pEEPROM, u32 nAddress, s32* pData);
static bool eepromGet64(EEPROM* pEEPROM, u32 nAddress, s64* pData);
static bool eepromGetBlock(EEPROM* pEEPROM, CpuBlock* pBlock);
static inline bool eepromEvent_UnknownInline(EEPROM* pEEPROM, void* pArgument);

static bool eepromPut8(EEPROM* pEEPROM, u32 nAddress, s8* pData) { return true; }

static bool eepromPut16(EEPROM* pEEPROM, u32 nAddress, s16* pData) { return true; }

static bool eepromPut32(EEPROM* pEEPROM, u32 nAddress, s32* pData) { return true; }

static bool eepromPut64(EEPROM* pEEPROM, u32 nAddress, s64* pData) { return true; }

static bool eepromGet8(EEPROM* pEEPROM, u32 nAddress, s8* pData) { return true; }

static bool eepromGet16(EEPROM* pEEPROM, u32 nAddress, s16* pData) { return true; }

static bool eepromGet32(EEPROM* pEEPROM, u32 nAddress, s32* pData) { return true; }

static bool eepromGet64(EEPROM* pEEPROM, u32 nAddress, s64* pData) { return true; }

static bool eepromGetBlock(EEPROM* pEEPROM, CpuBlock* pBlock) {
    if (pBlock->pfUnknown != NULL) {
        if (!pBlock->pfUnknown(pBlock, 1)) {
            return false;
        }
    }

    return true;
}

bool fn_80044708(EEPROM* pEEPROM, s32 arg2, u32 nUnknown, void* pBuffer) {
    SystemRomType eTypeROM;
    s32 nOffset;

    nOffset = (nUnknown & 0xFFFF) << 5;
    eTypeROM = gpSystem->eTypeROM;

    switch (eTypeROM) {
        case NKTJ:
        case NKTE:
        case NKTP:
            fn_80061B88(pEEPROM->pStore, pBuffer, nOffset + 0x200, 0x20);
            break;
        default:
            break;
    }

    return true;
}

bool fn_8004477C(EEPROM* pEEPROM, s32 arg2, u32 nUnknown, void* pBuffer) {
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
            fn_80061BC0(pEEPROM->pStore, pBuffer, nOffset + 0x200, 0x20);
            break;
        default:
            break;
    }

    return true;
}

static inline bool eepromEvent_UnknownInline(EEPROM* pEEPROM, void* pArgument) {
    s32 var_r6;

    if (pEEPROM->pStore != NULL && !storeFreeObject((void**)&pEEPROM->pStore)) {
        return false;
    }

    if ((s32)pArgument < 0x8000) {
        var_r6 = 0x8000;
    } else {
        var_r6 = 0x10000;
    }

    pEEPROM->unk_00 = var_r6;

    if (!fn_80061770((void**)&pEEPROM->pStore, "PAK", gpSystem->eTypeROM, (void*)var_r6)) {
        return false;
    }

    return true;
}

bool eepromEvent(EEPROM* pEEPROM, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            xlHeapTake((void**)&pEEPROM->pRAM, 0x8000);
            if (!eepromEvent_UnknownInline(pEEPROM, pArgument)) {
                return false;
            }
            break;
        case 3:
            xlHeapFree((void**)&pEEPROM->pRAM);
            break;
        case 0x1002:
            if (!cpuSetGetBlock(SYSTEM_CPU(gpSystem), (CpuDevice*)pArgument, (GetBlockFunc)eepromGetBlock)) {
                return false;
            }
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), (CpuDevice*)pArgument, (Put8Func)eepromPut8,
                                 (Put16Func)eepromPut16, (Put32Func)eepromPut32, (Put64Func)eepromPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), (CpuDevice*)pArgument, (Get8Func)eepromGet8,
                                 (Get16Func)eepromGet16, (Get32Func)eepromGet32, (Get64Func)eepromGet64)) {
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
