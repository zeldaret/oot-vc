#include "math.h"
#include "revolution/gx.h"

void GXSetFog(GXFogType type, GXColor color, f32 start, f32 end, f32 near, f32 far) {
    f32 a, c;
    u32 a_bits, c_bits;

    u32 fogColorReg = 0;
    u32 fogParamReg0 = 0;
    u32 fogParamReg1 = 0;
    u32 fogParamReg2 = 0;
    u32 fogParamReg3 = 0;

    u32 fsel = GX_FOG_GET_FSEL(type);
    bool ortho = GX_FOG_GET_PROJ(type);

    if (ortho) {
        if (far == near || end == start) {
            a = 0.0f;
            c = 0.0f;
        } else {
            a = (1.0f / (end - start)) * (far - near);
            c = (1.0f / (end - start)) * (start - near);
        }
    } else {
        f32 f28, f25, f24;
        u32 expB, magB, shiftB;

        if (far == near || end == start) {
            f28 = 0.0f;
            f25 = 0.5f;
            f24 = 0.0f;
        } else {
            f28 = (far * near) / ((far - near) * (end - start));
            f25 = far / (far - near);
            f24 = start / (end - start);
        }

        expB = 0;
        while (f25 > 1.0l) {
            f25 /= 2.0f;
            expB++;
        }
        while (f25 > 0.0f && f25 < 0.5l) {
            f25 *= 2.0f;
            expB--;
        }

        a = f28 / (1 << expB + 1);
        magB = 8388638.0f * f25;
        shiftB = expB + 1;
        c = f24;

        GX_BP_SET_FOGPARAM1_B_MAG(fogParamReg1, magB);
        GX_BP_SET_OPCODE(fogParamReg1, GX_BP_REG_FOGPARAM1);
        GX_BP_SET_FOGPARAM2_B_SHIFT(fogParamReg2, shiftB);
        GX_BP_SET_OPCODE(fogParamReg2, GX_BP_REG_FOGPARAM2);
    }

    a_bits = *(u32*)&a;
    c_bits = *(u32*)&c;

    GX_BP_SET_FOGPARAM0_A_MANT(fogParamReg0, a_bits >> 12 & 0x7FF);
    GX_BP_SET_FOGPARAM0_A_EXP(fogParamReg0, a_bits >> 23 & 0xFF);
    GX_BP_SET_FOGPARAM0_A_SIGN(fogParamReg0, a_bits >> 31);
    GX_BP_SET_OPCODE(fogParamReg0, GX_BP_REG_FOGPARAM0);

    GX_BP_SET_FOGPARAM3_C_MANT(fogParamReg3, c_bits >> 12 & 0x7FF);
    GX_BP_SET_FOGPARAM3_C_EXP(fogParamReg3, c_bits >> 23 & 0xFF);
    GX_BP_SET_FOGPARAM3_C_SIGN(fogParamReg3, c_bits >> 31);
    GX_BP_SET_FOGPARAM3_PROJ(fogParamReg3, ortho);
    GX_BP_SET_FOGPARAM3_FSEL(fogParamReg3, fsel);
    GX_BP_SET_OPCODE(fogParamReg3, GX_BP_REG_FOGPARAM3);

    GX_BP_SET_FOGCOLOR_RGB(fogColorReg, *(u32*)&color >> 8);
    GX_BP_SET_OPCODE(fogColorReg, GX_BP_REG_FOGCOLOR);

    GX_BP_LOAD_REG(fogParamReg0);
    GX_BP_LOAD_REG(fogParamReg1);
    GX_BP_LOAD_REG(fogParamReg2);
    GX_BP_LOAD_REG(fogParamReg3);
    GX_BP_LOAD_REG(fogColorReg);

    gx->bpSentNot = false;
}

void GXSetFogRangeAdj(GXBool enable, u16 center, const GXFogAdjTable* table) {
    u32 fogRangeReg;
    u32 fogRangeRegK;
    int i;

    if (enable) {
        for (i = 0; i < ARRAY_COUNT(table->r); i += 2) {
            fogRangeRegK = 0;
            GX_BP_SET_FOGRANGEK_HI(fogRangeRegK, table->r[i]);
            GX_BP_SET_FOGRANGEK_LO(fogRangeRegK, table->r[i + 1]);
            GX_BP_SET_OPCODE(fogRangeRegK, GX_BP_REG_FOGRANGEK0 + (i / 2));
            GX_BP_LOAD_REG(fogRangeRegK);
        }
    }

    fogRangeReg = 0;
    GX_BP_SET_FOGRANGE_CENTER(fogRangeReg, center + 342);
    GX_BP_SET_FOGRANGE_ENABLED(fogRangeReg, enable);
    GX_BP_SET_OPCODE(fogRangeReg, GX_BP_REG_FOGRANGE);
    GX_BP_LOAD_REG(fogRangeReg);

    gx->bpSentNot = false;
}

void GXSetBlendMode(GXBlendMode mode, GXBlendFactor src, GXBlendFactor dst, GXLogicOp op) {
    u32 blendModeReg = gx->cmode0;
    GX_BP_SET_BLENDMODE_SUBTRACT(blendModeReg, mode == GX_BM_SUBTRACT);
    GX_BP_SET_BLENDMODE_BLEND_ENABLE(blendModeReg, mode);
    GX_BP_SET_BLENDMODE_LOGIC_OP_ENABLE(blendModeReg, mode == GX_BM_LOGIC);
    GX_BP_SET_BLENDMODE_LOGIC_MODE(blendModeReg, op);
    GX_BP_SET_BLENDMODE_SRC_FACTOR(blendModeReg, src);
    GX_BP_SET_BLENDMODE_DST_FACTOR(blendModeReg, dst);

    GX_BP_LOAD_REG(blendModeReg);
    gx->cmode0 = blendModeReg;

    gx->bpSentNot = false;
}

void GXSetColorUpdate(GXBool enable) {
    u32 blendModeReg = gx->cmode0;
    GX_BP_SET_BLENDMODE_COLOR_UPDATE(blendModeReg, enable);

    GX_BP_LOAD_REG(blendModeReg);
    gx->cmode0 = blendModeReg;

    gx->bpSentNot = false;
}

void GXSetAlphaUpdate(GXBool enable) {
    u32 blendModeReg = gx->cmode0;
    GX_BP_SET_BLENDMODE_ALPHA_UPDATE(blendModeReg, enable);

    GX_BP_LOAD_REG(blendModeReg);
    gx->cmode0 = blendModeReg;

    gx->bpSentNot = false;
}

void GXSetZMode(GXBool enableTest, GXCompare func, GXBool enableUpdate) {
    u32 zModeReg = gx->zmode;
    GX_BP_SET_ZMODE_TEST_ENABLE(zModeReg, enableTest);
    GX_BP_SET_ZMODE_COMPARE(zModeReg, func);
    GX_BP_SET_ZMODE_UPDATE_ENABLE(zModeReg, enableUpdate);

    GX_BP_LOAD_REG(zModeReg);
    gx->zmode = zModeReg;

    gx->bpSentNot = false;
}

void GXSetZCompLoc(GXBool beforeTex) {
    GX_BP_SET_ZCONTROL_BEFORE_TEX(gx->peCtrl, beforeTex);
    GX_BP_LOAD_REG(gx->peCtrl);
    gx->bpSentNot = false;
}

void GXSetPixelFmt(GXPixelFmt pixelFmt, GXZFmt16 zFmt) {
    static u32 p2f[GX_MAX_PIXELFMT] = {
        GX_PF_RGB8_Z24, GX_PF_RGBA6_Z24, GX_PF_RGBA565_Z16, GX_PF_Z24, GX_PF_Y8, GX_PF_Y8, GX_PF_Y8, GX_PF_U8,
    };

    u32 zControlRegOld = gx->peCtrl;

    GX_BP_SET_ZCONTROL_PIXEL_FMT(gx->peCtrl, p2f[pixelFmt]);
    GX_BP_SET_ZCONTROL_Z_FMT(gx->peCtrl, zFmt);

    if (zControlRegOld != gx->peCtrl) {
        GX_BP_LOAD_REG(gx->peCtrl);
        GX_BP_SET_GENMODE_MULTISAMPLE(gx->genMode, pixelFmt == GX_PF_RGBA565_Z16);
        gx->dirtyState |= GX_DIRTY_GEN_MODE;
    }

    if (p2f[pixelFmt] == GX_PF_Y8) {
        GX_BP_SET_DSTALPHA_YUV_FMT(gx->cmode1, pixelFmt - GX_PF_Y8);
        GX_BP_SET_OPCODE(gx->cmode1, GX_BP_REG_DSTALPHA);
        GX_BP_LOAD_REG(gx->cmode1);
    }

    gx->bpSentNot = false;
}

void GXSetDither(GXBool enable) {
    u32 blendModeReg = gx->cmode0;
    GX_BP_SET_BLENDMODE_DITHER(blendModeReg, enable);

    GX_BP_LOAD_REG(blendModeReg);
    gx->cmode0 = blendModeReg;

    gx->bpSentNot = false;
}

void GXSetDstAlpha(GXBool enable, u8 alpha) {
    u32 dstAlphaReg = gx->cmode1;
    GX_BP_SET_DSTALPHA_ALPHA(dstAlphaReg, alpha);
    GX_BP_SET_DSTALPHA_ENABLE(dstAlphaReg, enable);

    GX_BP_LOAD_REG(dstAlphaReg);
    gx->cmode1 = dstAlphaReg;

    gx->bpSentNot = false;
}

void GXSetFieldMask(GXBool enableEven, GXBool enableOdd) {
    u32 fieldMaskReg = 0;
    GX_BP_SET_FIELDMASK_ODD(fieldMaskReg, enableOdd);
    GX_BP_SET_FIELDMASK_EVEN(fieldMaskReg, enableEven);
    GX_BP_SET_OPCODE(fieldMaskReg, GX_BP_REG_FIELDMASK);

    GX_BP_LOAD_REG(fieldMaskReg);
    gx->bpSentNot = false;
}

void GXSetFieldMode(GXBool texLOD, GXBool adjustAR) {
    GX_BP_SET_LINEPTWIDTH_ADJUST_AR(gx->lpSize, adjustAR);
    GX_BP_LOAD_REG(gx->lpSize);

    __GXFlushTextureState();
    GX_BP_LOAD_REG(GX_BP_REG_FIELDMODE << 24 | texLOD);
    __GXFlushTextureState();
}
