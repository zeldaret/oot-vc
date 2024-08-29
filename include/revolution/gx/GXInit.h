#ifndef _RVL_SDK_GX_INIT_H
#define _RVL_SDK_GX_INIT_H

#include "revolution/gx/GXFifo.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GXData {
    /* 0x0 */ union {
        UNKWORD WORD_0x0;
        struct {
            /* 0x0 */ u16 SHORT_0x0;
            /* 0x2 */ u16 lastWriteWasXF;
        };
    };
    u16 SHORT_0x4;
    /* 0x6 */ u16 vlim;
    /* 0x8 */ u32 cpCtrlReg;
    /* 0xC */ u32 cpStatReg;
    char UNK_0x10[0x4];
    /* 0x14 */ u32 vcdLoReg;
    /* 0x18 */ u32 vcdHiReg;
    /* 0x1C */ u32 vatA[GX_MAX_VTXFMT];
    /* 0x3C */ u32 vatB[GX_MAX_VTXFMT];
    /* 0x5C */ u32 vatC[GX_MAX_VTXFMT];
    /* 0x7C */ u32 linePtWidth;
    /* 0x80 */ u32 matrixIndex0;
    /* 0x84 */ u32 matrixIndex1;
    char UNK_0x88[0xA8 - 0x88];
    /* 0xA8 */ GXColor ambColors[2];
    /* 0xB0 */ GXColor matColors[2];
    /* 0xB8 */ u32 colorControl[4];
    /* 0xC8 */ u32 texRegs[GX_MAX_TEXCOORD];
    /* 0xE8 */ u32 dualTexRegs[GX_MAX_TEXCOORD];
    /* 0x108 */ u32 txcRegs[GX_MAX_TEXCOORD];
    char UNK_0x128[0x148 - 0x128];
    /* 0x148 */ u32 scissorTL;
    /* 0x14C */ u32 scissorBR;
    char UNK_0x150[0x170 - 0x150];
    /* 0x170 */ u32 ras1_iref;
    /* 0x174 */ u32 ind_imask;
    /* 0x178 */ u32 ras1_ss0;
    /* 0x17C */ u32 ras1_ss1;
    char UNK_0x180[0x220 - 0x180];
    /* 0x220 */ u32 blendMode;
    /* 0x224 */ u32 dstAlpha;
    /* 0x228 */ u32 zMode;
    /* 0x22C */ u32 zControl;
    char UNK_0x230[0x254 - 0x230];
    /* 0x254 */ u32 genMode;
    char UNK_0x258[0x520 - 0x258];
    /* 0x520 */ GXAttrType normalType;
    /* 0x524 */ GXBool normal;
    /* 0x525 */ GXBool binormal;
    /* 0x528 */ GXProjectionType projType;
    /* 0x52C */ f32 proj[6];
    /* 0x544 */ union {
        struct {
            /* 0x544 */ f32 vpOx;
            /* 0x548 */ f32 vpOy;
            /* 0x54C */ f32 vpSx;
            /* 0x550 */ f32 vpSy;
            /* 0x554 */ f32 vpNear;
            /* 0x558 */ f32 vpFar;
        };
        f32 view[6];
    };
    /* 0x55C */ f32 offsetZ;
    /* 0x560 */ f32 scaleZ;
    char UNK_0x564[0x5F8 - 0x564];
    /* 0x5F8 */ GXBool dlistActive;
    /* 0x5F9 */ GXBool dlistSave;
    u8 BYTE_0x5FA;
    /* 0x5FB */ u8 vatDirtyFlags;
    /* 0x5FC */ u32 gxDirtyFlags;
} GXData;

extern GXData* const __GXData;

// I hate typing this name out
#define gxdt __GXData

GXFifoObj* GXInit(void*, u32);

#ifdef __cplusplus
}
#endif

#endif
