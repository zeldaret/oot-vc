#ifndef _RVL_SDK_VI_H
#define _RVL_SDK_VI_H

#include "revolution/gx/GXFrameBuf.h"
#include "revolution/types.h"
#include "revolution/vi/vitypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*VIRetraceCallback)(u32 retraceCount);
typedef void (*VIPositionCallback)(s16 displayX, s16 displayY);

VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback callback);
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback callback);

void VIInit(void);
void VIWaitForRetrace(void);

void VIConfigure(const GXRenderModeObj* rmo);
void VIConfigurePan(u16 x, u16 y, u16 w, u16 h);
void VIFlush(void);

void VISetNextFrameBuffer(void* fb);
void* VIGetCurrentFrameBuffer(void);

void VISetBlack(bool black);
u32 VIGetRetraceCount(void);

VITvFormat VIGetTvFormat(void);
VIScanMode VIGetScanMode(void);
u32 VIGetDTVStatus(void);
void __VIGetCurrentPosition(s16* x, s16* y);

u32 VIGetDTVStatus(void);

#ifdef __cplusplus
}
#endif

#endif
