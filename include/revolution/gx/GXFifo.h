#ifndef _RVL_SDK_GX_FIFO_H
#define _RVL_SDK_GX_FIFO_H

#include "revolution/gx/GXInternal.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

GX_DECL_PUBLIC_STRUCT(GXFifoObj, 128);

void GXGetGPStatus(u8*, u8*, u8*, u8*, u8*);

void GXSetCPUFifo(GXFifoObj*);
bool GXGetCPUFifo(GXFifoObj*);

u32 GXGetFifoCount(GXFifoObj*);
u8 GXGetFifoWrap(GXFifoObj*);

#ifdef __cplusplus
}
#endif

#endif
