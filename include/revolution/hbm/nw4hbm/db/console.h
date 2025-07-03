#ifndef NW4R_DB_CONSOLE_H
#define NW4R_DB_CONSOLE_H

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/HBMConfig.h"

#include "cstdarg.hpp" // std::va_list

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/TextWriterBase.hpp"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * types
 */

namespace nw4hbm {
namespace db {
namespace detail {
// [SPQE7T]/ISpyD.elf:.debug_info::0x39a151
struct ConsoleHead {
    u8* textBuf; // size 0x04, offset 0x00
    u16 width; // size 0x02, offset 0x04
    u16 height; // size 0x02, offset 0x06
    u16 priority; // size 0x02, offset 0x08
    u16 attr; // size 0x02, offset 0x0a
    u16 printTop; // size 0x02, offset 0x0c
    u16 pad1;
    u16 printXPos; // size 0x02, offset 0x0e
    u16 ringTop; // size 0x02, offset 0x10
    /* 2 bytes padding */
    s32 ringTopLineCnt; // size 0x04, offset 0x14
    s32 viewTopLine; // size 0x04, offset 0x18
    s16 viewPosX; // size 0x02, offset 0x1c
    s16 viewPosY; // size 0x02, offset 0x1e
    u16 viewLines; // size 0x02, offset 0x20
    bool isVisible; // size 0x01, offset 0x22
    byte_t padding_[1];
    ut::TextWriterBase<char>* writer; // size 0x04, offset 0x24
    ConsoleHead* next; // size 0x04, offset 0x28
}; // size 0x2c
} // namespace detail

// [SPQE7T]/ISpyD.elf:.debug_info::0x39a40d
enum ConsoleOutputType {
    CONSOLE_OUTPUT_NONE = 0,

    CONSOLE_OUTPUT_TERMINAL = 1 << 0,
    CONSOLE_OUTPUT_DISPLAY = 1 << 1,

    CONSOLE_OUTPUT_ALL = CONSOLE_OUTPUT_TERMINAL | CONSOLE_OUTPUT_DISPLAY
};
} // namespace db
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace db {

extern detail::ConsoleHead* sAssertionConsole;

void Console_DrawDirect(detail::ConsoleHead* console);

void Console_VFPrintf(ConsoleOutputType type, detail::ConsoleHead* console, char const* format, std::va_list vlist);

inline void Console_VPrintf(detail::ConsoleHead* console, char const* format, std::va_list vlist) {
    Console_VFPrintf(CONSOLE_OUTPUT_ALL, console, format, vlist);
}

void Console_Printf(detail::ConsoleHead* console, char const* format, ...);

WEAK void VPanic(char const* file, int line, char const* fmt, std::va_list vlist, bool halt);

s32 Console_GetTotalLines(detail::ConsoleHead* console);

inline u16 Console_GetViewHeight(detail::ConsoleHead* console) {
    NW4HBMAssertHeaderPointerNonnull_Line(433, console);

    return console->viewLines;
}

inline bool Console_SetVisible(detail::ConsoleHead* console, bool isVisible) {
    NW4HBMAssertHeaderPointerNonnull_Line(496, console);

    bool before = console->isVisible;
    console->isVisible = isVisible;
    return before;
}

inline s32 Console_SetViewBaseLine(detail::ConsoleHead* console, s32 line) {
    NW4HBMAssertHeaderPointerNonnull_Line(556, console);

    s32 before = console->viewTopLine;
    console->viewTopLine = line;
    return before;
}

inline s32 Console_ShowLatestLine(detail::ConsoleHead* console) {
    s32 baseLine = Console_GetTotalLines(console) - Console_GetViewHeight(console);

    if (baseLine < 0) {
        baseLine = 0;
    }

    Console_SetViewBaseLine(console, baseLine);

    return baseLine;
}

} // namespace db
} // namespace nw4hbm

#endif // NW4R_DB_CONSOLE_H
