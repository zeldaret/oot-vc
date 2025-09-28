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

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47f750
typedef enum HBMSelectBtnNum {
    HBM_SELECT_NULL = -1,

    HBM_SELECT_HOMEBTN,
    HBM_SELECT_BTN1,
    HBM_SELECT_BTN2,
    HBM_SELECT_BTN3,
    HBM_SELECT_BTN4,

    HBM_SELECT_MAX
} HBMSelectBtnNum;

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x30779a
// Same arg names as UserCallback in HBMAxSound.cpp, so probably same type
typedef int HBMSoundCallback(int evt, int num);

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x307646
typedef struct HBMDataInfo {
    void* layoutBuf; // size 0x04, offset 0x00
    void* spkSeBuf; // size 0x04, offset 0x04
    void* msgBuf; // size 0x04, offset 0x08
    void* configBuf; // size 0x04, offset 0x0c
    void* mem; // size 0x04, offset 0x10
    HBMSoundCallback* sound_callback; // size 0x04, offset 0x14
    int backFlag; // size 0x04, offset 0x18
    int region; // size 0x04, offset 0x1c
    int cursor; // size 0x04, offset 0x20
    int messageFlag; // size 0x04, offset 0x24
    u32 configBufSize; // size 0x04, offset 0x28
    u32 memSize; // size 0x04, offset 0x2c
    f32 frameDelta; // size 0x04, offset 0x30
    Vec2 adjust; // size 0x08, offset 0x34
    MEMAllocator* pAllocator; // size 0x04, offset 0x3c
} HBMDataInfo; // size 0x40

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

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x30790d
typedef struct HBMKPadData {
    KPADStatus* kpad; // size 0x04, offset 0x00
    Vec2 pos; // size 0x08, offset 0x04
    u32 use_devtype; // size 0x04, offset 0x0c
} HBMKPadData; // size 0x10

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x3078d4
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

#ifdef __cplusplus
}
#endif

#endif // RVL_SDK_HBM_HOMEBUTTON_COMMON_H
