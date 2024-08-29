#ifndef _RVL_SDK_AXFX_REVERB_HI_EXP_DPL2_H
#define _RVL_SDK_AXFX_REVERB_HI_EXP_DPL2_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AXFX_REVERBHI_EXP_DPL2 {
    /* 0x0 */ f32* earlyLine[4];
    /* 0x10 */ u32 earlyPos[3];
    /* 0x1C */ u32 earlyLength;
    /* 0x20 */ u32 earlyMaxLength;
    /* 0x24 */ f32 earlyCoef[3];

    /* 0x30 */ f32* preDelayLine[4];
    /* 0x40 */ u32 preDelayPos;
    /* 0x44 */ u32 preDelayLength;
    /* 0x48 */ u32 preDelayMaxLength;

    /* 0x4C */ f32* combLine[4][3];
    /* 0x7C */ u32 combPos[3];
    /* 0x88 */ u32 combLength[3];
    /* 0x94 */ u32 combMaxLength[3];
    /* 0xA0 */ f32 combCoef[3];

    /* 0xAC */ f32* allpassLine[4][2];
    /* 0xCC */ u32 allpassPos[2];
    /* 0xD4 */ u32 allpassLength[2];
    /* 0xDC */ u32 allpassMaxLength[2];

    /* 0xE4 */ f32* lastAllpassLine[4];
    /* 0xF4 */ u32 lastAllpassPos[4];
    /* 0x104 */ u32 lastAllpassLength[4];
    /* 0x114 */ u32 lastAllpassMaxLength[4];

    /* 0x124 */ f32 allpassCoef;
    /* 0x128 */ f32 lastLpfOut[4];
    /* 0x138 */ f32 lpfCoef;
    /* 0x13C */ u32 active;
    /* 0x140 */ u32 earlyMode;
    /* 0x144 */ f32 preDelayTimeMax;
    /* 0x148 */ f32 preDelayTime;
    /* 0x14C */ u32 fusedMode;
    /* 0x150 */ f32 fusedTime;
    /* 0x154 */ f32 coloration;
    /* 0x158 */ f32 damping;
    /* 0x15C */ f32 crosstalk;
    /* 0x160 */ f32 earlyGain;
    /* 0x164 */ f32 fusedGain;
    /* 0x168 */ struct AXFX_BUS* busIn;
    /* 0x16C */ struct AXFX_BUS* busOut;
    /* 0x170 */ f32 outGain;
    /* 0x174 */ f32 sendGain;
} AXFX_REVERBHI_EXP_DPL2;

u32 AXFXReverbHiExpGetMemSizeDpl2(const AXFX_REVERBHI_EXP_DPL2* fx);
bool AXFXReverbHiExpInitDpl2(AXFX_REVERBHI_EXP_DPL2* fx);
void AXFXReverbHiExpShutdownDpl2(AXFX_REVERBHI_EXP_DPL2* fx);
bool AXFXReverbHiExpSettingsDpl2(AXFX_REVERBHI_EXP_DPL2* fx);
void AXFXReverbHiExpCallbackDpl2(struct AXFX_BUFFERUPDATE_DPL2* update, AXFX_REVERBHI_EXP_DPL2* fx);

#ifdef __cplusplus
}
#endif

#endif
