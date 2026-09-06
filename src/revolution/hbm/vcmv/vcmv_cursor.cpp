#include "macros.h"
#include "revolution/arc/arc.h"
#include "revolution/hbm.h"
#include "revolution/tpl/TPL.h"
#include "revolution/types.h"

#include <string.h>

static UnkStruct_80175290 lbl_80175290[] = {
    {50.0f, 50.0f, 0.38f, 0.92f, nullptr, {0, 140, 255, 255}}, // blue
    {50.0f, 50.0f, 0.38f, 0.92f, nullptr, {255, 56, 56, 255}}, // red
    {50.0f, 50.0f, 0.38f, 0.92f, nullptr, {16, 189, 13, 255}}, // green
    {50.0f, 50.0f, 0.38f, 0.92f, nullptr, {255, 156, 0, 255}}, // orange
};

#if VCMV_REVISION == 2
static UNKWORD lbl_8025D218 = 0;
static UNKWORD lbl_8025D21C = 0;

void VCMV_80083070(UNKWORD param1) {
    if (lbl_8025D21C == param1 && lbl_8025D2D8 - lbl_8025D218 < 3) {
        return;
    }

    lbl_8025D21C = param1;
    lbl_8025D218 = lbl_8025D2D8;

    switch (param1) {
        case 0: {
            VCMV_VOLATILE UnkStruct_801CA6B0* ptr = &lbl_801CA6B0[lbl_8025D2D4];

            if (!lbl_8025D2DC && ptr->unk_50 && lbl_8025D2D8 - lbl_8025D2E0 >= 0x0F) {
                lbl_8025D2BF = true;
            }

            HBMPlaySound(0x17);
            break;
        }
        case 1:
            HBMPlaySound(0x18);
            break;
        case 2:
            HBMPlaySound(0x16);
            break;
        case 3:
            HBMPlaySound(0x1B);
            break;
        case 5:
            HBMPlaySound(0x1A);
            break;
        default:
            break;
    }
}

void VCMV_80083140(void) {
    lbl_8025D218 = lbl_8025D2D8;
    lbl_8025D21C = -1;
}

void VCMV_80083154(void) {}
#endif

static TPLPalette* lbl_8025D220 = nullptr;
s32 lbl_8025D224;

void VCMV_80083158(void) {}

void VCMV_8008315C(void** param1) {
    char sp78[40] = "arc/timg/defcursor_final";
    ARCHandle sp5C;
    ARCHandle sp40;
    ARCHandle sp24;
    ARCFileInfo sp18;
    ARCFileInfo spC;
    UnkStruct_80175290* var_r15;
    int i;

    char* temp_r14 = &sp78[strlen(sp78)];
    ARCInitHandle(*param1, &sp5C);
    strcpy(temp_r14, "64_a.tpl");

    sp40 = sp5C;
    ARCOpen(&sp40, sp78, &sp18);
    lbl_8025D220 = (TPLPalette*)ARCGetStartAddrInMem(&sp18);
    ARCClose(&sp18);
    TPLBind(lbl_8025D220);

    strcpy(temp_r14, "_p1.tpl");

    var_r15 = &lbl_80175290[0];
    for (i = 0; i < ARRAY_COUNT(lbl_80175290); i++, var_r15++) {
        temp_r14[2] = i + '1'; // hacky way to get "_p{i}.tpl"
        sp24 = sp5C;
        ARCOpen(&sp24, sp78, &spC);
        var_r15->unk_10 = (TPLPalette*)ARCGetStartAddrInMem(&spC);
        ARCClose(&spC);
        TPLBind(var_r15->unk_10);
    }
}

void VCMV_8008345C(UNKWORD param1) {
    VCMV_VOLATILE UnkStruct_801CA6B0* temp_r6 = &lbl_801CA6B0[param1];
    f32 unk_34;
    f32 unk_38;
    f32 temp_f8;
    f32 temp_f9;
    f32 var_f10;
    f32 var_f11;
    UnkStruct_80175290* temp_r31;
    u8 var_r5;
    GXTexObj sp40;
    UnkStack20 sp20;

    if (temp_r6->unk_00 == 0) {
        return;
    }

    unk_34 = temp_r6->unk_34;
    unk_38 = -temp_r6->unk_38;
    temp_f8 = temp_r6->unk_2C;
    temp_f9 = temp_r6->unk_30;

    temp_r31 = &lbl_80175290[param1];
    var_f10 = temp_r31->unk_00;
    var_f11 = temp_r31->unk_04;

    if (param1 == lbl_8025D2D4) {
        if (temp_r6->unk_00 >= 0x240) {
            var_r5 = 0xC0;
        } else {
            var_r5 = temp_r6->unk_00 / 3;
        }

        if (lbl_8025D224 != 0) {
            f32 var_f0;

            if (lbl_8025D224 > 0x0A) {
                var_f0 = ((0x14 - lbl_8025D224) * 0.02f) + 1.0f;
            } else {
                var_f0 = (lbl_8025D224 * 0.02f) + 1.0f;
            }

            var_f10 *= var_f0;
            var_f11 *= var_f0;

            lbl_8025D224--;
        }
    } else {
        if (temp_r6->unk_00 >= 0x100) {
            var_r5 = 0x40;
        } else {
            var_r5 = temp_r6->unk_00 / 4;
        }
    }

    GXColor sp18 = {224, 224, 224, var_r5};

    sp20.x2 = unk_34 - (temp_f8 * var_f10 * temp_r31->unk_08) - (temp_f9 * var_f11 * temp_r31->unk_0C);
    sp20.x1 = sp20.x2 + (temp_f9 * var_f11);
    sp20.x4 = sp20.x1 + (temp_f8 * var_f10);
    sp20.x3 = sp20.x4 - (temp_f9 * var_f11);

    sp20.y2 = unk_38 + (temp_f9 * var_f10 * temp_r31->unk_08) - (temp_f8 * var_f11 * temp_r31->unk_0C);
    sp20.y1 = sp20.y2 + (temp_f8 * var_f11);
    sp20.y4 = sp20.y1 - (temp_f9 * var_f10);
    sp20.y3 = sp20.y4 - (temp_f8 * var_f11);

    TPLGetGXTexObjFromPalette(lbl_8025D220, &sp40, 0);
    GXLoadTexObj(&sp40, GX_TEXMAP1);
    TPLGetGXTexObjFromPalette(temp_r31->unk_10, &sp40, 0);
    GXLoadTexObj(&sp40, GX_TEXMAP0);

    GXColor sp14 = {255, 255, 255, 255};
    GXSetTevColor(GX_TEVREG0, temp_r31->unk_14);
    GXSetTevColor(GX_TEVREG1, sp14);
    GXSetTevColor(GX_TEVREG2, sp18);

    GXClearVtxDesc();
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_CLR_RGBA, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_CLR_RGBA, GX_F32, 0);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumIndStages(0);
    GXSetNumTevStages(2);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_A2, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    VCMV_80085798(&sp20);
}
