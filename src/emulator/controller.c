#include "emulator/controller.h"
#include "emulator/helpRVL.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlCoreRVL.h"
#include "emulator/xlHeap.h"
#include "macros.h"
#include "math.h"
#include "revolution/mem.h"
#include "revolution/vi.h"
#include "revolution/wpad.h"

u32 lbl_80174508[] = {
    PAD_CHAN0_BIT,
    PAD_CHAN1_BIT,
    PAD_CHAN2_BIT,
    PAD_CHAN3_BIT,
};

u32 lbl_80174518[] = {
    0x00000001, 0x00000002, 0x00008000, 0x00010000, 0x00020000, 0x00040000, 0x00000020,
    0x00000600, 0x00000080, 0x00000100, 0x00200000, 0x00400000, 0x00080000, 0x00100000,
    0x00800000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

_XL_OBJECTTYPE gClassController = {
    "Controller",
    sizeof(Controller),
    NULL,
    (EventFunc)controllerEvent,
};

MEMAllocator gControllerAllocator;
ControllerThread gControllerThread;

static void* sControllerHeap;
static VIRetraceCallback sControllerVICallback;

void* fn_80061FB0(u32 nSize) {
    void* pBuffer = MEMAllocFromAllocator(&gControllerAllocator, nSize);

    if (pBuffer == NULL) {
        xlExit();
    }

    return pBuffer;
}

bool fn_80061FF8(Controller* pController) {
    MEMFreeToAllocator(&gControllerAllocator, pController);
    return true;
}

static inline bool unk4C_UnknownInline(Controller* pController) {
    bool bRet = true;

    if (pController->unk_4C[0] != 3 && pController->unk_4C[0] != 2) {
        bRet = false;
    }

    return bRet;
}

s32 fn_80062028(EDString* pSTString) {
    Controller* pController;

    OSGetTime();
    pController = SYSTEM_CONTROLLER(gpSystem);

    switch (pController->iString) {
        case ERROR_NO_CONTROLLER:
            return 2;
        case ERROR_NEED_CLASSIC:
            if (unk4C_UnknownInline(pController)) {
                return 2;
            }
            break;
        default:
            break;
    }

    return 0;
}

bool fn_80080C04(Controller* pController, ErrorIndex iString) {
    pController->unk_248 = OSGetTime();
    pController->iString = iString;
    errorDisplayShow(iString);
    pController->iString = ERROR_NONE;

    if (!fn_800607C4(SYSTEM_HELP(gpSystem), 0)) {
        return false;
    }

    return true;
}

bool fn_800620A8(Controller* pController) {
    void* sp8;
    s32 i;

    pController->unk_220 = 1;
    pController->iString = ERROR_NONE;

    for (i = 0; i < PAD_MAX_CONTROLLERS; i++) {
        pController->unk_228[i] = 0;
        pController->unk_238[i] = 0;
        pController->unk_270[i] = 0;
        pController->unk_280[i] = 0;
        pController->unk_4C[i] = 0;
        pController->unk_BC[i] = pController->unk_CC[i] = 0;
        pController->stickLeft[i][AXIS_X] = pController->stickLeft[i][AXIS_Y] = 0;
        pController->stickRight[i][AXIS_X] = pController->stickRight[i][AXIS_Y] = 0;
    }

    for (i = 0; i < 19; i++) {
        pController->unk_00[i] = lbl_80174518[i];
    }

    PADInit();

    for (i = 0; i < ARRAY_COUNT(lbl_80174508); i++) {
        fn_800BBD18(lbl_80174508[i]);
    }

    if (!xlHeapTake(&sp8, 0x20000 | 0x70000000)) {
        return false;
    }

    MEMInitAllocatorForExpHeap(&gControllerAllocator, MEMCreateExpHeapEx(sp8, 0x20000, 0), 4);
    fn_800BE994(fn_80061FB0, fn_80061FF8);
    WPADInit();

    return true;
}

static inline bool controllerValidateIndex(s32 index) {
    bool ret;

    if (index >= 0 && index < PAD_MAX_CONTROLLERS) {
        ret = true;
    } else {
        ret = false;
    }

    return ret;
}

bool fn_800622B8(Controller* pController) {
    s32 var_r0;
    s32 var_r0_2;
    s32 var_r0_3;
    s32 var_r0_4;

    s32 i;
    bool ret;

    pController->unk_220 = 1;

    for (i = 0; i < PAD_MAX_CONTROLLERS; i++) {
        pController->unk_BC[i] = pController->unk_CC[i] = 0;
        pController->stickLeft[i][AXIS_X] = pController->stickLeft[i][AXIS_Y] = 0;
        pController->stickRight[i][AXIS_Y] = 0;
        pController->iString = ERROR_NONE; // why here
        pController->stickRight[i][AXIS_X] = 0;

        if (!controllerValidateIndex(i)) {
            return false;
        }
    }

    return true;
}

bool fn_800623F4(Controller* pController) { return true; }

bool simulatorDetectController(Controller* pController, s32 arg1) { return arg1 >= 0 && arg1 < PAD_MAX_CONTROLLERS; }

bool fn_80062C18(Controller* pController, s32 iController, s32* arg2, s32* arg3, s32* arg4, s32* arg5, s32* arg6,
                 s32* arg7) {
    if (iController >= 0 && iController < PAD_MAX_CONTROLLERS) {
        if (arg2 != NULL) {
            *arg2 = pController->unk_BC[iController];
        }

        if (arg3 != NULL) {
            *arg3 = pController->unk_CC[iController];
        }

        if (arg4 != NULL) {
            *arg4 = pController->stickLeft[iController][AXIS_X];
        }

        if (arg5 != NULL) {
            *arg5 = pController->stickLeft[iController][AXIS_Y];
        }

        if (arg6 != NULL) {
            *arg6 = pController->stickRight[iController][AXIS_X];
        }

        if (arg7 != NULL) {
            *arg7 = pController->stickRight[iController][AXIS_Y];
        }

        pController->unk_220 = 1;
        return !!pController->unk_4C[iController];
    }

    return false;
}

bool fn_80062CE4(Controller* pController, s32 iController, bool bUnknown) {
    if (iController >= 0 && iController < 4) {
        return true;
    }

    return false;
}

bool simulatorSetControllerMap(Controller* pController, s32 channel, u32* mapData) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(pController->controllerConfiguration[channel]); i++) {
        pController->controllerConfiguration[channel][i] = mapData[i];
    }

    return true;
}

bool simulatorCopyControllerMap(Controller* pController, u32* mapDataOutput, u32* mapDataInput) {
    s32 i;

    for (i = 0; i < GCN_BTN_COUNT; i++) {
        mapDataOutput[i] = mapDataInput[i];
    }

    return true;
}

bool fn_80062E5C(Controller* pController, s32 arg1, s32* arg2) { return true; }

static void* controllerThread(void* pArg) {
    Controller* pController = SYSTEM_CONTROLLER(gpSystem);

    while (true) {
        fn_800623F4(pController);
        OSSuspendThread(&gControllerThread.thread);
    }
}

void controllerVIPostRetrace(u32 retraceCount) {
    if (SYSTEM_CONTROLLER(gpSystem)->unk_224 != 0 && OSIsThreadSuspended(&gControllerThread.thread)) {
        OSResumeThread(&gControllerThread.thread);
    }

    if (sControllerVICallback != NULL) {
        sControllerVICallback(retraceCount);
    }
}

bool fn_800631B8(Controller* pController, s32 arg1) {
    s32 var_r31;

    pController->unk_224 = arg1;

    if (arg1 != 0) {
        for (var_r31 = 0; var_r31 < 8; var_r31++) {
            VIWaitForRetrace();
        }
    }

    return true;
}

bool controllerEvent(Controller* pController, s32 nEvent, void* pArgument) {
    s32 i;

    switch (nEvent) {
        case 0:
            fn_800620A8(pController);
            pController->unk_224 = 1;
            if (xlHeapTake(&sControllerHeap, 0x4000 | 0x30000000)) {
                OSCreateThread(&gControllerThread.thread, (OSThreadFunc)controllerThread, pController,
                               (s32*)sControllerHeap + 0x4000, 0x4000, 0, 1);
                sControllerVICallback = VISetPostRetraceCallback(controllerVIPostRetrace);
            }
            break;
        case 1:
            fn_800622B8(pController);
            VISetPostRetraceCallback(NULL);
            xlHeapFree(&sControllerHeap);
            break;
        case 2:
        case 3:
            break;
        case 0x1003:
            if (!unk4C_UnknownInline(pController)) {
                for (i = 0; i < 0x78; i++) {
                    VIWaitForRetrace();
                }
            }

            if (!unk4C_UnknownInline(pController)) {
                if (!fn_80080C04(pController, ERROR_NEED_CLASSIC)) {
                    return false;
                }

                if (!unk4C_UnknownInline(pController)) {
                    VISetBlack(true);
                    VIFlush();
                    VIWaitForRetrace();
                    OSReturnToMenu();
                }
            }
            break;
        case 0x1004:
        case 0x1007:
            break;
        default:
            return false;
    }

    return true;
}
