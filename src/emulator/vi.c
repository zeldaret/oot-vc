#include "emulator/vi.h"
#include "emulator/frame.h"
#include "emulator/ram.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlCoreRVL.h"

#include "versions.h"

// Note: video.c in oot-gc

bool viPut8(VI* pVI, u32 nAddress, s8* pData) { return false; }

bool viPut16(VI* pVI, u32 nAddress, s16* pData) { return false; }

bool viPut32(VI* pVI, u32 nAddress, s32* pData) {
    void* pRAM;
    Frame* pFrame;
    FrameBuffer* pBuffer;
    s32 nAddressData;

    switch (nAddress & 0x3F) {
        case 0x0:
            pVI->nStatus = *pData & 0xFFFF;
            break;
        case 0x4:
            nAddressData = *pData & 0xFFFFFF;
            if (nAddressData != pVI->nAddress) {
                if (!rspFrameComplete(SYSTEM_RSP(gpSystem))) {
                    return false;
                }

                pVI->nAddress = nAddressData;
                pFrame = SYSTEM_FRAME(gpSystem);
                pBuffer = &pFrame->aBuffer[FBT_COLOR_SHOW];

                if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pRAM, pVI->nAddress, NULL)) {
                    return false;
                }

                if (pBuffer->pData != pRAM) {
                    pBuffer->nFormat = 0;
                    pBuffer->nSize = 2;
                    pBuffer->nWidth = pVI->nSizeX;
                    pBuffer->pData = pRAM;

                    if (!frameSetBuffer(pFrame, 2)) {
                        return false;
                    }
                }
            }
            break;
        case 0x8:
            pVI->nSizeX = *pData & 0xFFF;
            break;
        case 0xC:
            pVI->nScanInterrupt = *pData & 0x3FF;
            break;
        case 0x10:
            xlObjectEvent(gpSystem, 0x1001, (void*)8);
            pVI->nScanInterrupt = 0x10000;
            break;
        case 0x14:
            pVI->nTiming = *pData;
            break;
        case 0x18:
            pVI->nSyncV = *pData & 0x3FF;
            break;
        case 0x1C:
            pVI->nSyncH = *pData & 0x1FFFFF;
            break;
        case 0x20:
            pVI->nSyncLeap = *pData & 0x0FFFFFFF;
            break;
        case 0x24:
            if ((pVI->nStartH = *pData & 0x03FF03FF) == 0) {
                if (pVI->bBlack != true) {
                    pVI->bBlack = true;
                }
            } else {
                if (pVI->bBlack != false) {
                    pVI->bBlack = false;
                }
            }
            break;
        case 0x28:
            pVI->nStartV = *pData & 0x03FF03FF;
            break;
        case 0x2C:
            pVI->nBurst = *pData & 0x03FF03FF;
            break;
        case 0x30:
            pVI->nScaleX = *pData & 0xFFF;
            if (!frameSetSize(SYSTEM_FRAME(gpSystem), FS_SOURCE, (s32)(pVI->nScaleX * N64_FRAME_WIDTH) / 512,
                              (s32)(pVI->nScaleY * N64_FRAME_HEIGHT) / 1024)) {
                return false;
            }
            break;
        case 0x34:
            pVI->nScaleY = *pData & 0xFFF;

#if VERSION >= MK64_J
            if (fn_8007FC84() && (pVI->nScaleY == 0x354 || pVI->nScaleY == 0x361)) {
                pVI->nScaleY = 0x400;
            }
#endif

            if (!frameSetSize(SYSTEM_FRAME(gpSystem), FS_SOURCE, (s32)(pVI->nScaleX * N64_FRAME_WIDTH) / 512,
                              (s32)(pVI->nScaleY * N64_FRAME_HEIGHT) / 1024)) {
                return false;
            }
            break;
        default:
            return false;
    }

    return true;
}

bool viPut64(VI* pVI, u32 nAddress, s64* pData) { return false; }

bool viGet8(VI* pVI, u32 nAddress, s8* pData) { return false; }

bool viGet16(VI* pVI, u32 nAddress, s16* pData) { return false; }

bool viGet32(VI* pVI, u32 nAddress, s32* pData) {
    switch (nAddress & 0x3F) {
        case 0x0:
            *pData = pVI->nStatus;
            break;
        case 0x4:
            *pData = pVI->nAddress;
            break;
        case 0x8:
            *pData = pVI->nSizeX;
            break;
        case 0xC:
            *pData = pVI->nScanInterrupt & 0xFFFF;
            break;
        case 0x10:
            pVI->nScan = VIGetCurrentLine() * 2;
            *pData = pVI->nScan;
            break;
        case 0x14:
            *pData = pVI->nTiming;
            break;
        case 0x18:
            *pData = pVI->nSyncV;
            break;
        case 0x1C:
            *pData = pVI->nSyncH;
            break;
        case 0x20:
            *pData = pVI->nSyncLeap;
            break;
        case 0x24:
            *pData = pVI->nStartH;
            break;
        case 0x28:
            *pData = pVI->nStartV;
            break;
        case 0x2C:
            *pData = pVI->nBurst;
            break;
        case 0x30:
            *pData = pVI->nScaleX;
            break;
        case 0x34:
            *pData = pVI->nScaleY;
            break;
        default:
            return false;
    }

    return true;
}

bool viGet64(VI* pVI, u32 nAddress, s64* pData) { return false; }

bool viForceRetrace(VI* pVI) {
    if (!systemExceptionPending(gpSystem, SIT_VI) && (pVI->nStatus & 3)) {
        pVI->nScan = pVI->nScanInterrupt;
        xlObjectEvent(gpSystem, 0x1000, (void*)8);
        return true;
    }

    return false;
}

bool viEvent(VI* pVI, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            pVI->nScan = 0;
            pVI->nBurst = 0;
            pVI->nSizeX = 0;
            pVI->nStatus = 0;
            pVI->nTiming = 0;
            pVI->nAddress = 0;
            pVI->nScaleX = 0;
            pVI->nScaleY = 0;
            pVI->nStartH = 0;
            pVI->nStartV = 0;
            pVI->nSyncH = 0;
            pVI->nSyncV = 0;
            pVI->nSyncLeap = 0;
            pVI->bBlack = false;
            pVI->nScanInterrupt = 0x10000;
        case 0:
        case 1:
        case 3:
        case 5:
            break;
        case 0x1002:
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)viPut8, (Put16Func)viPut16,
                                 (Put32Func)viPut32, (Put64Func)viPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)viGet8, (Get16Func)viGet16,
                                 (Get32Func)viGet32, (Get64Func)viGet64)) {
                return false;
            }
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

_XL_OBJECTTYPE gClassVI = {
    "VI",
    sizeof(VI),
    NULL,
    (EventFunc)viEvent,
};
