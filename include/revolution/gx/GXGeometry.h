#ifndef _RVL_SDK_GX_GEOMETRY_H
#define _RVL_SDK_GX_GEOMETRY_H

#include "revolution/gx/GXTypes.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void GXBegin(GXPrimitive prim, GXVtxFmt fmt, u16 verts);
static inline void GXEnd(void) {}

void GXSetLineWidth(u8 width, u32 offset);
void GXSetPointSize(u8 size, u32 offset);
void GXEnableTexOffsets(GXTexCoordID coordId, GXBool lineOfs, GXBool pointOfs);
void GXSetCullMode(GXCullMode mode);
void GXGetCullMode(GXCullMode* out);
void GXSetCoPlanar(GXBool coplanar);

void __GXSetDirtyState(void);
void __GXSendFlushPrim(void);
void __GXSetGenMode(void);

#ifdef __cplusplus
}
#endif

#endif
