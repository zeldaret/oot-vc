#ifndef _RVL_SDK_GX_ATTR_H
#define _RVL_SDK_GX_ATTR_H

#include "revolution/gx/GXTypes.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GXVtxDescList {
    /* 0x0 */ GXAttr attr;
    /* 0x4 */ GXAttrType type;
} GXVtxDescList;

typedef struct _GXVtxAttrFmtList {
    /* 0x0 */ GXAttr attr;
    /* 0x4 */ GXCompCnt compCnt;
    /* 0x8 */ GXCompType compType;
    /* 0xC */ u8 shift;
} GXVtxAttrFmtList;

void GXSetVtxDesc(GXAttr name, GXAttrType type);
void GXSetVtxDescv(const GXVtxDescList* list);
void GXGetVtxDesc(GXAttr name, GXAttrType* type);
void GXGetVtxDescv(GXVtxDescList* list);
void GXClearVtxDesc(void);
void GXSetVtxAttrFmt(GXVtxFmt fmt, GXAttr attr, GXCompCnt compCnt, GXCompType compType, u8 shift);

// TODO: Please find a way to get rid of this
#ifdef GXATTR_MATCH_HACK
void GXSetVtxAttrFmtv(s16 fmt, const GXVtxAttrFmtList* list);
#else
void GXSetVtxAttrFmtv(GXVtxFmt fmt, const GXVtxAttrFmtList* list);
#endif

void GXGetVtxAttrFmt(GXVtxFmt fmt, GXAttr attr, GXCompCnt* compCnt, GXCompType* compType, u8* shift);
void GXGetVtxAttrFmtv(GXVtxFmt fmt, GXVtxAttrFmtList* list);
void GXSetArray(GXAttr attr, const void* base, u8 stride);
void GXInvalidateVtxCache(void);
void GXSetTexCoordGen2(GXTexCoordID id, GXTexGenType type, GXTexGenSrc src, u32 texMtxIdx, GXBool normalize,
                       u32 dualTexMtxIdx);
void GXSetNumTexGens(u8 num);

void __GXSetVCD(void);
void __GXCalculateVLim(void);
void __GXSetVAT(void);

static inline void GXSetTexCoordGen(GXTexCoordID id, GXTexGenType type, GXTexGenSrc src, u32 texMtxIdx) {
    GXSetTexCoordGen2(id, type, src, texMtxIdx, false, GX_DUALMTX_IDENT);
}

#ifdef __cplusplus
}
#endif

#endif
