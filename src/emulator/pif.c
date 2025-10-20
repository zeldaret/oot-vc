#include "emulator/pif.h"
#include "emulator/controller.h"
#include "emulator/flash.h"
#include "emulator/pak.h"
#include "emulator/storeRVL.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlHeap.h"

_XL_OBJECTTYPE gClassPIF = {
    "PIF",
    sizeof(Pif),
    NULL,
    (EventFunc)pifEvent,
};

// this function is a copy-paste of `__osContDataCrc`
#if VERSION < MK64_J 
static u8 pifContDataCrc(Pif* pPIF, u8* data) {
    u32 temp = 0;
    u32 i;
    u32 j;

    for (i = PIF_DATA_CRC_MESSAGE_BYTES; i != 0; data++, i--) {
        // Loop over each bit in the byte starting with most significant
        for (j = (1 << (PIF_DATA_CRC_LENGTH - 1)); j != 0; j >>= 1) {
            temp <<= 1;
            if (*data & j) {
                if (temp & (1 << PIF_DATA_CRC_LENGTH)) {
                    // Same as ret++; ret ^= 0x85 since last bit always 0 after the shift
                    temp ^= PIF_DATA_CRC_GENERATOR - 1;
                } else {
                    temp++;
                }
            } else if (temp & (1 << PIF_DATA_CRC_LENGTH)) {
                temp ^= PIF_DATA_CRC_GENERATOR;
            }
        }
    }

    // Act like a byte of zeros is appended to data
    do {
        temp <<= 1;
        if (temp & (1 << PIF_DATA_CRC_LENGTH)) {
            temp ^= PIF_DATA_CRC_GENERATOR;
        }
        i++;
    } while (i < PIF_DATA_CRC_LENGTH);

    // Discarding the excess is done automatically by the return type
    return temp;
}
#else
static u8 pifContDataCrc(u8* data) {
    u32 i;
    u32 j;
    u32 temp = 0;

    for (i = PIF_DATA_CRC_MESSAGE_BYTES; i != 0; data++, i--) {
        // Loop over each bit in the byte starting with most significant
        for (j = (1 << (PIF_DATA_CRC_LENGTH - 1)); j != 0; j >>= 1) {
            temp <<= 1;
            if (*data & j) {
                if (temp & (1 << PIF_DATA_CRC_LENGTH)) {
                    // Same as ret++; ret ^= 0x85 since last bit always 0 after the shift
                    temp ^= PIF_DATA_CRC_GENERATOR - 1;
                } else {
                    temp++;
                }
            } else if (temp & (1 << PIF_DATA_CRC_LENGTH)) {
                temp ^= PIF_DATA_CRC_GENERATOR;
            }
        }
    }

    // Act like a byte of zeros is appended to data
    do {
        temp <<= 1;
        if (temp & (1 << PIF_DATA_CRC_LENGTH)) {
            temp ^= PIF_DATA_CRC_GENERATOR;
        }
        i++;
    } while (i < PIF_DATA_CRC_LENGTH);

    // Discarding the excess is done automatically by the return type
    return temp;
}
#endif

static bool fn_80040BF4(Pif* pPIF, u8* buffer, u8* ptx, u8* prx, s32 channel) {
    *prx &= 0x3F;

    switch (pPIF->eControllerType[channel]) {
        case CT_CONTROLLER:
            buffer[1] = 5;
            buffer[2] = 0;
            buffer[3] = 2;
            break;
        case CT_CONTROLLER_W_RPAK:
            buffer[1] = 5;
            buffer[2] = 0;
            buffer[3] = 1;
            break;
        case CT_CONTROLLER_W_PAK:
            buffer[1] = 5;
            buffer[2] = 0;
            buffer[3] = channel == 0 ? 1 : 2;
            break;
        default:
            *prx |= 0xC0;
            break;
    }

    return true;
}

static inline bool pifQueryController(Pif* pPIF, u8* buffer, s32 channel) {
    if (pPIF->eControllerType[channel] == CT_NONE) {
        return false;
    }

    *(u16*)(buffer + 1) = pPIF->controllerType[channel];
    *(buffer + 3) = pPIF->controllerStatus[channel];

    return true;
}

static inline bool pifReadController(Pif* pPIF, u8* buffer, u8* ptx, u8* prx, s32 channel) {
    if (pPIF->eControllerType[channel] == CT_NONE) {
        *prx |= 0x80;
    }

    if (!simulatorReadController(channel, (u32*)&buffer[1], ptx)) {
        return false;
    }

    return true;
}

bool pifExecuteCommand(Pif* pPIF, u8* buffer, u8* ptx, u8* prx, s32 channel) {
    switch (*buffer) {
        case 0x00:
#if VERSION < MK64_J 
            if (!fn_80040BF4(pPIF, buffer, ptx, prx, channel)) {
                return false;
            }
#else
            if (!fn_80040BF4(pPIF, buffer, ptx, prx, channel % 4)) {
                return false;
            }
#endif
            break;
        case 0xFF:
            if (!fn_80040BF4(pPIF, buffer, ptx, prx, channel)) {
                return false;
            }
            break;
        case 0x01: {
            bool result = fn_80062E5C(SYSTEM_CONTROLLER(gpSystem), channel, (s32*)&buffer[1]) ? true : false;

            if (!result) {
                return false;
            }
            break;
        }
        case 0x02: {
            u16 nAddress = (buffer[1] << 3) | (buffer[2] >> 5);
            u8* pBuffer = buffer + 3;
            int i = 0;

#if VERSION < MK64_J 
            switch (pPIF->eControllerType[channel]) {
                case CT_CONTROLLER_W_RPAK:
                    //! TODO: fake match?
                    pBuffer[i++] = 0;
                    pBuffer[1] = 0;
                    pBuffer[2] = 0;
                    pBuffer[3] = 0;
                    pBuffer[4] = 0;
                    pBuffer[5] = 0;
                    pBuffer[6] = 0;
                    pBuffer[7] = 0;
                    pBuffer[8] = 0;
                    pBuffer[9] = 0;
                    pBuffer[10] = 0;
                    pBuffer[11] = 0;
                    pBuffer[11] = 0;
                    pBuffer[12] = 0;
                    pBuffer[13] = 0;
                    pBuffer[14] = 0;
                    pBuffer[15] = 0;
                    pBuffer[16] = 0;
                    pBuffer[17] = 0;
                    pBuffer[18] = 0;
                    pBuffer[19] = 0;
                    pBuffer[20] = 0;
                    pBuffer[21] = 0;
                    pBuffer[22] = 0;
                    pBuffer[23] = 0;
                    pBuffer[24] = 0;
                    pBuffer[25] = 0;
                    pBuffer[26] = 0;
                    pBuffer[27] = 0;
                    pBuffer[28] = 0;
                    pBuffer[29] = 0;
                    pBuffer[30] = 0;
                    pBuffer[31] = 0;

                    switch (nAddress) {
                        case 0x400:
                            pBuffer[0] = 0x80;
                            pBuffer[1] = 0x80;
                            pBuffer[2] = 0x80;
                            pBuffer[3] = 0x80;
                            pBuffer[4] = 0x80;
                            pBuffer[5] = 0x80;
                            pBuffer[6] = 0x80;
                            pBuffer[7] = 0x80;
                            pBuffer[8] = 0x80;
                            pBuffer[9] = 0x80;
                            pBuffer[10] = 0x80;
                            pBuffer[11] = 0x80;
                            pBuffer[11] = 0x80;
                            pBuffer[12] = 0x80;
                            pBuffer[13] = 0x80;
                            pBuffer[14] = 0x80;
                            pBuffer[15] = 0x80;
                            pBuffer[16] = 0x80;
                            pBuffer[17] = 0x80;
                            pBuffer[18] = 0x80;
                            pBuffer[19] = 0x80;
                            pBuffer[20] = 0x80;
                            pBuffer[21] = 0x80;
                            pBuffer[22] = 0x80;
                            pBuffer[23] = 0x80;
                            pBuffer[24] = 0x80;
                            pBuffer[25] = 0x80;
                            pBuffer[26] = 0x80;
                            pBuffer[27] = 0x80;
                            pBuffer[28] = 0x80;
                            pBuffer[29] = 0x80;
                            pBuffer[30] = 0x80;
                            pBuffer[31] = 0x80;
                            break;
                        default:
                            break;
                    }

                    break;
                case CT_CONTROLLER_W_PAK:
                    fn_80044708(SYSTEM_PAK(gpSystem), channel, nAddress, (void*)pBuffer);
                    break;
                default:
                    break;
            }

            buffer[0x23] = pifContDataCrc(pPIF, buffer + 3);
#else
            switch (pPIF->eControllerType[channel]) {
                case CT_CONTROLLER_W_RPAK:
                    for (i = 0; i < 32U; i++) {
                        pBuffer[i] = nAddress == 0x400 ? 0x80 : 0;
                    }
                    break;
                case CT_CONTROLLER_W_PAK:
                    fn_80044708(SYSTEM_PAK(gpSystem), channel, nAddress, (void*)pBuffer);
                    break;
                default:
                    break;
            }

            buffer[0x23] = pifContDataCrc(buffer + 3);
#endif
            break;
        }
        case 0x03: {
            u16 nAddress = (buffer[1] << 3) | (buffer[2] >> 5);
            u8* pBuffer = buffer + 3;

            switch (pPIF->eControllerType[channel]) {
                case CT_CONTROLLER_W_RPAK:
#if VERSION < MK64_J 
                    switch (nAddress) {
                        case 0x600:
                            if (*pBuffer == 1) {
                                fn_80062CE4(SYSTEM_CONTROLLER(gpSystem), channel, true);
                            } else if (*pBuffer == 0) {
                                fn_80062CE4(SYSTEM_CONTROLLER(gpSystem), channel, false);
                            }
                            break;
                        default:
                            break;
                    }
#else
                    if (nAddress == 0x600) {
                        fn_80062CE4(SYSTEM_CONTROLLER(gpSystem), channel, *pBuffer ? true : false);
                    }
#endif
                    break;
                case CT_CONTROLLER_W_PAK:
                    fn_8004477C(SYSTEM_PAK(gpSystem), channel, nAddress, (void*)pBuffer);
                    break;
                default:
                    break;
            }

#if VERSION < MK64_J 
            buffer[0x23] = pifContDataCrc(pPIF, buffer + 3);
#else
            buffer[0x23] = pifContDataCrc(buffer + 3);
#endif
            break;
        }
        case 0x04:
            if (!fn_80045260(SYSTEM_FLASH(gpSystem), buffer[1], (void*)&buffer[2])) {
                return false;
            }
            break;
        case 0x05:
            if (!fn_800452B0(SYSTEM_FLASH(gpSystem), buffer[1], (void*)&buffer[2])) {
                return false;
            }
            break;
        case 0x06:
            buffer[1] = 0;
            buffer[2] = 0x10;
            buffer[3] = 0x80;
            break;
        case 0x07:
            buffer[3] = 2;
            buffer[6] = 0x59;
            buffer[7] = 0x23;
            break;
        case 0x08:
            buffer[0xA] = 0;
            break;
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0xFE:
            break;
        default:
            return false;
    }

    return true;
}
bool pifProcessInputData(Pif* pPIF) {
    u8* prx;
    u8* ptx;
    int iData;
    s32 channel;

    iData = 0;
    channel = 0;

    while ((PIF_GET_RAM_DATA(pPIF, iData) != 0xFE && PIF_GET_RAM_DATA(pPIF, iData + 1) != 0xFE)) {
        while (PIF_GET_RAM_DATA(pPIF, iData) == 0xFF || PIF_GET_RAM_DATA(pPIF, iData) == 0xFD) {
            iData++;
        }

        if (PIF_GET_RAM_DATA(pPIF, iData) == 0xFE) {
            break;
        }

        ptx = PIF_GET_RAM_ADDR(pPIF, iData++);
        if (*ptx == 0) {
            channel++;
            continue;
        }

        prx = PIF_GET_RAM_ADDR(pPIF, iData++);
        if (!pifExecuteCommand(pPIF, PIF_GET_RAM_ADDR(pPIF, iData), ptx, prx, channel)) {
            return false;
        }

        channel++;
        switch (PIF_GET_RAM_DATA(pPIF, iData)) {
            case 0xFF:
            case 0x00:
                iData += 4;
                break;
            case 0x01:
                iData += 5;
                break;
            case 0x02:
            case 0x03:
                iData += 0x24;
                break;
            case 0x04:
            case 0x05:
                iData += 0xA;
                break;
            case 0x06:
                iData += 4;
                break;
            case 0x07:
                iData += 0xB;
                break;
            case 0x08:
                iData += 0xB;
                break;
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
            case 0xFE:
                return false;
            default:
                return false;
        }
    }

    ((u8*)pPIF->pRAM)[0x3F] = 0;
    return true;
}

bool pifProcessOutputData(Pif* pPIF) {
    u8* prx;
    u8* ptx;
    int iData;
    s32 channel;
    u8* address;

    iData = 0;
    channel = 0;

    while ((PIF_GET_RAM_DATA(pPIF, iData) != 0xFE && PIF_GET_RAM_DATA(pPIF, iData + 1) != 0xFE)) {
        while (PIF_GET_RAM_DATA(pPIF, iData) == 0xFF || PIF_GET_RAM_DATA(pPIF, iData) == 0xFD) {
            iData++;
        }

        if (PIF_GET_RAM_DATA(pPIF, iData) == 0xFE) {
            break;
        }

        ptx = PIF_GET_RAM_ADDR(pPIF, iData++);
        if (*ptx == 0) {
            channel++;
            continue;
        }

        prx = PIF_GET_RAM_ADDR(pPIF, iData++);
        address = PIF_GET_RAM_ADDR(pPIF, iData);
        if (*address == 1) {
            if (!pifExecuteCommand(pPIF, address, ptx, prx, channel)) {
                return false;
            }
        }

        channel++;
        switch (PIF_GET_RAM_DATA(pPIF, iData)) {
            case 0xFF:
            case 0x00:
                iData += 4;
                break;
            case 0x01:
                iData += 5;
                break;
            case 0x02:
            case 0x03:
                iData += 0x24;
                break;
            case 0x04:
            case 0x05:
                iData += 0xA;
                break;
            case 0x06:
                iData += 4;
                break;
            case 0x07:
                iData += 0xB;
                break;
            case 0x08:
                iData += 0xB;
                break;
            case 0x09:
            case 0x0A:
            case 0x0B:
            case 0x0C:
            case 0x0D:
            case 0xFE:
                return false;
            default:
                return false;
        }
    }

    ((u8*)pPIF->pRAM)[0x3F] = 0;
    return true;
}

#if VERSION < MK64_J 
bool pifSetData(Pif* pPIF, u8* acData) {
    if (!xlHeapCopy(pPIF->pRAM, acData, 0x40)) {
        return false;
    }

    if (!pifProcessInputData(pPIF)) {
        return false;
    }

    return true;
}

bool pifGetData(Pif* pPIF, u8* acData) {
    if (!pifProcessOutputData(pPIF)) {
        return false;
    }

    if (!xlHeapCopy(acData, pPIF->pRAM, 0x40)) {
        return false;
    }

    return true;
}
#endif

static bool pifPut8(Pif* pPIF, u32 nAddress, s8* pData) {
    nAddress &= 0x7FF;

    if (nAddress >= PIF_RAM_START) {
        *((s8*)(pPIF->pRAM) + (nAddress - PIF_RAM_START)) = *pData;
    }

    return true;
}

static bool pifPut16(Pif* pPIF, u32 nAddress, s16* pData) {
    if ((nAddress & 0x7FF) >= PIF_RAM_START) {
        *((s16*)(pPIF->pRAM) + (((nAddress & 0x7FF) - PIF_RAM_START) >> 1)) = *pData;
    }

    return true;
}

static bool pifPut32(Pif* pPIF, u32 nAddress, s32* pData) {
    if ((nAddress & 0x7FF) >= PIF_RAM_START) {
        *((s32*)(pPIF->pRAM) + (((nAddress & 0x7FF) - PIF_RAM_START) >> 2)) = *pData;
    }

    return true;
}

static bool pifPut64(Pif* pPIF, u32 nAddress, s64* pData) {
    if ((nAddress & 0x7FF) >= PIF_RAM_START) {
        *((s64*)(pPIF->pRAM) + (((nAddress & 0x7FF) - PIF_RAM_START) >> 3)) = *pData;
    }

    return true;
}

static bool pifGet8(Pif* pPIF, u32 nAddress, s8* pData) {
    nAddress &= 0x7FF;

    if (nAddress < PIF_RAM_START) {
        *pData = *((s8*)pPIF->pROM + (nAddress & 0x7FF));
    } else {
        *pData = *((s8*)pPIF->pROM + (nAddress - PIF_RAM_START));
    }

    return true;
}

static bool pifGet16(Pif* pPIF, u32 nAddress, s16* pData) {
    nAddress &= 0x7FF;

    if (nAddress < PIF_RAM_START) {
        *pData = *((s16*)pPIF->pROM + (nAddress >> 1));
    } else {
        *pData = *((s16*)pPIF->pROM + ((nAddress - PIF_RAM_START) >> 1));
    }

    return true;
}

static bool pifGet32(Pif* pPIF, u32 nAddress, s32* pData) {
    nAddress &= 0x7FF;

    if (nAddress < PIF_RAM_START) {
        *pData = *((s32*)pPIF->pROM + (nAddress >> 2));
    } else {
        *pData = *((s32*)pPIF->pROM + ((nAddress - PIF_RAM_START) >> 2));
    }

    return true;
}

static bool pifGet64(Pif* pPIF, u32 nAddress, s64* pData) {
    nAddress &= 0x7FF;

    if (nAddress < PIF_RAM_START) {
        *pData = *((s64*)pPIF->pROM + (nAddress >> 3));
    } else {
        *pData = *((s64*)pPIF->pROM + ((nAddress - PIF_RAM_START) >> 3));
    }

    return true;
}

#if VERSION >= MK64_J
bool pifSetData(Pif* pPIF, u8* acData) {
    if (!xlHeapCopy(pPIF->pRAM, acData, 0x40)) {
        return false;
    }

    if (!pifProcessInputData(pPIF)) {
        return false;
    }

    return true;
}

bool pifGetData(Pif* pPIF, u8* acData) {
    if (!pifProcessOutputData(pPIF)) {
        return false;
    }

    if (!xlHeapCopy(acData, pPIF->pRAM, 0x40)) {
        return false;
    }

    return true;
}
#endif

bool pifSetControllerType(Pif* pPIF, s32 channel, ControllerType type) {
    if (!simulatorDetectController(SYSTEM_CONTROLLER(gpSystem), channel)) {
        type = CT_NONE;
    }

    pPIF->controllerStatus[channel] = 0;
    pPIF->eControllerType[channel] = type;

    switch (type) {
        case CT_NONE:
            pPIF->controllerType[channel] = 0;
            break;
        case CT_CONTROLLER:
            pPIF->controllerType[channel] = 0x500;
            break;
        case CT_CONTROLLER_W_PAK:
        case CT_CONTROLLER_W_RPAK:
            pPIF->controllerType[channel] = 0x500;
            pPIF->controllerStatus[channel] = 1;
            break;
        case CT_MOUSE:
            pPIF->controllerType[channel] = 0x200;
            break;
        case CT_VOICE:
            pPIF->controllerType[channel] = 1;
            break;
        default:
            return false;
    }

    return true;
}

bool pifEvent(Pif* pPIF, s32 nEvent, void* pArgument) {
    s32 i;

    switch (nEvent) {
        case 2:
            if (!xlHeapTake(&pPIF->pROM, 0x800)) {
                return false;
            }

            if (!xlHeapTake(&pPIF->pRAM, 0x40)) {
                return false;
            }

            for (i = 0; i < ARRAY_COUNT(pPIF->controllerType); i++) {
                if (!pifSetControllerType(pPIF, i, CT_NONE)) {
                    return false;
                }
            }
            break;
        case 3:
            for (i = 0; i < ARRAY_COUNT(pPIF->controllerType); i++) {
                if (!pifSetControllerType(pPIF, i, CT_NONE)) {
                    return false;
                }
            }

            xlHeapFree(&pPIF->pROM);
            xlHeapFree(&pPIF->pRAM);
            break;
        case 0x1002:
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)pifPut8, (Put16Func)pifPut16,
                                 (Put32Func)pifPut32, (Put64Func)pifPut64)) {
                return false;
            }

            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)pifGet8, (Get16Func)pifGet16,
                                 (Get32Func)pifGet32, (Get64Func)pifGet64)) {
                return false;
            }
            break;
        case 0:
        case 1:
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
