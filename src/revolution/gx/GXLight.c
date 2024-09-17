#include "math.h"
#include "revolution/gx.h"

void GXSetChanAmbColor(GXChannelID chan, GXColor color) {
    u32 ambColor;
    u32 colorId;

    switch (chan) {
        case GX_COLOR0:
            ambColor = GX_BITSET_TRUNC(*(u32*)&gx->ambColor[0], 0, 24, *(u32*)&color);
            colorId = 0;
            break;
        case GX_COLOR1:
            ambColor = GX_BITSET_TRUNC(*(u32*)&gx->ambColor[1], 0, 24, *(u32*)&color);
            colorId = 1;
            break;
        case GX_ALPHA0:
            ambColor = GX_BITSET_TRUNC(*(u32*)&gx->ambColor[0], 24, 8, color.a);
            colorId = 0;
            break;
        case GX_ALPHA1:
            ambColor = GX_BITSET_TRUNC(*(u32*)&gx->ambColor[1], 24, 8, color.a);
            colorId = 1;
            break;
        case GX_COLOR0A0:
            ambColor = *(u32*)&color;
            colorId = 0;
            break;
        case GX_COLOR1A1:
            ambColor = *(u32*)&color;
            colorId = 1;
            break;
        default:
            return;
    }

    gx->dirtyState |= GX_DIRTY_AMB_COLOR0 << colorId;
    *(u32*)&gx->ambColor[colorId] = ambColor;
}

void GXSetChanMatColor(GXChannelID chan, GXColor color) {
    u32 matColor;
    u32 colorId;

    switch (chan) {
        case GX_COLOR0:
            matColor = GX_BITSET_TRUNC(*(u32*)&gx->matColor[0], 0, 24, *(u32*)&color);
            colorId = 0;
            break;
        case GX_COLOR1:
            matColor = GX_BITSET_TRUNC(*(u32*)&gx->matColor[1], 0, 24, *(u32*)&color);
            colorId = 1;
            break;
        case GX_ALPHA0:
            matColor = GX_BITSET_TRUNC(*(u32*)&gx->matColor[0], 24, 8, color.a);
            colorId = 0;
            break;
        case GX_ALPHA1:
            matColor = GX_BITSET_TRUNC(*(u32*)&gx->matColor[1], 24, 8, color.a);
            colorId = 1;
            break;
        case GX_COLOR0A0:
            matColor = *(u32*)&color;
            colorId = 0;
            break;
        case GX_COLOR1A1:
            matColor = *(u32*)&color;
            colorId = 1;
            break;
        default:
            return;
    }

    gx->dirtyState |= GX_DIRTY_MAT_COLOR0 << colorId;
    *(u32*)&gx->matColor[colorId] = matColor;
}

void GXSetNumChans(u8 num) {
    GX_BP_SET_GENMODE_NUMCOLORS(gx->genMode, num);
    gx->dirtyState |= GX_DIRTY_NUM_COLORS;
    gx->dirtyState |= GX_DIRTY_GEN_MODE;
}

void GXSetChanCtrl(GXChannelID chan, GXBool enable, GXColorSrc ambSrc, GXColorSrc matSrc, GXLightID lightMask,
                   GXDiffuseFn diffFn, GXAttnFn attnFn) {
    u32 regIdx = (u32)chan % 4;
    u32 reg = 0;

    GX_XF_SET_COLOR0CNTRL_LIGHT(reg, enable);
    GX_XF_SET_COLOR0CNTRL_MATSRC(reg, matSrc);
    GX_XF_SET_COLOR0CNTRL_AMBSRC(reg, ambSrc);
    GX_XF_SET_COLOR0CNTRL_DIFFUSEATTN(reg, attnFn == GX_AF_SPEC ? GX_DF_NONE : diffFn);
    GX_XF_SET_COLOR0CNTRL_ATTNENABLE(reg, attnFn != GX_AF_NONE);
    GX_XF_SET_COLOR0CNTRL_ATTNSELECT(reg, attnFn != GX_AF_SPEC);
    GX_XF_SET_COLOR0CNTRL_LMASKHI(reg, (u32)lightMask);
    GX_XF_SET_COLOR0CNTRL_LMASKLO(reg, (u32)lightMask >> 4);

    gx->chanCtrl[regIdx] = reg;
    gx->dirtyState |= GX_DIRTY_CHAN_COLOR0 << (regIdx);

    if (chan == GX_COLOR0A0) {
        gx->chanCtrl[GX_ALPHA0] = reg;
        gx->dirtyState |= GX_DIRTY_CHAN_COLOR0;
        gx->dirtyState |= GX_DIRTY_CHAN_ALPHA0;
    } else if (chan == GX_COLOR1A1) {
        gx->chanCtrl[GX_ALPHA1] = reg;
        gx->dirtyState |= GX_DIRTY_CHAN_COLOR1;
        gx->dirtyState |= GX_DIRTY_CHAN_ALPHA1;
    }
}
