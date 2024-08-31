#include "emulator/ram.h"
#include "emulator/cpu.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlHeap.h"

static bool ramPutControl8(Ram* pRAM, u32 nAddress, s8* pData) { return false; }

static bool ramPutControl16(Ram* pRAM, u32 nAddress, s16* pData) { return false; }

static bool ramPutControl32(Ram* pRAM, u32 nAddress, s32* pData) {
    switch (nAddress & 0x3F) {
        case RDRAM_CONFIG:
            pRAM->RDRAM_CONFIG_REG = *pData;
            break;
        case RDRAM_DEVICE_ID:
            pRAM->RDRAM_DEVICE_ID_REG = *pData;
            break;
        case RDRAM_DELAY:
            pRAM->RDRAM_DELAY_REG = *pData;
            break;
        case RDRAM_MODE:
            pRAM->RDRAM_MODE_REG = *pData;
            break;
        case RDRAM_REF_INTERVAL:
            pRAM->RDRAM_REF_INTERVAL_REG = *pData;
            break;
        case RDRAM_REF_NOW:
            pRAM->RDRAM_REF_ROW_REG = *pData;
            break;
        case RDRAM_RAS_INTERVAL:
            pRAM->RDRAM_RAS_INTERVAL_REG = *pData;
            break;
        case RDRAM_MIN_INTERVAL:
            pRAM->RDRAM_MIN_INTERVAL_REG = *pData;
            break;
        case RDRAM_ADDR_SELECT:
            pRAM->RDRAM_ADDR_SELECT_REG = *pData;
            break;
        case RDRAM_DEVICE_MANUF:
            pRAM->RDRAM_DEVICE_MANUF_REG = *pData;
            break;
        default:
            return false;
    }

    return true;
}

static bool ramPutControl64(Ram* pRAM, u32 nAddress, s64* pData) { return false; }

static bool ramGetControl8(Ram* pRAM, u32 nAddress, s8* pData) { return false; }

static bool ramGetControl16(Ram* pRAM, u32 nAddress, s16* pData) { return false; }

static bool ramGetControl32(Ram* pRAM, u32 nAddress, s32* pData) {
    *pData = 0;

    switch (nAddress & 0x3F) {
        case RDRAM_CONFIG:
            *pData = pRAM->RDRAM_CONFIG_REG;
            break;
        case RDRAM_DEVICE_ID:
            *pData = pRAM->RDRAM_DEVICE_ID_REG;
            break;
        case RDRAM_DELAY:
            *pData = pRAM->RDRAM_DELAY_REG;
            break;
        case RDRAM_MODE:
            *pData = pRAM->RDRAM_MODE_REG;
            break;
        case RDRAM_REF_INTERVAL:
            *pData = pRAM->RDRAM_REF_INTERVAL_REG;
            break;
        case RDRAM_REF_NOW:
            *pData = pRAM->RDRAM_REF_ROW_REG;
            break;
        case RDRAM_RAS_INTERVAL:
            *pData = pRAM->RDRAM_RAS_INTERVAL_REG;
            break;
        case RDRAM_MIN_INTERVAL:
            *pData = pRAM->RDRAM_MIN_INTERVAL_REG;
            break;
        case RDRAM_ADDR_SELECT:
            *pData = pRAM->RDRAM_ADDR_SELECT_REG;
            break;
        case RDRAM_DEVICE_MANUF:
            *pData = pRAM->RDRAM_DEVICE_MANUF_REG;
            break;
        default:
            return false;
    }

    return true;
}

static bool ramGetControl64(Ram* pRAM, u32 nAddress, s64* pData) { return false; }

static bool ramPutRI8(Ram* pRAM, u32 nAddress, s8* pData) { return false; }

static bool ramPutRI16(Ram* pRAM, u32 nAddress, s16* pData) { return false; }

static bool ramPutRI32(Ram* pRAM, u32 nAddress, s32* pData) {
    switch (nAddress & 0x1F) {
        case RI_MODE:
            pRAM->RI_MODE_REG = *pData & 0xF;
            break;
        case RI_CONFIG:
            pRAM->RI_CONFIG_REG = *pData & 0x7F;
        case RI_CURRENT_LOAD:
        case RI_RERROR:
        case RI_WERROR:
            break;
        case RI_SELECT:
            pRAM->RI_SELECT_REG = *pData & 0x7;
            break;
        case RI_REFRESH:
            pRAM->RI_REFRESH_REG = *pData;
            break;
        case RI_LATENCY:
            pRAM->RI_LATENCY_REG = *pData & 0xF;
            break;
        default:
            return false;
    }

    return true;
}

static bool ramPutRI64(Ram* pRAM, u32 nAddress, s64* pData) { return false; }

static bool ramGetRI8(Ram* pRAM, u32 nAddress, s8* pData) { return false; }

static bool ramGetRI16(Ram* pRAM, u32 nAddress, s16* pData) { return false; }

static bool ramGetRI32(Ram* pRAM, u32 nAddress, s32* pData) {
    switch (nAddress & 0x1F) {
        case RI_MODE:
            *pData = pRAM->RI_MODE_REG & 0xF;
            break;
        case RI_CONFIG:
            *pData = pRAM->RI_CONFIG_REG & 0x7F;
            break;
        case RI_CURRENT_LOAD:
        case RI_WERROR:
            break;
        case RI_SELECT:
            *pData = pRAM->RI_SELECT_REG & 7;
            break;
        case RI_REFRESH:
            *pData = pRAM->RI_REFRESH_REG;
            break;
        case RI_LATENCY:
            *pData = pRAM->RI_LATENCY_REG & 0xF;
            break;
        case RI_RERROR:
            *pData = 0;
            break;
        default:
            return false;
    }

    return true;
}

static bool ramGetRI64(Ram* pRAM, u32 nAddress, s64* pData) { return false; }

static bool ramPut8(Ram* pRAM, u32 nAddress, s8* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        pRAM->pBuffer[nAddress] = *pData;
    }

    return true;
}

static bool ramPut16(Ram* pRAM, u32 nAddress, s16* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        *((u16*)pRAM->pBuffer + (nAddress >> 1)) = *pData;
    }

    return true;
}

static bool ramPut32(Ram* pRAM, u32 nAddress, s32* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        *((s32*)pRAM->pBuffer + (nAddress >> 2)) = *pData;
    }

    return true;
}

static bool ramPut64(Ram* pRAM, u32 nAddress, s64* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        *((s64*)pRAM->pBuffer + (nAddress >> 3)) = *pData;
    }

    return true;
}

static bool ramGet8(Ram* pRAM, u32 nAddress, s8* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        *pData = pRAM->pBuffer[nAddress];
    } else {
        *pData = 0;
    }

    return true;
}

static bool ramGet16(Ram* pRAM, u32 nAddress, s16* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        *pData = *((s16*)pRAM->pBuffer + (nAddress >> 1));
    } else {
        *pData = 0;
    }

    return true;
}

static bool ramGet32(Ram* pRAM, u32 nAddress, s32* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        *pData = *((s32*)pRAM->pBuffer + (nAddress >> 2));
    } else {
        *pData = 0;
    }

    return true;
}

static bool ramGet64(Ram* pRAM, u32 nAddress, s64* pData) {
    nAddress &= 0x03FFFFFF;

    if (nAddress < pRAM->nSize) {
        *pData = *((s64*)pRAM->pBuffer + (nAddress >> 3));
    } else {
        *pData = 0;
    }

    return true;
}

static bool ramGetBlock(Ram* pRAM, CpuBlock* pBlock) {
    if (pBlock->pfUnknown != NULL) {
        if (!pBlock->pfUnknown(pBlock, 1)) {
            return false;
        }
    }

    return true;
}

bool ramGetBuffer(Ram* pRAM, void** ppRAM, u32 nOffset, u32* pnSize) {
    s32 nSize;

    nOffset &= 0x03FFFFFF;

    if (pRAM->nSize == 0) {
        return false;
    }

    if ((pnSize != NULL) && ((u32)(nOffset + *pnSize) >= pRAM->nSize)) {
        nSize = pRAM->nSize - nOffset;
        *pnSize = nSize;

        if (nSize < 0) {
            *pnSize = 0;
        }
    }

    *((u8**)ppRAM) = (u8*)pRAM->pBuffer + nOffset;
    return true;
}

bool ramWipe(Ram* pRAM) {
    if (pRAM->nSize != 0 && !xlHeapFill32(pRAM->pBuffer, pRAM->nSize, 0)) {
        return false;
    }

    return true;
}

bool ramSetSize(Ram* pRAM, s32 nSize) {
    s32 nSizeRAM;

    if (pRAM->pBuffer != NULL) {
        return false;
    }

    nSizeRAM = (nSize + 0x3FF) & 0xFFFFFC00;

    if (!xlHeapTake((void**)&pRAM->pBuffer, nSizeRAM | 0x30000000)) {
        return false;
    }

    pRAM->nSize = nSizeRAM;

    return true;
}

bool ramGetSize(Ram* pRAM, s32* nSize) {
    if (nSize != NULL) {
        *nSize = pRAM->nSize;
    }

    return true;
}

bool ramEvent(Ram* pRAM, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            pRAM->nSize = 0;
            pRAM->pBuffer = NULL;
            break;
        case 0x1002:
            switch (((CpuDevice*)pArgument)->nType & 0xFF) {
                case 0:
                    if (!cpuSetGetBlock(SYSTEM_CPU(gpSystem), pArgument, (GetBlockFunc)ramGetBlock)) {
                        return false;
                    }

                    if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)ramPut8, (Put16Func)ramPut16,
                                         (Put32Func)ramPut32, (Put64Func)ramPut64)) {
                        return false;
                    }

                    if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)ramGet8, (Get16Func)ramGet16,
                                         (Get32Func)ramGet32, (Get64Func)ramGet64)) {
                        return false;
                    }
                    break;
                case 1:
                    if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)ramPutRI8, (Put16Func)ramPutRI16,
                                         (Put32Func)ramPutRI32, (Put64Func)ramPutRI64)) {
                        return false;
                    }

                    if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)ramGetRI8, (Get16Func)ramGetRI16,
                                         (Get32Func)ramGetRI32, (Get64Func)ramGetRI64)) {
                        return false;
                    }
                    break;
                case 2:
                    if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)ramPutControl8,
                                         (Put16Func)ramPutControl16, (Put32Func)ramPutControl32,
                                         (Put64Func)ramPutControl64)) {
                        return false;
                    }

                    if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)ramGetControl8,
                                         (Get16Func)ramGetControl16, (Get32Func)ramGetControl32,
                                         (Get64Func)ramGetControl64)) {
                        return false;
                    }
                    break;
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

_XL_OBJECTTYPE gClassRAM = {
    "RAM",
    sizeof(Ram),
    NULL,
    (EventFunc)ramEvent,
};
