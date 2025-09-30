#ifndef RVL_SDK_HBM_HOMEBUTTON_COMMON_H
#define RVL_SDK_HBM_HOMEBUTTON_COMMON_H

#include "revolution/gx/GXTypes.h"
#include "revolution/hbm/HBMConfig.h"
#include "revolution/kpad/KPAD.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/mtx/mtx.h"
#include "revolution/types.h"
#include "revolution/wpad/WPAD.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HBMSelectBtnNum {
    HBM_SELECT_NULL = -1,

    HBM_SELECT_HOMEBTN,
    HBM_SELECT_BTN1,
    HBM_SELECT_BTN2,
    HBM_SELECT_BTN3,
    HBM_SELECT_BTN4,

    HBM_SELECT_MAX
} HBMSelectBtnNum;

// Same arg names as UserCallback in HBMAxSound.cpp, so probably same type
typedef int HBMSoundCallback(int evt, int num);

typedef struct HBMDataInfo {
    /* 0x00 */ void* layoutBuf;
    /* 0x04 */ void* spkSeBuf;
    /* 0x08 */ void* msgBuf;
    /* 0x0C */ void* configBuf;
    /* 0x10 */ void* mem;
    /* 0x14 */ HBMSoundCallback* sound_callback;
    /* 0x18 */ int backFlag;
    /* 0x1C */ int region;
    /* 0x20 */ int cursor;
    /* 0x24 */ int messageFlag;
    /* 0x28 */ u32 memSize;
    /* 0x2C */ f32 frameDelta;
    /* 0x30 */ Vec2 adjust;
    /* 0x38 */ MEMAllocator* pAllocator;
} HBMDataInfo; // size 0x3C

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
#define HBMDataInfoEx void
#elif HBM_APP_TYPE == HBM_APP_TYPE_NAND
typedef struct HBMDataInfoEx {
    void* layoutBuf; // size 0x04, offset 0x00 // see homebutton::HomeButton::createInfoEx
    void* msgBuf; // size 0x04, offset 0x04 // see homebutton::set_other_text
    void* texImage; // size 0x04, offset 0x08
    byte1_t pad[4];
    u16 texImageWidth; // size 0x02, offset 0x10
    u16 texImageHeight; // size 0x02, offset 0x12
    GXTexFmt texImageFormat; // size 0x04, offset 0x14
} HBMDataInfoEx; // size 0x??
#endif

typedef struct HBMKPadData {
    KPADStatus* kpad; // size 0x04, offset 0x00
    Vec2 pos; // size 0x08, offset 0x04
    u32 use_devtype; // size 0x04, offset 0x0c
} HBMKPadData; // size 0x10

typedef struct HBMControllerData {
    HBMKPadData wiiCon[WPAD_MAX_CONTROLLERS]; // size 0x40, offset 0x00
} HBMControllerData; // size 0x40

void HBMCreate(const HBMDataInfo* pHBInfo);
void HBMDelete(void);
void HBMInit(void);
HBMSelectBtnNum HBMCalc(const HBMControllerData* pController);
void HBMDraw(void);
HBMSelectBtnNum HBMGetSelectBtnNum(void);
void HBMSetAdjustFlag(bool flag);
void HBMStartBlackOut(void);
bool HBMIsReassignedControllers(void);

void HBMCreateSound(void* soundData, void* memBuf, u32 memSize);
void HBMDeleteSound(void);
void HBMUpdateSound(void);

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
// guesses based on dealing with member homebutton::HomeButton::mpHBInfoEx
void HBMCreateInfoEx(const HBMDataInfoEx* pHBInfoEx);
void HBMDeleteInfoEx(void);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

//! TODO: figure out names
void fn_80100AEC(int num);
void fn_80100B88(void);
void fn_80100BA0(f32 volume);
void fn_80100C38(void);
void fn_80100CD8(const char* path, void* param1, int param2);
void fn_80100E0C(void);
void fn_80100E40(void);

#ifdef __cplusplus
}
#endif

#endif // RVL_SDK_HBM_HOMEBUTTON_COMMON_H
