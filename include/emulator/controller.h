#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "emulator/errordisplay.h"
#include "emulator/system.h"
#include "emulator/xlObject.h"
#include "macros.h"
#include "revolution/kpad/KPAD.h"
#include "revolution/pad.h"
#include "revolution/types.h"
#include "versions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ControllerStickAxis {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_MAX = 2
} ControllerStickAxis;

typedef struct struct_801C7DC8 {
    /* 0x000 */ KPADStatus status[KPAD_MAX_CONTROLLERS];
#if IS_OOT
    /* 0x220 */ u8 pad[0x330];
#elif IS_MK64
    /* 0x220 */ u8 pad[0x1320];
#endif
} struct_801C7DC8; // size = 0x550 ; 0x1540

typedef struct ControllerThread {
    /* 0x000 */ OSThread thread;
#if IS_OOT
    /* 0x318 */ u8 unk_318[0xA];
#endif
} ControllerThread; // size = 0x328 ; 0x318

typedef struct Controller {
    /* 0x000 */ u32 unk_00[19]; // see lbl_80174518 for values
    /* 0x04C */ s32 unk_4C[PAD_MAX_CONTROLLERS];
    /* 0x05C */ s32 analogTriggerLeft[PAD_MAX_CONTROLLERS];
    /* 0x06C */ s32 analogTriggerRight[PAD_MAX_CONTROLLERS];
    /* 0x07C */ volatile s32 stickLeft[PAD_MAX_CONTROLLERS][AXIS_MAX];
    /* 0x09C */ volatile s32 stickRight[PAD_MAX_CONTROLLERS][AXIS_MAX];

    //! TODO: fake match?
    /* 0x0BC */ union {
        s32 unk_BC[PAD_MAX_CONTROLLERS];
        volatile s32 vunk_BC[PAD_MAX_CONTROLLERS];
    };

    /* 0x0CC */ volatile s32 unk_CC[PAD_MAX_CONTROLLERS];
    /* 0x0DC */ u32 controllerConfiguration[PAD_MAX_CONTROLLERS][GCN_BTN_COUNT];
    /* 0x21C */ ErrorIndex iString;
    /* 0x220 */ volatile s32 unk_220;
    /* 0x224 */ s32 unk_224;
    /* 0x228 */ u32 unk_228[PAD_MAX_CONTROLLERS];
    /* 0x238 */ u32 unk_238[PAD_MAX_CONTROLLERS];
    /* 0x248 */ s64 unk_248;
    /* 0x250 */ u8 unk_250[0x20];
    /* 0x270 */ u32 unk_270[PAD_MAX_CONTROLLERS];
    /* 0x280 */ u32 unk_280[PAD_MAX_CONTROLLERS];
} Controller; // size = 0x290

s32 fn_80062028(EDString* pSTString);
bool simulatorDetectController(Controller* pController, s32 arg1);
bool fn_80062C18(Controller* pController, s32 iController, s32* pnButton, s32* arg3, s32* pnStickLeftX,
                 s32* pnStickLeftY, s32* pnStickRightX, s32* pnStickRightY);
bool fn_80062CE4(Controller* pController, s32 iController, bool bUnknown);
bool simulatorSetControllerMap(Controller* pController, s32 channel, u32* mapData);
bool simulatorCopyControllerMap(Controller* pController, u32* mapDataOutput, u32* mapDataInput);
bool fn_80062E5C(Controller* pController, s32 iController, s32* pnInput);
bool fn_800631B8(Controller* pController, s32 arg1);
bool controllerEvent(Controller* pController, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassController;

#ifdef __cplusplus
}
#endif

#endif
