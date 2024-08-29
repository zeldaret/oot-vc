#ifndef _RVL_SDK_AXFX_CHORUS_EXP_H
#define _RVL_SDK_AXFX_CHORUS_EXP_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct AXFX_BUS;
typedef struct AXFX_BUFFERUPDATE;

typedef struct AXFX_CHORUS_EXP_DELAY {
    /* 0x0 */ f32* line[3];
    /* 0xC */ u32 inPos;
    /* 0x10 */ u32 outPos;
    /* 0x14 */ u32 lastPos;
    /* 0x18 */ u32 sizeFP;
    /* 0x1C */ u32 size;
} AXFX_CHORUS_EXP_DELAY;

typedef struct AXFX_CHORUS_EXP_LFO {
    /* 0x0 */ s32* table;
    /* 0x4 */ s32 phaseAdd;
    /* 0x8 */ s32 stepSamp;
    /* 0xC */ s32 depthSamp;
    /* 0x10 */ u32 phase;
    /* 0x14 */ u32 sign;
    /* 0x18 */ u32 lastNum;
    /* 0x1C */ s32 lastValue;
    /* 0x20 */ s32 grad;
    /* 0x24 */ s32 gradFactor;
} AXFX_CHORUS_EXP_LFO;

typedef struct AXFX_CHORUS_EXP {
    /* 0x0 */ AXFX_CHORUS_EXP_DELAY delay;
    /* 0x20 */ AXFX_CHORUS_EXP_LFO lfo;
    /* 0x48 */ f32 history[3][4];
    /* 0x78 */ u32 histIndex;
    /* 0x7C */ u32 active;
    /* 0x80 */ f32 delayTime;
    /* 0x84 */ f32 depth;
    /* 0x88 */ f32 rate;
    /* 0x8C */ f32 feedback;
    /* 0x90 */ struct AXFX_BUS* busIn;
    /* 0x94 */ struct AXFX_BUS* busOut;
    /* 0x98 */ f32 outGain;
    /* 0x9C */ f32 sendGain;
} AXFX_CHORUS_EXP;

u32 AXFXChorusExpGetMemSize(const AXFX_CHORUS_EXP* fx);
bool AXFXChorusExpInit(AXFX_CHORUS_EXP* fx);
void AXFXChorusExpShutdown(AXFX_CHORUS_EXP* fx);
bool AXFXChorusExpSettings(AXFX_CHORUS_EXP* fx);
void AXFXChorusExpCallback(struct AXFX_BUFFERUPDATE* update, AXFX_CHORUS_EXP* fx);

#ifdef __cplusplus
}
#endif

#endif
