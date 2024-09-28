#include "emulator/system.h"
#include "emulator/ai.h"
#include "emulator/codeRVL.h"
#include "emulator/controller.h"
#include "emulator/cpu.h"
#include "emulator/disk.h"
#include "emulator/flash.h"
#include "emulator/frame.h"
#include "emulator/helpRVL.h"
#include "emulator/library.h"
#include "emulator/mi.h"
#include "emulator/pak.h"
#include "emulator/pi.h"
#include "emulator/pif.h"
#include "emulator/ram.h"
#include "emulator/rdb.h"
#include "emulator/rdp.h"
#include "emulator/rom.h"
#include "emulator/rsp.h"
#include "emulator/si.h"
#include "emulator/soundRVL.h"
#include "emulator/sram.h"
#include "emulator/vc64_RVL.h"
#include "emulator/vi.h"
#include "emulator/video.h"
#include "emulator/xlHeap.h"
#include "macros.h"
#include "revolution/os.h"
#include "revolution/vi.h"
#include "stdlib.h"
#include "string.h"

// clang-format off
static u32 contMap[][GCN_BTN_COUNT] = {
    // Controller Configuration No. 1
    {
        N64_BTN_A,      // GCN_BTN_A
        N64_BTN_B,      // GCN_BTN_B
        N64_BTN_UNSET,  // GCN_BTN_X
        N64_BTN_UNSET,  // GCN_BTN_Y
        N64_BTN_L,      // GCN_BTN_L
        N64_BTN_R,      // GCN_BTN_R
        N64_BTN_Z,      // GCN_BTN_Z
        N64_BTN_START,  // GCN_BTN_START
        0x08000000,     // GCN_BTN_UNK8
        0x04000000,     // GCN_BTN_UNK9
        0x02000000,     // GCN_BTN_UNK10
        0x01000000,     // GCN_BTN_UNK11
        N64_BTN_DUP,    // GCN_BTN_DPAD_UP
        N64_BTN_DDOWN,  // GCN_BTN_DPAD_DOWN
        N64_BTN_DLEFT,  // GCN_BTN_DPAD_LEFT
        N64_BTN_DRIGHT, // GCN_BTN_DPAD_RIGHT
        N64_BTN_CUP,    // GCN_BTN_CSTICK_UP
        N64_BTN_CDOWN,  // GCN_BTN_CSTICK_DOWN
        N64_BTN_CLEFT,  // GCN_BTN_CSTICK_LEFT
        N64_BTN_CRIGHT, // GCN_BTN_CSTICK_RIGHT
    },
    // Controller Configuration No. 2
    {
        N64_BTN_A,      // GCN_BTN_A
        N64_BTN_B,      // GCN_BTN_B
        N64_BTN_UNSET,  // GCN_BTN_X
        N64_BTN_UNSET,  // GCN_BTN_Y
        N64_BTN_Z,      // GCN_BTN_L
        N64_BTN_R,      // GCN_BTN_R
        N64_BTN_Z,      // GCN_BTN_Z
        N64_BTN_START,  // GCN_BTN_START
        0x08000000,     // GCN_BTN_UNK8
        0x04000000,     // GCN_BTN_UNK9
        0x02000000,     // GCN_BTN_UNK10
        0x01000000,     // GCN_BTN_UNK11
        N64_BTN_DUP,    // GCN_BTN_DPAD_UP
        N64_BTN_DDOWN,  // GCN_BTN_DPAD_DOWN
        N64_BTN_DLEFT,  // GCN_BTN_DPAD_LEFT
        N64_BTN_DRIGHT, // GCN_BTN_DPAD_RIGHT
        N64_BTN_CUP,    // GCN_BTN_CSTICK_UP
        N64_BTN_CDOWN,  // GCN_BTN_CSTICK_DOWN
        N64_BTN_CLEFT,  // GCN_BTN_CSTICK_LEFT
        N64_BTN_CRIGHT, // GCN_BTN_CSTICK_RIGHT
    },
    // Controller Configuration No. 3
    {
        N64_BTN_A,      // GCN_BTN_A
        N64_BTN_B,      // GCN_BTN_B
        N64_BTN_CRIGHT, // GCN_BTN_X
        N64_BTN_CLEFT,  // GCN_BTN_Y
        N64_BTN_Z,      // GCN_BTN_L
        N64_BTN_R,      // GCN_BTN_R
        N64_BTN_CDOWN,  // GCN_BTN_Z
        N64_BTN_START,  // GCN_BTN_START
        0x08000000,     // GCN_BTN_UNK8
        0x04000000,     // GCN_BTN_UNK9
        0x02000000,     // GCN_BTN_UNK10
        0x01000000,     // GCN_BTN_UNK11
        N64_BTN_L,      // GCN_BTN_DPAD_UP
        N64_BTN_L,      // GCN_BTN_DPAD_DOWN
        N64_BTN_L,      // GCN_BTN_DPAD_LEFT
        N64_BTN_L,      // GCN_BTN_DPAD_RIGHT
        N64_BTN_CUP,    // GCN_BTN_CSTICK_UP
        N64_BTN_CDOWN,  // GCN_BTN_CSTICK_DOWN
        N64_BTN_CLEFT,  // GCN_BTN_CSTICK_LEFT
        N64_BTN_CRIGHT, // GCN_BTN_CSTICK_RIGHT
    },
    // Controller Configuration No. 4
    {
        N64_BTN_A,      // GCN_BTN_A
        N64_BTN_B,      // GCN_BTN_B
        N64_BTN_Z,      // GCN_BTN_X
        N64_BTN_Z,      // GCN_BTN_Y
        N64_BTN_Z,      // GCN_BTN_L
        N64_BTN_R,      // GCN_BTN_R
        N64_BTN_L,      // GCN_BTN_Z
        N64_BTN_START,  // GCN_BTN_START
        0x08000000,     // GCN_BTN_UNK8
        0x04000000,     // GCN_BTN_UNK9
        0x02000000,     // GCN_BTN_UNK10
        0x01000000,     // GCN_BTN_UNK11
        N64_BTN_DUP,    // GCN_BTN_DPAD_UP
        N64_BTN_DDOWN,  // GCN_BTN_DPAD_DOWN
        N64_BTN_DLEFT,  // GCN_BTN_DPAD_LEFT
        N64_BTN_DRIGHT, // GCN_BTN_DPAD_RIGHT
        N64_BTN_CUP,    // GCN_BTN_CSTICK_UP
        N64_BTN_CDOWN,  // GCN_BTN_CSTICK_DOWN
        N64_BTN_CLEFT,  // GCN_BTN_CSTICK_LEFT
        N64_BTN_CRIGHT, // GCN_BTN_CSTICK_RIGHT
    }, 
    // Controller Configuration No. 5
    {
        N64_BTN_A,      // GCN_BTN_A
        N64_BTN_B,      // GCN_BTN_B
        N64_BTN_CDOWN,  // GCN_BTN_X
        N64_BTN_CLEFT,  // GCN_BTN_Y
        N64_BTN_Z,      // GCN_BTN_L
        N64_BTN_R,      // GCN_BTN_R
        N64_BTN_CRIGHT, // GCN_BTN_Z
        N64_BTN_START,  // GCN_BTN_START
        0x08000000,     // GCN_BTN_UNK8
        0x04000000,     // GCN_BTN_UNK9
        0x02000000,     // GCN_BTN_UNK10
        0x01000000,     // GCN_BTN_UNK11
        N64_BTN_CUP,    // GCN_BTN_DPAD_UP
        N64_BTN_CUP,    // GCN_BTN_DPAD_DOWN
        N64_BTN_CUP,    // GCN_BTN_DPAD_LEFT
        N64_BTN_CUP,    // GCN_BTN_DPAD_RIGHT
        N64_BTN_CUP,    // GCN_BTN_CSTICK_UP
        N64_BTN_CDOWN,  // GCN_BTN_CSTICK_DOWN
        N64_BTN_CLEFT,  // GCN_BTN_CSTICK_LEFT
        N64_BTN_CRIGHT, // GCN_BTN_CSTICK_RIGHT
    },
};
// clang-format on

static SystemDevice gaSystemDevice[] = {
    {
        SOT_HELP,
        &gClassHelpMenu,
        0,
        {0},
    },
    {
        SOT_FRAME,
        &gClassFrame,
        0,
        {0},
    },
    {
        SOT_LIBRARY,
        &gClassLibrary,
        0,
        {0},
    },
    {
        SOT_CODE,
        &gClassCode,
        0,
        {0},
    },
    {
        SOT_AUDIO,
        &gClassAudio,
        0,
        {0},
    },
    {
        SOT_VIDEO,
        &gClassVideo,
        0,
        {0},
    },
    {
        SOT_CONTROLLER,
        &gClassController,
        0,
        {0},
    },
    {
        SOT_CPU,
        &gClassCPU,
        1,
        {{0, 0x00000000, 0xFFFFFFFF}},
    },
    {
        SOT_RAM,
        &gClassRAM,
        3,
        {{256, 0x00000000, 0x03EFFFFF}, {2, 0x03F00000, 0x03FFFFFF}, {1, 0x04700000, 0x047FFFFF}},
    },
    {
        SOT_RDP,
        &gClassRDP,
        2,
        {{0, 0x04100000, 0x041FFFFF}, {1, 0x04200000, 0x042FFFFF}},
    },
    {
        SOT_RSP,
        &gClassRSP,
        1,
        {{0, 0x04000000, 0x040FFFFF}},
    },
    {
        SOT_MI,
        &gClassMI,
        1,
        {{0, 0x04300000, 0x043FFFFF}},
    },
    {
        SOT_VI,
        &gClassVI,
        1,
        {{0, 0x04400000, 0x044FFFFF}},
    },
    {
        SOT_AI,
        &gClassAI,
        1,
        {{0, 0x04500000, 0x045FFFFF}},
    },
    {
        SOT_PI,
        &gClassPI,
        1,
        {{0, 0x04600000, 0x046FFFFF}},
    },
    {
        SOT_SI,
        &gClassSI,
        1,
        {{0, 0x04800000, 0x048FFFFF}},
    },
    {
        SOT_RDB,
        &gClassRdb,
        1,
        {{0, 0x04900000, 0x0490FFFF}},
    },
    {
        SOT_DISK,
        &gClassDD,
        2,
        {{0, 0x05000000, 0x05FFFFFF}, {1, 0x06000000, 0x06FFFFFF}},
    },
    {
        SOT_ROM,
        &gClassROM,
        2,
        {{0, 0x10000000, 0x1FBFFFFF}, {1, 0x1FF00000, 0x1FF0FFFF}},
    },
    {
        SOT_PIF,
        &gClassPIF,
        1,
        {{0, 0x1FC00000, 0x1FC007FF}},
    },
    {
        SOT_NONE,
        NULL,
        0,
        {0},
    },
};

#if VERSION == MK64_U
u32 lbl_8016E268[] = {
    0x3C1A8007, 0x275ACEC0, 0x03400008, 0x00000000, 0x3C010010, 0x012A4824, 0x01214823, 0x3C01A460, 0xAC290000,
    0x3C08A460, 0x8D080010, 0x31080002, 0x5500FFFD, 0x3C08A460, 0x24081000, 0x010B4020, 0x010A4024, 0x3C01A460,
    0xAC280004, 0x3C0A0010, 0x254A0003, 0x3C01A460, 0xAC2A000C, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x3C1A8007, 0x275ACEC0, 0x03400008, 0x00000000,
    0x8D6B0010, 0x316B0001, 0x1560FFF0, 0x00000000, 0x3C0BB000, 0x8D640008, 0x3C010010, 0x02C02825, 0x00812023,
    0x3C016C07, 0x34218965, 0x00A10019, 0x27BDFFE0, 0xAFBF001C, 0xAFB00014, 0x3C1F0010, 0x00001825, 0x00004025,
    0x00804825, 0x240D0020, 0x00001012, 0x24420001, 0x00403825, 0x00405025, 0x00405825, 0x00408025, 0x00403025,
    0x00406025, 0x3C1A8007, 0x275ACEC0, 0x03400008, 0x00000000, 0x00602825, 0x254A0001, 0x3043001F, 0x01A37823,
    0x01E2C006, 0x00627004, 0x01D82025, 0x00C2082B, 0x00A03825, 0x01625826, 0x10200004, 0x02048021, 0x00E2C826,
    0x10000002, 0x03263026, 0x00C43026, 0x25080004, 0x00507826, 0x25290004, 0x151FFFE8, 0x01EC6021, 0x00EA7026,
    0x01CB3821, 0x0206C026, 0x030C8021, 0x3C0BB000, 0x8D680010, 0x14E80006, 0x08018B0A, 0x00000000, 0xAFBAFFF0,
    0x3C1A8006, 0x04110003, 0x00000000, 0x0411FFFF, 0x00000000, 0x3C09A408, 0x8D290000, 0x8FB00014, 0x8FBF001C,
    0x11200006, 0x27BD0020, 0x240A0041, 0x3C01A404, 0xAC2A0010, 0x3C01A408, 0xAC200000, 0x3C0B00AA, 0x356BAAAE,
    0x3C01A404, 0xAC2B0010, 0x3C01A430, 0x24080555, 0xAC28000C, 0x3C01A480, 0xAC200018, 0x3C01A450, 0xAC20000C,
    0x3C01A430, 0x24090800, 0xAC290000, 0x24090002, 0x3C01A460, 0xAC290010, 0x3C08A000, 0x35080300, 0x240917D7,
    0xAD090010, 0xAD140000, 0xAD130004, 0xAD15000C, 0x12600004, 0xAD170014, 0x3C09A600, 0x10000003, 0x25290000,
    0x3C09B000, 0x25290000, 0xAD090008, 0x3C08A400, 0x25080000, 0x21091000, 0x240AFFFF, 0x25080004, 0x1509FFFE,
    0xAD0AFFFC, 0x3C08A400, 0x25081000, 0x21091000, 0x25080004, 0x1509FFFE, 0xAD0AFFFC, 0x3C0AA400, 0x240B17D7,
    0xAD4B1000, 0x3C0BB000, 0x254A1000, 0x8D690008, 0x3C010010, 0x01214823, 0x01200008, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};
#else
// used by Diddy Kong Racing and Paper Mario in systemSetupGameALL
u32 lbl_8016FEA0[] = {
    0x3C1A8007, 0x275ACEC0, 0x03400008, 0x00000000, 0x3C010010, 0x012A4824, 0x01214823, 0x3C01A460, 0xAC290000,
    0x3C08A460, 0x8D080010, 0x31080002, 0x5500FFFD, 0x3C08A460, 0x24081000, 0x010B4020, 0x010A4024, 0x3C01A460,
    0xAC280004, 0x3C0A0010, 0x254A0003, 0x3C01A460, 0xAC2A000C, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x3C1A8007, 0x275ACEC0, 0x03400008, 0x00000000,
    0x8D6B0010, 0x316B0001, 0x1560FFF0, 0x00000000, 0x3C0BB000, 0x8D640008, 0x3C010010, 0x02C02825, 0x00812023,
    0x3C016C07, 0x34218965, 0x00A10019, 0x27BDFFE0, 0xAFBF001C, 0xAFB00014, 0x3C1F0010, 0x00001825, 0x00004025,
    0x00804825, 0x240D0020, 0x00001012, 0x24420001, 0x00403825, 0x00405025, 0x00405825, 0x00408025, 0x00403025,
    0x00406025, 0x3C1A8007, 0x275ACEC0, 0x03400008, 0x00000000, 0x00602825, 0x254A0001, 0x3043001F, 0x01A37823,
    0x01E2C006, 0x00627004, 0x01D82025, 0x00C2082B, 0x00A03825, 0x01625826, 0x10200004, 0x02048021, 0x00E2C826,
    0x10000002, 0x03263026, 0x00C43026, 0x25080004, 0x00507826, 0x25290004, 0x151FFFE8, 0x01EC6021, 0x00EA7026,
    0x01CB3821, 0x0206C026, 0x030C8021, 0x3C0BB000, 0x8D680010, 0x14E80006, 0x08018B0A, 0x00000000, 0xAFBAFFF0,
    0x3C1A8006, 0x04110003, 0x00000000, 0x0411FFFF, 0x00000000, 0x3C09A408, 0x8D290000, 0x8FB00014, 0x8FBF001C,
    0x11200006, 0x27BD0020, 0x240A0041, 0x3C01A404, 0xAC2A0010, 0x3C01A408, 0xAC200000, 0x3C0B00AA, 0x356BAAAE,
    0x3C01A404, 0xAC2B0010, 0x3C01A430, 0x24080555, 0xAC28000C, 0x3C01A480, 0xAC200018, 0x3C01A450, 0xAC20000C,
    0x3C01A430, 0x24090800, 0xAC290000, 0x24090002, 0x3C01A460, 0xAC290010, 0x3C08A000, 0x35080300, 0x240917D7,
    0xAD090010, 0xAD140000, 0xAD130004, 0xAD15000C, 0x12600004, 0xAD170014, 0x3C09A600, 0x10000003, 0x25290000,
    0x3C09B000, 0x25290000, 0xAD090008, 0x3C08A400, 0x25080000, 0x21091000, 0x240AFFFF, 0x25080004, 0x1509FFFE,
    0xAD0AFFFC, 0x3C08A400, 0x25081000, 0x21091000, 0x25080004, 0x1509FFFE, 0xAD0AFFFC, 0x3C0AA400, 0x240B17D7,
    0xAD4B1000, 0x3C0BB000, 0x254A1000, 0x8D690008, 0x3C010010, 0x01214823, 0x01200008, 0x00000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
};
#endif

static SystemRomConfig gSystemRomConfigurationList;

bool systemSetStorageDevice(System* pSystem, SystemObjectType eStorageDevice, void* pArgument) {
    switch (eStorageDevice) {
        case SOT_PAK:
            if (!xlObjectMake(&pSystem->apObject[SOT_PAK], pArgument, &gClassPak)) {
                return false;
            }

            if (!cpuMapObject(SYSTEM_CPU(gpSystem), pSystem->apObject[SOT_PAK], 0x08000000, 0x0801FFFF, 0)) {
                return false;
            }
            break;
        case SOT_SRAM:
            if (!xlObjectMake(&pSystem->apObject[SOT_SRAM], pArgument, &gClassSram)) {
                return false;
            }

            if (!cpuMapObject(SYSTEM_CPU(gpSystem), pSystem->apObject[SOT_SRAM], 0x08000000, 0x08007FFF, 0)) {
                return false;
            }
            break;
        case SOT_FLASH:
            if (!xlObjectMake(&pSystem->apObject[SOT_FLASH], pArgument, &gClassFlash)) {
                return false;
            }

            if (!cpuMapObject(SYSTEM_CPU(gpSystem), pSystem->apObject[SOT_FLASH], 0x08000000, 0x0801FFFF, 0)) {
                return false;
            }
            break;
        default:
            return false;
    }

    return true;
}

bool systemCreateStorageDevice(System* pSystem, void* pArgument) {
    SystemDevice* pDevice;
    SystemDeviceInfo* pInfo;
    s32 i;
    s32 nSlotUsed;
    SystemObjectType storageDevice;
    void** ppObject;
    s32 iDevice;

    pSystem->apObject[SOT_CPU] = NULL;
    pSystem->apObject[SOT_PIF] = NULL;
    pSystem->apObject[SOT_RAM] = NULL;
    pSystem->apObject[SOT_ROM] = NULL;
    pSystem->apObject[SOT_RSP] = NULL;
    pSystem->apObject[SOT_RDP] = NULL;
    pSystem->apObject[SOT_MI] = NULL;
    pSystem->apObject[SOT_DISK] = NULL;
    pSystem->apObject[SOT_AI] = NULL;
    pSystem->apObject[SOT_VI] = NULL;
    pSystem->apObject[SOT_SI] = NULL;
    pSystem->apObject[SOT_PI] = NULL;
    pSystem->apObject[SOT_RDB] = NULL;
    pSystem->apObject[SOT_PAK] = NULL;
    pSystem->apObject[SOT_SRAM] = NULL;
    pSystem->apObject[SOT_FLASH] = NULL;
    pSystem->apObject[SOT_CODE] = NULL;
    pSystem->apObject[SOT_HELP] = NULL;
    pSystem->apObject[SOT_LIBRARY] = NULL;
    pSystem->apObject[SOT_FRAME] = NULL;
    pSystem->apObject[SOT_AUDIO] = NULL;
    pSystem->apObject[SOT_VIDEO] = NULL;
    pSystem->apObject[SOT_CONTROLLER] = NULL;

    iDevice = 0;

    while ((pDevice = &gaSystemDevice[iDevice], storageDevice = pDevice->storageDevice, storageDevice) != SOT_NONE) {
        ppObject = &pSystem->apObject[storageDevice];

        if (pSystem->apObject[storageDevice] == NULL) {
            if (!xlObjectMake(ppObject, pArgument, pDevice->pClass)) {
                return false;
            }
        } else {
            return false;
        }

        nSlotUsed = pDevice->nSlotUsed;

        if (nSlotUsed > 0) {
            for (i = 0; i < nSlotUsed; i++) {
                pInfo = &pDevice->aDeviceSlot[i];

                if (storageDevice == SOT_CPU) {
                    if (!cpuMapObject(SYSTEM_CPU(pSystem), pSystem, pInfo->nAddress0, pInfo->nAddress1, pInfo->nType)) {
                        return false;
                    }
                } else {
                    if (!cpuMapObject(SYSTEM_CPU(pSystem), *ppObject, pInfo->nAddress0, pInfo->nAddress1,
                                      pInfo->nType)) {
                        return false;
                    }
                }
            }
        }

        iDevice++;
    }

    return true;
}

static bool systemSetRamMode(System* pSystem) {
    s32 nSize;
    u32* anMode;
#if VERSION == MK64_U
    s32* anUnknown;
#endif

    if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&anMode, 0x300, NULL)) {
        return false;
    }

    anMode[0] = 1;
    anMode[1] = 0;
    anMode[2] = 0xB0000000;
    anMode[3] = 0;

#if VERSION == MK64_U
    anMode[4] = 0x17D7;
#else
    anMode[4] = 0x17D5;
#endif

    anMode[5] = 1;

    if (!ramGetSize(SYSTEM_RAM(gpSystem), &nSize)) {
        return false;
    }

    anMode[6] = nSize;

#if VERSION == MK64_U
    if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&anUnknown, 0, NULL)) {
        return false;
    }

    if (!xlHeapCopy(anUnknown, &lbl_8016E268, sizeof(lbl_8016E268))) {
        return false;
    }
#endif

    SYSTEM_CPU(gpSystem)->nTimeRetrace = OSSecondsToTicks(1.0f / 60.0f);

    return true;
}

static bool systemSetupGameRAM(System* pSystem) {
    char* szExtra;
    bool bExpansion;
    s32 nSizeRAM;
    s32 nSizeCacheROM;
    s32 nSizeExtra;
    Rom* pROM;
    u32 nCode;
    u32 iCode;
    u32 anCode[0x100]; // size = 0x400

    bExpansion = false;
    pROM = SYSTEM_ROM(pSystem);

    if (!romCopy(SYSTEM_ROM(gpSystem), anCode, 0x1000, sizeof(anCode), NULL)) {
        return false;
    }

    nCode = 0;
    for (iCode = 0; iCode < ARRAY_COUNT(anCode); iCode++) {
        nCode += anCode[iCode];
    }

    // Ocarina of Time or Majora's Mask
    if (pSystem->eTypeROM == NZSJ || pSystem->eTypeROM == NZSE || pSystem->eTypeROM == NZSP) {
        bExpansion = true;
    } else if (nCode == 0x184CED80 || nCode == 0x184CED18 || nCode == 0x7E8BEE60) {
        bExpansion = true;
    }

    // Conker's Bad Fur Day
    if (pSystem->eTypeROM == NFUJ || pSystem->eTypeROM == NFUE || pSystem->eTypeROM == NFUP) {
        bExpansion = true;
    }

    if (bExpansion) {
        nSizeRAM = 0x800000;
        nSizeCacheROM = 0x400000;
    } else {
        nSizeRAM = 0x400000;
        nSizeCacheROM = 0x800000;
    }

    if (!ramSetSize(SYSTEM_RAM(gpSystem), nSizeRAM)) {
        return false;
    }

    if (!ramWipe(SYSTEM_RAM(gpSystem))) {
        return false;
    }

    return true;
}

static inline void systemSetControllerConfiguration(SystemRomConfig* pRomConfig, s32 controllerConfig1,
                                                    s32 controllerConfig2, bool bSetControllerConfig,
                                                    bool bSetRumbleConfig) {
    s32 iConfigList;

    if (bSetRumbleConfig) {
        pRomConfig->rumbleConfiguration = 0;
    }

    for (iConfigList = 0; iConfigList < 4; iConfigList++) {
        simulatorCopyControllerMap(SYSTEM_CONTROLLER(gpSystem), (u32*)pRomConfig->controllerConfiguration[iConfigList],
                                   contMap[((controllerConfig1 >> (iConfigList * 8)) & 0x7F)]);
        pRomConfig->rumbleConfiguration |= (1 << (iConfigList * 8)) & (controllerConfig1 >> 7);
    }

    if (bSetControllerConfig) {
        pRomConfig->normalControllerConfig = controllerConfig2;
        pRomConfig->currentControllerConfig = controllerConfig2;
    }
}

static inline void systemSetupGameALL_Inline(void) {
    s32 iController;

    for (iController = 0; iController < 4; iController++) {
        simulatorCopyControllerMap(SYSTEM_CONTROLLER(gpSystem),
                                   (u32*)&gSystemRomConfigurationList.controllerConfiguration[iController],
                                   (u32*)&contMap[0]);
    }
}

static bool systemSetupGameALL(System* pSystem) {
    char* szArgument;
    s32* pBuffer2;
    s32* pBuffer;
    int i;
    s32 iController;
    s32 nSizeSound;
    u32 pArgument;
    s32 var_r28;
    s64 var_r27;
    s32 var_r26;
    SystemObjectType storageDevice;
    Rom* pROM;
    Cpu* pCPU;
    Pif* pPIF;

    pCPU = SYSTEM_CPU(gpSystem);
    pROM = SYSTEM_ROM(gpSystem);
    pPIF = SYSTEM_PIF(gpSystem);

    pArgument = 0;
    storageDevice = SOT_NONE;
    var_r26 = 0xFF;
    var_r27 = 0xFF;
    var_r28 = 0xFF;
    nSizeSound = 0x2000;

    gSystemRomConfigurationList.rumbleConfiguration = pArgument;

    systemSetupGameALL_Inline();

    if (!romGetCode(pROM, (s32*)&pSystem->eTypeROM)) {
        return false;
    }

    switch (pSystem->eTypeROM) {
        case NSMJ:
        case NSME:
        case NSMP:
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            nSizeSound = 0x2000;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            var_r28 = 0xBE;
            var_r27 = 0xBE;
            var_r26 = 0xBE;
            if (!cpuSetCodeHack(pCPU, 0x80317938, 0x5420FFFE, 0)) {
                return false;
            }
            if (pSystem->eTypeROM == NSMJ) {
                systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x81818181, 0x81818181, true, true);
                if (!cpuSetCodeHack(pCPU, 0x802F2458, 0x83250002, -1)) {
                    return false;
                }
            } else {
                systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x01010101, 0x01010101, true, true);
            }
            break;
        case NKTJ:
        case NKTE:
        case NKTP:
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            gSystemRomConfigurationList.rumbleConfiguration = 0;
            var_r28 = 0xBE;
            var_r27 = 0xBE;
            var_r26 = 0xBE;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x03030303, 0x83838383, true, false);
            if (pSystem->eTypeROM == NKTJ) {
                if (!cpuSetCodeHack(pCPU, 0x802A4118, 0x3C068015, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x800729D4, 0x27BDFFD8, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8003FBC4, 0x20A50001, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8003FBD4, 0x00084040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8003FC68, 0x20A50001, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8003FC74, 0x00084040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x800987E8, 0x25AD8008, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80098888, 0x3C00E700, -1)) {
                    return false;
                }
            } else if (pSystem->eTypeROM == NKTP) {
                if (!cpuSetCodeHack(pCPU, 0x802A4160, 0x3C068015, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80072E34, 0x27BDFFD8, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80040054, 0x20A50001, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80040094, 0x00084040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x800400F8, 0x20A50001, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80040134, 0x00084040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80098F04, 0x25AD8008, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80098FA4, 0x3C0DE700, -1)) {
                    return false;
                }
            } else if (pSystem->eTypeROM == NKTE) {
                if (!cpuSetCodeHack(pCPU, 0x802A4160, 0x3C068015, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80072E54, 0x27BDFFD8, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80040074, 0x20A50001, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x800400B4, 0x00084040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80040118, 0x20A50001, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80040154, 0x00084040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80098F04, 0x25AD8008, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80098FA4, 0x3C0DE700, -1)) {
                    return false;
                }
            }
            pCPU->nCompileFlag |= 0x110;
            break;
        case NZLP:
        case CZLJ:
        case CZLE:
            pArgument = 0x8000;
            nSizeSound = 0x1000;
#if VERSION == MK64_U
            storageDevice = SOT_SRAM;
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
            if (!simulatorGetArgument(SAT_RESET, &szArgument) || *szArgument == '1') {
                if (!simulatorGetArgument(SAT_CONTROLLER, &szArgument) || *szArgument == '0') {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x82828282, 0x82828282, true, true);
                } else {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x80808080, 0x80808080, true, true);
                }
            } else {
                if (!simulatorGetArgument(SAT_CONTROLLER, &szArgument) || *szArgument == '0') {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x02020202, 0x02020202, true, true);
                } else {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0, 0, true, true);
                }
            }
#else
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D9;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer, 0, NULL)) {
                return false;
            }
            pBuffer[0xBA] = 0xC86E2000;
            pBuffer[0xBEC7D] = 0xAD090010;
            pBuffer[0xBF870] = 0xAD170014;
            storageDevice = SOT_SRAM;
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
            systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x02020202, 0x02020202, true, true);
#endif
            if (pSystem->eTypeROM == CZLE) {
                if (!cpuSetCodeHack(pCPU, 0x80062D64, 0x94639680, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8006E468, 0x97040000, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8005BB14, 0x9463D040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80066638, 0x97040000, -1)) {
                    return false;
                }
            } else if (pSystem->eTypeROM == CZLJ) {
                if (!cpuSetCodeHack(pCPU, 0x80062D64, 0x94639680, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8006E468, 0x97040000, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8005BB34, 0x9463D040, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x80066658, 0x97040000, -1)) {
                    return false;
                }
            } else if (pSystem->eTypeROM == NZLP) {
                if (!cpuSetCodeHack(pCPU, 0x80062D64, 0x94639680, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8006E468, 0x97040000, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8005BB3C, 0x9502000C, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x800665E8, 0x97040000, -1)) {
                    return false;
                }
            }
            pCPU->nCompileFlag |= 0x110;
            break;
        case NZSE:
        case NZSP:
        case NZSJ:
            nSizeSound = 0x1000;
            storageDevice = SOT_FLASH;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D9;
            gSystemRomConfigurationList.storageDevice = SOT_RAM;
            if (!simulatorGetArgument(SAT_RESET, &szArgument) || *szArgument == '1') {
                if (!simulatorGetArgument(SAT_CONTROLLER, &szArgument) || *szArgument == '0') {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x82828282, 0x82828282, true, true);
                } else {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x80808080, 0x80808080, true, true);
                }
            } else {
                if (!simulatorGetArgument(SAT_CONTROLLER, &szArgument) || *szArgument == '0') {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x02020202, 0x02020202, true, true);
                } else {
                    systemSetControllerConfiguration(&gSystemRomConfigurationList, 0, 0, true, true);
                }
            }
            if (!cpuSetCodeHack(pCPU, 0x801C6FC0, 0x95630000, -1)) {
                return false;
            }
            if (pSystem->eTypeROM == NZSJ) {
                if (!cpuSetCodeHack(pCPU, 0x80177CF4, 0x95630000, -1)) {
                    return false;
                }
            } else if (pSystem->eTypeROM == NZSE) {
                if (!cpuSetCodeHack(pCPU, 0x80177D34, 0x95630000, -1)) {
                    return false;
                }
            } else {
                if (!cpuSetCodeHack(pCPU, 0x801786B4, 0x95630000, -1)) {
                    return false;
                }
            }
            pCPU->nCompileFlag |= 0x1010;
            break;
        case NFXJ:
        case NFXP:
        case NFXE:
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            systemSetControllerConfiguration(&gSystemRomConfigurationList, 0x84848484, 0x84848484, true, true);
            if (pSystem->eTypeROM == NFXJ) {
                if (!cpuSetCodeHack(pCPU, 0x8019F548, 0xA2000000, 0)) {
                    return false;
                }
            } else if (pSystem->eTypeROM == NFXE) {
                if (!cpuSetCodeHack(pCPU, 0x801989D0, 0xA2000000, 0)) {
                    return false;
                }
            }
            GXSetDispCopyGamma(GX_GM_1_7);
            pCPU->nCompileFlag |= 0x110;
            break;
        case NPWE:
        case NPWP:
        case NPWJ:
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            break;
        case NAFE:
        case NAFP:
        case NAFJ:
            storageDevice = SOT_FLASH;
            gSystemRomConfigurationList.storageDevice = SOT_RAM;
            break;
        case NBCJ:
        case NBCP:
        case NBCE:
#if VERSION != MK64_U
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D6;
#endif
            if (!cpuSetCodeHack(pCPU, 0x80244CFC, 0x1420FFFA, 0)) {
                return false;
            }
            break;
        case NBYP:
        case NBYJ:
        case NBYE:
            if (!cpuSetCodeHack(pCPU, 0x8007ADD0, 0x1440FFF9, 0)) {
                return false;
            }
            break;
        case NCUJ:
        case NCUP:
        case NCUE:
#if VERSION == MK64_U
            gSystemRomConfigurationList.storageDevice = SOT_AUDIO;
#else
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
#endif
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
#if VERSION != MK64_U
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D6;
#endif
            if (!cpuSetCodeHack(pCPU, 0x80103E0C, 0x1616FFF2, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80111B00, 0x51E0FFFF, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80111B04, 0x8C4F0000, 0)) {
                return false;
            }
            break;
#if VERSION != MK64_U
        case NDYE:
        case NDYP:
        case NDYJ:
            gSystemRomConfigurationList.storageDevice = SOT_RAM;
            storageDevice = SOT_FLASH;
            pArgument = 0x4000;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D7;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer, 0, NULL)) {
                return false;
            }
            if (!xlHeapCopy(pBuffer, lbl_8016FEA0, 0x300)) {
                return false;
            }
            if (!fn_8007D6A0(SYSTEM_RSP(gpSystem), (void**)&pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = 0x17D7;
            if (!fn_8007D688(SYSTEM_RSP(gpSystem), (void**)&pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = -1;
            break;
#endif
        case NDOE:
        case NDOP:
        case NDOJ:
            if (!cpuSetCodeHack(pCPU, 0x80000A04, 0x1462FFFF, 0)) {
                return false;
            }
            break;
        case NN6P:
        case NN6J:
        case NN6E:
            if (!cpuSetCodeHack(pCPU, 0x800005EC, 0x3C028001, -1)) {
                return false;
            }
            if (pSystem->eTypeROM == NN6J) {
                if (!cpuSetCodeHack(pCPU, 0x8006D458, 0x0C0189E9, 0x0C0189A3)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8006D664, 0x0C0189E9, 0x0C0189A3)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8006D6D0, 0x0C0189E9, 0x0C0189A3)) {
                    return false;
                }
            } else {
                if (!cpuSetCodeHack(pCPU, 0x8006D338, 0x0C0189A9, 0x0C018963)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8006D544, 0x0C0189A9, 0x0C018963)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8006D5B0, 0x0C0189A9, 0x0C018963)) {
                    return false;
                }
            }
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            pCPU->nCompileFlag |= 0x10;
            break;
#if VERSION != MK64_U
        case NSIJ:
            pArgument = 0x8000;
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
            storageDevice = SOT_SRAM;
            break;
#endif
        case NFZP:
        case NFZJ:
        case CFZE:
#if VERSION == MK64_U
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            if (!cpuSetCodeHack(pCPU, 0x800673A8, 0x144DFFFC, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80067564, 0x144FFFFC, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x8006759C, 0x1459FFFC, 0)) {
                return false;
            }
#else
            nSizeSound = 0x8000;
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
            pArgument = 0x8000;
            storageDevice = SOT_SRAM;
#endif
            break;
        case NLRJ:
        case NLRP:
        case NLRE:
            if (!cpuSetCodeHack(pCPU, 0x80097B6C, 0x1443FFF9, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80097BF4, 0x1443FFF9, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80096D08, 0x08025B40, 0x1000FFFF)) {
                return false;
            }
            break;
        case NMFJ:
        case NMFP:
        case NMFE:
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            if (!cpuSetCodeHack(pCPU, 0x800B2DCC, 0x8C430004, -1)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x800B2E70, 0x8C430004, -1)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80029EB8, 0x8C4252CC, -1)) {
                return false;
            }
            break;
        case NK4E:
        case NK4P:
        case NK4J:
            if (!aiEnable(SYSTEM_AI(gpSystem), false)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80020BCC, 0x8DF80034, -1)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80020EBC, 0x8DEFF330, -1)) {
                return false;
            }
            gSystemRomConfigurationList.storageDevice = SOT_AI;
#if VERSION != MK64_U
            pArgument = 0x4000;
#endif
            storageDevice = SOT_FLASH;
            pCPU->nCompileFlag |= 0x110;
            break;
        case CLBP:
        case CLBE:
        case CLBJ:
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            break;
        case NMWP:
        case NMWE:
        case NMWJ:
#if VERSION != MK64_U
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
#endif
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
#if VERSION != MK64_U
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D6;
#endif
            break;
        case NMVJ:
        case NMVP:
        case NMVE:
            gSystemRomConfigurationList.storageDevice = SOT_AI;
#if VERSION != MK64_U
            pArgument = 0x4000;
#endif
            storageDevice = SOT_FLASH;
            break;
        case NRIP:
        case NRIE:
        case NRIJ:
            gSystemRomConfigurationList.storageDevice = SOT_AI;
#if VERSION != MK64_U
            pArgument = 0x4000;
#endif
            storageDevice = SOT_FLASH;
            break;
        case NMQJ:
        case NMQP:
        case NMQE:
            gSystemRomConfigurationList.storageDevice = SOT_RAM;
            storageDevice = SOT_FLASH;
#if VERSION != MK64_U
            pArgument = 0x20000;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer, 0, NULL)) {
                return false;
            }
            if (!xlHeapCopy(pBuffer, lbl_8016FEA0, 0x300)) {
                return false;
            }
            if (!fn_8007D6A0(SYSTEM_RSP(gpSystem), &pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = 0x17D7;
            if (!fn_8007D688(SYSTEM_RSP(gpSystem), (void**)&pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = -1;
            if (pSystem->eTypeROM == NMQE) {
                if (!cpuSetCodeHack(pCPU, 0x8005E98C, 0x1040FFFF, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8005F2D8, 0x1440FFFD, -1)) {
                    return false;
                }
            } else if (pSystem->eTypeROM == NMQJ) {
                if (!cpuSetCodeHack(pCPU, 0x8005E63C, 0x1040FFFF, -1)) {
                    return false;
                }
                if (!cpuSetCodeHack(pCPU, 0x8005EF88, 0x1440FFFD, -1)) {
                    return false;
                }
            }
#endif
            break;
        case NPOE:
        case NPOP:
        case NPOJ:
            storageDevice = SOT_FLASH;
            gSystemRomConfigurationList.storageDevice = SOT_RAM;
            break;
        case NQKJ:
        case NQKP:
        case NQKE:
            if (!cpuSetCodeHack(pCPU, 0x8004989C, 0x1459FFFB, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80049FF0, 0x1608FFFB, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x8004A384, 0x15E0FFFB, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x8004A97C, 0x15E0FFFB, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80048FF8, 0x1000FFFD, 0x1000FFFF)) {
                return false;
            }
            break;
        case NGUJ:
        case NGUP:
        case NGUE:
#if VERSION != MK64_U
            gSystemRomConfigurationList.storageDevice = SOT_RSP;
            pArgument = 0x1000;
            storageDevice = SOT_FLASH;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D6;
#endif
            if (!cpuSetCodeHack(pCPU, 0x80025D30, 0x3C018006, -1)) {
                return false;
            }
            break;
#if VERSION != MK64_U
        case NSQP:
        case NSQJ:
        case NSQE:
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D6;
            break;
        case NOBJ:
        case NOBP:
        case NOBE:
            pArgument = 0x8000;
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
            storageDevice = SOT_SRAM;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D6;
            break;
        case NRXP:
        case NRXJ:
        case NRXE:
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D6;
            break;
#endif
        case NALJ:
        case NALP:
        case NALE:
            gSystemRomConfigurationList.storageDevice = SOT_AI;
#if VERSION != MK64_U
            pArgument = 0x4000;
#endif
            storageDevice = SOT_FLASH;
            if (!cpuSetCodeHack(pCPU, 0x8000092C, 0x3C028004, -1)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x8002103C, 0x3C028004, -1)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80021048, 0x3C028004, -1)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x800A1BB8, 0x1440FFFD, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x800A1BE0, 0x1440FFFD, 0)) {
                return false;
            }
            pCPU->nCompileFlag |= 0x110;
            break;
        case NTEJ:
        case NTEP:
        case NTEA:
#if VERSION == MK64_U
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D7;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x200, NULL)) {
                return false;
            }
            pBuffer2[0] = 0xAC290000;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x284, NULL)) {
                return false;
            }
            pBuffer2[0] = 0x240B17D7;
#else
            pArgument = 0x8000;
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
            storageDevice = SOT_SRAM;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D7;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer, 0, NULL)) {
                return false;
            }
            pBuffer[0x80] = 0xAC290000;
            pBuffer[0xA1] = 0x240B17D7;
            if (!fn_8007D6A0(SYSTEM_RSP(gpSystem), &pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = 0x17D7;

            if (!fn_8007D688(SYSTEM_RSP(gpSystem), (void**)&pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = -1;
#endif
            if (!cpuSetCodeHack(pCPU, 0x8000017C, 0x14E80006, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80000188, 0x16080003, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x800F04E8, 0x1218FFFB, 0)) {
                return false;
            }
#if VERSION == MK64_U
            storageDevice = SOT_SRAM;
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
#endif
            break;
        case NYLJ:
        case NYLP:
        case NYLE:
            gSystemRomConfigurationList.storageDevice = SOT_PIF;
            storageDevice = SOT_SRAM;
            if (!cpuSetCodeHack(pCPU, 0x800A58F8, 0x8C62FF8C, -1)) {
                return false;
            }
            pCPU->nCompileFlag |= 0x10;
            break;
        case NTUE:
        case NTUP:
        case NTUJ:
            if (!cpuSetCodeHack(pCPU, 0x8002BDD0, 0xA0000000, 0)) {
                return false;
            }
            break;
        case NWRE:
        case NWRP:
        case NWRJ:
            if (!cpuSetCodeHack(pCPU, 0x8004795C, 0x1448FFFC, 0)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80047994, 0x144AFFFC, 0)) {
                return false;
            }
            pCPU->nCompileFlag |= 0x10;
            break;
        case NYSJ:
        case NYSP:
        case NYSE:
            gSystemRomConfigurationList.storageDevice = SOT_AI;
#if VERSION != MK64_U
            pArgument = 0x4000;
#endif
            storageDevice = SOT_FLASH;
#if VERSION != MK64_U
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer2, 0x300, NULL)) {
                return false;
            }
            pBuffer2[4] = 0x17D8;
            if (!ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pBuffer, 0, NULL)) {
                return false;
            }
            pBuffer[0x59] = 0x01EC6021;
            pBuffer[0xAE] = 0x8941680C;
            if (!fn_8007D6A0(SYSTEM_RSP(gpSystem), &pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = 0x17D8;
            if (!fn_8007D688(SYSTEM_RSP(gpSystem), (void**)&pBuffer, 0, 4)) {
                return false;
            }
            pBuffer[0] = -1;
#endif
            break;
        case NBNP:
        case NBNE:
        case NBNJ:
            gSystemRomConfigurationList.storageDevice = SOT_AI;
#if VERSION != MK64_U
            pArgument = 0x4000;
#endif
            storageDevice = SOT_FLASH;
            if (!cpuSetCodeHack(pCPU, 0x80000548, 0x08000156, 0x1000FFFF)) {
                return false;
            }
            if (!cpuSetCodeHack(pCPU, 0x80000730, 0x3C02800C, -1)) {
                return false;
            }
            break;
        case NRBJ:
        case NRBP:
        case NRBE:
#if VERSION != MK64_U
            gSystemRomConfigurationList.storageDevice = SOT_AI;
            pArgument = 0x4000;
#endif
            storageDevice = SOT_FLASH;
            if (!cpuSetCodeHack(pCPU, 0x80066884, 0x8C62FF8C, 0xFFFFFFFF)) {
                return false;
            }
            pCPU->nCompileFlag |= 0x110;
            break;
    }

    if (storageDevice != SOT_NONE && !systemSetStorageDevice(pSystem, storageDevice, (void*)pArgument)) {
        return false;
    }

    if (!fn_8005329C(SYSTEM_FRAME(gpSystem), var_r28, var_r27, var_r26)) {
        return false;
    }

    if (!soundSetBufferSize(SYSTEM_SOUND(gpSystem), nSizeSound)) {
        return false;
    }

    systemSetControllerConfiguration(&gSystemRomConfigurationList, gSystemRomConfigurationList.currentControllerConfig,
                                     gSystemRomConfigurationList.currentControllerConfig, false, true);

    for (iController = 0; iController < 4; iController++) {
        simulatorSetControllerMap(SYSTEM_CONTROLLER(gpSystem), iController,
                                  (u32*)&gSystemRomConfigurationList.controllerConfiguration[iController]);

        if (gSystemRomConfigurationList.storageDevice & 0x10) {
            if (!pifSetControllerType(pPIF, iController, CT_CONTROLLER_W_PAK)) {
                return false;
            }
        } else if (gSystemRomConfigurationList.rumbleConfiguration & (1 << (iController << 3))) {
            if (!pifSetControllerType(pPIF, iController, CT_CONTROLLER_W_RPAK)) {
                return false;
            }
        } else {
            if (!pifSetControllerType(pPIF, iController, CT_CONTROLLER)) {
                return false;
            }
        }
    }

    return true;
}

static bool systemGetException(System* pSystem, SystemInterruptType eType, SystemException* pException) {
    pException->nMask = 0;
    pException->szType = "";
    pException->eType = eType;
    pException->eCode = CEC_NONE;
    pException->eTypeMips = MIT_NONE;

    switch (eType) {
        case SIT_SW0:
            pException->nMask = 5;
            pException->szType = "SW0";
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_SW1:
            pException->nMask = 6;
            pException->szType = "SW1";
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_CART:
            pException->nMask = 0xC;
            pException->szType = "CART";
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_COUNTER:
            pException->nMask = 0x84;
            pException->szType = "COUNTER";
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_RDB:
            pException->nMask = 0x24;
            pException->szType = "RDB";
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_SP:
            pException->nMask = 4;
            pException->szType = "SP";
            pException->eTypeMips = MIT_SP;
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_SI:
            pException->nMask = 4;
            pException->szType = "SI";
            pException->eTypeMips = MIT_SI;
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_AI:
            pException->nMask = 4;
            pException->szType = "AI";
            pException->eTypeMips = MIT_AI;
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_VI:
            pException->nMask = 4;
            pException->szType = "VI";
            pException->eTypeMips = MIT_VI;
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_PI:
            pException->nMask = 4;
            pException->szType = "PI";
            pException->eTypeMips = MIT_PI;
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_DP:
            pException->nMask = 4;
            pException->szType = "DP";
            pException->eTypeMips = MIT_DP;
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_CPU_BREAK:
            pException->szType = "BREAK (CPU)";
            pException->eCode = CEC_BREAK;
            break;
        case SIT_SP_BREAK:
            pException->nMask = 4;
            pException->szType = "BREAK (SP)";
            pException->eCode = CEC_INTERRUPT;
            break;
        case SIT_FAULT:
            pException->szType = "FAULT";
            break;
        case SIT_THREADSTATUS:
            pException->szType = "THREADSTATUS";
            break;
        case SIT_PRENMI:
            pException->szType = "PRENMI";
            pException->eCode = CEC_INTERRUPT;
            break;
        default:
            return false;
    }

    return true;
}

static bool systemGet8(System* pSystem, u32 nAddress, s8* pData) {
    s64 pnPC;
    *pData = 0;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemGet16(System* pSystem, u32 nAddress, s16* pData) {
    s64 pnPC;
    *pData = 0;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemGet32(System* pSystem, u32 nAddress, s32* pData) {
    s64 pnPC;
    *pData = 0;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemGet64(System* pSystem, u32 nAddress, s64* pData) {
    s64 pnPC;
    *pData = 0;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemPut8(System* pSystem, u32 nAddress, s8* pData) {
    s64 pnPC;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemPut16(System* pSystem, u32 nAddress, s16* pData) {
    s64 pnPC;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemPut32(System* pSystem, u32 nAddress, s32* pData) {
    s64 pnPC;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemPut64(System* pSystem, u32 nAddress, s64* pData) {
    s64 pnPC;
    cpuGetXPC(SYSTEM_CPU(gpSystem), &pnPC, NULL, NULL);
    return false;
}

static bool systemGetBlock(System* pSystem, CpuBlock* pBlock) {
    void* pBuffer;

#if VERSION != MK64_U
    if (pBlock->nAddress1 < 0x04000000) {
        if (!ramGetBuffer(SYSTEM_RAM(gpSystem), &pBuffer, pBlock->nAddress1, &pBlock->nSize)) {
            return false;
        }

        xlHeapFill8(pBuffer, pBlock->nSize, 0xFF);
    }
#endif

    if (pBlock->pfUnknown != NULL && !pBlock->pfUnknown(pBlock, 1)) {
        return false;
    }

    return true;
}

static inline bool fn_8000A504_UnknownInline(System* pSystem, CpuBlock** pBlock) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (*pBlock == (CpuBlock*)(pSystem->unk_78 + (i * 5))) {
            pSystem->storageDevice &= ~(1 << i);
            return true;
        }
    }

    return false;
}

static bool fn_8000A504(CpuBlock* pBlock, bool bUnknown) {
    u32 nAddressOffset[32];
    u32 nAddress;
    u32* pnAddress;
    u32 nAddressEnd;
    s32 nCount;
    s32 i;

    if (bUnknown == true) {
        nAddress = pBlock->nAddress1;

        if (nAddress < 0x04000000) {
            nAddressEnd = (nAddress + pBlock->nSize) - 1;

            if (!rspInvalidateCache(SYSTEM_RSP(gpSystem), nAddress, nAddressEnd)) {
                return false;
            }

            if (!frameInvalidateCache(SYSTEM_FRAME(gpSystem), nAddress, nAddressEnd)) {
                return false;
            }

            if (!cpuGetOffsetAddress(SYSTEM_CPU(gpSystem), nAddressOffset, &nCount, pBlock->nAddress1, pBlock->nSize)) {
                return false;
            }

            for (i = 0, pnAddress = nAddressOffset; i < nCount; pnAddress++, i++) {
                if (!cpuInvalidateCache(SYSTEM_CPU(gpSystem), *pnAddress, (*pnAddress + pBlock->nSize) - 1)) {
                    return false;
                }
            }
        }

        if (pBlock->pNext->pfUnknown != NULL) {
            pBlock->pNext->pfUnknown(pBlock->pNext, bUnknown);
        }

        if (!fn_8000A504_UnknownInline(gpSystem, &pBlock)) {
            return false;
        }
    }

    return true;
}

static inline bool systemGetNewBlock(System* pSystem, CpuBlock** ppBlock) {
    s32 i;

    for (i = 0; i < 4; i++) {
        if (!(pSystem->storageDevice & (1 << i))) {
            pSystem->storageDevice |= (1 << i);
            *ppBlock = (CpuBlock*)(pSystem->unk_78 + (i * 5));
            return true;
        }
    }

    *ppBlock = NULL;
    return false;
}

bool fn_8000A6A4(System* pSystem, CpuBlock* pBlock) {
    CpuBlock* pNewBlock;

    if (!systemGetNewBlock(pSystem, &pNewBlock)) {
        return false;
    }

    pNewBlock->pNext = pBlock;
    pNewBlock->nSize = pBlock->nSize;
    pNewBlock->pfUnknown = fn_8000A504;
    pNewBlock->nAddress0 = pBlock->nAddress0;
    pNewBlock->nAddress1 = pBlock->nAddress1;

    if (!cpuGetBlock(SYSTEM_CPU(gpSystem), pNewBlock)) {
        return false;
    }

    return true;
}

bool systemSetMode(System* pSystem, SystemMode eMode) {
    if (xlObjectTest(pSystem, &gClassSystem)) {
        pSystem->eMode = eMode;

        if (eMode == SM_STOPPED) {
            pSystem->nAddressBreak = -1;
        }

        return true;
    }

    return false;
}

bool systemGetMode(System* pSystem, SystemMode* peMode) {
    if (xlObjectTest(pSystem, &gClassSystem) && (peMode != NULL)) {
        *peMode = pSystem->eMode;
        return true;
    }

    return false;
}

bool fn_8000A830(System* pSystem, s32 nEvent, void* pArgument) {
    s32 i;

    for (i = 0; i < SOT_COUNT; i++) {
        if (pSystem->apObject[i] != NULL) {
            xlObjectEvent(pSystem->apObject[i], nEvent, pArgument);
        }
    }

    return true;
}

bool fn_8000A8A8(System* pSystem) {
    fn_8000A830(pSystem, 0x1004, NULL);
    VISetBlack(true);
    VIFlush();
    VIWaitForRetrace();
    LCDisable();
    OSRestart(0x1234);
    return true;
}

bool systemReset(System* pSystem) {
    s64 nPC;
    s32 nOffsetRAM;
    int eObject; // SystemObjectType
    CpuBlock block;

    pSystem->nAddressBreak = -1;

    if (romGetImage(SYSTEM_ROM(gpSystem), NULL)) {
        if (!systemSetupGameRAM(pSystem)) {
            return false;
        }

        if (!romGetPC(SYSTEM_ROM(gpSystem), (u64*)&nPC)) {
            return false;
        }

        block.nSize = 0x100000;
        block.pfUnknown = NULL;
        block.nAddress0 = 0x10001000;
        block.nAddress1 = nPC & 0x00FFFFFF;

        if (!fn_8000A6A4(pSystem, &block)) {
            return false;
        }

        if (!cpuReset(SYSTEM_CPU(gpSystem))) {
            return false;
        }

        if (!systemSetRamMode(pSystem)) {
            return false;
        }

        cpuSetXPC(SYSTEM_CPU(gpSystem), nPC, 0, 0);

        if (!systemSetupGameALL(pSystem)) {
            return false;
        }

        for (eObject = 0; eObject < SOT_COUNT; eObject++) {
            if (pSystem->apObject[eObject] != NULL) {
                xlObjectEvent(pSystem->apObject[eObject], 0x1003, NULL);
            }
        }
    }

    return true;
}

static inline bool systemTestClassObject(System* pSystem) {
    if (xlObjectTest(pSystem, &gClassSystem)) {
        pSystem->eMode = SM_STOPPED;
        pSystem->nAddressBreak = -1;

        return true;
    }

    return false;
}

bool systemExecute(System* pSystem, s32 nCount) {
    if (!cpuExecute(SYSTEM_CPU(gpSystem), nCount, pSystem->nAddressBreak)) {
        if (!systemTestClassObject(pSystem)) {
            return false;
        }

        return false;
    }

    if (pSystem->nAddressBreak == SYSTEM_CPU(gpSystem)->nPC) {
        if (!systemTestClassObject(pSystem)) {
            return false;
        }
    }

    return true;
}

bool systemCheckInterrupts(System* pSystem) {
    s32 iException;
    s32 nMaskFinal;
    bool bUsed;
    bool bDone;
    SystemException exception;
    CpuExceptionCode eCodeFinal;

    nMaskFinal = 0;
    eCodeFinal = CEC_NONE;
    bDone = false;
    pSystem->bException = false;

    for (iException = 0; iException < ARRAY_COUNT(pSystem->anException); iException++) {
        if (pSystem->anException[iException] != 0) {
            pSystem->bException = true;

            if (!bDone) {
                if (!systemGetException(pSystem, iException, &exception)) {
                    return false;
                }

                bUsed = false;

                if (exception.eCode == CEC_INTERRUPT) {
                    if (cpuTestInterrupt(SYSTEM_CPU(gpSystem), exception.nMask) &&
                        (exception.eTypeMips == MIT_NONE || miSetInterrupt(SYSTEM_MI(gpSystem), exception.eTypeMips))) {
                        bUsed = true;
                    }
                } else {
                    bDone = true;

                    if (nMaskFinal == 0) {
                        bUsed = true;
                        eCodeFinal = exception.eCode;
                    }
                }

                if (bUsed) {
                    nMaskFinal |= exception.nMask;
                    pSystem->anException[iException] = 0;
                }
            }
        }
    }

    if (nMaskFinal != 0) {
        if (!cpuException(SYSTEM_CPU(gpSystem), CEC_INTERRUPT, nMaskFinal)) {
            return false;
        }
    } else {
        if ((eCodeFinal != CEC_NONE)) {
            if (!cpuException(SYSTEM_CPU(gpSystem), eCodeFinal, 0)) {
                return false;
            }
        }
    }

    return true;
}

bool systemExceptionPending(System* pSystem, SystemInterruptType nException) {
    if ((nException > -1) && (nException < ARRAY_COUNT(pSystem->anException))) {
        if (pSystem->anException[nException] != 0) {
            return true;
        }

        return false;
    }

    return false;
}

static inline bool systemClearExceptions(System* pSystem) {
    int iException;

    pSystem->bException = false;

    for (iException = 0; iException < 16; iException++) {
        pSystem->anException[iException] = 0;
    }

    return true;
}

static inline bool systemFreeDevices(System* pSystem) {
    int storageDevice; // SystemObjectType

    for (storageDevice = 0; storageDevice < SOT_COUNT; storageDevice++) {
        if (pSystem->apObject[storageDevice] != NULL && !xlObjectFree(&pSystem->apObject[storageDevice])) {
            return false;
        }
    }

    return true;
}

bool systemEvent(System* pSystem, s32 nEvent, void* pArgument) {
    Cpu* pCPU;
    SystemException exception;
    SystemObjectType eObject;
    SystemObjectType storageDevice;

    switch (nEvent) {
        case 2:
            pSystem->storageDevice = SOT_CPU;
            pSystem->eMode = SM_STOPPED;
            pSystem->eTypeROM = NONE;
            pSystem->nAddressBreak = -1;
            systemClearExceptions(pSystem);
            if (!systemCreateStorageDevice(pSystem, pArgument)) {
                return false;
            }
            break;
        case 3:
            if (!systemFreeDevices(pSystem)) {
                return false;
            }
            break;
        case 0x1001:
            if (!systemGetException(pSystem, (SystemInterruptType)(s32)pArgument, &exception)) {
                return false;
            }
            if (exception.eTypeMips != MIT_NONE) {
                miResetInterrupt(SYSTEM_MI(gpSystem), exception.eTypeMips);
            }
            break;
        case 0x1000:
            if (((SystemInterruptType)(s32)pArgument > SIT_NONE) && ((SystemInterruptType)(s32)pArgument < SIT_COUNT)) {
                pSystem->bException = true;
                pSystem->anException[(SystemInterruptType)(s32)pArgument]++;
                break;
            }
            return false;
        case 0x1002:
            if (!cpuSetGetBlock(SYSTEM_CPU(gpSystem), pArgument, (GetBlockFunc)systemGetBlock)) {
                return false;
            }
            if (!cpuSetDevicePut(SYSTEM_CPU(gpSystem), pArgument, (Put8Func)systemPut8, (Put16Func)systemPut16,
                                 (Put32Func)systemPut32, (Put64Func)systemPut64)) {
                return false;
            }
            if (!cpuSetDeviceGet(SYSTEM_CPU(gpSystem), pArgument, (Get8Func)systemGet8, (Get16Func)systemGet16,
                                 (Get32Func)systemGet32, (Get64Func)systemGet64)) {
                return false;
            }
            break;
        case 0:
        case 1:
        case 5:
        case 6:
        case 7:
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

_XL_OBJECTTYPE gClassSystem = {
    "SYSTEM",
    sizeof(System),
    NULL,
    (EventFunc)systemEvent,
}; // size = 0x10
