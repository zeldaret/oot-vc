#include "revolution/hbm/nw4hbm/lyt/lyt_textBox.hpp"

/*******************************************************************************
 * headers
 */

#include "cstring.hpp"
#include "cwchar.hpp" // std::wcslen
#include "new.hpp"

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_layout.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_resourceAccessor.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_types.hpp" // detail::ConvertOffsToPointer
#include "revolution/hbm/nw4hbm/lyt/material.h"
#include "revolution/hbm/nw4hbm/lyt/pane.h"

#include "revolution/hbm/nw4hbm/math/math_types.hpp" // math::VEC2
#include "revolution/hbm/nw4hbm/ut/ut_CharStrmReader.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_CharWriter.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Color.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Font.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Rect.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_ResFont.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_TagProcessorBase.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_TextWriterBase.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"

#include "revolution/gx/GXTypes.h"

/*******************************************************************************
 * local function declarations
 */

namespace {
// pretend this is nw4hbm::lyt
using namespace nw4hbm;
using namespace nw4hbm::lyt;

ut::Color GetColor(const GXColorS10& src);
u8 ClampColor(s16 colVal);

// not putting the definitions inline here; they would be too big
template <typename charT>
int CalcLineStrNum(f32* pWidth, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length, f32 maxWidth,
                   bool* pbOver);

template <typename charT>
int CalcLineRectImpl(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                     f32 maxWidth, bool* pbOver);

template <typename charT>
void CalcStringRect(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                    f32 maxWidth);

template <typename charT>
void CalcStringRectImpl(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                        f32 maxWidth);
} // unnamed namespace

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace lyt {
// .bss
const ut::detail::RuntimeTypeInfo TextBox::typeInfo(&Pane::typeInfo);
} // namespace lyt
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace {

ut::Color GetColor(const GXColorS10& src) {
    GXColor dst;

    dst.r = ClampColor(src.r);
    dst.g = ClampColor(src.g);
    dst.b = ClampColor(src.b);
    dst.a = ClampColor(src.a);

    return dst;
}

u8 ClampColor(s16 colVal) { return CLAMP(colVal, 0, 0xff); }

template <typename charT>
int CalcLineStrNum(f32* pWidth, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length, f32 maxWidth,
                   bool* pbOver) {
    ut::Rect rect;
    ut::TextWriterBase<charT> myCopy = *pTextWriter;
    myCopy.SetCursor(0.0f, 0.0f);

    int ret = CalcLineRectImpl(&rect, &myCopy, str, length, maxWidth, pbOver);

    *pWidth = rect.GetWidth();
    return ret;
}

template <typename charT>
int CalcLineRectImpl(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                     f32 maxWidth, bool* pbOver) {
    ut::PrintContext<charT> context = {pTextWriter, str, 0.0f, 0.0f, 0};
    const ut::Font* font = pTextWriter->GetFont();

    f32 x = 0.0f;
    bool bCharSpace = false;

    ut::CharStrmReader reader = font->GetCharStrmReader();
    const charT* prStrPos = static_cast<const charT*>(reader.GetCurrentPos());

    pRect->left = 0.0f;
    pRect->right = 0.0f;
    pRect->top = ut::Min(0.0f, pTextWriter->GetLineHeight());
    pRect->bottom = ut::Max(0.0f, pTextWriter->GetLineHeight());

    *pbOver = false;
    reader.Set(str);

    ut::Rect prMaxRect = *pRect;

    for (char16_t code = reader.Next(); static_cast<const charT*>(reader.GetCurrentPos()) - str <= length;
         prStrPos = static_cast<const charT*>(reader.GetCurrentPos()), code = reader.Next(), prMaxRect = *pRect) {
        if ((int)code < L' ') {
            ut::Operation operation;
            ut::Rect rect(x, 0.0f, 0.0f, 0.0f);

            context.str = static_cast<const charT*>(reader.GetCurrentPos());
            context.flags = 0;
            context.flags |= BOOLIFY_FALSE_TERNARY(bCharSpace);

            pTextWriter->SetCursorX(x);

            operation = pTextWriter->GetTagProcessor().CalcRect(&rect, code, &context);

            reader.Set(context.str);

            pRect->left = ut::Min(pRect->left, rect.left);
            pRect->top = ut::Min(pRect->top, rect.top);
            pRect->right = ut::Max(pRect->right, rect.right);
            pRect->bottom = ut::Max(pRect->bottom, rect.bottom);

            x = pTextWriter->GetCursorX();

            if (pRect->GetWidth() > maxWidth) {
                *pbOver = true;
                break;
            }

            if (operation == ut::OPERATION_END_DRAW) {
                return length;
            } else if (operation == ut::OPERATION_NO_CHAR_SPACE) {
                bCharSpace = false;
            } else if (operation == ut::OPERATION_CHAR_SPACE) {
                bCharSpace = true;
            } else if (operation == ut::OPERATION_NEXT_LINE) {
                goto end_draw;
            }
        } else {
            if (bCharSpace) {
                x += pTextWriter->GetCharSpace();
            }

            bCharSpace = true;

            if (pTextWriter->IsWidthFixed()) {
                x += pTextWriter->GetFixedWidth();
            } else {
                x += pTextWriter->GetFont()->GetCharWidth(code) * pTextWriter->GetScaleH();
            }

            pRect->left = ut::Min(pRect->left, x);
            pRect->right = ut::Max(pRect->right, x);

            if (pRect->GetWidth() > maxWidth) {
                *pbOver = true;
                goto end_draw;
            }
        }
    }

end_draw:
    if (*pbOver && prStrPos) {
        *pRect = prMaxRect;
        return prStrPos - str;
    } else {
        return static_cast<const charT*>(reader.GetCurrentPos()) - str;
    }
}

template <typename charT>
void CalcStringRect(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                    f32 maxWidth) {
    ut::TextWriterBase<charT> myCopy = *pTextWriter;

    CalcStringRectImpl(pRect, &myCopy, str, length, maxWidth);
}

template <typename charT>
void CalcStringRectImpl(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                        f32 maxWidth) {
    int remain = length;
    const charT* pos = str;

    pRect->left = 0.0f;
    pRect->right = 0.0f;
    pRect->top = 0.0f;
    pRect->bottom = 0.0f;
    pTextWriter->SetCursor(0.0f, 0.0f);

    do {
        ut::Rect rect;
        bool bOver;
        int read = CalcLineRectImpl(&rect, pTextWriter, pos, remain, maxWidth, &bOver);

        if (bOver) {
            CalcLineRectImpl(&rect, pTextWriter, L"\n", 1, maxWidth, &bOver);
        }

        pos += read;
        remain -= read;

        pRect->left = ut::Min(pRect->left, rect.left);
        pRect->top = ut::Min(pRect->top, rect.top);
        pRect->right = ut::Max(pRect->right, rect.right);
        pRect->bottom = ut::Max(pRect->bottom, rect.bottom);
    } while (remain > 0);
}

} // unnamed namespace

namespace nw4hbm {
namespace lyt {

TextBox::TextBox(const res::TextBox* pBlock, const ResBlockSet& resBlockSet) : Pane(pBlock) {
    u16 allocStrBufLen = pBlock->textBufBytes / sizeof(wchar_t);
    if (allocStrBufLen) {
        allocStrBufLen = allocStrBufLen - 1; // NOTE: not a compound operator
    }

    Init(allocStrBufLen);

    if (pBlock->textStrBytes >= 2 && mTextBuf) {
        const wchar_t* pBlockText = detail::ConvertOffsToPtr<wchar_t>(pBlock, pBlock->textStrOffset);
        const u16 resStrLen = pBlock->textStrBytes / sizeof(wchar_t) - 1;

        SetString(pBlockText, 0, resStrLen);
    }

    for (int i = 0; i < (int)ARRAY_COUNT(mTextColors); i++) {
        mTextColors[i] = pBlock->textCols[i];
    }

    mFontSize = pBlock->fontSize;
    mTextPosition = pBlock->textPosition;
    mCharSpace = pBlock->charSpace;
    mLineSpace = pBlock->lineSpace;

    const res::Font* fonts = detail::ConvertOffsToPtr<res::Font>(resBlockSet.pFontList, sizeof *resBlockSet.pFontList);

    const char* fontName = detail::ConvertOffsToPtr<char>(fonts, fonts[pBlock->fontIdx].nameStrOffset);

    if (ut::Font* pFont = resBlockSet.pResAccessor->GetFont(fontName)) {
        mpFont = pFont;
    } else if (void* fontRes = resBlockSet.pResAccessor->GetResource('font', fontName, nullptr)) {
        if (void* pMemFont = Layout::AllocMemory(sizeof(ut::ResFont))) {
            ut::ResFont* pResFont = new (pMemFont) ut::ResFont();
            bool bSuccess ATTRIBUTE_UNUSED = pResFont->SetResource(fontRes);

            mpFont = pResFont;
            mTextBoxFlag.allocFont = true;
        }
    }

    if (void* pMemMaterial = Layout::AllocMemory(sizeof(Material))) {
        const u32* matOffsTbl = detail::ConvertOffsToPtr<u32>(resBlockSet.pMaterialList, 12);

        const res::Material* pResMaterial =
            detail::ConvertOffsToPtr<res::Material>(resBlockSet.pMaterialList, matOffsTbl[pBlock->materialIdx]);

        mpMaterial = new (pMemMaterial) Material(pResMaterial, resBlockSet);
    }
}

void TextBox::Init(u16 allocStrLen) {
    mTextBuf = nullptr;
    mTextBufBytes = 0;
    mTextLen = 0;

    mpFont = nullptr;
    mFontSize = Size(0.0f, 0.0f);

    SetTextPositionH(1);
    SetTextPositionV(1);

    mLineSpace = 0.0f;
    mCharSpace = 0.0f;
    mpTagProcessor = nullptr;

    std::memset(&mTextBoxFlag, 0, sizeof mTextBoxFlag);

    if (allocStrLen != 0) {
        AllocStringBuffer(allocStrLen);
    }
}

TextBox::~TextBox() {
    SetFont(nullptr);

    if (mpMaterial && !mpMaterial->IsUserAllocated()) {
        mpMaterial->~Material();
        Layout::FreeMemory(mpMaterial);
        mpMaterial = nullptr;
    }

    FreeStringBuffer();
}

const ut::Color TextBox::GetVtxColor(u32 idx) const { return GetTextColor(idx / 2); }

void TextBox::SetVtxColor(u32 idx, ut::Color value) { SetTextColor(idx / 2, value); }

void TextBox::SetTextColor(u32 type, ut::Color value) { mTextColors[type] = value; }

u8 TextBox::GetVtxColorElement(u32 idx) const { return reinterpret_cast<const u8*>(mTextColors + idx / 8)[idx % 4]; }

void TextBox::SetVtxColorElement(u32 idx, u8 value) { reinterpret_cast<u8*>(mTextColors + idx / 8)[idx % 4] = value; }

void TextBox::DrawSelf(const DrawInfo& drawInfo) {
    if (!mTextBuf || !mpFont || !mpMaterial) {
        return;
    }

    LoadMtx(drawInfo);

    ut::TextWriterBase<wchar_t> writer;
    writer.SetFont(*mpFont);
    writer.SetFontSize(mFontSize.width, mFontSize.height);
    writer.SetLineSpace(mLineSpace);
    writer.SetCharSpace(mCharSpace);

    ut::Color topCol = detail::MultipleAlpha(mTextColors[0], mGlbAlpha);
    ut::Color btmCol = detail::MultipleAlpha(mTextColors[1], mGlbAlpha);
    writer.SetGradationMode(topCol != btmCol ? ut::CharWriter::GRADMODE_V : ut::CharWriter::GRADMODE_NONE);

    writer.SetTextColor(topCol, btmCol);

    ut::Color minCol = GetColor(mpMaterial->GetTevColor(0));
    ut::Color maxCol = GetColor(mpMaterial->GetTevColor(1));
    writer.SetColorMapping(minCol, maxCol);

    if (mpTagProcessor) {
        writer.SetTagProcessor(mpTagProcessor);
    }

    writer.SetupGX();

    ut::Rect textRect = GetTextDrawRect(&writer);

    f32 hMag = GetTextMagH();
    wchar_t* strPos = mTextBuf;
    f32 textWidth = textRect.GetWidth();

    writer.SetCursor(textRect.left, textRect.top);

    for (int remain = mTextLen; remain > 0;) {
        f32 lineWidth;
        bool bOver;
        int lineStrNum = CalcLineStrNum(&lineWidth, &writer, strPos, remain, mSize.width, &bOver);
        f32 textPosX = hMag * (textWidth - lineWidth);

        writer.SetCursorX(textRect.left + textPosX);
        writer.Print(strPos, lineStrNum);

        if (bOver) {
            writer.Print(L"\n");
        }

        strPos += lineStrNum;
        remain -= lineStrNum;
    }
}

u16 TextBox::GetStringBufferLength() const {
    if (mTextBufBytes == 0) {
        return 0;
    }

    return mTextBufBytes / sizeof(wchar_t) - 1;
}

void TextBox::AllocStringBuffer(u16 minLen) {
    if (minLen == 0) {
        return;
    }

    u16 allocBytes = sizeof(wchar_t) * (minLen + 1);
    if (allocBytes <= mTextBufBytes) {
        return;
    }

    FreeStringBuffer();

    mTextBuf = static_cast<wchar_t*>(Layout::AllocMemory(allocBytes));
    if (mTextBuf) {
        mTextBufBytes = allocBytes;
    }
}

void TextBox::FreeStringBuffer() {
    if (mTextBuf) {
        Layout::FreeMemory(mTextBuf);
        mTextBuf = nullptr;
        mTextBufBytes = 0;
    }
}

u16 TextBox::SetString(const wchar_t* str, u16 dstIdx) { return SetString(str, dstIdx, std::wcslen(str)); }

u16 TextBox::SetString(const wchar_t* str, u16 dstIdx, u16 strLen) {
    if (!mTextBuf) {
        return 0;
    }

    u16 bufLen = GetStringBufferLength();
    if (dstIdx >= bufLen) {
        return 0;
    }

    const u16 cpLen = ut::Min<u16>(strLen, bufLen - dstIdx);
    std::memcpy(&mTextBuf[dstIdx], str, sizeof(wchar_t) * cpLen);

    mTextLen = dstIdx + cpLen;
    mTextBuf[mTextLen] = L'\0';

    return cpLen;
}

void TextBox::SetFont(const ut::Font* pFont) {
    if (mTextBoxFlag.allocFont) {
        mpFont->~Font();
        Layout::FreeMemory(const_cast<ut::Font*>(mpFont));
        mTextBoxFlag.allocFont = false;
    }

    mpFont = pFont;

    if (mpFont) {
        SetFontSize(Size(mpFont->GetWidth(), mpFont->GetHeight()));
    } else {
        SetFontSize(Size(0.0f, 0.0f));
    }
}

ut::Rect TextBox::GetTextDrawRect(ut::TextWriterBase<wchar_t>* pWriter) const {
    ut::Rect textRect;

    pWriter->SetCursor(0.0f, 0.0f);
    CalcStringRect(&textRect, pWriter, mTextBuf, mTextLen, mSize.width);

    math::VEC2 basePt = Pane::GetVtxPos();

    textRect.MoveTo(basePt.x + (mSize.width - textRect.GetWidth()) * GetTextMagH(),
                    basePt.y + (mSize.height - textRect.GetHeight()) * GetTextMagV());

    return textRect;
}

f32 TextBox::GetTextMagH() const {
    f32 hMag = 0.0f;

    switch (GetTextPositionH()) {
        default:
        case 0:
            hMag = 0.0f;
            break;

        case 1:
            hMag = 0.5f;
            break;

        case 2:
            hMag = 1.0f;
            break;
    }

    return hMag;
}

f32 TextBox::GetTextMagV() const {
    f32 vMag = 0.0f;

    switch (GetTextPositionV()) {
        default:
        case 0:
            vMag = 0.0f;
            break;

        case 1:
            vMag = 0.5f;
            break;

        case 2:
            vMag = 1.0f;
            break;
    }

    return vMag;
}

} // namespace lyt
} // namespace nw4hbm
