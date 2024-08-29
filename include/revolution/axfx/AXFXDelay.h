#ifndef _RVL_SDK_AXFX_DELAY_H
#define _RVL_SDK_AXFX_DELAY_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AXFX_DELAY {
    /* 0x0 */ s32* line[3];
    /* 0xC */ u32 curPos[3];
    /* 0x18 */ u32 length[3];
    /* 0x24 */ s32 feedbackGain[3];
    /* 0x30 */ s32 outGain[3];
    /* 0x3C */ u32 active;
    /* 0x40 */ u32 delay[3];
    /* 0x4C */ u32 feedback[3];
    /* 0x58 */ u32 output[3];
} AXFX_DELAY;

u32 AXFXDelayGetMemSize(const AXFX_DELAY* fx);
bool AXFXDelayInit(AXFX_DELAY* fx);
bool AXFXDelaySettings(AXFX_DELAY* fx);
void AXFXDelayShutdown(AXFX_DELAY* fx);
void AXFXDelayCallback(void* chans, void* context);

#ifdef __cplusplus
}
#endif

#endif
