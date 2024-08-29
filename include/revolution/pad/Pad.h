#ifndef _RVL_SDK_PAD_H
#define _RVL_SDK_PAD_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAD_SPEC_0 0
#define PAD_SPEC_1 1
#define PAD_SPEC_2 2
#define PAD_SPEC_3 3
#define PAD_SPEC_4 4
#define PAD_SPEC_5 5

#define PAD_MOTOR_STOP 0
#define PAD_MOTOR_RUMBLE 1
#define PAD_MOTOR_STOP_HARD 2

#define PAD_CHAN0 0
#define PAD_CHAN1 1
#define PAD_CHAN2 2
#define PAD_CHAN3 3

#define PAD_CHAN0_BIT 0x80000000
#define PAD_CHAN1_BIT 0x40000000
#define PAD_CHAN2_BIT 0x20000000
#define PAD_CHAN3_BIT 0x10000000

#define PAD_MAX_CONTROLLERS 4

#define PAD_BUTTON_LEFT (1 << 0) // 0x0001
#define PAD_BUTTON_RIGHT (1 << 1) // 0x0002
#define PAD_BUTTON_DOWN (1 << 2) // 0x0004
#define PAD_BUTTON_UP (1 << 3) // 0x0008
#define PAD_TRIGGER_Z (1 << 4) // 0x0010
#define PAD_TRIGGER_R (1 << 5) // 0x0020
#define PAD_TRIGGER_L (1 << 6) // 0x0040
#define PAD_BUTTON_A (1 << 8) // 0x0100
#define PAD_BUTTON_B (1 << 9) // 0x0200
#define PAD_BUTTON_X (1 << 10) // 0x0400
#define PAD_BUTTON_Y (1 << 11) // 0x0800
#define PAD_BUTTON_MENU (1 << 12) // 0x1000
#define PAD_BUTTON_START (1 << 12) // 0x1000

#define PAD_ALL                                                                                             \
    (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_DOWN | PAD_BUTTON_UP | PAD_TRIGGER_Z | PAD_TRIGGER_R | \
     PAD_TRIGGER_L | PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_MENU | 0x2000 | 0x0080)

#define PAD_ERR_NONE 0
#define PAD_ERR_NO_CONTROLLER -1
#define PAD_ERR_NOT_READY -2
#define PAD_ERR_TRANSFER -3

typedef enum {
    PAD_FLAG_NO_RECALIBRATE = (1 << 6),
} PADFlag;

typedef void (*PADSamplingCallback)(void);

typedef struct PADStatus {
    /* 0x00 */ u16 button;
    /* 0x02 */ s8 stickX;
    /* 0x03 */ s8 stickY;
    /* 0x04 */ s8 substickX;
    /* 0x05 */ s8 substickY;
    /* 0x06 */ u8 triggerLeft;
    /* 0x07 */ u8 triggerRight;
    /* 0x08 */ u8 analogA;
    /* 0x09 */ u8 analogB;
    /* 0x0A */ s8 err;
} PADStatus; // size = 0xC

extern u32 __PADFixBits;
extern u32 __PADSpec;

bool PADInit(void);
u32 PADRead(PADStatus* status);
bool PADRecalibrate(u32 mask);
bool PADReset(u32 mask);
void PADControlMotor(s32 chan, u32 command);
void PADSetSpec(u32 spec);
void PADClamp(PADStatus* status);
bool __PADDisableRecalibration(bool disable);
PADSamplingCallback PADSetSamplingCallback(PADSamplingCallback callback);

#ifdef __cplusplus
}
#endif

#endif
