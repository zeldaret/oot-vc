#include "revolution/hbm/nw4hbm/ut/TagProcessorBase.h"

#include "revolution/hbm/nw4hbm/ut/TextWriter.h"

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace ut {

template <typename T> TagProcessorBase<T>::TagProcessorBase() {}

template <typename T> TagProcessorBase<T>::~TagProcessorBase() {}

template <typename T> Operation TagProcessorBase<T>::Process(u16 code, PrintContext<T>* context) {
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

template <typename T> Operation TagProcessorBase<T>::CalcRect(Rect* pRect, u16 code, PrintContext<T>* context) {
    NW4HBMAssertPointerValid_Line(pRect, 132);
    NW4HBMAssert_Line(code < ' ', 133);
    NW4HBMAssertPointerValid_Line(context, 134);

    switch (code) {
        case '\n': {
            TextWriterBase<T>& writer = *context->writer;

            pRect->right = writer.GetCursorX();
            pRect->top = writer.GetCursorY();

            ProcessLinefeed(context);

            pRect->left = writer.GetCursorX();
            pRect->bottom = writer.GetCursorY() + context->writer->GetFontHeight();

            pRect->Normalize();
        }
            return OPERATION_NEXT_LINE;

        case '\t': {
            TextWriterBase<T>& writer = *context->writer;

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

template <typename T> void TagProcessorBase<T>::ProcessLinefeed(PrintContext<T>* context) {
    NW4HBMAssertPointerValid_Line(context, 195);
    TextWriterBase<T>& writer = *context->writer;

    f32 x = context->xOrigin;
    f32 y = writer.GetCursorY() + writer.GetLineHeight();

    writer.SetCursor(x, y);
}

template <typename T> void TagProcessorBase<T>::ProcessTab(PrintContext<T>* context) {
    NW4HBMAssertPointerValid_Line(context, 211);
    TextWriterBase<T>& writer = *context->writer;
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
