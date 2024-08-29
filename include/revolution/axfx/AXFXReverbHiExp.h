#ifndef _RVL_SDK_AXFX_REVERB_HI_EXP_H
#define _RVL_SDK_AXFX_REVERB_HI_EXP_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct AXFX_BUS;
typedef struct AXFX_BUFFERUPDATE;

typedef struct AXFX_REVERBHI_EXP {
    /* 0x0 */ f32* earlyLine[3];
    /* 0xC */ u32 earlyPos[3];
    /* 0x18 */ u32 earlyLength;
    /* 0x1C */ u32 earlyMaxLength;
    /* 0x20 */ f32 earlyCoef[3];

    /* 0x2C */ f32* preDelayLine[3];
    /* 0x38 */ u32 preDelayPos;
    /* 0x3C */ u32 preDelayLength;
    /* 0x40 */ u32 preDelayMaxLength;

    /* 0x44 */ f32* combLine[3][3];
    /* 0x68 */ u32 combPos[3];
    /* 0x74 */ u32 combLength[3];
    /* 0x80 */ u32 combMaxLength[3];
    /* 0x8C */ f32 combCoef[3];

    /* 0x98 */ f32* allpassLine[3][2];
    /* 0xB0 */ u32 allpassPos[2];
    /* 0xB8 */ u32 allpassLength[2];
    /* 0xC0 */ u32 allpassMaxLength[2];

    /* 0xC8 */ f32* lastAllpassLine[3];
    /* 0xD4 */ u32 lastAllpassPos[3];
    /* 0xE0 */ u32 lastAllpassLength[3];
    /* 0xEC */ u32 lastAllpassMaxLength[3];

    /* 0xF8 */ f32 allpassCoef;
    /* 0xFC */ f32 lastLpfOut[3];
    /* 0x108 */ f32 lpfCoef;
    /* 0x10C */ u32 active;
    /* 0x110 */ u32 earlyMode;
    /* 0x114 */ f32 preDelayTimeMax;
    /* 0x118 */ f32 preDelayTime;
    /* 0x11C */ u32 fusedMode;
    /* 0x120 */ f32 fusedTime;
    /* 0x124 */ f32 coloration;
    /* 0x128 */ f32 damping;
    /* 0x12C */ f32 crosstalk;
    /* 0x130 */ f32 earlyGain;
    /* 0x134 */ f32 fusedGain;
    /* 0x138 */ struct AXFX_BUS* busIn;
    /* 0x13C */ struct AXFX_BUS* busOut;
    /* 0x140 */ f32 outGain;
    /* 0x144 */ f32 sendGain;
} AXFX_REVERBHI_EXP;

u32 AXFXReverbHiExpGetMemSize(const AXFX_REVERBHI_EXP* fx);
bool AXFXReverbHiExpInit(AXFX_REVERBHI_EXP* fx);
void AXFXReverbHiExpShutdown(AXFX_REVERBHI_EXP* fx);
bool AXFXReverbHiExpSettings(AXFX_REVERBHI_EXP* fx);
void AXFXReverbHiExpCallback(struct AXFX_BUFFERUPDATE* update, AXFX_REVERBHI_EXP* fx);

#ifdef __cplusplus
}
#endif

#endif
