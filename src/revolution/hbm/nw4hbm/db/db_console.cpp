#include "revolution/hbm/nw4hbm/db/console.h"

#include "revolution/hbm/HBMAssert.hpp"
#include "revolution/hbm/nw4hbm/db/directPrint.h"
#include "revolution/os.h"

#include "decomp.h"

namespace nw4hbm {
namespace db {

static OSMutex sMutex;

static inline u8* GetTextPtr_(ConsoleHandle console, u16 line, u16 xPos) {
    return console->textBuf + xPos + (console->width + 1) * line;
}

static inline u32 CodeWidth_(u8 const* p) { return *p >= 0x81 ? sizeof(wchar_t) : sizeof(char); }

static inline u32 GetTabSize_(ConsoleHandle console) {
    s32 tab = (console->attr & /* REGISTER16_BITFIELD(12, 13) */ 0xC) >> 2;
    // EXTRACT_BIT_FIELD does not generate srawi

    return static_cast<u32>(2 << tab);
}

static inline u8 const* SearchEndOfLine_(u8 const* str) {
    while (*str != '\n' && *str != '\0') {
        str++;
    }

    return str;
}

static inline u16 GetRingUsedLines_(ConsoleHandle console) {
    NW4HBMAssertPointerNonnull_Line(console, 112);

    { // 39ac92 wants lexical_block
        s32 lines = console->printTop - console->ringTop;

        if (lines < 0) {
            lines += console->height;
        }

        return static_cast<u16>(lines);
    }
}

static inline u16 GetActiveLines_(ConsoleHandle console) {
    u16 lines = GetRingUsedLines_(console);

    if (console->printTopUsed) {
        lines++;
    }

    return lines;
}

static void TerminateLine_(ConsoleHandle console) {
    *GetTextPtr_(console, console->printTop, console->printXPos) = '\0';
}

static u8* NextLine_(ConsoleHandle console) {
    *GetTextPtr_(console, console->printTop, console->printXPos) = '\0';
    console->printXPos = 0;
    console->printTop++;
    console->printTopUsed = 0;

    if (console->printTop == console->height && !(console->attr & 2)) {
        console->printTop = 0;
    }

    if (console->printTop == console->ringTop) {
        console->ringTopLineCnt++;

        if (++console->ringTop == console->height) {
            console->ringTop = 0;
        }
    }

    return GetTextPtr_(console, console->printTop, 0);
}

static u8* PutTab_(ConsoleHandle console, u8* dstPtr) {
    u32 tabWidth = GetTabSize_(console);

    do {
        *dstPtr++ = ' ';
        console->printXPos++;

        if (console->printXPos >= console->width) {
            break;
        }
    } while (console->printXPos & (tabWidth - 1));

    return dstPtr;
}

static u32 PutChar_(ConsoleHandle console, u8 const* str, u8* dstPtr) {
    u32 codeWidth = CodeWidth_(str);
    u32 cnt;

    if (console->printXPos + codeWidth <= console->width == 0) { return false; }

    console->printXPos += codeWidth;

    for (cnt = codeWidth; cnt; cnt--) {
        *dstPtr++ = *str++;
    }

    return codeWidth;
}

static void UnlockMutex_(OSMutex* mutex) { OSUnlockMutex(mutex); }

static bool TryLockMutex_(OSMutex* mutex) {
    OSLockMutex(mutex);
    return true;
}

static void DoDrawString_(ConsoleHandle console, u32 printLine, u8 const* str, ut::TextWriterBase<char>* writer) {
    if (writer) {
        writer->Printf("%s\n", str);
    } else {
        s32 height = (s32)((u32)console->viewPosY + printLine * 10);

        DirectPrint_DrawString(console->viewPosX, height, false, "%s\n", str);
    }
}

static void DoDrawConsole_(ConsoleHandle console, ut::TextWriterBase<char>* writer) {
    s32 viewOffset;
    u16 line;
    u16 printLines;

    viewOffset = console->viewTopLine - console->ringTopLineCnt;
    printLines = 0;

    if (viewOffset < 0) {
        viewOffset = 0;
    } else if (viewOffset > GetActiveLines_(console)) {
        return;
    }

    line = static_cast<u16>(console->ringTop + viewOffset);

    if (line >= console->height) {
        line -= console->height;
    }

    while (true) {
        if (line == console->printTop && console->printTopUsed == 0) {
            break;
        }

        DoDrawString_(console, printLines, GetTextPtr_(console, line, 0), writer);

        printLines++;

        if (line == console->printTop) {
            break;
        }

        line++;

        if (line == console->height) {
            if (console->attr & 2) {
                return;
            }

            line = 0;
        }

        if (printLines >= console->viewLines) {
            return;
        }
    }
}

void Console_DrawDirect(ConsoleHandle console) {
    NW4HBMAssertPointerNonnull_Line(console, 621);

    if (DirectPrint_IsActive() && console->isVisible) {
        TryLockMutex_(&sMutex);
        int width = console->width * 6 + 12, height = console->viewLines * 10 + 4;

        DirectPrint_EraseXfb(console->viewPosX - 6, console->viewPosY - 3, width, height);
        DoDrawConsole_(console, nullptr);
        DirectPrint_StoreCache();
        UnlockMutex_(&sMutex);
    }
}

DECOMP_FORCE(NW4HBMAssertPointerNonnull_String(pConsole));
DECOMP_FORCE("illegal console handle");
DECOMP_FORCE(NW4HBMAssertPointerNonnull_String(buffer));
DECOMP_FORCE(NW4HBMAssertAligned_String(buffer, 4));

static void PrintToBuffer_(ConsoleHandle console, u8 const* str) {
    u8* storePtr;

    NW4HBMAssertPointerNonnull_Line(console, 747);
    NW4HBMAssertPointerNonnull_Line(str, 748);

    storePtr = GetTextPtr_(console, console->printTop, console->printXPos);

    while (*str) {
        if (console->attr & 2 && console->printTop == console->height) {
            break;
        }

        while (*str) // ? just use continue? am i missing something?
        {
            bool newLineFlag = false;

            if (*str == '\n') {
                str++;
                storePtr = NextLine_(console);
                break;
            }

            if (*str == '\t') {
                str++;
                storePtr = PutTab_(console, storePtr);
                console->printTopUsed = 1;
            } else {
                u32 bytes = PutChar_(console, str, storePtr);

                if (bytes) {
                    console->printTopUsed = 1;
                    str += bytes;
                    storePtr += bytes;
                } else {
                    newLineFlag = true;
                }
            }

            if (console->printXPos >= console->width) {
                newLineFlag = true;
            }

            if (newLineFlag) {
                if (console->attr & 1) {
                    str = SearchEndOfLine_(str);
                    break;
                }

                if (*str == '\n') {
                    str++;
                }

                storePtr = NextLine_(console);
                break;
            }
        }
    }
}

static void Console_PrintString_(ConsoleOutputType type, ConsoleHandle console, u8 const* str) {
    NW4HBMAssertPointerNonnull_Line(console, 843);

    if (type & CONSOLE_OUTPUT_DISPLAY) {
        OSReport("%s", str);
    }

    if (type & CONSOLE_OUTPUT_TERMINAL) {
        PrintToBuffer_(console, str);
    }
}

void Console_VFPrintf(ConsoleOutputType type, ConsoleHandle console, char const* format, std::va_list vlist) {
#if !defined(NDEBUG)
    static u8 sStrBuf[1024];

    NW4HBMAssertPointerNonnull_Line(console, 872);

    if (TryLockMutex_(&sMutex)) {
        std::vsnprintf(reinterpret_cast<char*>(sStrBuf), sizeof sStrBuf, format, vlist);

        Console_PrintString_(type, console, sStrBuf);

        UnlockMutex_(&sMutex);
    }
#endif // !defined(NDEBUG)
}

void Console_Printf(ConsoleHandle console, char const* format, ...) {
    std::va_list vlist;

    va_start(vlist, format);
#if NW4R_APP_TYPE == NW4R_APP_TYPE_DVD
    Console_VFPrintf(CONSOLE_OUTPUT_ALL, console, format, vlist);
#endif // NW4R_APP_TYPE == NW4R_APP_TYPE_DVD
    va_end(vlist);
}

s32 Console_GetTotalLines(ConsoleHandle console) {
    s32 count;

    // this is not part of this function but it's required to generate the dtor (`nw4hbm::ut::Color::~Color()`)
    // it was probably part of a function that got stripped by the linker
    ::nw4hbm::ut::Color unused;

    NW4HBMAssertPointerNonnull_Line(console, 1050);

    TryLockMutex_(&sMutex);
    count = GetActiveLines_(console) + console->ringTopLineCnt;
    UnlockMutex_(&sMutex);

    return count;
}

} // namespace db
} // namespace nw4hbm
