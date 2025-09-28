#include "revolution/hbm/nw4hbm/lyt/textBox.h"

#include "revolution/hbm/nw4hbm/lyt/layout.h"

#include "new.hpp"

namespace {
// pretend this is nw4hbm::lyt
using namespace nw4hbm;
using namespace nw4hbm::lyt;

ut::Color GetColor(const GXColorS10& src);
s16 ClampColor(s16 colVal);

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

namespace nw4hbm {
namespace lyt {

const ut::detail::RuntimeTypeInfo TextBox::typeInfo(&Pane::typeInfo);

} // namespace lyt
} // namespace nw4hbm

namespace {

ut::Color GetColor(const GXColorS10& src) {
    GXColor dst;

    dst.r = ClampColor(src.r);
    dst.g = ClampColor(src.g);
    dst.b = ClampColor(src.b);
    dst.a = ClampColor(src.a);

    return dst;
}

s16 ClampColor(s16 colVal) {
    if (colVal < 0) {
        return 0;
    }

    if (colVal > 255) {
        return 255;
    }

    return colVal;
}

inline void TextBoxAssert(ut::TextWriterBase<wchar_t>* pTextWriter, const wchar_t* str, int length) {
    NW4HBMAssertPointerValid_Line(pTextWriter, 275);
    NW4HBMAssertPointerValid_Line(str, 276);
    NW4HBMAssertMinimumValue_Line(length, 0, 277);
}

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
    NW4HBMAssertPointerValid_Line(pTextWriter, 71);
    NW4HBMAssertPointerValid_Line(pRect, 72);
    NW4HBMAssertPointerValid_Line(str, 73);
    NW4HBMAssertMinimumValue_Line(length, 0, 74);
    ut::PrintContext<charT> context = {pTextWriter, str, 0.0f, 0.0f, 0};
    const ut::Font* font = pTextWriter->GetFont();

    f32 x = 0.0f;
    bool bCharSpace = false;

    NW4HBMAssertPointerValid_Line(font, 83);
    ut::CharStrmReader reader = font->GetCharStrmReader();

    const charT* prStrPos = (const charT*)reader.GetCurrentPos();

    pRect->left = 0.0f;
    pRect->right = 0.0f;
    pRect->top = ut::Min(0.0f, pTextWriter->GetLineHeight());
    pRect->bottom = ut::Max(0.0f, pTextWriter->GetLineHeight());

    *pbOver = false;

    reader.Set(str);
    ut::Rect prMaxRect = *pRect;
    u16 code = reader.Next();
    while (((const charT*)reader.GetCurrentPos() - str) <= length) {
        if (code < ' ') {
            ut::Operation operation;
            ut::Rect rect(x, 0.0f, 0.0f, 0.0f);

            context.str = (const charT*)reader.GetCurrentPos();
            context.flags = !bCharSpace;

            pTextWriter->SetCursorX(x);
            operation = pTextWriter->GetTagProcessor().CalcRect(&rect, code, &context);

            NW4HBMAssertPointerValid_Line(context.str, 123);
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
                break;
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
                break;
            }
        }
        prStrPos = (const charT*)reader.GetCurrentPos();
        code = reader.Next();
        prMaxRect = *pRect;
    }

    if (*pbOver) {
        if (prStrPos) {
            *pRect = prMaxRect;
            return (prStrPos - str);
        }
    }

    return ((const charT*)reader.GetCurrentPos() - str);
}

template <typename charT>
inline void CalcStringRect(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                           f32 maxWidth) {
    NW4HBMAssertPointerValid_Line(pTextWriter, 311);
    NW4HBMAssertPointerValid_Line(pRect, 312);
    NW4HBMAssertPointerValid_Line(str, 313);
    NW4HBMAssertMinimumValue_Line(length, 0, 314);
    ut::TextWriterBase<charT> myCopy = *pTextWriter;

    CalcStringRectImpl(pRect, &myCopy, str, length, maxWidth);
}

template <typename charT>
void CalcStringRectImpl(ut::Rect* pRect, ut::TextWriterBase<charT>* pTextWriter, const charT* str, int length,
                        f32 maxWidth) {
    NW4HBMAssertPointerValid_Line(pTextWriter, 218);
    NW4HBMAssertPointerValid_Line(pRect, 219);
    NW4HBMAssertPointerValid_Line(str, 220);
    NW4HBMAssertMinimumValue_Line(length, 0, 221);
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

    NW4HBMAssertPointerNonnull_Line(resBlockSet.pFontList, 395);
    NW4HBMAssert_Line(pBlock->fontIdx < resBlockSet.pFontList->fontNum, 396);

    const res::Font* fonts = detail::ConvertOffsToPtr<res::Font>(resBlockSet.pFontList, sizeof *resBlockSet.pFontList);

    const char* fontName = detail::ConvertOffsToPtr<char>(fonts, fonts[pBlock->fontIdx].nameStrOffset);

    if (ut::Font* pFont = resBlockSet.pResAccessor->GetFont(fontName)) {
        mpFont = pFont;
    } else if (void* fontRes = resBlockSet.pResAccessor->GetResource('font', fontName, nullptr)) {
        if (void* pMemFont = Layout::AllocMemory(sizeof(ut::ResFont))) {
            ut::ResFont* pResFont = new (pMemFont) ut::ResFont();
            bool bSuccess = pResFont->SetResource(fontRes);

            if (!bSuccess) {
                NW4HBMPanicMessage_Line(410, "Fail to load ResFont.");
            }

            mpFont = pResFont;
            mTextBoxFlag.allocFont = true;
        }
    }

    if (void* pMemMaterial = Layout::AllocMemory(sizeof(Material))) {
        NW4HBMAssertPointerNonnull_Line(resBlockSet.pMaterialList, 420);
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

ut::Color TextBox::GetVtxColor(u32 idx) const {
    NW4HBMAssert_Line(idx < VERTEXCOLOR_MAX, 467);
    return GetTextColor(idx / 2);
}

void TextBox::SetVtxColor(u32 idx, ut::Color value) {
    NW4HBMAssert_Line(idx < VERTEXCOLOR_MAX, 478);
    SetTextColor(idx / 2, value);
}

// void TextBox::SetTextColor(u32 type, ut::Color value) { __SetTextColor(type, value); }

u8 TextBox::GetVtxColorElement(u32 idx) const {
    NW4HBMAssert_Line(idx < ANIMTARGET_VERTEXCOLOR_MAX, 486);
    return reinterpret_cast<const u8*>(mTextColors + idx / 8)[idx % 4];
}

void TextBox::SetVtxColorElement(u32 idx, u8 value) {
    NW4HBMAssert_Line(idx < ANIMTARGET_VERTEXCOLOR_MAX, 494);
    reinterpret_cast<u8*>(mTextColors + idx / 8)[idx % 4] = value;
}

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
        TextBoxAssert(&writer, strPos, mTextLen);
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
    } else {
        NW4HBMAssert_Line(mTextBufBytes >= sizeof(wchar_t), 605);
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
        NW4HBMWarningMessage_Line(708, "mTextBuf is NULL.\n");
        return 0;
    }

    u16 bufLen = GetStringBufferLength();
    if (dstIdx >= bufLen) {
        NW4HBMWarningMessage_Line(716, "dstIdx is out of range.\n");
        return 0;
    }

    const u16 cpLen = ut::Min<u16>(strLen, bufLen - dstIdx);

    if (cpLen < strLen) {
        NW4HBMWarningMessage_Line(721, "%d character(s) droped.\n", strLen - cpLen);
    }

    std::memcpy(&mTextBuf[dstIdx], str, sizeof(wchar_t) * cpLen);

    mTextLen = dstIdx + cpLen;
    mTextBuf[mTextLen] = L'\0';

    return cpLen;
}

void TextBox::SetFont(const ut::Font* pFont) {
    if (mTextBoxFlag.allocFont) {
        NW4HBMAssertPointerNonnull_Line(mpFont, 775);
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

const ut::Rect TextBox::GetTextDrawRect(ut::TextWriterBase<wchar_t>* pWriter) const {
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
