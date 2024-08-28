#ifndef _RVL_SDK_OS_CONTEXT_H
#define _RVL_SDK_OS_CONTEXT_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OS_CONTEXT_STATE_FP_SAVED = (1 << 0),
} OSContextState;

typedef struct OSContext {
    /* 0x0 */ u32 gprs[32];
    /* 0x80 */ u32 cr;
    /* 0x84 */ u32 lr;
    /* 0x88 */ u32 ctr;
    /* 0x8C */ u32 xer;
    /* 0x90 */ f64 fprs[32];
    /* 0x190 */ u32 fpscr_pad;
    /* 0x194 */ u32 fpscr;
    /* 0x198 */ u32 srr0;
    /* 0x19C */ u32 srr1;
    /* 0x1A0 */ u16 mode;
    /* 0x1A2 */ u16 state;
    /* 0x1A4 */ u32 gqrs[8];
    /* 0x1C4 */ u32 psf_pad;
    /* 0x1C8 */ f64 psfs[32];
} OSContext;

void OSSaveFPUContext(OSContext* ctx);
void OSSetCurrentContext(OSContext* ctx);
OSContext* OSGetCurrentContext(void);
bool OSSaveContext(OSContext* ctx);
void OSLoadContext(OSContext* ctx);
void* OSGetStackPointer(void);
void OSSwitchFiber(void* func, void* stack);
void OSSwitchFiberEx(u32 r3, u32 r4, u32 r5, u32 r6, void* func, void* stack);
void OSClearContext(OSContext* ctx);
void OSInitContext(OSContext* ctx, void* _srr0, void* stack);
void OSDumpContext(const OSContext* ctx);
void __OSContextInit(void);

#ifdef __cplusplus
}
#endif

#endif
