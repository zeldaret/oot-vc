#ifndef _RVL_SDK_GX_FIFO_H
#define _RVL_SDK_GX_FIFO_H

#include "revolution/gx/GXInternal.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GXBreakPtCallback)(void);

GX_DECL_PUBLIC_STRUCT(GXFifoObj, 128);

typedef struct __GXFifoObj {
    /* 0x00 */ u8* base;
    /* 0x04 */ u8* top;
    /* 0x08 */ u32 size;
    /* 0x0C */ u32 hiWatermark;
    /* 0x10 */ u32 loWatermark;
    /* 0x14 */ void* rdPtr;
    /* 0x18 */ void* wrPtr;
    /* 0x1C */ s32 count;
    /* 0x20 */ GXBool wrap;
    /* 0x21 */ GXBool bind_cpu;
    /* 0x22 */ GXBool bind_gp;
} __GXFifoObj; // size = 0x24

// Internal struct for FIFO access.
typedef struct _GXFifoObjPriv {
    /* 0x00 */ void* base;
    /* 0x04 */ void* end;
    /* 0x08 */ u32 size;
    /* 0x0C */ u32 highWatermark;
    /* 0x10 */ u32 lowWatermark;
    /* 0x14 */ void* readPtr;
    /* 0x18 */ void* writePtr;
    /* 0x1C */ s32 rwDistance;
    /* 0x20 */ u8 _20[0x60];
} GXFifoObjPriv;

void GXGetGPStatus(u8*, u8*, u8*, u8*, u8*);

void GXSetCPUFifo(GXFifoObj*);
bool GXGetCPUFifo(void);

u32 GXGetFifoCount(GXFifoObj*);
u8 GXGetFifoWrap(GXFifoObj*);

GXBool __GXIsGPFifoReady(void);

#ifdef __cplusplus
}
#endif

#endif
