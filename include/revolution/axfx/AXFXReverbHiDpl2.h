#ifndef _RVL_SDK_AXFX_REVERB_HI_DPL2_H
#define _RVL_SDK_AXFX_REVERB_HI_DPL2_H

#include "revolution/axfx/AXFXReverbHiExpDpl2.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AXFX_REVERBHI_DPL2 {
    /* 0x0 */ AXFX_REVERBHI_EXP_DPL2 exp;
    /* 0x148 */ f32 coloration;
    /* 0x14C */ f32 mix;
    /* 0x150 */ f32 time;
    /* 0x154 */ f32 damping;
    /* 0x158 */ f32 preDelay;
    /* 0x15C */ f32 crosstalk;
} AXFX_REVERBHI_DPL2;

u32 AXFXReverbHiGetMemSizeDpl2(AXFX_REVERBHI_DPL2* fx);
bool AXFXReverbHiInitDpl2(AXFX_REVERBHI_DPL2* fx);
bool AXFXReverbHiShutdownDpl2(AXFX_REVERBHI_DPL2* fx);
bool AXFXReverbHiSettingsDpl2(AXFX_REVERBHI_DPL2* fx);
void AXFXReverbHiCallbackDpl2(void* chans, void* context);

#ifdef __cplusplus
}
#endif

#endif
