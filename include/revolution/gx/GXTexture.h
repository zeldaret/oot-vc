#ifndef _RVL_SDK_GX_TEXTURE_H
#define _RVL_SDK_GX_TEXTURE_H

#include "revolution/gx/GXInternal.h"
#include "revolution/gx/GXTypes.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GXTexObj {
    u32 dummy[8];
} GXTexObj;

typedef struct _GXTlutObj {
    u32 dummy[3];
} GXTlutObj;

// Internal struct for texture objects.
typedef struct _GXTexObjPriv {
    /* 0x00 */ u32 mode0;
    /* 0x04 */ u32 mode1;
    /* 0x08 */ u32 image0;
    /* 0x0C */ u32 image3;
    /* 0x10 */ void* userData;
    /* 0x14 */ GXTexFmt format;
    /* 0x18 */ u32 tlutName;
    /* 0x1C */ u16 loadCount;
    /* 0x1E */ u8 loadFormat;
    /* 0x1F */ u8 flags;
} GXTexObjPriv;

typedef struct _GXTexRegionPriv {
    /* 0x00 */ u32 image1;
    /* 0x04 */ u32 image2;
    /* 0x08 */ u32 unk8;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u8 padding[2];
} GXTexRegionPriv;

typedef struct _GXTlutObjPriv {
    /* 0x00*/ u32 tlut;
    /* 0x04*/ u32 loadTlut0;
    /* 0x08*/ u16 numEntries;
} GXTlutObjPriv;

typedef struct _GXTlutRegionPriv {
    /* 0x00 */ u32 loadTlut1;
    /* 0x04 */ GXTlutObjPriv tlutObj;
} GXTlutRegionPriv;

typedef GXTexRegion* (*GXTexRegionCallback)(GXTexObj* t_obj, GXTexMapID id);
typedef GXTlutRegion* (*GXTlutRegionCallback)(u32 idx);

void __GXSetSUTexRegs(void);

void GXInitTexObj(GXTexObj* obj, void* image, u16 w, u16 h, GXTexFmt fmt, GXTexWrapMode wrap_s, GXTexWrapMode wrap_t,
                  GXBool mipmap);
void GXInitTexObjCI(GXTexObj* obj, void* imagePtr, u16 width, u16 height, GXCITexFmt format, GXTexWrapMode sWrap,
                    GXTexWrapMode tWrap, GXBool useMIPmap, u32 tlutName);
void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter min_filt, GXTexFilter mag_filt, f32 min_lod, f32 max_lod, f32 lod_bias,
                     GXBool bias_clamp, GXBool do_edge_lod, GXAnisotropy max_aniso);
GXTexFmt GXGetTexObjFmt(GXTexObj* obj);
GXBool GXGetTexObjMipMap(GXTexObj* obj);
void GXLoadTexObj(GXTexObj*, GXTexMapID);

u32 GXGetTexObjTlut(GXTexObj*);

void GXInitTlutObj(GXTlutObj*, void*, GXTlutFmt, u16);

void GXInvalidateTexAll(void);

// TODO
UNKTYPE GXSetTexCoordScaleManually(UNKWORD, UNKWORD, UNKWORD, UNKWORD);
UNKTYPE GXSetTexCoordCylWrap(UNKWORD, UNKWORD, UNKWORD);

#ifdef __cplusplus
}
#endif

#endif
