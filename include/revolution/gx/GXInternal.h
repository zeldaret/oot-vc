#ifndef _RVL_SDK_GX_INTERNAL_H
#define _RVL_SDK_GX_INTERNAL_H

#include "revolution/gx/GXTypes.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GX internal structures.
 *
 * Wouldn't be necessary if the public ones didn't include padding; but they do,
 * so there has to be different structure definitions.
 *
 * These internal structures are implemented like the RFL ones since we don't
 * have DWARF info for most GX structures.
 */

/**
 * Declare a public structure from the corresponding internal structure.
 * (Implementation size is included to require that such a structure already
 * exists.)
 */
#define GX_DECL_PUBLIC_STRUCT(name, size)                           \
    typedef struct _##name {                                        \
        u8 dummy[(size) - sizeof(name##Impl) + sizeof(name##Impl)]; \
    } name;

typedef struct _GXFifoObjImpl {
    /* 0x00 */ void* base;
    /* 0x04 */ void* end;
    /* 0x08 */ u32 size;
    /* 0x0C */ u32 highWatermark;
    /* 0x10 */ u32 lowWatermark;
    /* 0x14 */ void* readPtr;
    /* 0x18 */ void* writePtr;
    /* 0x1C */ s32 rwDistance;
    /* 0x20 */ GXBool wrap;
    /* 0x21 */ GXBool bindCPU;
    /* 0x22 */ GXBool bindGP;
} GXFifoObjImpl;

typedef struct _GXLightObjImpl {
    char UNK_0x0[0xC];
    /* 0xC */ GXColor color;
    /* 0x10 */ f32 aa;
    /* 0x14 */ f32 ab;
    /* 0x18 */ f32 ac;
    /* 0x1C */ f32 ka;
    /* 0x20 */ f32 kb;
    /* 0x24 */ f32 kc;
    /* 0x28 */ f32 posX;
    /* 0x2C */ f32 posY;
    /* 0x30 */ f32 posZ;
    /* 0x34 */ f32 dirX;
    /* 0x38 */ f32 dirY;
    /* 0x3C */ f32 dirZ;
} GXLightObjImpl;

typedef struct _GXTexObjImpl {
    u8 todo;
} GXTexObjImpl;

typedef struct _GXTlutObjImpl {
    u8 todo;
} GXTlutObjImpl;

#ifdef __cplusplus
}
#endif

#endif
