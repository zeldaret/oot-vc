#ifndef _RVL_SDK_GX_FRAMEBUF_H
#define _RVL_SDK_GX_FRAMEBUF_H

#include "revolution/types.h"
#include "revolution/vi/vitypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VIXFBMode;

typedef struct _GXRenderModeObj {
    /* 0x0 */ VITvFormat viTVmode;
    /* 0x4 */ u16 fbWidth;
    /* 0x6 */ u16 efbHeight;
    /* 0x8 */ u16 xfbHeight;
    /* 0xA */ u16 viXOrigin;
    /* 0xC */ u16 viYOrigin;
    /* 0xE */ u16 viWidth;
    /* 0x10 */ u16 viHeight;
    /* 0x14 */ VIXFBMode xfbMode;
    /* 0x18 */ u8 field_rendering;
    /* 0x19 */ u8 aa;
    /* 0x1A */ u8 sample_pattern[12][2];
    /* 0x32 */ u8 vfilter[7];
} GXRenderModeObj;

extern GXRenderModeObj GXNtsc480IntDf;
extern GXRenderModeObj GXNtsc480Prog;
extern GXRenderModeObj GXPal528IntDf;
extern GXRenderModeObj GXEurgb60Hz480IntDf;
extern GXRenderModeObj GXMpal480IntDf;

void GXSetTexCopySrc(u16 x, u16 y, u16 w, u16 h);
void GXSetTexCopyDst(u16 w, u16 h, GXTexFmt fmt, GXBool mipmap);

void GXSetCopyClamp(GXCopyClamp clamp);

void GXSetCopyClear(GXColor color, u32 z);
void GXSetCopyFilter(GXBool, u8 sample_pattern[12][2], GXBool, u8 vfilter[7]);

void GXCopyDisp(void*, GXBool);
void GXCopyTex(void*, GXBool);

u16 GXGetNumXfbLines(const u16 efbHeight, f32 yScale);

#ifdef __cplusplus
}
#endif

#endif
