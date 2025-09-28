#include "revolution/hbm/nw4hbm/ut/CharWriter.h"

#include "revolution/types.h"

#include "macros.h"
#include "revolution/gx.h"
#include "revolution/hbm/HBMAssert.hpp"
#include "revolution/hbm/nw4hbm/db/console.h"
#include "revolution/hbm/nw4hbm/math/types.h"
#include "revolution/hbm/nw4hbm/ut/Color.h"
#include "revolution/hbm/nw4hbm/ut/Font.h"

namespace nw4hbm {
static void SetupGXCommon();
} // namespace nw4hbm

namespace nw4hbm {
namespace ut {
// .bss
/* CharWriter::LoadingTexture CharWriter::mLoadingTexture; */ // pooling
} // namespace ut
} // namespace nw4hbm

namespace nw4hbm {

void SetupGXCommon() {
    static ut::Color fog(0x00000000);

    GXSetFog(GX_FOG_NONE, fog, 0.0f, 0.0f, 0.0f, 0.0f);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 60);
    GXSetNumIndStages(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
}

namespace ut {

// pooling
CharWriter::LoadingTexture CharWriter::mLoadingTexture;

CharWriter::CharWriter() : mAlpha(0xff), mIsWidthFixed(false), mFixedWidth(0.0f), mFont(nullptr) {
    mLoadingTexture.Reset();
    ResetColorMapping();
    SetGradationMode(GRADMODE_NONE);
    SetTextColor(0xffffffff);
    SetScale(1.0f, 1.0f);
    SetCursor(0.0f, 0.0f, 0.0f);
    EnableLinearFilter(true, true);
}

CharWriter::~CharWriter() {}

void CharWriter::SetFont(const Font& font) {
    // clang-format off
    NW4HBMAssertPointerValid_Line(this, 133);
    NW4HBMAssertPointerValid_Line(& font, 134);
    mFont = &font;
    // clang-format on
}

const Font* CharWriter::GetFont() const {
    NW4HBMAssertPointerValid_Line(this, 151);
    return mFont;
}

void CharWriter::SetupGX() {
    NW4HBMAssertPointerValid_Line(this, 173);
    ResetTextureCache();

    if (mColorMapping.min != 0x00000000 || mColorMapping.max != 0xffffffff) {
        SetupGXWithColorMapping(mColorMapping.min, mColorMapping.max);
    } else if (mFont) {
        GXTexFmt format = mFont->GetTextureFormat();
        switch (format) {
            case GX_TF_I4:
            case GX_TF_I8:
                SetupGXForI();
                break;

            case GX_TF_IA4:
            case GX_TF_IA8:
                SetupGXDefault();
                break;

            case GX_TF_RGB565:
            case GX_TF_RGB5A3:
            case GX_TF_RGBA8:
                SetupGXForRGBA();
                break;

            default:
                NW4HBMWarningMessage_Line(207, "CharWriter::SetupGX: Unknown font sheet format(=%d)", format);
                SetupGXDefault();
                break;
        }
    } else {
        SetupGXDefault();
    }
}

void CharWriter::SetColorMapping(Color min, Color max) {
    NW4HBMAssertPointerValid_Line(this, 235);
    mColorMapping.min = min;
    mColorMapping.max = max;
}

void CharWriter::ResetColorMapping() {
    NW4HBMAssertPointerValid_Line(this, 284);
    SetColorMapping(0x00000000, 0xffffffff);
}

void CharWriter::ResetTextureCache() {
    NW4HBMAssertPointerValid_Line(this, 300);
    mLoadingTexture.Reset();
}

void CharWriter::SetGradationMode(GradationMode mode) {
    NW4HBMAssertPointerValid_Line(this, 355);
    NW4HBMAssertHeaderClampedLRValue_Line(mode, 0, 2, 356);
    mTextColor.gradationMode = mode;
    UpdateVertexColor();
}

void CharWriter::SetTextColor(Color color) {
    NW4HBMAssertPointerValid_Line(this, 389);
    mTextColor.start = color;
    UpdateVertexColor();
}

void CharWriter::SetTextColor(Color start, Color end) {
    NW4HBMAssertPointerValid_Line(this, 410);
    mTextColor.start = start;
    mTextColor.end = end;
    UpdateVertexColor();
}

void CharWriter::SetScale(f32 hScale, f32 vScale) {
    NW4HBMAssertPointerValid_Line(this, 487);
    mScale.x = hScale;
    mScale.y = vScale;
}

f32 CharWriter::GetScaleH() const {
    NW4HBMAssertPointerValid_Line(this, 522);
    return mScale.x;
}

f32 CharWriter::GetScaleV() const {
    NW4HBMAssertPointerValid_Line(this, 538);
    return mScale.y;
}

void CharWriter::SetFontSize(f32 width, f32 height) {
    NW4HBMAssertPointerValid_Line(this, 559);
    NW4HBMAssertPointerValid_Line(mFont, 560);
    NW4HBMAssertHeaderMinimumValue_Line(mFont->GetWidth(), 1, 561);
    NW4HBMAssertHeaderMinimumValue_Line(mFont->GetHeight(), 1, 562);
    SetScale(width / mFont->GetWidth(), height / mFont->GetHeight());
}

f32 CharWriter::GetFontWidth() const {
    NW4HBMAssertPointerValid_Line(this, 601);
    NW4HBMAssertPointerValid_Line(mFont, 602);
    return mFont->GetWidth() * mScale.x;
}

f32 CharWriter::GetFontHeight() const {
    NW4HBMAssertPointerValid_Line(this, 618);
    NW4HBMAssertPointerValid_Line(mFont, 619);
    return mFont->GetHeight() * mScale.y;
}

f32 CharWriter::GetFontAscent() const {
    NW4HBMAssertPointerValid_Line(this, 635);
    NW4HBMAssertPointerValid_Line(mFont, 636);
    return mFont->GetAscent() * mScale.y;
}

void CharWriter::EnableLinearFilter(bool atSmall, bool atLarge) {
    NW4HBMAssertPointerValid_Line(this, 681);
    mFilter.atSmall = atSmall ? GX_LINEAR : GX_NEAR;
    mFilter.atLarge = atLarge ? GX_LINEAR : GX_NEAR;
}

bool CharWriter::IsWidthFixed() const {
    NW4HBMAssertPointerValid_Line(this, 738);
    return mIsWidthFixed;
}

f32 CharWriter::GetFixedWidth() const {
    NW4HBMAssertPointerValid_Line(this, 769);
    return mFixedWidth;
}

f32 CharWriter::Print(char16_t code) {
    NW4HBMAssertPointerValid_Line(this, 808);
    NW4HBMAssertPointerValid_Line(mFont, 809);
    NW4HBMAssert_Line(code != Font::INVALID_CHARACTER_CODE, 810);
    Glyph glyph;
    f32 width;
    f32 left;

    mFont->GetGlyph(&glyph, code);

    CharWidths& widths = glyph.widths;

    if (mIsWidthFixed) {
        f32 margin = (mFixedWidth - widths.charWidth * mScale.x) / 2.0f;

        width = mFixedWidth;
        left = margin + widths.left * mScale.x;
    } else {
        width = widths.charWidth * mScale.x;
        left = widths.left * mScale.x;
    }

    PrintGlyph(mCursorPos.x + left, mCursorPos.y, mCursorPos.z, glyph);

    mCursorPos.x += width;
    return width;
}

void CharWriter::SetCursor(f32 x, f32 y) {
    NW4HBMAssertPointerValid_Line(this, 879);
    mCursorPos.x = x;
    mCursorPos.y = y;
}

void CharWriter::SetCursor(f32 x, f32 y, f32 z) {
    NW4HBMAssertPointerValid_Line(this, 902);
    mCursorPos.x = x;
    mCursorPos.y = y;
    mCursorPos.z = z;
}

void CharWriter::SetCursorX(f32 x) {
    NW4HBMAssertPointerValid_Line(this, 965);
    mCursorPos.x = x;
}

void CharWriter::SetCursorY(f32 y) {
    NW4HBMAssertPointerValid_Line(this, 981);
    mCursorPos.y = y;
}

void CharWriter::MoveCursorX(f32 dx) {
    NW4HBMAssertPointerValid_Line(this, 1013);
    mCursorPos.x += dx;
}

void CharWriter::MoveCursorY(f32 dy) {
    NW4HBMAssertPointerValid_Line(this, 1029);
    mCursorPos.y += dy;
}

f32 CharWriter::GetCursorX() const {
    NW4HBMAssertPointerValid_Line(this, 1061);
    return mCursorPos.x;
}

f32 CharWriter::GetCursorY() const {
    NW4HBMAssertPointerValid_Line(this, 1077);
    return mCursorPos.y;
}

void CharWriter::PrintGlyph(f32 x, f32 y, f32 z, const Glyph& glyph) {
    // clang-format off
    NW4HBMAssertPointerValid_Line(this, 1127);
    NW4HBMAssertPointerValid_Line(& glyph, 1128);
    NW4HBMAssertHeaderMinimumValue_Line(glyph.texWidth, 1, 1129);
    NW4HBMAssertHeaderMinimumValue_Line(glyph.texHeight, 1, 1130);
    // clang-format on

    f32 posLeft = x;
    f32 posTop = y;
    f32 posRight = posLeft + glyph.widths.glyphWidth * mScale.x;
    f32 posBottom = posTop + glyph.height * mScale.y;
    f32 posZ = z;
    u16 texLeft = static_cast<u32>(glyph.cellX << 15) / glyph.texWidth;
    u16 texTop = static_cast<u32>(glyph.cellY << 15) / glyph.texHeight;
    u16 texRight = static_cast<u32>((glyph.cellX + glyph.widths.glyphWidth) << 15) / glyph.texWidth;
    u16 texBottom = static_cast<u32>((glyph.cellY + glyph.height) << 15) / glyph.texHeight;

    LoadTexture(glyph, GX_TEXMAP0);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(posLeft, posTop, posZ);
    GXColor1u32(mVertexColor.lu);
    GXTexCoord2u16(texLeft, texTop);

    GXPosition3f32(posRight, posTop, posZ);
    GXColor1u32(mVertexColor.ru);
    GXTexCoord2u16(texRight, texTop);

    GXPosition3f32(posRight, posBottom, posZ);
    GXColor1u32(mVertexColor.rd);
    GXTexCoord2u16(texRight, texBottom);

    GXPosition3f32(posLeft, posBottom, posZ);
    GXColor1u32(mVertexColor.ld);
    GXTexCoord2u16(texLeft, texBottom);
    GXEnd();
}

void CharWriter::LoadTexture(const Glyph& glyph, GXTexMapID slot) {
    // clang-format off
    NW4HBMAssertPointerValid_Line(this, 1192);
    NW4HBMAssertPointerValid_Line(& glyph, 1193);
    // clang-format on

    LoadingTexture loadInfo;
    loadInfo.slot = slot;
    loadInfo.texture = glyph.pTexture;
    loadInfo.filter = mFilter;

    if (loadInfo != mLoadingTexture) {
        GXTexObj tobj;

        GXInitTexObj(&tobj, glyph.pTexture, glyph.texWidth, glyph.texHeight, glyph.texFormat, GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(&tobj, mFilter.atSmall, mFilter.atLarge, 0.0f, 0.0f, 0.0f, false, false, GX_ANISO_1);
        GXLoadTexObj(&tobj, slot);

        mLoadingTexture = loadInfo;
    }
}

void CharWriter::UpdateVertexColor() {
    NW4HBMAssertPointerValid_Line(this, 1242);

    mVertexColor.lu = mTextColor.start;
    mVertexColor.ru = mTextColor.gradationMode != GRADMODE_H ? mTextColor.start : mTextColor.end;
    mVertexColor.ld = mTextColor.gradationMode != GRADMODE_V ? mTextColor.start : mTextColor.end;
    mVertexColor.rd = mTextColor.gradationMode == GRADMODE_NONE ? mTextColor.start : mTextColor.end;

    mVertexColor.lu.a = mVertexColor.lu.a * mAlpha / 255;
    mVertexColor.ru.a = mVertexColor.ru.a * mAlpha / 255;
    mVertexColor.ld.a = mVertexColor.ld.a * mAlpha / 255;
    mVertexColor.rd.a = mVertexColor.rd.a * mAlpha / 255;
}

void CharWriter::SetupVertexFormat() {
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 15);

    GXClearVtxDesc();

    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
}

void CharWriter::SetupGXDefault() {
    SetupGXCommon();

    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);

    SetupVertexFormat();
}

void CharWriter::SetupGXWithColorMapping(Color min, Color max) {
    SetupGXCommon();

    GXSetNumTevStages(2);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColor(GX_TEVREG0, min);
    GXSetTevColor(GX_TEVREG1, max);

    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A0, GX_CA_A1, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

    SetupVertexFormat();
}

void CharWriter::SetupGXForI() {
    SetupGXCommon();

    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

    SetupVertexFormat();
}

void CharWriter::SetupGXForRGBA() { SetupGXDefault(); }

} // namespace ut

} // namespace nw4hbm
