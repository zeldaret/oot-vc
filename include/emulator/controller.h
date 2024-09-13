#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "emulator/errordisplay.h"
#include "emulator/system.h"
#include "emulator/xlObject.h"
#include "macros.h"
#include "revolution/pad.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ControllerStickAxis {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_MAX = 2
} ControllerStickAxis;

typedef struct lbl_801C7DB8_Struct {
    /* 0x00 */ void* unk_00;
    /* 0x04 */ u32 unk_04;
    /* 0x08 */ u32 unk_08;
    /* 0x0C */ u32 unk_0C;
} lbl_801C7DB8_Struct; // size = 0x10

typedef struct ControllerThread {
    /* 0x000 */ OSThread thread;
    /* 0x318 */ u8 unk_318[0xA];
} ControllerThread; // size = 0x328

typedef struct Controller {
    /* 0x000 */ u32 unk_00[19];
    /* 0x04C */ s32 unk_4C[PAD_MAX_CONTROLLERS];
    /* 0x05C */ s32 analogTriggerLeft[PAD_MAX_CONTROLLERS];
    /* 0x06C */ s32 analogTriggerRight[PAD_MAX_CONTROLLERS];
    /* 0x07C */ s32 stickLeft[PAD_MAX_CONTROLLERS][AXIS_MAX];
    /* 0x09C */ s32 stickRight[PAD_MAX_CONTROLLERS][AXIS_MAX];
    /* 0x0BC */ s32 unk_BC[PAD_MAX_CONTROLLERS];
    /* 0x0CC */ s32 unk_CC[PAD_MAX_CONTROLLERS];
    /* 0x0DC */ u32 controllerConfiguration[PAD_MAX_CONTROLLERS][GCN_BTN_COUNT];
    /* 0x21C */ s32 unk_21C;
    /* 0x220 */ s32 unk_220;
    /* 0x224 */ s32 unk_224;
    /* 0x228 */ u32 unk_228[PAD_MAX_CONTROLLERS];
    /* 0x238 */ u32 unk_238[PAD_MAX_CONTROLLERS];
    /* 0x248 */ s32 unk_248;
    /* 0x24C */ s32 unk_24C;
    /* 0x250 */ u8 unk_250[0x20];
    /* 0x270 */ u32 unk_270[PAD_MAX_CONTROLLERS];
    /* 0x280 */ u32 unk_280[PAD_MAX_CONTROLLERS];
} Controller; // size = 0x290

s32 fn_80062028(EDString* pSTString);
bool simulatorSetControllerMap(Controller* pController, s32 channel, u32* mapData);
bool simulatorCopyControllerMap(Controller* pController, u32* mapDataOutput, u32* mapDataInput);
bool fn_80062E5C(Controller* pController, s32, s32*) NO_INLINE;
bool simulatorDetectController(Controller* pController, s32 arg1);
bool fn_800622B8(Controller* pController) NO_INLINE;
bool fn_800623F4(Controller* pController) NO_INLINE;
bool fn_80062C18(Controller* pController, s32 iController, s32* arg2, s32* arg3, s32* arg4, s32* arg5, s32* arg6,
                 s32* arg7);
bool fn_80062CE4(Controller* pController, s32 iController, bool bUnknown);
bool fn_800631B8(Controller* pController, s32 arg1);
bool controllerEvent(Controller* pController, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassController;

#ifdef __cplusplus
}
#endif

#endif
