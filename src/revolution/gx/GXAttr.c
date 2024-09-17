#include "revolution/gx.h"

static void SETVCDATTR(GXAttr name, GXAttrType type);
static void SETVAT(u32* vatA, u32* vatB, u32* vatC, GXAttr attr, GXCompCnt compCnt, GXCompType compType, u8 shift);
static u8 GetFracForNrm(GXCompType compType);

static void __GXXfVtxSpecs(void) {
    unsigned int cmd;
    u32 nrm;
    u32 clr;
    u32 txc;

    nrm = gx->hasBiNrms ? 2 : (gx->hasNrms ? 1 : 0);

    // Both fields in one access
    clr = 32 -
          __cntlzw(GX_BITGET(gx->vcdLo, GX_CP_VCD_LO_COLORSPECULAR_ST,
                             GX_CP_VCD_LO_COLORSPECULAR_SZ + GX_CP_VCD_LO_COLORDIFFUSED_SZ)) +
          1;

    // All 16 bits in VCD_Hi
    txc = 32 - __cntlzw(GX_BITGET(gx->vcdHi, GX_CP_VCD_HI_TEX7COORD_ST, sizeof(u16) * 8)) + 1;

    // TODO: Fakematch? Bitset macros don't work here
    GX_XF_LOAD_REG_HDR(GX_XF_REG_INVERTEXSPEC);
    cmd = 0;
    cmd = cmd | clr >> 1;
    cmd = (txc << 3) & ~0xF | cmd;
    WGPIPE.i = cmd | nrm << 2;

    gx->bpSentNot = true;
}

void GXSetVtxDesc(GXAttr name, GXAttrType type) {
    SETVCDATTR(name, type);

    if (gx->hasNrms || gx->hasBiNrms) {
        GX_CP_SET_VCD_LO_NORMAL(gx->vcdLo, gx->nrmType);
    } else {
        GX_CP_SET_VCD_LO_NORMAL(gx->vcdLo, GX_NONE);
    }

    gx->dirtyState |= GX_DIRTY_VCD;
}

static inline void SETVCDATTR(GXAttr name, GXAttrType type) {
    switch (name) {
        case GX_VA_PNMTXIDX:
            GX_CP_SET_VCD_LO_POSMATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX0MTXIDX:
            GX_CP_SET_VCD_LO_TEX0MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX1MTXIDX:
            GX_CP_SET_VCD_LO_TEX1MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX2MTXIDX:
            GX_CP_SET_VCD_LO_TEX2MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX3MTXIDX:
            GX_CP_SET_VCD_LO_TEX3MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX4MTXIDX:
            GX_CP_SET_VCD_LO_TEX4MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX5MTXIDX:
            GX_CP_SET_VCD_LO_TEX5MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX6MTXIDX:
            GX_CP_SET_VCD_LO_TEX6MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_TEX7MTXIDX:
            GX_CP_SET_VCD_LO_TEX7MATIDX(gx->vcdLo, type);
            break;
        case GX_VA_POS:
            GX_CP_SET_VCD_LO_POSITION(gx->vcdLo, type);
            break;
        case GX_VA_NRM:
            if (type != GX_NONE) {
                gx->hasNrms = true;
                // GX_VA_NRM and GX_VA_NBT should not be enabled at the same time
                gx->hasBiNrms = false;
                gx->nrmType = type;
            } else {
                gx->hasNrms = false;
            }
            break;
        case GX_VA_NBT:
            if (type != GX_NONE) {
                gx->hasBiNrms = true;
                // GX_VA_NRM and GX_VA_NBT should not be enabled at the same time
                gx->hasNrms = false;
                gx->nrmType = type;
            } else {
                gx->hasBiNrms = false;
            }
            break;
        case GX_VA_CLR0:
            GX_CP_SET_VCD_LO_COLORDIFFUSED(gx->vcdLo, type);
            break;
        case GX_VA_CLR1:
            GX_CP_SET_VCD_LO_COLORSPECULAR(gx->vcdLo, type);
            break;
        case GX_VA_TEX0:
            GX_CP_SET_VCD_HI_TEX0COORD(gx->vcdHi, type);
            break;
        case GX_VA_TEX1:
            GX_CP_SET_VCD_HI_TEX1COORD(gx->vcdHi, type);
            break;
        case GX_VA_TEX2:
            GX_CP_SET_VCD_HI_TEX2COORD(gx->vcdHi, type);
            break;
        case GX_VA_TEX3:
            GX_CP_SET_VCD_HI_TEX3COORD(gx->vcdHi, type);
            break;
        case GX_VA_TEX4:
            GX_CP_SET_VCD_HI_TEX4COORD(gx->vcdHi, type);
            break;
        case GX_VA_TEX5:
            GX_CP_SET_VCD_HI_TEX5COORD(gx->vcdHi, type);
            break;
        case GX_VA_TEX6:
            GX_CP_SET_VCD_HI_TEX6COORD(gx->vcdHi, type);
            break;
        case GX_VA_TEX7:
            GX_CP_SET_VCD_HI_TEX7COORD(gx->vcdHi, type);
            break;
        default:
            break;
    }
}

void __GXSetVCD(void) {
    GX_CP_LOAD_REG(GX_CP_REG_VCD_LO, gx->vcdLo);
    GX_CP_LOAD_REG(GX_CP_REG_VCD_HI, gx->vcdHi);
    __GXXfVtxSpecs();
}

void __GXCalculateVLim(void) {
    static u8 tbl1[] = {0, 4, 1, 2};
    static u8 tbl2[] = {0, 8, 1, 2};
    static u8 tbl3[] = {0, 12, 1, 2};

    u32 vLim;
    u32 vcdLo;
    u32 vcdHi;
    s32 compCnt;

    if (gx->vNum == 0) {
        return;
    }

    vcdLo = gx->vcdLo;
    vcdHi = gx->vcdHi;

    // GXCompCnt bit of hasNrms parameters
    compCnt = gx->vatA[GX_VTXFMT0];
    compCnt = (compCnt & 0x200) >> 9;

    vLim = GX_CP_GET_VCD_LO_POSMATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX0MATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX1MATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX2MATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX3MATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX4MATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX5MATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX6MATIDX(vcdLo);
    vLim += GX_CP_GET_VCD_LO_TEX7MATIDX(vcdLo);

    vLim += tbl3[GX_CP_GET_VCD_LO_POSITION(vcdLo)];
    vLim += tbl3[GX_CP_GET_VCD_LO_NORMAL(vcdLo)] * (compCnt == GX_NRM_NBT ? 3 : 1);
    vLim += tbl1[GX_CP_GET_VCD_LO_COLORDIFFUSED(vcdLo)];
    vLim += tbl1[GX_CP_GET_VCD_LO_COLORSPECULAR(vcdLo)];

    vLim += tbl2[GX_CP_GET_VCD_HI_TEX0COORD(vcdHi)];
    vLim += tbl2[GX_CP_GET_VCD_HI_TEX1COORD(vcdHi)];
    vLim += tbl2[GX_CP_GET_VCD_HI_TEX2COORD(vcdHi)];
    vLim += tbl2[GX_CP_GET_VCD_HI_TEX3COORD(vcdHi)];
    vLim += tbl2[GX_CP_GET_VCD_HI_TEX4COORD(vcdHi)];
    vLim += tbl2[GX_CP_GET_VCD_HI_TEX5COORD(vcdHi)];
    vLim += tbl2[GX_CP_GET_VCD_HI_TEX6COORD(vcdHi)];
    vLim += tbl2[GX_CP_GET_VCD_HI_TEX7COORD(vcdHi)];

    gx->vLim = vLim;
}

void GXClearVtxDesc(void) {
    gx->vcdLo = 0;
    // GX_VA_POS is required for every vertex descriptor
    GX_CP_SET_VCD_LO_POSITION(gx->vcdLo, GX_DIRECT);
    gx->vcdHi = 0;
    gx->hasNrms = false;
    gx->hasBiNrms = false;
    gx->dirtyState |= GX_DIRTY_VCD;
}

void GXSetVtxAttrFmt(GXVtxFmt fmt, GXAttr attr, GXCompCnt compCnt, GXCompType compType, u8 shift) {
    u32* vatA;
    u32* vatB;
    u32* vatC;

    vatA = &gx->vatA[fmt];
    vatB = &gx->vatB[fmt];
    vatC = &gx->vatC[fmt];

    SETVAT(vatA, vatB, vatC, attr, compCnt, compType, shift);
    gx->dirtyState |= GX_DIRTY_VAT;
    gx->dirtyVAT |= (u8)(1 << (u8)fmt);
}

static inline void SETVAT(u32* vatA, u32* vatB, u32* vatC, GXAttr attr, GXCompCnt compCnt, GXCompType compType,
                          u8 shift) {
    switch (attr) {
        case GX_VA_POS:
            GX_CP_SET_VAT_GROUP0_POS_CNT(*vatA, compCnt);
            GX_CP_SET_VAT_GROUP0_POS_TYPE(*vatA, compType);
            GX_CP_SET_VAT_GROUP0_POS_SHIFT(*vatA, shift);
            break;
        case GX_VA_NRM:
        case GX_VA_NBT:
            GX_CP_SET_VAT_GROUP0_NRM_TYPE(*vatA, compType);
            if (compCnt == GX_NRM_NBT3) {
                // Probably because the compCnt can only be one bit?
                GX_CP_SET_VAT_GROUP0_NRM_CNT(*vatA, GX_NRM_NBT);
                GX_CP_SET_VAT_GROUP0_NORMALINDEX3(*vatA, true);
            } else {
                GX_CP_SET_VAT_GROUP0_NRM_CNT(*vatA, compCnt);
                GX_CP_SET_VAT_GROUP0_NORMALINDEX3(*vatA, false);
            }
            break;
        case GX_VA_CLR0:
            GX_CP_SET_VAT_GROUP0_COLORDIFF_CNT(*vatA, compCnt);
            GX_CP_SET_VAT_GROUP0_COLORDIFF_TYPE(*vatA, compType);
            break;
        case GX_VA_CLR1:
            GX_CP_SET_VAT_GROUP0_COLORSPEC_CNT(*vatA, compCnt);
            GX_CP_SET_VAT_GROUP0_COLORSPEC_TYPE(*vatA, compType);
            break;
        case GX_VA_TEX0:
            GX_CP_SET_VAT_GROUP0_TXC0_CNT(*vatA, compCnt);
            GX_CP_SET_VAT_GROUP0_TXC0_TYPE(*vatA, compType);
            GX_CP_SET_VAT_GROUP0_TXC0_SHIFT(*vatA, shift);
            break;
        case GX_VA_TEX1:
            GX_CP_SET_VAT_GROUP1_TXC1_CNT(*vatB, compCnt);
            GX_CP_SET_VAT_GROUP1_TXC1_TYPE(*vatB, compType);
            GX_CP_SET_VAT_GROUP1_TXC1_SHIFT(*vatB, shift);
            break;
        case GX_VA_TEX2:
            GX_CP_SET_VAT_GROUP1_TXC2_CNT(*vatB, compCnt);
            GX_CP_SET_VAT_GROUP1_TXC2_TYPE(*vatB, compType);
            GX_CP_SET_VAT_GROUP1_TXC2_SHIFT(*vatB, shift);
            break;
        case GX_VA_TEX3:
            GX_CP_SET_VAT_GROUP1_TXC3_CNT(*vatB, compCnt);
            GX_CP_SET_VAT_GROUP1_TXC3_TYPE(*vatB, compType);
            GX_CP_SET_VAT_GROUP1_TXC3_SHIFT(*vatB, shift);
            break;
        case GX_VA_TEX4:
            GX_CP_SET_VAT_GROUP1_TXC4_CNT(*vatB, compCnt);
            GX_CP_SET_VAT_GROUP1_TXC4_TYPE(*vatB, compType);
            GX_CP_SET_VAT_GROUP2_TXC4_SHIFT(*vatC, shift);
            break;
        case GX_VA_TEX5:
            GX_CP_SET_VAT_GROUP2_TXC5_CNT(*vatC, compCnt);
            GX_CP_SET_VAT_GROUP2_TXC5_TYPE(*vatC, compType);
            GX_CP_SET_VAT_GROUP2_TXC5_SHIFT(*vatC, shift);
            break;
        case GX_VA_TEX6:
            GX_CP_SET_VAT_GROUP2_TXC6_CNT(*vatC, compCnt);
            GX_CP_SET_VAT_GROUP2_TXC6_TYPE(*vatC, compType);
            GX_CP_SET_VAT_GROUP2_TXC6_SHIFT(*vatC, shift);
            break;
        case GX_VA_TEX7:
            GX_CP_SET_VAT_GROUP2_TXC7_CNT(*vatC, compCnt);
            GX_CP_SET_VAT_GROUP2_TXC7_TYPE(*vatC, compType);
            GX_CP_SET_VAT_GROUP2_TXC7_SHIFT(*vatC, shift);
            break;
    }
}

void GXSetVtxAttrFmtv(GXVtxFmt fmt, const GXVtxAttrFmtList* list) {
    u32* vatA;
    u32* vatB;
    u32* vatC;

    vatA = &gx->vatA[fmt];
    vatB = &gx->vatB[fmt];
    vatC = &gx->vatC[fmt];

    for (; list->attr != GX_VA_NULL; list++) {
        SETVAT(vatA, vatB, vatC, list->attr, list->compCnt, list->compType, list->shift);
    }

    gx->dirtyState |= GX_DIRTY_VAT;
    gx->dirtyVAT |= (u8)(1 << (u8)fmt);
}

void __GXSetVAT(void) {
    int i = 0;
    u32 flags = gx->dirtyVAT;

    do {
        if (flags & 1) {
            GX_CP_LOAD_REG(GX_CP_REG_VAT_GROUP0 | i, gx->vatA[i]);
            GX_CP_LOAD_REG(GX_CP_REG_VAT_GROUP1 | i, gx->vatB[i]);
            GX_CP_LOAD_REG(GX_CP_REG_VAT_GROUP2 | i, gx->vatC[i]);
        }

        i++;
        flags >>= 1;
    } while (flags != 0);

    WGPIPE.c = GX_FIFO_CMD_NOOP;
    gx->dirtyVAT = 0;
}

static u8 GetFracForNrm(GXCompType compType) {
    switch (compType) {
        case GX_S8:
            return 6;
        case GX_S16:
            return 14;
        case GX_U16:
        default:
            return 0;
    }
}

void GXSetArray(GXAttr attr, const void* base, u8 stride) {
    u32 idx;

    if (attr == GX_VA_NBT) {
        attr = GX_VA_NRM;
    }

    idx = attr - GX_VA_POS;

    GX_CP_LOAD_REG(GX_BP_REG_SETMODE0_TEX4 | idx,
                   // Address -> offset?
                   (u32)base & ~0xC0000000);

    GX_CP_LOAD_REG(GX_BP_REG_SETIMAGE2_TEX4 | idx, stride);
}

void GXInvalidateVtxCache(void) { WGPIPE.c = GX_FIFO_CMD_INVAL_VTX; }

void GXSetTexCoordGen2(GXTexCoordID id, GXTexGenType type, GXTexGenSrc src, u32 texMtxIdx, GXBool normalize,
                       u32 dualTexMtxIdx) {
    u32 reg;
    u32 inputRow;
    GXXfTexForm inputForm;

    reg = 0;
    inputForm = GX_XF_TEX_FORM_AB11;
    inputRow = 5;

    switch (src) {
        case GX_TG_POS:
            inputRow = 0;
            inputForm = GX_XF_TEX_FORM_ABC1;
            break;
        case GX_TG_NRM:
            inputRow = 1;
            inputForm = GX_XF_TEX_FORM_ABC1;
            break;
        case GX_TG_BINRM:
            inputRow = 3;
            inputForm = GX_XF_TEX_FORM_ABC1;
            break;
        case GX_TG_TANGENT:
            inputRow = 4;
            inputForm = GX_XF_TEX_FORM_ABC1;
            break;
        case GX_TG_COLOR0:
            inputRow = 2;
            break;
        case GX_TG_COLOR1:
            inputRow = 2;
            break;
        case GX_TG_TEX0:
            inputRow = 5;
            break;
        case GX_TG_TEX1:
            inputRow = 6;
            break;
        case GX_TG_TEX2:
            inputRow = 7;
            break;
        case GX_TG_TEX3:
            inputRow = 8;
            break;
        case GX_TG_TEX4:
            inputRow = 9;
            break;
        case GX_TG_TEX5:
            inputRow = 10;
            break;
        case GX_TG_TEX6:
            inputRow = 11;
            break;
        case GX_TG_TEX7:
            inputRow = 12;
            break;
        default:
            break;
    }

    switch (type) {
        case GX_TG_MTX2x4:
            reg = 0;
            GX_XF_SET_TEX_PROJTYPE(reg, GX_XF_TEX_PROJ_ST); // 2x4 projection
            GX_XF_SET_TEX_INPUTFORM(reg, inputForm);
            GX_XF_SET_TEX_SRCROW(reg, inputRow);
            break;
        case GX_TG_MTX3x4:
            reg = 0;
            GX_XF_SET_TEX_PROJTYPE(reg, GX_XF_TEX_PROJ_STQ); // 3x4 projection
            GX_XF_SET_TEX_INPUTFORM(reg, inputForm);
            GX_XF_SET_TEX_SRCROW(reg, inputRow);
            break;
        case GX_TG_BUMP0:
        case GX_TG_BUMP1:
        case GX_TG_BUMP2:
        case GX_TG_BUMP3:
        case GX_TG_BUMP4:
        case GX_TG_BUMP5:
        case GX_TG_BUMP6:
        case GX_TG_BUMP7:
            reg = 0;
            GX_XF_SET_TEX_INPUTFORM(reg, inputForm);
            GX_XF_SET_TEX_TEXGENTYPE(reg, GX_XF_TG_BUMP);
            GX_XF_SET_TEX_SRCROW(reg, inputRow);
            GX_XF_SET_TEX_BUMPSRCTEX(reg, src - GX_TG_TEXCOORD0);
            GX_XF_SET_TEX_BUMPSRCLIGHT(reg, type - GX_TG_BUMP0);
            break;
        case GX_TG_SRTG:
            reg = 0;
            GX_XF_SET_TEX_INPUTFORM(reg, inputForm);

            if (src == GX_TG_COLOR0) {
                GX_XF_SET_TEX_TEXGENTYPE(reg, GX_XF_TG_CLR0);
            } else {
                GX_XF_SET_TEX_TEXGENTYPE(reg, GX_XF_TG_CLR1);
            }

            GX_XF_SET_TEX_SRCROW(reg, 2);
            break;
        default:
            break;
    }

    gx->texGenCtrl[id] = reg;
    gx->dirtyState |= GX_DIRTY_TEX0 << id;

    reg = 0;
    GX_XF_SET_DUALTEX_BASEROW(reg, dualTexMtxIdx - GX_DUALMTX0);
    GX_XF_SET_DUALTEX_NORMALIZE(reg, normalize);
    gx->dualTexGenCtrl[id] = reg;

    switch (id) {
        case GX_TEXCOORD0:
            GX_XF_SET_MATRIXINDEX0_TEX0(gx->matIdxA, texMtxIdx);
            break;
        case GX_TEXCOORD1:
            GX_XF_SET_MATRIXINDEX0_TEX1(gx->matIdxA, texMtxIdx);
            break;
        case GX_TEXCOORD2:
            GX_XF_SET_MATRIXINDEX0_TEX2(gx->matIdxA, texMtxIdx);
            break;
        case GX_TEXCOORD3:
            GX_XF_SET_MATRIXINDEX0_TEX3(gx->matIdxA, texMtxIdx);
            break;
        case GX_TEXCOORD4:
            GX_XF_SET_MATRIXINDEX1_TEX4(gx->matIdxB, texMtxIdx);
            break;
        case GX_TEXCOORD5:
            GX_XF_SET_MATRIXINDEX1_TEX5(gx->matIdxB, texMtxIdx);
            break;
        case GX_TEXCOORD6:
            GX_XF_SET_MATRIXINDEX1_TEX6(gx->matIdxB, texMtxIdx);
            break;
        default:
            GX_XF_SET_MATRIXINDEX1_TEX7(gx->matIdxB, texMtxIdx);
            break;
    }

    gx->dirtyState |= GX_DIRTY_MTX_IDX;
}

void GXSetNumTexGens(u8 num) {
    GX_BP_SET_GENMODE_NUMTEX(gx->genMode, num);
    gx->dirtyState |= GX_DIRTY_NUM_TEX;
    gx->dirtyState |= GX_DIRTY_GEN_MODE;
}
