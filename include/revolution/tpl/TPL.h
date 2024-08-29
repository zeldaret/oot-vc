#ifndef _RVL_SDK_TPL_H
#define _RVL_SDK_TPL_H

#include "revolution/gx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TPLHeader {
    /* 0x0 */ u16 height;
    /* 0x2 */ u16 width;
    /* 0x4 */ u32 format;
    /* 0x8 */ char* data;
    /* 0xC */ GXTexWrapMode wrapS;
    /* 0x10 */ GXTexWrapMode wrapT;
    /* 0x14 */ GXTexFilter minFilt;
    /* 0x18 */ GXTexFilter magFilt;
    /* 0x1C */ f32 lodBias;
    /* 0x20 */ u8 edgeLodEnable;
    /* 0x21 */ u8 minLod;
    /* 0x22 */ u8 maxLod;
    /* 0x23 */ u8 unpacked;
} TPLHeader;

typedef struct TPLClutHeader {
    /* 0x0 */ u16 numEntries;
    /* 0x1 */ u8 unpacked;
    /* 0x2 */ u8 padding;
    /* 0x4 */ GXTlutFmt format;
    /* 0x8 */ char* data;
} TPLClutHeader;

typedef struct TPLDescriptor {
    /* 0x0 */ TPLHeader* texHeader;
    /* 0x4 */ TPLClutHeader* clutHeader;
} TPLDescriptor;

typedef struct TPLPalette {
    /* 0x0 */ u32 version;
    /* 0x4 */ u32 numImages;
    /* 0x8 */ TPLDescriptor* descriptors;
} TPLPalette;

void TPLBind(TPLPalette* pal);
TPLDescriptor* TPLGet(TPLPalette* pal, u32 id);

#ifdef __cplusplus
}
#endif

#endif
