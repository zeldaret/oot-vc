#ifndef _RVL_SDK_SC_SCAPI_H
#define _RVL_SDK_SC_SCAPI_H

#include "revolution/bte.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SC_MAX_DEV_ENTRY_FOR_STD 10
#define SC_MAX_DEV_ENTRY_FOR_SMP 6
#define SC_MAX_DEV_ENTRY (SC_MAX_DEV_ENTRY_FOR_STD + SC_MAX_DEV_ENTRY_FOR_SMP)

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
    SC_MOTOR_OFF = 0,
    SC_MOTOR_ON = 1
} SCMotorMode;

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
    u8 wc24; // at 0x0
    u8 slotLight; // at 0x1
} SCIdleModeInfo;

typedef struct SCDevInfo {
    char devName[20]; // at 0x0
    char at_0x14[1];
    char UNK_0x15[0xB];
    LINK_KEY linkKey; // at 0x20
    char UNK_0x30[0x10];
} SCDevInfo;

typedef struct SCBtDeviceInfo {
    BD_ADDR addr; // at 0x0
    SCDevInfo info; // at 0x6
} SCBtDeviceInfo;

typedef struct SCBtDeviceInfoArray {
    u8 numRegist; // at 0x0
    union {
        struct {
            SCBtDeviceInfo regist[SC_MAX_DEV_ENTRY_FOR_STD]; // at 0x1
            SCBtDeviceInfo active[SC_MAX_DEV_ENTRY_FOR_SMP]; // at 0x2BD
        };

        SCBtDeviceInfo devices[SC_MAX_DEV_ENTRY];
    };
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
