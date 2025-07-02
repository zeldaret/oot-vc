/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution.h"

/*******************************************************************************
 * functions
 */

//! TODO: remove once matched
extern "C" void fn_8010CBAC(char*, int, ...);

namespace nw4hbm {
namespace ut {

template <typename charT> TagProcessorBase<charT>::TagProcessorBase() {}

template <typename charT> TagProcessorBase<charT>::~TagProcessorBase() {}

template <typename charT> Operation TagProcessorBase<charT>::Process(char16_t code, PrintContext<charT>* context) {
    NW4HBMAssert_Line(code < ' ', 85);
    NW4HBMAssertPointerValid_Line(context, 86);

    switch (code) {
        case '\n':
            ProcessLinefeed(context);
            return OPERATION_NEXT_LINE;

        case '\t':
            ProcessTab(context);
            return OPERATION_NO_CHAR_SPACE;

        default:
            return OPERATION_DEFAULT;
    }
}

template <typename charT>
Operation TagProcessorBase<charT>::CalcRect(Rect* pRect, char16_t code, PrintContext<charT>* context) {
    NW4HBMAssertPointerValid_Line(pRect, 132);
    NW4HBMAssert_Line(code < ' ', 133);
    NW4HBMAssertPointerValid_Line(context, 134);

    switch (code) {
        case '\n': {
            TextWriterBase<charT>& writer = *context->writer;

            pRect->right = writer.GetCursorX();
            pRect->top = writer.GetCursorY();

            ProcessLinefeed(context);

            pRect->left = writer.GetCursorX();
            pRect->bottom = writer.GetCursorY() + context->writer->GetFontHeight();

            pRect->Normalize();
        }
            return OPERATION_NEXT_LINE;

        case '\t': {
            TextWriterBase<charT>& writer = *context->writer;

            pRect->left = writer.GetCursorX();

            ProcessTab(context);

            pRect->right = writer.GetCursorX();
            pRect->top = writer.GetCursorY();
            pRect->bottom = pRect->top + writer.GetFontHeight();

            pRect->Normalize();
        }
            return OPERATION_NO_CHAR_SPACE;

        default:
            return OPERATION_DEFAULT;
    }
}

template <typename charT> void TagProcessorBase<charT>::ProcessLinefeed(PrintContext<charT>* context) {
    NW4HBMAssertPointerValid_Line(context, 195);
    TextWriterBase<charT>& writer = *context->writer;

    f32 x = context->xOrigin;
    f32 y = writer.GetCursorY() + writer.GetLineHeight();

    writer.SetCursor(x, y);
}

template <typename charT> void TagProcessorBase<charT>::ProcessTab(PrintContext<charT>* context) {
    NW4HBMAssertPointerValid_Line(context, 211);
    TextWriterBase<charT>& writer = *context->writer;
    int tabWidth = writer.GetTabWidth();

    if (tabWidth > 0) {
        f32 aCharWidth = writer.IsWidthFixed() ? writer.GetFixedWidth() : writer.GetFontWidth();
        f32 dx = writer.GetCursorX() - context->xOrigin;
        f32 tabPixel = static_cast<f32>(tabWidth) * aCharWidth;
        int numTab = static_cast<int>(dx / tabPixel) + 1;
        f32 x = tabPixel * static_cast<f32>(numTab) + context->xOrigin;

        writer.SetCursorX(x);
    }
}

} // namespace ut
} // namespace nw4hbm

/*******************************************************************************
 * explicit template instantiations
 */

namespace nw4hbm {
namespace ut {
template class TagProcessorBase<char>;
template class TagProcessorBase<wchar_t>;
} // namespace ut
} // namespace nw4hbm
