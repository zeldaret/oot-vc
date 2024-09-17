#include "revolution/gx.h"
#include "revolution/gx/GXRegs.h"

static u32 TEVCOpTableST0[] = {
    0xC008F8AF, // modulate
    0xC008A89F, // decal
    0xC008AC8F, // blend
    0xC008FFF8, // replace
    0xC008FFFA, // passclr
};

static u32 TEVCOpTableST1[] = {
    0xC008F80F, // modulate
    0xC008089F, // decal
    0xC0080C8F, // blend
    0xC008FFF8, // replace
    0xC008FFF0, // passclr
};

static u32 TEVAOpTableST0[] = {
    0xC108F2F0, // modulate
    0xC108FFD0, // decal
    0xC108F2F0, // blend
    0xC108FFC0, // replace
    0xC108FFD0, // passclr
};

static u32 TEVAOpTableST1[] = {
    0xC108F070, // modulate
    0xC108FF80, // decal
    0xC108F070, // blend
    0xC108FFC0, // replace
    0xC108FF80, // passclr
};

void GXSetTevOp(GXTevStageID stage, GXTevMode mode) {
    u32* color;
    u32* alpha;
    u32 tevReg;

    if (stage == GX_TEVSTAGE0) {
        color = &TEVCOpTableST0[mode];
        alpha = &TEVAOpTableST0[mode];
    } else {
        color = &TEVCOpTableST1[mode];
        alpha = &TEVAOpTableST1[mode];
    }

    tevReg = gx->tevc[stage];
    tevReg = (*color & ~0xFF000000) | (tevReg & 0xFF000000);

    GX_BP_LOAD_REG(tevReg);

    gx->tevc[stage] = tevReg;

    tevReg = gx->teva[stage];
    tevReg = (*alpha & ~0xFF00000F) | (tevReg & 0xFF00000F);

    GX_BP_LOAD_REG(tevReg);

    gx->teva[stage] = tevReg;

    gx->bpSentNot = GX_FALSE;
}

void GXSetTevColorIn(GXTevStageID stage, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c, GXTevColorArg d) {
    u32 tevReg;

    tevReg = gx->tevc[stage];

    GX_SET_REG2(tevReg, a, 16, 19);
    GX_SET_REG2(tevReg, b, 20, 23);
    GX_SET_REG2(tevReg, c, 24, 27);
    GX_SET_REG2(tevReg, d, 28, 31);

    GX_BP_LOAD_REG(tevReg);

    gx->tevc[stage] = tevReg;
    gx->bpSentNot = GX_FALSE;
}

void GXSetTevAlphaIn(GXTevStageID stage, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c, GXTevAlphaArg d) {
    u32 tevReg;

    tevReg = gx->teva[stage];

    GX_SET_REG2(tevReg, a, 16, 18);
    GX_SET_REG2(tevReg, b, 19, 21);
    GX_SET_REG2(tevReg, c, 22, 24);
    GX_SET_REG2(tevReg, d, 25, 27);

    GX_BP_LOAD_REG(tevReg);

    gx->teva[stage] = tevReg;
    gx->bpSentNot = GX_FALSE;
}

void GXSetTevColorOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool doClamp,
                     GXTevRegID outReg) {
    u32 tevReg;

    tevReg = gx->tevc[stage];
    GX_SET_REG2(tevReg, op & 1, 13, 13);

    if (op <= 1) {
        GX_SET_REG2(tevReg, scale, 10, 11);
        GX_SET_REG2(tevReg, bias, 14, 15);
    } else {
        GX_SET_REG2(tevReg, (op >> 1) & 3, 10, 11);
        GX_SET_REG2(tevReg, 3, 14, 15);
    }

    GX_SET_REG2(tevReg, doClamp, 12, 12);
    GX_SET_REG2(tevReg, outReg, 8, 9);

    GX_BP_LOAD_REG(tevReg);

    gx->tevc[stage] = tevReg;
    gx->bpSentNot = GX_FALSE;
}

void GXSetTevAlphaOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool doClamp,
                     GXTevRegID outReg) {
    u32 tevReg;

    tevReg = gx->teva[stage];
    GX_SET_REG2(tevReg, op & 1, 13, 13);

    if (op <= 1) {
        GX_SET_REG2(tevReg, scale, 10, 11);
        GX_SET_REG2(tevReg, bias, 14, 15);
    } else {
        GX_SET_REG2(tevReg, (op >> 1) & 3, 10, 11);
        GX_SET_REG2(tevReg, 3, 14, 15);
    }

    GX_SET_REG2(tevReg, doClamp, 12, 12);
    GX_SET_REG2(tevReg, outReg, 8, 9);

    GX_BP_LOAD_REG(tevReg);

    gx->teva[stage] = tevReg;
    gx->bpSentNot = GX_FALSE;
}

void GXSetTevColor(GXTevRegID id, GXColor color) {
    u32 rgba;
    u32 regRA;
    u32 regBG;

    rgba = *(u32*)&color;

    regRA = (0xE0 + id * 2) << 24;
    SET_REG_FIELD(regRA, 8, 0, rgba >> 24);
    SET_REG_FIELD(regRA, 8, 12, rgba & 0xFF);

    regBG = (0xE1 + id * 2) << 24;
    SET_REG_FIELD(regBG, 8, 0, (rgba >> 8) & 0xFF);
    SET_REG_FIELD(regBG, 8, 12, (rgba >> 16) & 0xFF);

    GX_WRITE_RAS_REG(regRA);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);

    __GXData->bpSentNot = 0;
}

void GXSetTevColorS10(GXTevRegID id, GXColorS10 color) {
    u32 sRG;
    u32 sBA;
    u32 regRA;
    u32 regBG;

    sRG = *(u32*)&color;
    sBA = *((u32*)&color + 1);

    regRA = (0xE0 + id * 2) << 24;
    SET_REG_FIELD(regRA, 11, 0, (sRG >> 16) & 0x7FF);
    SET_REG_FIELD(regRA, 11, 12, sBA & 0x7FF);

    regBG = (0xE1 + id * 2) << 24;
    SET_REG_FIELD(regBG, 11, 0, (sBA >> 16) & 0x7FF);
    SET_REG_FIELD(regBG, 11, 12, sRG & 0x7FF);

    GX_WRITE_RAS_REG(regRA);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);
    GX_WRITE_RAS_REG(regBG);

    __GXData->bpSentNot = 0;
}

void GXSetTevKColor(GXTevKColorID id, GXColor color) {
    u32 rgba;
    u32 regRA;
    u32 regBG;

    rgba = *(u32*)&color;

    regRA = (0xE0 + id * 2) << 24;
    SET_REG_FIELD(regRA, 8, 0, rgba >> 24);
    SET_REG_FIELD(regRA, 8, 12, rgba & 0xFF);
    SET_REG_FIELD(regRA, 4, 20, 8);

    regBG = (0xE1 + id * 2) << 24;
    SET_REG_FIELD(regBG, 8, 0, (rgba >> 8) & 0xFF);
    SET_REG_FIELD(regBG, 8, 12, (rgba >> 16) & 0xFF);
    SET_REG_FIELD(regBG, 4, 20, 8);

    GX_WRITE_RAS_REG(regRA);
    GX_WRITE_RAS_REG(regBG);

    __GXData->bpSentNot = 0;
}

void GXSetTevKColorSel(GXTevStageID stage, GXTevKColorSel sel) {
    u32* reg;

    reg = &gx->tevKsel[stage >> 1];

    if (stage & 1) {
        GX_SET_REG(*reg, sel, 13, 17);
    } else {
        GX_SET_REG(*reg, sel, 23, 27);
    }

    GX_BP_LOAD_REG(*reg);

    gx->bpSentNot = GX_FALSE;
}

void GXSetTevKAlphaSel(GXTevStageID stage, GXTevKAlphaSel sel) {
    u32* reg;

    reg = &gx->tevKsel[stage >> 1];

    if (stage & 1) {
        GX_SET_REG(*reg, sel, 8, 12);
    } else {
        GX_SET_REG(*reg, sel, 18, 22);
    }

    GX_BP_LOAD_REG(*reg);

    gx->bpSentNot = GX_FALSE;
}

void GXSetTevSwapMode(GXTevStageID stage, GXTevSwapSel rasSel, GXTevSwapSel texSel) {
    u32* reg = &gx->teva[stage];
    GX_SET_REG(*reg, rasSel, 30, 31);
    GX_SET_REG(*reg, texSel, 28, 29);

    GX_BP_LOAD_REG(*reg);

    gx->bpSentNot = GX_FALSE;
}

void GXSetTevSwapModeTable(GXTevSwapSel table, GXTevColorChan red, GXTevColorChan green, GXTevColorChan blue,
                           GXTevColorChan alpha) {
    u32* reg;

    reg = &gx->tevKsel[table << 1];
    GX_SET_REG(*reg, red, 30, 31);
    GX_SET_REG(*reg, green, 28, 29);

    GX_BP_LOAD_REG(*reg);

    reg = &gx->tevKsel[(table << 1) + 1];
    GX_SET_REG(*reg, blue, 30, 31);
    GX_SET_REG(*reg, alpha, 28, 29);

    GX_BP_LOAD_REG(*reg);

    gx->bpSentNot = GX_FALSE;
}

void GXSetAlphaCompare(GXCompare comp0, u8 ref0, GXAlphaOp op, GXCompare comp1, u8 ref1) {
    u32 reg = 0xF3000000;

    GX_SET_REG(reg, ref0, 24, 31);
    GX_SET_REG(reg, ref1, 16, 23);
    GX_SET_REG(reg, comp0, 13, 15);
    GX_SET_REG(reg, comp1, 10, 12);
    GX_SET_REG(reg, op, 8, 9);

    GX_BP_LOAD_REG(reg);

    gx->bpSentNot = GX_FALSE;
}

void GXSetZTexture(GXZTexOp op, GXTexFmt format, u32 bias) {
    u32 val1;
    u32 val2;
    u32 val3;

    val1 = 0;
    GX_SET_REG(val1, bias, 8, 31);
    GX_SET_REG(val1, 0xF4, 0, 7);

    val2 = 0;
    switch (format) {
        case GX_TF_Z8:
            val3 = 0;
            break;
        case GX_TF_Z16:
            val3 = 1;
            break;
        case GX_TF_Z24X8:
            val3 = 2;
            break;
        default:
            val3 = 2;
            break;
    }

    GX_SET_REG(val2, val3, 30, 31);
    GX_SET_REG(val2, op, 28, 29);
    GX_SET_REG(val2, 0xF5, 0, 7);

    GX_BP_LOAD_REG(val1);

    GX_BP_LOAD_REG(val2);

    gx->bpSentNot = GX_FALSE;
}

void GXSetTevOrder(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color) {
    static int c2r[] = {0, 1, 0, 1, 0, 1, 7, 5, 6};

    u32* reg;
    u32 tempMap;
    u32 tempCoord;

    reg = &gx->tref[stage / 2];
    gx->texmapId[stage] = map;

    tempMap = map & ~0x100;
    tempMap = (tempMap >= GX_MAX_TEXMAP) ? GX_TEXMAP0 : tempMap;

    if (coord >= GX_MAX_TEXCOORD) {
        tempCoord = GX_TEXCOORD0;
        gx->tevTcEnab = gx->tevTcEnab & ~(1 << stage);
    } else {
        tempCoord = coord;
        gx->tevTcEnab = gx->tevTcEnab | (1 << stage);
    }

    if (stage & 1) {
        GX_SET_REG(*reg, tempMap, 17, 19);
        GX_SET_REG(*reg, tempCoord, 14, 16);
        GX_SET_REG(*reg, (color == GX_COLOR_NULL ? 7 : c2r[color]), 10, 12);
        GX_SET_REG(*reg, ((map != GX_TEXMAP_NULL) && !(map & 0x100)), 13, 13);

    } else {
        GX_SET_REG(*reg, tempMap, 29, 31);
        GX_SET_REG(*reg, tempCoord, 26, 28);
        GX_SET_REG(*reg, (color == GX_COLOR_NULL ? 7 : c2r[color]), 22, 24);
        GX_SET_REG(*reg, ((map != GX_TEXMAP_NULL) && !(map & 0x100)), 25, 25);
    }

    GX_BP_LOAD_REG(*reg);

    gx->bpSentNot = GX_FALSE;
    gx->dirtyState |= 1;
}

void GXSetNumTevStages(u8 count) {
    GX_SET_REG(gx->genMode, count - 1, 18, 21);

    gx->dirtyState |= 0x4;
}
