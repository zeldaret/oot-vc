#ifndef _RVL_SDK_GX_INIT_H
#define _RVL_SDK_GX_INIT_H

#include "revolution/gx/GXFifo.h"
#include "revolution/gx/GXTexture.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GXData {
    /* 0x000 */ u16 vNumNot;
    /* 0x002 */ u16 bpSentNot;
    /* 0x004 */ u16 vNum;
    /* 0x006 */ u16 vLim;
    /* 0x008 */ u32 cpEnable;
    /* 0x00C */ u32 cpStatus;
    /* 0x010 */ u32 cpClr;
    /* 0x014 */ u32 vcdLo;
    /* 0x018 */ u32 vcdHi;
    /* 0x01C */ u32 vatA[GX_MAX_VTXFMT];
    /* 0x03C */ u32 vatB[GX_MAX_VTXFMT];
    /* 0x05C */ u32 vatC[GX_MAX_VTXFMT];
    /* 0x07C */ u32 lpSize;
    /* 0x080 */ u32 matIdxA;
    /* 0x084 */ u32 matIdxB;
    /* 0x088 */ u32 indexBase[4];
    /* 0x098 */ u32 indexStride[4];
    /* 0x0A8 */ u32 ambColor[2];
    /* 0x0B0 */ u32 matColor[2];
    /* 0x0B8 */ u32 chanCtrl[4];
    /* 0x0C8 */ u32 texGenCtrl[GX_MAX_TEXCOORD];
    /* 0x0E8 */ u32 dualTexGenCtrl[GX_MAX_TEXCOORD];
    /* 0x108 */ u32 suTs0[8];
    /* 0x128 */ u32 suTs1[8];
    /* 0x148 */ u32 suScis0;
    /* 0x14C */ u32 suScis1;
    /* 0x150 */ u32 tref[8];
    /* 0x170 */ u32 iref;
    /* 0x174 */ u32 bpMask;
    /* 0x178 */ u32 IndTexScale0;
    /* 0x17C */ u32 IndTexScale1;
    /* 0x180 */ u32 tevc[16];
    /* 0x1C0 */ u32 teva[16];
    /* 0x200 */ u32 tevKsel[8];
    /* 0x220 */ u32 cmode0;
    /* 0x224 */ u32 cmode1;
    /* 0x228 */ u32 zmode;
    /* 0x22C */ u32 peCtrl;
    /* 0x230 */ u32 cpDispSrc;
    /* 0x234 */ u32 cpDispSize;
    /* 0x238 */ u32 cpDispStride;
    /* 0x23C */ u32 cpDisp;
    /* 0x240 */ u32 cpTexSrc;
    /* 0x244 */ u32 cpTexSize;
    /* 0x248 */ u32 cpTexStride;
    /* 0x24C */ u32 cpTex;
    /* 0x250 */ GXBool cpTexZ;
    /* 0x254 */ u32 genMode;
    /* 0x258 */ GXTexRegion TexRegions0[GX_MAX_TEXMAP];
    /* 0x2D8 */ GXTexRegion TexRegions1[GX_MAX_TEXMAP];
    /* 0x358 */ GXTexRegion TexRegions2[GX_MAX_TEXMAP];
    /* 0x3D8 */ GXTlutRegion TlutRegions[GX_MAX_TLUT_ALL];
    /* 0x518 */ GXTexRegionCallback texRegionCallback;
    /* 0x51C */ GXTlutRegionCallback tlutRegionCallback;
    /* 0x520 */ GXAttrType nrmType;
    /* 0x524 */ GXBool hasNrms;
    /* 0x525 */ GXBool hasBiNrms;
    /* 0x528 */ u32 projType;
    /* 0x52C */ f32 projMtx[6];
    union {
        struct {
            /* 0x544 */ f32 vpLeft;
            /* 0x548 */ f32 vpTop;
            /* 0x54C */ f32 vpWd;
            /* 0x550 */ f32 vpHt;
            /* 0x554 */ f32 vpNearz;
            /* 0x558 */ f32 vpFarz;
        };
        f32 view[6];
    };
    /* 0x55C */ f32 zOffset;
    /* 0x560 */ f32 zScale;
    /* 0x564 */ u32 tImage0[8];
    /* 0x584 */ u32 tMode0[8];
    /* 0x5A4 */ u32 texmapId[16];
    /* 0x5E4 */ u32 tcsManEnab;
    /* 0x5E8 */ u32 tevTcEnab;
    /* 0x5EC */ GXPerf0 perf0;
    /* 0x5F0 */ GXPerf1 perf1;
    /* 0x5F4 */ u32 perfSel;
    /* 0x5F8 */ GXBool inDispList;
    /* 0x5F9 */ GXBool dlSaveContext;
    /* 0x5FA */ GXBool abtWaitPECopy;
    /* 0x5FB */ u8 dirtyVAT;
    /* 0x5FC */ u32 dirtyState;
} GXData; // size = 0x600

extern GXData* const __GXData;

// I hate typing this name out
#define gx __GXData

GXFifoObj* GXInit(void*, u32);
void __GXInitGX(void);

#ifdef __cplusplus
}
#endif

#endif
