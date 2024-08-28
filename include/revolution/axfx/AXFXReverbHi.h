#ifndef _RVL_SDK_AXFX_REVERB_HI_H
#define _RVL_SDK_AXFX_REVERB_HI_H

#include "revolution/axfx/AXFXReverbHiExp.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AXFX_REVERBHI {
    /* 0x0 */ AXFX_REVERBHI_EXP exp;
    /* 0x148 */ f32 coloration;
    /* 0x14C */ f32 mix;
    /* 0x150 */ f32 time;
    /* 0x154 */ f32 damping;
    /* 0x158 */ f32 preDelay;
    /* 0x15C */ f32 crosstalk;
} AXFX_REVERBHI;

u32 AXFXReverbHiGetMemSize(AXFX_REVERBHI* fx);
bool AXFXReverbHiInit(AXFX_REVERBHI* fx);
bool AXFXReverbHiShutdown(AXFX_REVERBHI* fx);
bool AXFXReverbHiSettings(AXFX_REVERBHI* fx);
void AXFXReverbHiCallback(void* chans, void* context);

#ifdef __cplusplus
}
#endif

#endif
