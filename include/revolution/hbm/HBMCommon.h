#ifndef HOMEBUTTON_COMMON_H
#define HOMEBUTTON_COMMON_H

#include "revolution/kpad/KPAD.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/mtx/mtx.h"
#include "revolution/wpad/WPAD.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NW4HBM_VERSION(major, minor) ((major & 0xFF) << 8 | minor & 0xFF)

typedef enum HBMSelectBtnNum {
    /* -1 */ HBM_SELECT_NULL = -1,
    /* 0 */ HBM_SELECT_HOMEBTN,
    /* 1 */ HBM_SELECT_BTN1,
    /* 2 */ HBM_SELECT_BTN2,
    /* 3 */ HBM_SELECT_BTN3,
    /* 4 */ HBM_SELECT_BTN4,
    /* 5 */ HBM_SELECT_MAX
} HBMSelectBtnNum;

typedef bool (*HBMSoundCallback)(s32 evt, s32 num);

typedef struct HBMDataInfo {
    /* 0x00 */ void* layoutBuf;
    /* 0x04 */ void* spkSeBuf;
    /* 0x08 */ void* msgBuf;
    /* 0x0C */ void* configBuf;
    /* 0x10 */ void* mem;
    /* 0x14 */ HBMSoundCallback sound_callback;
    /* 0x18 */ int backFlag;
    /* 0x1C */ int region;
    /* 0x20 */ int cursor;
    /* 0x24 */ int messageFlag;
    /* 0x28 */ u32 memSize;
    /* 0x2C */ f32 frameDelta;
    /* 0x30 */ Vec2 adjust;
    /* 0x38 */ MEMAllocator* pAllocator;
} HBMDataInfo; // size = 0x3C

typedef struct HBMKPadData {
    /* 0x00 */ KPADStatus* kpad;
    /* 0x04 */ Vec2 pos;
    /* 0x0C */ u32 use_devtype;
} HBMKPadData; // size = 0x10

typedef struct HBMControllerData {
    /* 0x00 */ HBMKPadData wiiCon[WPAD_MAX_CONTROLLERS];
} HBMControllerData; // size = 0x40

void HBMCreate(const HBMDataInfo* pHBInfo);
void HBMDelete(void);
void HBMInit(void);
HBMSelectBtnNum HBMCalc(const HBMControllerData* pController);
void HBMDraw(void);
HBMSelectBtnNum HBMGetSelectBtnNum(void);
void HBMSetAdjustFlag(bool flag);
void HBMStartBlackOut(void);
bool HBMIsReassignedControllers(void);

void HBMCreateSound(const char* soundData, void* memBuf, u32 memSize);
void HBMDeleteSound(void);
void HBMUpdateSound(void);

void HBMUpdateSoundArchivePlayer(void);
void HBMSetSoundVolume(f32 volume);
void HBMPlaySound(int num);
void HBMStopSound(void);

#ifdef __cplusplus
}
#endif

#endif
