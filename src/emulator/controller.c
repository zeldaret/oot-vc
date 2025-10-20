#include "emulator/controller.h"
#include "emulator/helpRVL.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlCoreRVL.h"
#include "emulator/xlHeap.h"
#include "macros.h"
#include "math.h"
#include "revolution/kpad/KPAD.h"
#include "revolution/mem.h"
#include "revolution/vi.h"
#include "revolution/wpad.h"
#include "versions.h"

static void* fn_80061FB0(u32 nSize);
static bool fn_80061FF8(Controller* pController);
static bool fn_80080C04(Controller* pController, ErrorIndex iString);
static bool fn_800620A8(Controller* pController);
static bool fn_800622B8(Controller* pController);
static bool fn_800623F4(Controller* pController);
static void* controllerThread(void* pArg);
static void controllerVIPostRetrace(u32 retraceCount);

static inline bool unk4C_UnknownInline(Controller* pController);
static inline bool controllerValidateIndex(s32 index);
static inline s32 fn_800623F4_UnknownInline(f32 value);

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

#if VERSION > MK64_E
struct_801C7DC8 lbl_801C7DC8;
#endif

ControllerThread gControllerThread;
static void* sControllerHeap;
static VIRetraceCallback sControllerVICallback;

static void* fn_80061FB0(u32 nSize) {
    void* pBuffer = MEMAllocFromAllocator(&gControllerAllocator, nSize);

    if (pBuffer == NULL) {
        xlExit();
    }

    return pBuffer;
}

static bool fn_80061FF8(Controller* pController) {
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
        case ERROR_REMOTE_BATTERY:
        default:
            break;
    }

    return 0;
}

static bool fn_80080C04(Controller* pController, ErrorIndex iString) {
    pController->unk_248 = OSGetTime();
    pController->iString = iString;
    errorDisplayShow(iString);
    pController->iString = ERROR_NONE;

    if (!helpMenu_800607C4(SYSTEM_HELP(gpSystem), false)) {
        return false;
    }

    return true;
}

static bool fn_800620A8(Controller* pController) {
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

    for (i = 0; i < ARRAY_COUNT(pController->unk_00); i++) {
        pController->unk_00[i] = lbl_80174518[i];
    }

    PADInit();

    for (i = 0; i < ARRAY_COUNT(lbl_80174508); i++) {
        PADReset(lbl_80174508[i]);
    }

    if (!xlHeapTake(&sp8, 0x20000 | 0x70000000)) {
        return false;
    }

    MEMInitAllocatorForExpHeap(&gControllerAllocator, MEMCreateExpHeapEx(sp8, 0x20000, 0), 4);
    fn_800BE994(fn_80061FB0, fn_80061FF8);
    KPADInit();

    return true;
}

static inline bool controllerValidateIndex(s32 index) {
    bool ret;
    if ((index >= 0) && (index < 4)) {
        ret = true;
    } else {
        ret = false;
    }
    return ret;
}

static bool fn_800622B8(Controller* pController) {
    s32 i;

#if VERSION >= MK64_J
    pController->unk_220 = 1;
    pController->iString = ERROR_NONE;

    for (i = 0; i < PAD_MAX_CONTROLLERS; i++) {
        pController->vunk_BC[i] = pController->unk_CC[i] = 0;
        pController->stickLeft[i][AXIS_X] = pController->stickLeft[i][AXIS_Y] = 0;
        pController->stickRight[i][AXIS_X] = pController->stickRight[i][AXIS_Y] = 0;
    }
#endif

    for (i = 0; i < PAD_MAX_CONTROLLERS; i++) {
        if (!controllerValidateIndex(i)) {
            return false;
        }
    }

    return true;
}

static inline s32 fn_800623F4_UnknownInline(f32 value) {
    value /= 127.0f;

    if (value < 0.0f) {
        return 127.0f * (-1.0f + sqrtf(1.0f + value));
    } else {
        return 127.0f * (1.0f - sqrtf(1.0f - value));
    }
}

extern bool fn_800CAFB8(s32, struct_801C7DC8*, s32);

static bool fn_800623F4(Controller* pController) {
#if VERSION < OOT_J 
    struct_801C7DC8 lbl_801C7DC8;
#endif

    PADStatus status[PAD_MAX_CONTROLLERS];
    s32 i;
    s32 var_r26;
    s32 value4;
    s32 var_r24;
    s32 var_r23;
    s32 value2;
    s32 value3;
    s32 var_r20;
    s32 var_r19;
    s32 var_r18;
    s32 var_r17;
    s32 sp98;
    s32 sp94;
    s32 sp90;
    s32 sp8C;
    s32 sp88;
    s32 sp84;
    s32 sp80;
    s32 var_r14;
    s32 var_r16;
    s32 var_r15;
    s32 var_r3;
    s32 var_r4;
    s32 var_r5;

    PADRead(status);
    PADClamp(status);

    for (i = 0; i < 4; i++) {
        if (status[i].err == -1) {
            PADReset(lbl_80174508[i]);
        }
    }

    for (i = 0; i < PAD_MAX_CONTROLLERS; i++) {
        var_r23 = 0;
        var_r24 = 0;
        value2 = value3 = 0;
        value4 = 0;
        fn_800CB890(i);
        var_r15 = 0;
        var_r16 = 0;
        if (status[i].err == 0) {
            value4 |= 8;
            if (status[i].button & 1) {
                value2 |= 0x80000;
            }
            if (status[i].button & 2) {
                value2 |= 0x100000;
            }
            if (status[i].button & 8) {
                value2 |= 0x200000;
            }
            if (status[i].button & 4) {
                value2 |= 0x400000;
            }
            if (status[i].button & 0x10) {
                value2 |= 0x600;
            }
            if (status[i].button & 0x40) {
                value2 |= 0x80;
            }
            if (status[i].button & 0x20) {
                value2 |= 0x100;
            }
            if (status[i].button & 0x100) {
                value2 |= 1;
            }
            if (status[i].button & 0x200) {
                value2 |= 2;
            }
            if (status[i].button & 0x400) {
                value2 |= 4;
            }
            if (status[i].button & 0x800) {
                value2 |= 8;
            }
            if (status[i].button & 0x1000) {
                value2 |= 0x20;
            }
            sp98 = 127.0f * (status[i].stickX / 56.0f);
            sp94 = 127.0f * (status[i].stickY / 56.0f);
            sp90 = 127.0f * (status[i].substickX / 44.0f);
            sp8C = 127.0f * (status[i].substickY / 44.0f);
            if (value2 != 0) {
                var_r16 = 0x641;
            } else {
                s32 temp_r4_2 = (status[i].stickX * status[i].stickX) + (status[i].stickY * status[i].stickY);
                var_r16 = (status[i].substickX * status[i].substickX) + (status[i].substickY * status[i].substickY);
                if (var_r16 < temp_r4_2) {
                    var_r16 = temp_r4_2;
                }
            }
        }

#if VERSION > MK64_E
        fn_800CAFB8(i, &lbl_801C7DC8, 10);
        if (lbl_801C7DC8.status[0].wpad_err == 0)
#elif VERSION < OOT_J 
        if (fn_800CAFB8(i, &lbl_801C7DC8, 10) && lbl_801C7DC8.status[0].wpad_err == 0)
#endif
        {
            value4 |= 2;
            if (lbl_801C7DC8.status[0].hold & 0x8000) {
                value3 |= 0x10;
            }
            if (lbl_801C7DC8.status[0].hold & 1) {
                value3 |= 0x02000000;
            }
            if (lbl_801C7DC8.status[0].hold & 2) {
                value3 |= 0x04000000;
            }
            if (lbl_801C7DC8.status[0].hold & 8) {
                value3 |= 0x08000000;
            }
            if (lbl_801C7DC8.status[0].hold & 4) {
                value3 |= 0x10000000;
            }
            if (lbl_801C7DC8.status[0].hold & 0x800) {
                value3 |= 0x20000000;
            }
            if (lbl_801C7DC8.status[0].dev_type == 2) {
                value4 |= 4;
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x10) {
                    value3 |= 1;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x40) {
                    value3 |= 2;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 8) {
                    value3 |= 4;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x20) {
                    value3 |= 8;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x800) {
                    value3 |= 0x10;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x400) {
                    value3 |= 0x20;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x1000) {
                    value3 |= 0x40;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x2000) {
                    value3 |= 0x80;
                }
                if ((lbl_801C7DC8.status[0].ex_status.cl.hold & 0xFFFFFFFFFFFFFFFFu) & 0x200) {
                    value3 |= 0x100;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x80) {
                    value3 |= 0x200;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 4) {
                    value3 |= 0x400;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 2) {
                    value3 |= 0x80000;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x8000) {
                    value3 |= 0x100000;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 1) {
                    value3 |= 0x200000;
                }
                if (lbl_801C7DC8.status[0].ex_status.cl.hold & 0x4000) {
                    value3 |= 0x400000;
                }
                var_r24 = 127.0f * lbl_801C7DC8.status[0].ex_status.cl.ltrigger;
                var_r23 = 127.0f * lbl_801C7DC8.status[0].ex_status.cl.rtrigger;
                sp88 = 127.0f * lbl_801C7DC8.status[0].ex_status.cl.lstick.x;
                sp84 = 127.0f * lbl_801C7DC8.status[0].ex_status.cl.lstick.y;
                sp80 = 127.0f * lbl_801C7DC8.status[0].ex_status.cl.rstick.x;
                var_r14 = 127.0f * lbl_801C7DC8.status[0].ex_status.cl.rstick.y;
                if (value3 != 0) {
                    var_r15 = 0x641;
                } else {
                    s32 temp_r0_2 =
                        3249.0f *
                        ((lbl_801C7DC8.status[0].ex_status.cl.lstick.x * lbl_801C7DC8.status[0].ex_status.cl.lstick.x) +
                         (lbl_801C7DC8.status[0].ex_status.cl.lstick.y * lbl_801C7DC8.status[0].ex_status.cl.lstick.y));
                    var_r15 =
                        3249.0f *
                        ((lbl_801C7DC8.status[0].ex_status.cl.rstick.x * lbl_801C7DC8.status[0].ex_status.cl.rstick.x) +
                         (lbl_801C7DC8.status[0].ex_status.cl.rstick.y * lbl_801C7DC8.status[0].ex_status.cl.rstick.y));
                    if (var_r15 < temp_r0_2) {
                        var_r15 = temp_r0_2;
                    }
                }
            }
        }
        pController->unk_270[i]++;
        pController->unk_280[i]++;
        var_r17 = 0;
        var_r18 = 0;
        var_r19 = 0;
        var_r20 = 0;
        if (value4 == 0) {
            var_r26 = 0;
        } else {
            var_r4 = 0;
            var_r5 = 0;
            if ((value4 & 8) && (!(value4 & 4))) {
                var_r4 = 1;
            } else if ((!(value4 & 8)) && (value4 & 4)) {
                var_r5 = 1;
            } else if (value4 != 2) {
                s32 temp_r0_3 = pController->unk_280[i] - pController->unk_270[i];
                if ((var_r15 > 0x640) || ((var_r15 > 0x384) && (temp_r0_3 < (-0x14)))) {
                    var_r5 = 1;
                } else if ((var_r16 > 0x640) || ((var_r16 > 0x384) && (temp_r0_3 > 0x14))) {
                    var_r4 = 1;
                } else {
                    if (temp_r0_3 < 0) {
                        var_r4 = 1;
                    }
                    if (temp_r0_3 >= 0) {
                        var_r5 = 1;
                    }
                }
            }
            if (var_r4 != 0) {
                pController->unk_280[i] = 0;
                var_r26 = 3;
                var_r20 = sp98;
                var_r19 = sp94;
                var_r18 = sp90;
                var_r17 = sp8C;
            } else if (var_r5 != 0) {
                var_r26 = 1;
                pController->unk_270[i] = 0;
                if (value4 & 4) {
                    var_r26 = 2;
                }
                var_r20 = sp88;
                var_r19 = sp84;
                var_r18 = sp80;
                var_r17 = var_r14;
            }

#if VERSION > MK64_E
            var_r20 = fn_800623F4_UnknownInline(var_r20);
            var_r19 = fn_800623F4_UnknownInline(var_r19);
#endif

            if (var_r20 < -0x3F) {
                value3 |= 0x800;
            }
            if (var_r20 > 0x3F) {
                value3 |= 0x1000;
            }
            if (var_r19 < -0x3F) {
                value3 |= 0x2000;
            }
            if (var_r19 > 0x3F) {
                value3 |= 0x4000;
            }
            if (var_r18 < -0x3F) {
                value3 |= 0x8000;
            }
            if (var_r18 > 0x3F) {
                value3 |= 0x10000;
            }
            if (var_r17 < -0x3F) {
                value3 |= 0x20000;
            }
            if (var_r17 > 0x3F) {
                value3 |= 0x40000;
            }
        }

        pController->unk_4C[i] = var_r26;
        pController->unk_CC[i] = pController->unk_BC[i];

        if (pController->unk_220 != 0) {
            var_r3 = 0;
        } else {
            var_r3 = pController->unk_BC[i];
            value4 = value3;
        }

        pController->unk_BC[i] = value3 | value2;
        pController->unk_BC[i] |= var_r3;
        pController->analogTriggerLeft[i] = var_r24;
        pController->analogTriggerRight[i] = var_r23;
        pController->stickLeft[i][AXIS_X] = var_r20;
        pController->stickLeft[i][AXIS_Y] = var_r19;
        pController->stickRight[i][AXIS_X] = var_r18;
        pController->stickRight[i][AXIS_Y] = var_r17;

        if ((pController->unk_BC[i] & 0x10) && !(pController->unk_CC[i] & 0x10) &&
            !helpMenu_800607C4(SYSTEM_HELP(gpSystem), true)) {
            return 0;
        }
    }

    pController->unk_220 = 0;
    return true;
}

bool simulatorDetectController(Controller* pController, s32 arg1) { return arg1 >= 0 && arg1 < PAD_MAX_CONTROLLERS; }

bool fn_80062C18(Controller* pController, s32 iController, s32* pnButton, s32* arg3, s32* pnStickLeftX,
                 s32* pnStickLeftY, s32* pnStickRightX, s32* pnStickRightY) {
    if (iController >= 0 && iController < PAD_MAX_CONTROLLERS) {
        if (pnButton != NULL) {
            *pnButton = pController->unk_BC[iController];
        }

        if (arg3 != NULL) {
            *arg3 = pController->unk_CC[iController];
        }

        if (pnStickLeftX != NULL) {
            *pnStickLeftX = pController->stickLeft[iController][AXIS_X];
        }

        if (pnStickLeftY != NULL) {
            *pnStickLeftY = pController->stickLeft[iController][AXIS_Y];
        }

        if (pnStickRightX != NULL) {
            *pnStickRightX = pController->stickRight[iController][AXIS_X];
        }

        if (pnStickRightY != NULL) {
            *pnStickRightY = pController->stickRight[iController][AXIS_Y];
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

bool fn_80062E5C(Controller* pController, s32 iController, s32* pnInput) {
    s32 nButton;
    s32 nStickLeftX;
    s32 nStickLeftY;
    s32 nStickRightX;
    s32 nStickRightY;

    fn_80062C18(pController, iController, &nButton, NULL, &nStickLeftX, &nStickLeftY, &nStickRightX, &nStickRightY);

    *pnInput = (nStickLeftX & 0xFF) << 0x8 | nStickLeftY & 0xFF;

    if (nStickRightX < -0x3F) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_CSTICK_LEFT];
    }

    if (nStickRightX > 0x3F) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_CSTICK_RIGHT];
    }

    if (nStickRightY < -0x3F) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_CSTICK_DOWN];
    }

    if (nStickRightY > 0x3F) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_CSTICK_UP];
    }

    if (nButton & 0x20) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_START];
    }

    if (nButton & 0x600) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_Z];
    }

    if (nButton & 0x100) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_R];
    }

    if (nButton & 0x80) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_L];
    }

    if (nButton & 1) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_A];
    }

    if (nButton & 2) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_B];
    }

    if (nButton & 4) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_X];
    }

    if (nButton & 8) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_Y];
    }

    if (nButton & 0x200000) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_DPAD_UP];
    }

    if (nButton & 0x400000) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_DPAD_DOWN];
    }

    if (nButton & 0x80000) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_DPAD_LEFT];
    }

    if (nButton & 0x100000) {
        *pnInput |= pController->controllerConfiguration[iController][GCN_BTN_DPAD_RIGHT];
    }

    return true;
}

static void* controllerThread(void* pArg) {
    Controller* pController = SYSTEM_CONTROLLER(gpSystem);

    while (true) {
        fn_800623F4(pController);
        OSSuspendThread(&gControllerThread.thread);
    }
}

static void controllerVIPostRetrace(u32 retraceCount) {
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

#if VERSION >= MK64_J
    if (arg1 != 0) {
        for (var_r31 = 0; var_r31 < 8; var_r31++) {
            VIWaitForRetrace();
        }
    }
#endif

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
                               (void*)((u8*)sControllerHeap + 0x4000), 0x4000, OS_PRIORITY_MIN, 1);
                sControllerVICallback = VISetPostRetraceCallback(controllerVIPostRetrace);
            }
            break;
        case 1:
            fn_800622B8(pController);
            VISetPostRetraceCallback(NULL);
            xlHeapFree(&sControllerHeap);
            break;
        case 0x1003:
            for (i = 0; i < 0x78; i++) {
                if (unk4C_UnknownInline(pController)) {
                    break;
                }

                VIWaitForRetrace();
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
        case 2:
        case 3:
        case 5:
        case 6:
        case 0x1000:
        case 0x1001:
        case 0x1002:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return false;
    }

    return true;
}
