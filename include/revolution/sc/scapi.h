#ifndef _RVL_SDK_SC_SCAPI_H
#define _RVL_SDK_SC_SCAPI_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SC_ASPECT_STD,
    SC_ASPECT_WIDE
} SCAspectRatio;

typedef enum {
    SC_EURGB_50_HZ,
    SC_EURGB_60_HZ
} SCEuRgb60Mode;

typedef enum {
    SC_LANG_NONE = -1,
    SC_LANG_JP = 0,
    SC_LANG_EN = 1,
    SC_LANG_DE = 2,
    SC_LANG_FR = 3,
    SC_LANG_SP = 4,
    SC_LANG_IT = 5,
    SC_LANG_NL = 6,
    SC_LANG_ZH_S = 7,
    SC_LANG_ZH_T = 8,
    SC_LANG_KR = 9
} SCLanguage;

typedef enum {
    SC_SND_MONO,
    SC_SND_STEREO,
    SC_SND_SURROUND
} SCSoundMode;

typedef enum {
    SC_SENSOR_BAR_BOTTOM,
    SC_SENSOR_BAR_TOP
} SCSensorBarPos;

typedef struct SCIdleModeInfo {
    /* 0x0 */ u8 wc24;
    /* 0x1 */ u8 slotLight;
} SCIdleModeInfo;

typedef struct SCBtDeviceInfo {
    /* 0x0 */ u8 mac[6];
    /* 0x6 */ char name[64];
} SCBtDeviceInfo;

typedef struct SCBtDeviceInfoArray {
    /* 0x0 */ u8 numRegist;
    /* 0x1 */ SCBtDeviceInfo regist[10];
    /* 0x2BD */ SCBtDeviceInfo active[6];
} SCBtDeviceInfoArray;

u8 SCGetAspectRatio(void);
s8 SCGetDisplayOffsetH(void);
u8 SCGetEuRgb60Mode(void);
void SCGetIdleMode(SCIdleModeInfo* mode);
u8 SCGetLanguage(void);
u8 SCGetProgressiveMode(void);
u8 SCGetScreenSaverMode(void);
u8 SCGetSoundMode(void);
u32 SCGetCounterBias(void);
void SCGetBtDeviceInfoArray(SCBtDeviceInfoArray* info);
void SCSetBtDeviceInfoArray(const SCBtDeviceInfoArray* info);
u32 SCGetBtDpdSensibility(void);
u8 SCGetWpadMotorMode(void);
void SCSetWpadMotorMode(u8 mode);
u8 SCGetWpadSensorBarPosition(void);
u8 SCGetWpadSpeakerVolume(void);
void SCSetWpadSpeakerVolume(u8 vol);

#ifdef __cplusplus
}
#endif

#endif
