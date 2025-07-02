/*******************************************************************************
 * headers
 */

#include "cstdarg.hpp"
#include "macros.h"
#include "revolution.h"

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace ut {
// .data
template <typename charT> u32 TextWriterBase<charT>::mFormatBufferSize = 0x100;

// .bss
template <typename charT> charT* TextWriterBase<charT>::mFormatBuffer;

template <typename charT> TagProcessorBase<charT> TextWriterBase<charT>::mDefaultTagProcessor;
} // namespace ut
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

//! TODO: remove once matched
extern "C" void fn_8010CBAC(char*, int, ...);

namespace nw4hbm {
namespace ut {

template <typename charT>
TextWriterBase<charT>::TextWriterBase()
    : mCharSpace(0.0f), mLineSpace(0.0f), mTabWidth(4), mDrawFlag(0), mTagProcessor(&mDefaultTagProcessor) {}

template <typename charT> TextWriterBase<charT>::~TextWriterBase() {}

template <typename charT> void TextWriterBase<charT>::SetLineHeight(f32 lineHeight) {
    const Font* font = GetFont();
    int linefeed = font ? font->GetLineFeed() : 0;

    mLineSpace = lineHeight - linefeed * GetScaleV();
}

template <typename charT> f32 TextWriterBase<charT>::GetLineHeight() const {
    NW4HBMAssertPointerValid_Line(this, 241);
    const Font* font = GetFont();
    int linefeed = font ? font->GetLineFeed() : 0;

    return mLineSpace + linefeed * GetScaleV();
}

template <typename charT> void TextWriterBase<charT>::SetLineSpace(f32 lineSpace) { mLineSpace = lineSpace; }

template <typename charT> void TextWriterBase<charT>::SetCharSpace(f32 charSpace) { mCharSpace = charSpace; }

template <typename charT> f32 TextWriterBase<charT>::GetLineSpace() const { return mLineSpace; }

template <typename charT> f32 TextWriterBase<charT>::GetCharSpace() const { return mCharSpace; }

template <typename charT> void TextWriterBase<charT>::SetTabWidth(int tabWidth) { mTabWidth = tabWidth; }

template <typename charT> int TextWriterBase<charT>::GetTabWidth() const {
    NW4HBMAssertPointerValid_Line(this, 346);
    return mTabWidth;
}

template <typename charT> void TextWriterBase<charT>::SetDrawFlag(u32 flags) { mDrawFlag = flags; }

template <typename charT> u32 TextWriterBase<charT>::GetDrawFlag() const { return mDrawFlag; }

template <typename charT> void TextWriterBase<charT>::SetTagProcessor(TagProcessorBase<charT>* tagProcessor) {
    NW4HBMAssertPointerValid_Line(this, 114);
    NW4HBMAssertPointerValid_Line(tagProcessor, 115);
    mTagProcessor = tagProcessor;
}

template <typename charT> void TextWriterBase<charT>::ResetTagProcessor() { mTagProcessor = &mDefaultTagProcessor; }

template <typename charT> TagProcessorBase<charT>& TextWriterBase<charT>::GetTagProcessor() const {
    return *mTagProcessor;
}

template <typename charT> f32 TextWriterBase<charT>::CalcFormatStringWidth(const charT* format, ...) const {
    Rect rect;
    std::va_list vargs;

    va_start(vargs, format);
    CalcVStringRect(&rect, format, vargs);
    va_end(vargs);

    return rect.GetWidth();
}

template <typename charT> f32 TextWriterBase<charT>::CalcFormatStringHeight(const charT* format, ...) const {
    Rect rect;
    std::va_list vargs;

    va_start(vargs, format);
    CalcVStringRect(&rect, format, vargs);
    va_end(vargs);

    return rect.GetHeight();
}

template <typename charT>
void TextWriterBase<charT>::CalcFormatStringRect(Rect* pRect, const charT* format, ...) const {
    std::va_list vargs;

    va_start(vargs, format);
    CalcVStringRect(pRect, format, vargs);
    va_end(vargs);
}

template <typename charT>
f32 TextWriterBase<charT>::AdjustCursor(float* xOrigin, float* yOrigin, const charT* str, int length) {
    f32 textWidth = 0.0f;
    f32 textHeight = 0.0f;

    if (!IsDrawFlagSet(0x333, 0x300) && !IsDrawFlagSet(0x333, 0x0)) {
        Rect textRect;

        CalcStringRect(&textRect, str, length);
        textWidth = textRect.left + textRect.right;
        textHeight = textRect.top + textRect.bottom;
    }

    if (IsDrawFlagSet(0x30, 0x10)) {
        *xOrigin -= textWidth / 2.0f;
    } else if (IsDrawFlagSet(0x30, 0x20)) {
        *xOrigin -= textWidth;
    }

    if (IsDrawFlagSet(0x300, 0x100)) {
        *yOrigin -= textHeight / 2.0f;
    } else if (IsDrawFlagSet(0x300, 0x200)) {
        *yOrigin -= textHeight;
    }

    if (IsDrawFlagSet(0x3, 0x1)) {
        f32 width = CalcLineWidth(str, length);
        f32 offset = (textWidth - width) / 2.0f;
        SetCursorX(*xOrigin + offset);
    } else if (IsDrawFlagSet(0x3, 0x2)) {
        f32 width = CalcLineWidth(str, length);
        f32 offset = textWidth - width;
        SetCursorX(*xOrigin + offset);
    } else {
        SetCursorX(*xOrigin);
    }

    if (IsDrawFlagSet(0x300, 0x300)) {
        SetCursorY(*yOrigin);
    } else {
        SetCursorY(*yOrigin + GetFontAscent());
    }

    return textWidth;
}

template <typename charT> f32 TextWriterBase<charT>::PrintImpl(const charT* str, int length) {
    f32 xOrigin = GetCursorX();
    f32 yOrigin = GetCursorY();
    f32 orgCursorX = xOrigin;
    f32 orgCursorY = yOrigin;
    f32 xCursorAdj ATTRIBUTE_UNUSED = 0.0f;
    f32 yCursorAdj = 0.0f;
    f32 textWidth = 0.0f;
    bool bCharSpace = false;

    textWidth = AdjustCursor(&xOrigin, &yOrigin, str, length);
    xCursorAdj = orgCursorX - GetCursorX();
    yCursorAdj = orgCursorY - GetCursorY();

    PrintContext<charT> context = {this, str, xOrigin, yOrigin, 0};
    CharStrmReader reader = GetFont()->GetCharStrmReader();
    reader.Set(str);

    for (char16_t code = reader.Next(); static_cast<const charT*>(reader.GetCurrentPos()) - str <= length;
         code = reader.Next()) {
        if (code < ' ') // C0 control characters under space
        {
            context.str = static_cast<const charT*>(reader.GetCurrentPos());
            context.flags = 0;
            context.flags |= BOOLIFY_FALSE_TERNARY(bCharSpace);

            Operation operation = mTagProcessor->Process(code, &context);

            if (operation == OPERATION_NEXT_LINE) {
                NW4HBMAssertPointerValid_Line(context.str, 1137);

                if (IsDrawFlagSet(0x3, 0x1)) {
                    int remain = length - (context.str - str);
                    f32 width = CalcLineWidth(context.str, remain);
                    f32 offset = (textWidth - width) / 2.0f;

                    SetCursorX(context.xOrigin + offset);
                } else if (IsDrawFlagSet(0x3, 0x2)) {
                    int remain = length - (context.str - str);
                    f32 width = CalcLineWidth(context.str, remain);
                    f32 offset = textWidth - width;

                    SetCursorX(context.xOrigin + offset);
                } else {
                    f32 width = GetCursorX() - context.xOrigin;

                    textWidth = Max(textWidth, width);
                    SetCursorX(context.xOrigin);
                }
                bCharSpace = false;
            } else if (operation == OPERATION_NO_CHAR_SPACE) {
                bCharSpace = false;
            } else if (operation == OPERATION_CHAR_SPACE) {
                bCharSpace = true;
            } else if (operation == OPERATION_END_DRAW) {
                break;
            }

            reader.Set(context.str);
        } else {
            f32 baseY = GetCursorY();

            if (bCharSpace) {
                MoveCursorX(GetCharSpace());
            }

            bCharSpace = true;

            { // 0x4a7507 wants lexical_block
                const Font* pFont = GetFont();
                f32 adj = -pFont->GetBaselinePos() * GetScaleV();

                MoveCursorY(adj);
            }

            CharWriter::Print(code);
            SetCursorY(baseY);
        }
    }

    if (IsDrawFlagSet(0x300, 0x100) || IsDrawFlagSet(0x300, 0x200)) {
        SetCursorY(orgCursorY);
    } else {
        MoveCursorY(yCursorAdj);
    }

    return textWidth;
}

template <typename charT> void TextWriterBase<charT>::CalcStringRectImpl(Rect* pRect, const charT* str, int length) {
    int remain = length;
    const charT* pos = str;

    pRect->left = 0.0;
    pRect->right = 0.0;
    pRect->top = 0.0;
    pRect->bottom = 0.0;

    SetCursor(0.0f, 0.0f);

    do {
        Rect rect;
        int read = CalcLineRectImpl(&rect, pos, remain);

        pos += read;
        remain -= read;

        pRect->left = Min(pRect->left, rect.left);
        pRect->top = Min(pRect->top, rect.top);
        pRect->right = Max(pRect->right, rect.right);
        pRect->bottom = Max(pRect->bottom, rect.bottom);
    } while (remain > 0);
}

template <typename charT> int TextWriterBase<charT>::CalcLineRectImpl(Rect* pRect, const charT* str, int length) {
    PrintContext<charT> context = {this, str, 0.0f, 0.0f, 0};
    const Font* font = GetFont();
    f32 x = 0.0f;
    bool bCharSpace = false;
    CharStrmReader reader = font->GetCharStrmReader();

    pRect->left = 0.0f;
    pRect->right = 0.0f;
    pRect->top = Min(0.0f, GetLineHeight());
    pRect->bottom = Max(0.0f, GetLineHeight());

    reader.Set(str);

    for (char16_t code = reader.Next(); static_cast<const charT*>(reader.GetCurrentPos()) - str <= length;
         code = reader.Next()) {
        if (code < ' ') {
            Operation operation;
            Rect rect(x, 0.0f, 0.0f, 0.0f);

            context.str = static_cast<const charT*>(reader.GetCurrentPos());
            context.flags = 0;
            context.flags |= BOOLIFY_FALSE_TERNARY(bCharSpace);

            SetCursorX(x);

            operation = mTagProcessor->CalcRect(&rect, code, &context);
            reader.Set(context.str);

            pRect->left = Min(pRect->left, rect.left);
            pRect->top = Min(pRect->top, rect.top);
            pRect->right = Max(pRect->right, rect.right);
            pRect->bottom = Max(pRect->bottom, rect.bottom);

            x = GetCursorX();

            if (operation == OPERATION_END_DRAW) {
                return length;
            } else if (operation == OPERATION_NO_CHAR_SPACE) {
                bCharSpace = false;
            } else if (operation == OPERATION_CHAR_SPACE) {
                bCharSpace = true;
            } else if (operation == OPERATION_NEXT_LINE) {
                break;
            }
        } else {
            if (bCharSpace) {
                x += GetCharSpace();
            }

            bCharSpace = true;

            if (IsWidthFixed()) {
                x += GetFixedWidth();
            } else {
                x += GetFont()->GetCharWidth(code) * GetScaleH();
            }

            pRect->left = Min(pRect->left, x);
            pRect->right = Max(pRect->right, x);
        }
    }

    return static_cast<const charT*>(reader.GetCurrentPos()) - str;
}

template <typename charT> f32 TextWriterBase<charT>::CalcLineWidth(const charT* str, int length) {
    Rect rect;
    TextWriterBase<charT> myCopy(*this);

    myCopy.SetCursor(0.0f, 0.0f);
    myCopy.CalcLineRectImpl(&rect, str, length);
    return rect.GetWidth();
}

template <typename charT> charT* TextWriterBase<charT>::GetBuffer() { return mFormatBuffer; }

template <typename charT> u32 TextWriterBase<charT>::GetBufferSize() { return mFormatBufferSize; }

template <typename charT> charT* TextWriterBase<charT>::SetBuffer(u32 size) {
    charT* oldBuffer = mFormatBuffer;

    mFormatBuffer = nullptr;
    mFormatBufferSize = size;

    return oldBuffer;
}

template <typename charT> charT* TextWriterBase<charT>::SetBuffer(charT* buffer, u32 size) {
    charT* oldBuffer = mFormatBuffer;

    mFormatBuffer = buffer;
    mFormatBufferSize = size;

    return oldBuffer;
}

template <typename charT> f32 TextWriterBase<charT>::Print(const charT* str) { return Print(str, StrLen(str)); }

template <typename charT> f32 TextWriterBase<charT>::Print(const charT* str, int length) {
    NW4HBMAssertPointerValid_Line(this, 705);
    NW4HBMAssertPointerValid_Line(str, 706);
    NW4HBMAssertHeaderMinimumValue_Line(length, 0, 707);
    TextWriterBase<charT> myCopy(*this);

    f32 width = myCopy.PrintImpl(str, length);
    SetCursor(myCopy.GetCursorX(), myCopy.GetCursorY());

    return width;
}

template <typename charT> f32 TextWriterBase<charT>::VPrintf(const charT* format, std::va_list args) {
    // i did not know about alloca before this TU so thank you kiwi
    charT* buffer = mFormatBuffer ? mFormatBuffer : static_cast<charT*>(__alloca(mFormatBufferSize));

    int length = VSNPrintf(buffer, mFormatBufferSize, format, args);
    f32 width = Print(buffer, length);
    return width;
}

template <typename charT> f32 TextWriterBase<charT>::Printf(const charT* format, ...) {
    std::va_list vargs;

    va_start(vargs, format);
    f32 width = VPrintf(format, vargs);
    va_end(vlist);

    return width;
}

template <typename charT> void TextWriterBase<charT>::CalcStringRect(Rect* pRect, const charT* str) const {
    CalcStringRect(pRect, str, StrLen(str));
}

template <typename charT> void TextWriterBase<charT>::CalcStringRect(Rect* pRect, const charT* str, int length) const {
    TextWriterBase<charT> myCopy(*this);

    myCopy.CalcStringRectImpl(pRect, str, length);
}

template <typename charT> f32 TextWriterBase<charT>::CalcStringHeight(const charT* str) const {
    return CalcStringHeight(str, StrLen(str));
}

template <typename charT> f32 TextWriterBase<charT>::CalcStringHeight(const charT* str, int length) const {
    Rect rect;

    CalcStringRect(&rect, str, length);
    return rect.GetHeight();
}

template <typename charT> f32 TextWriterBase<charT>::CalcStringWidth(const charT* str) const {
    return CalcStringWidth(str, StrLen(str));
}

template <typename charT> f32 TextWriterBase<charT>::CalcStringWidth(const charT* str, int length) const {
    Rect rect;

    CalcStringRect(&rect, str, length);
    return rect.GetWidth();
}

template <typename charT>
void TextWriterBase<charT>::CalcVStringRect(Rect* pRect, const charT* format, std::va_list args) const {
    NW4HBMAssertPointerValid_Line(this, 0);
    NW4HBMAssertPointerValid_Line(format, 0);
    NW4HBMAssertPointerValid_Line(pRect, 0);
    charT* buffer = mFormatBuffer ? mFormatBuffer : static_cast<charT*>(__alloca(mFormatBufferSize));

    int length = VSNPrintf(buffer, mFormatBufferSize, format, args);
    CalcStringRect(pRect, buffer, length);
}

} // namespace ut
} // namespace nw4hbm

/*******************************************************************************
 * explicit template instantiations
 */

namespace nw4hbm {
namespace ut {
template class TextWriterBase<char>;
template class TextWriterBase<wchar_t>;
} // namespace ut
} // namespace nw4hbm
