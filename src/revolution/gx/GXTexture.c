#include "intrinsics.h"
#include "revolution/gx.h"
#include "revolution/gx/GXRegs.h"
#include "revolution/os.h"
#include "string.h"

static u8 GXTexMode0Ids[8] = {0x80, 0x81, 0x82, 0x83, 0xA0, 0xA1, 0xA2, 0xA3};
static u8 GXTexMode1Ids[8] = {0x84, 0x85, 0x86, 0x87, 0xA4, 0xA5, 0xA6, 0xA7};
static u8 GXTexImage0Ids[8] = {0x88, 0x89, 0x8A, 0x8B, 0xA8, 0xA9, 0xAa, 0xAb};
static u8 GXTexImage1Ids[8] = {0x8C, 0x8D, 0x8E, 0x8F, 0xAc, 0xAd, 0xAe, 0xAf};
static u8 GXTexImage2Ids[8] = {0x90, 0x91, 0x92, 0x93, 0xB0, 0xB1, 0xB2, 0xB3};
static u8 GXTexImage3Ids[8] = {0x94, 0x95, 0x96, 0x97, 0xB4, 0xB5, 0xB6, 0xB7};
static u8 GXTexTlutIds[8] = {0x98, 0x99, 0x9A, 0x9B, 0xB8, 0xB9, 0xBa, 0xBb};

static u8 GX2HWFiltConv[8] = {0x00, 0x04, 0x01, 0x05, 0x02, 0x06, 0, 0};

#define GET_TILE_COUNT(a, b) (((a) + (1 << (b)) - 1) >> (b))

static inline void __GXGetTexTileShift(GXTexFmt format, u32* widthTiles, u32* heightTiles) {
    switch (format) {
        case GX_TF_I4:
        case 0x8:
        case GX_TF_CMPR:
        case GX_CTF_R4:
        case GX_CTF_Z4:
            *widthTiles = 3;
            *heightTiles = 3;
            break;
        case GX_TF_I8:
        case GX_TF_IA4:
        case 0x9:
        case GX_TF_Z8:
        case GX_CTF_RA4:
        case GX_TF_A8:
        case GX_CTF_R8:
        case GX_CTF_G8:
        case GX_CTF_B8:
        case GX_CTF_Z8M:
        case GX_CTF_Z8L:
            *widthTiles = 3;
            *heightTiles = 2;
            break;
        case GX_TF_IA8:
        case GX_TF_RGB565:
        case GX_TF_RGB5A3:
        case GX_TF_RGBA8:
        case 0xA:
        case GX_TF_Z16:
        case GX_TF_Z24X8:
        case GX_CTF_RA8:
        case GX_CTF_RG8:
        case GX_CTF_GB8:
        case GX_CTF_Z16L:
            *widthTiles = 2;
            *heightTiles = 2;
            break;
        default:
            *widthTiles = *heightTiles = 0;
            break;
    }
}

void __GetImageTileCount(GXTexFmt format, u16 width, u16 height, u32* a, u32* b, u32* c) {
    u32 widthTiles, heightTiles;

    __GXGetTexTileShift(format, &widthTiles, &heightTiles);

    if (width <= 0) {
        width = 1;
    }

    if (height <= 0) {
        height = 1;
    }

    *a = GET_TILE_COUNT(width, widthTiles);
    *b = GET_TILE_COUNT(height, heightTiles);
    *c = (format == GX_TF_RGBA8 || format == GX_TF_Z24X8) ? 2 : 1;
}

void GXInitTexObj(GXTexObj* obj, void* imagePtr, u16 width, u16 height, GXTexFmt format, GXTexWrapMode sWrap,
                  GXTexWrapMode tWrap, GXBool useMIPmap) {
    u32 imageBase;
    u16 a, b;
    u32 c, d;

    GXTexObjPriv* internal = (GXTexObjPriv*)obj;
    memset(internal, 0, sizeof(*internal));

    GX_SET_REG(internal->mode0, sWrap, 30, 31);
    GX_SET_REG(internal->mode0, tWrap, 28, 29);
    GX_SET_REG(internal->mode0, GX_TRUE, 27, 27);

    if (useMIPmap) {
        u32 maxDimSize;
        internal->flags |= 1;
        if (format == 8 || format == 9 || format == 10) {
            GX_SET_REG(internal->mode0, 5, 24, 26);
        } else {
            GX_SET_REG(internal->mode0, 6, 24, 26);
        }

        maxDimSize = width > height ? 31 - __cntlzw(width) : 31 - __cntlzw(height);

        GX_SET_REG(internal->mode1, (maxDimSize) * 16.f, 16, 23);
    } else {
        GX_SET_REG(internal->mode0, 4, 24, 26);
    }

    internal->format = format;
    GX_SET_REG(internal->image0, width - 1, 22, 31);
    GX_SET_REG(internal->image0, height - 1, 12, 21);

    GX_SET_REG(internal->image0, format & 0xF, 8, 11);
    imageBase = (u32)imagePtr >> 5;
    GX_SET_REG(internal->image3, imageBase, 8, 31);

    switch (format & 0xF) {
        case 0:
        case 8:
            internal->loadFormat = 1;
            a = 3;
            b = 3;
            break;
        case 1:
        case 2:
        case 9:
            internal->loadFormat = 2;
            a = 3;
            b = 2;
            break;
        case 3:
        case 4:
        case 5:
        case 10:
            internal->loadFormat = 2;
            a = 2;
            b = 2;
            break;
        case 6:
            internal->loadFormat = 3;
            a = 2;
            b = 2;
            break;
        case 0xE:
            internal->loadFormat = 0;
            a = 3;
            b = 3;
            break;
        default:
            internal->loadFormat = 2;
            a = 2;
            b = 2;
            break;
    }

    internal->loadCount = (GET_TILE_COUNT(width, a) * GET_TILE_COUNT(height, b)) & 0x7FFF;

    internal->flags |= 2;
}

void GXInitTexObjCI(GXTexObj* obj, void* imagePtr, u16 width, u16 height, GXCITexFmt format, GXTexWrapMode sWrap,
                    GXTexWrapMode tWrap, GXBool useMIPmap, u32 tlutName) {
    GXTexObjPriv* internal = (GXTexObjPriv*)obj;

    GXInitTexObj(obj, imagePtr, width, height, format, sWrap, tWrap, useMIPmap);

    internal->flags &= ~2;
    internal->tlutName = tlutName;
}

void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter minFilter, GXTexFilter maxFilter, f32 minLOD, f32 maxLOD, f32 lodBias,
                     GXBool doBiasClamp, GXBool doEdgeLOD, GXAnisotropy maxAniso) {
    GXTexObjPriv* internal = (GXTexObjPriv*)obj;
    u8 reg1;
    u8 reg2;

    if (lodBias < -4.0f) {
        lodBias = -4.0f;
    } else if (lodBias >= 4.0f) {
        lodBias = 3.99f;
    }

    GX_SET_REG(internal->mode0, (u8)(lodBias * 32.0f), 15, 22);

    GX_SET_REG(internal->mode0, maxFilter == 1 ? 1 : 0, 27, 27);
    GX_SET_REG(internal->mode0, GX2HWFiltConv[minFilter], 24, 26);
    GX_SET_REG(internal->mode0, doEdgeLOD ? 0 : 1, 23, 23);
    GX_SET_REG(internal->mode0, 0, 14, 14);
    GX_SET_REG(internal->mode0, 0, 13, 13);
    GX_SET_REG(internal->mode0, maxAniso, 11, 12);
    GX_SET_REG(internal->mode0, doBiasClamp, 10, 10);

    if (minLOD < 0.0f) {
        minLOD = 0.0f;
    } else if (minLOD > 10.0f) {
        minLOD = 10.0f;
    }
    reg1 = minLOD * 16.0f;

    if (maxLOD < 0.0f) {
        maxLOD = 0.0f;
    } else if (maxLOD > 10.0f) {
        maxLOD = 10.0f;
    }
    reg2 = maxLOD * 16.0f;

    GX_SET_REG(internal->mode1, reg1, 24, 31);
    GX_SET_REG(internal->mode1, reg2, 16, 23);
}

void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode sm, GXTexWrapMode tm) {
    GXTexObjPriv* t = (GXTexObjPriv*)obj;
    SET_REG_FIELD(t->mode0, 2, 0, sm);
    SET_REG_FIELD(t->mode0, 2, 2, tm);
}

void GXInitTexObjTlut(GXTexObj* obj, u32 tlut_name) {
    GXTexObjPriv* t = (GXTexObjPriv*)obj;
    t->tlutName = tlut_name;
}

void GXInitTexObjFilter(GXTexObj* obj, GXTexFilter min_filt, GXTexFilter mag_filt) {
    GXTexObjPriv* t = (GXTexObjPriv*)obj;
    SET_REG_FIELD(t->mode0, 1, 4, mag_filt == 1 ? 1 : 0);
    SET_REG_FIELD(t->mode0, 3, 5, GX2HWFiltConv[min_filt]);
}

void GXInitTexObjUserData(GXTexObj* obj, void* user_data) {
    GXTexObjPriv* t = (GXTexObjPriv*)obj;
    t->userData = user_data;
}

void* GXGetTexObjUserData(const GXTexObj* obj) {
    const GXTexObjPriv* t = (const GXTexObjPriv*)obj;
    return t->userData;
}

u16 GXGetTexObjWidth(const GXTexObj* to) {
    const GXTexObjPriv* t = (const GXTexObjPriv*)to;
    return (u32)GET_REG_FIELD(t->image0, 10, 0) + 1;
}

u16 GXGetTexObjHeight(const GXTexObj* to) {
    const GXTexObjPriv* t = (const GXTexObjPriv*)to;
    return (u32)GET_REG_FIELD(t->image0, 10, 10) + 1;
}

GXTexFmt GXGetTexObjFmt(GXTexObj* obj) {
    GXTexObjPriv* pObj = (GXTexObjPriv*)obj;
    return pObj->format;
}

GXTexWrapMode GXGetTexObjWrapS(const GXTexObj* to) {
    const GXTexObjPriv* t = (const GXTexObjPriv*)to;
    return GET_REG_FIELD(t->mode0, 2, 0);
}

GXTexWrapMode GXGetTexObjWrapT(const GXTexObj* to) {
    const GXTexObjPriv* t = (const GXTexObjPriv*)to;
    return GET_REG_FIELD(t->mode0, 2, 2);
}

GXBool GXGetTexObjMipMap(GXTexObj* obj) {
    GXTexObjPriv* internal = (GXTexObjPriv*)obj;
    return (internal->flags & 1) == 1;
}

void GXLoadTexObjPreLoaded(GXTexObj* obj, GXTexRegion* region, GXTexMapID id) {
    GXTlutRegionPriv* tlr;
    u32 m0;
    u32 m1;
    u32 img0;
    u32 img1;
    u32 img2;
    u32 img3;
    GXTexObjPriv* t = (GXTexObjPriv*)obj;
    GXTexRegionPriv* r = (GXTexRegionPriv*)region;
    u32 flags;

    m0 = t->mode0;
    m1 = t->mode1;
    img0 = t->image0;
    img1 = r->image1;
    img2 = r->image2;
    img3 = t->image3;

    // SET_REG_FIELD(t->mode0, 8, 24, GXTexMode0Ids[id]);
    // SET_REG_FIELD(t->mode1, 8, 24, GXTexMode1Ids[id]);
    SET_REG_FIELD(t->image0, 8, 24, GXTexImage0Ids[id]);
    SET_REG_FIELD(r->image1, 8, 24, GXTexImage1Ids[id]);
    SET_REG_FIELD(r->image2, 8, 24, GXTexImage2Ids[id]);
    SET_REG_FIELD(t->image3, 8, 24, GXTexImage3Ids[id]);

    flags = t->flags & 2;

    // GX_WRITE_RAS_REG(t->mode0);
    // GX_WRITE_RAS_REG(t->mode1);
    GX_WRITE_RAS_REG(t->image0);
    GX_WRITE_RAS_REG(r->image1);
    GX_WRITE_RAS_REG(r->image2);
    GX_WRITE_RAS_REG(t->image3);

    if (!flags) {
        tlr = (GXTlutRegionPriv*)gx->tlutRegionCallback(t->tlutName);

        SET_REG_FIELD(tlr->tlutObj.tlut, 8, 24, GXTexTlutIds[id]);
        GX_WRITE_RAS_REG(tlr->tlutObj.tlut);
    }

    gx->tImage0[id] = t->image0;
    gx->tMode0[id] = t->mode0;
    gx->dirtyState |= 1;
    gx->bpSentNot = 0;
}

void GXLoadTexObj(GXTexObj* obj, GXTexMapID map) {
    GXTexRegion* ret = (GXTexRegion*)gx->texRegionCallback(obj, map);
    GXLoadTexObjPreLoaded(obj, ret, map);
}

void GXInitTlutObj(GXTlutObj* obj, void* table, GXTlutFmt format, u16 numEntries) {
    GXTlutObjPriv* internal = (GXTlutObjPriv*)obj;

    internal->tlut = 0;

    GX_SET_REG(internal->tlut, format, 20, 21);
    GX_SET_REG(internal->loadTlut0, ((u32)table & 0x3FFFFFFF) >> 5, 8, 31);
    GX_SET_REG(internal->loadTlut0, 100, 0, 7);

    internal->numEntries = numEntries;
}

void GXLoadTlut(GXTlutObj* obj, u32 tlutName) {
    GXTlutObjPriv* internal = (GXTlutObjPriv*)obj;
    GXTlutRegionPriv* ret = (GXTlutRegionPriv*)gx->tlutRegionCallback(tlutName);
    u32 reg;

    __GXFlushTextureState();

    GX_BP_LOAD_REG(internal->loadTlut0);
    GX_BP_LOAD_REG(ret->loadTlut1);

    __GXFlushTextureState();

    reg = ret->loadTlut1 & 0x3FF;
    GX_SET_REG(internal->tlut, reg, 22, 31);

    ret->tlutObj = *internal;
}

void GXInitTexCacheRegion(GXTexRegion* region, GXBool is32bMIPmap, u32 memEven, GXTexCacheSize sizeEven, u32 memOdd,
                          GXTexCacheSize sizeOdd) {
    GXTexRegionPriv* internal = (GXTexRegionPriv*)region;

    u32 reg;
    switch (sizeEven) {
        case 0:
            reg = 3;
            break;
        case 1:
            reg = 4;
            break;
        case 2:
            reg = 5;
            break;
    }

    internal->image1 = 0;

    GX_SET_REG(internal->image1, memEven >> 5, 17, 31);
    GX_SET_REG(internal->image1, reg, 14, 16);
    GX_SET_REG(internal->image1, reg, 11, 13);
    GX_SET_REG(internal->image1, 0, 10, 10);

    switch (sizeOdd) {
        case 0:
            reg = 3;
            break;
        case 1:
            reg = 4;
            break;
        case 2:
            reg = 5;
            break;
        case 3:
            reg = 0;
            break;
    }

    internal->image2 = 0;
    GX_SET_REG(internal->image2, memOdd >> 5, 17, 31);
    GX_SET_REG(internal->image2, reg, 14, 16);
    GX_SET_REG(internal->image2, reg, 11, 13);

    internal->unkC = is32bMIPmap;
    internal->unkD = 1;
}

void GXInitTlutRegion(GXTlutRegion* region, u32 memAddr, GXTlutSize tlutSize) {
    GXTlutRegionPriv* internal = (GXTlutRegionPriv*)region;

    internal->loadTlut1 = 0;
    GX_SET_REG(internal->loadTlut1, (memAddr - 0x80000) >> 9, 22, 31);
    GX_SET_REG(internal->loadTlut1, tlutSize, 11, 21);
    GX_SET_REG(internal->loadTlut1, 0x65, 0, 7);
}

void GXInvalidateTexAll(void) {
    __GXFlushTextureState();
    GX_BP_LOAD_REG(0x66001000);
    GX_BP_LOAD_REG(0x66001100);
    __GXFlushTextureState();
}

GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback func) {
    GXTexRegionCallback oldFunc = gx->texRegionCallback;

    gx->texRegionCallback = func;

    return oldFunc;
}

GXTlutRegionCallback GXSetTlutRegionCallback(GXTlutRegionCallback func) {
    GXTlutRegionCallback oldFunc = gx->tlutRegionCallback;

    gx->tlutRegionCallback = func;

    return oldFunc;
}

void __SetSURegs(u32 texImgIndex, u32 setUpRegIndex) {
    u32 a1;
    u32 a2;
    GXBool b;
    GXBool c;

    a1 = GX_GET_REG(gx->tImage0[texImgIndex], 22, 31);
    a2 = (gx->tImage0[texImgIndex] & (0x3FF << 10)) >> 10;

    GX_SET_REG(gx->suTs0[setUpRegIndex], a1, 16, 31);
    GX_SET_REG(gx->suTs1[setUpRegIndex], a2, 16, 31);

    b = GX_GET_REG(gx->tMode0[texImgIndex], 30, 31) == 1;
    c = GX_GET_REG(gx->tMode0[texImgIndex], 28, 29) == 1;

    GX_SET_REG(gx->suTs0[setUpRegIndex], b, 15, 15);
    GX_SET_REG(gx->suTs1[setUpRegIndex], c, 15, 15);

    GX_BP_LOAD_REG(gx->suTs0[setUpRegIndex]);
    GX_BP_LOAD_REG(gx->suTs1[setUpRegIndex]);

    gx->bpSentNot = GX_FALSE;
}

#pragma dont_inline on
void __GXSetSUTexRegs(void) {
    u32 b;
    u32 i;
    u32 a;
    u32 c;
    u32 d;
    if (gx->tcsManEnab != 0xFf) {
        a = GX_GET_REG(gx->genMode, 18, 21) + 1;
        b = GX_GET_REG(gx->genMode, 13, 15);
        for (i = 0; i < b; i++) {
            switch (i) {
                case 0:
                    c = GX_GET_REG(gx->iref, 29, 31);
                    d = GX_GET_REG(gx->iref, 26, 28);
                    break;
                case 1:
                    c = GX_GET_REG(gx->iref, 23, 25);
                    d = GX_GET_REG(gx->iref, 20, 22);
                    break;
                case 2:
                    c = GX_GET_REG(gx->iref, 17, 19);
                    d = GX_GET_REG(gx->iref, 14, 16);
                    break;
                case 3:
                    c = GX_GET_REG(gx->iref, 11, 13);
                    d = GX_GET_REG(gx->iref, 8, 10);
                    break;
            }

            if (!(gx->tcsManEnab & (1 << d))) {
                __SetSURegs(c, d);
            }
        }

        for (i = 0; i < a; i++) {
            u32* g = &gx->tref[i / 2];

            c = gx->texmapId[i] & ~0x100;

            if (i & 1) {
                d = GX_GET_REG(*g, 14, 16);
            } else {
                d = GX_GET_REG(*g, 26, 28);
            }

            if (c != 0xFf && !(gx->tcsManEnab & (1 << d)) && gx->tevTcEnab & (1 << i)) {
                __SetSURegs(c, d);
            }
        }
    }
}
#pragma dont_inline reset

void __GXSetTmemConfig(u32 config) {
    switch (config) {
        case 2:
            GX_BP_LOAD_REG(0x8C0d8000);
            GX_BP_LOAD_REG(0x900DC000);

            GX_BP_LOAD_REG(0x8D0d8800);
            GX_BP_LOAD_REG(0x910DC800);

            GX_BP_LOAD_REG(0x8E0d9000);
            GX_BP_LOAD_REG(0x920DD000);

            GX_BP_LOAD_REG(0x8F0d9800);
            GX_BP_LOAD_REG(0x930DD800);

            GX_BP_LOAD_REG(0xAC0DA000);
            GX_BP_LOAD_REG(0xB00DC400);

            GX_BP_LOAD_REG(0xAD0DA800);
            GX_BP_LOAD_REG(0xB10DCC00);

            GX_BP_LOAD_REG(0xAE0DB000);
            GX_BP_LOAD_REG(0xB20DD400);

            GX_BP_LOAD_REG(0xAF0DB800);
            GX_BP_LOAD_REG(0xB30DDC00);
            break;
        case 1:
            GX_BP_LOAD_REG(0x8C0d8000);
            GX_BP_LOAD_REG(0x900DC000);

            GX_BP_LOAD_REG(0x8D0d8800);
            GX_BP_LOAD_REG(0x910DC800);

            GX_BP_LOAD_REG(0x8E0d9000);
            GX_BP_LOAD_REG(0x920DD000);

            GX_BP_LOAD_REG(0x8F0d9800);
            GX_BP_LOAD_REG(0x930DD800);

            GX_BP_LOAD_REG(0xAC0DA000);
            GX_BP_LOAD_REG(0xB00DE000);

            GX_BP_LOAD_REG(0xAD0DA800);
            GX_BP_LOAD_REG(0xB10DE800);

            GX_BP_LOAD_REG(0xAE0DB000);
            GX_BP_LOAD_REG(0xB20DF000);

            GX_BP_LOAD_REG(0xAF0DB800);
            GX_BP_LOAD_REG(0xB30DF800);

            break;
        case 0:
        default:
            GX_BP_LOAD_REG(0x8C0d8000);
            GX_BP_LOAD_REG(0x900DC000);

            GX_BP_LOAD_REG(0x8D0d8400);
            GX_BP_LOAD_REG(0x910DC400);

            GX_BP_LOAD_REG(0x8E0d8800);
            GX_BP_LOAD_REG(0x920DC800);

            GX_BP_LOAD_REG(0x8F0d8c00);
            GX_BP_LOAD_REG(0x930DCC00);

            GX_BP_LOAD_REG(0xAC0D9000);
            GX_BP_LOAD_REG(0xB00DD000);

            GX_BP_LOAD_REG(0xAD0D9400);
            GX_BP_LOAD_REG(0xB10DD400);

            GX_BP_LOAD_REG(0xAE0D9800);
            GX_BP_LOAD_REG(0xB20DD800);

            GX_BP_LOAD_REG(0xAF0D9C00);
            GX_BP_LOAD_REG(0xB30DDC00);

            break;
    }
}
