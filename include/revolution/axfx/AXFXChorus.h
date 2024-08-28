#ifndef _RVL_SDK_AXFX_CHORUS_H
#define _RVL_SDK_AXFX_CHORUS_H

#include "revolution/axfx/AXFXChorusExp.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AXFX_CHORUS {
    /* 0x0 */ AXFX_CHORUS_EXP exp;
    /* 0xA0 */ u32 baseDelay;
    /* 0xA4 */ u32 variation;
    /* 0xA8 */ u32 period;
} AXFX_CHORUS;

u32 AXFXChorusGetMemSize(const AXFX_CHORUS* fx);
bool AXFXChorusInit(AXFX_CHORUS* fx);
bool AXFXChorusShutdown(AXFX_CHORUS* fx);
bool AXFXChorusSettings(AXFX_CHORUS* fx);
void AXFXChorusCallback(void* chans, void* context);

#ifdef __cplusplus
}
#endif

#endif
