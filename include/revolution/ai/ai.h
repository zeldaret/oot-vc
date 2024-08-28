#ifndef _RVL_SDK_AI_H
#define _RVL_SDK_AI_H

#include "revolution/os/OSContext.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AIDMACallback)(void);

typedef enum {
    AI_DSP_32KHZ,
    AI_DSP_48KHZ,
} AIDSPSampleRate;

AIDMACallback AIRegisterDMACallback(AIDMACallback callback);
void AIInitDMA(void* buffer, u32 length);
void AIStartDMA(void);
u32 AIGetDMABytesLeft(void);
void AISetDSPSampleRate(u32 rate);
u32 AIGetDSPSampleRate(void);
void AIInit(void* stack);
void __AIDHandler(s16 intr, struct OSContext* ctx);

#ifdef __cplusplus
}
#endif

#endif
