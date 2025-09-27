#include "revolution/hbm/nw4hbm/db/assert.h"

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/HBMConfig.h"

#include "cstdarg.hpp"

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/db/console.h"
#include "revolution/hbm/nw4hbm/db/directPrint.h"
#include "revolution/hbm/nw4hbm/db/mapFile.h"

#include "revolution/base/PPCArch.h"
#include "revolution/os/OSAlarm.h"
#include "revolution/os/OSContext.h"
#include "revolution/os/OSError.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/os/OSMemory.h"
#include "revolution/os/OSThread.h"
#include "revolution/vi/vi.h"

#include "decomp.h"

/*******************************************************************************
 * local function declarations
 */

namespace nw4hbm {
namespace db {
static void Assertion_Printf_(char const* fmt, ...);

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
static bool ShowMapInfoSubroutine_(u32 address, bool preCRFlag);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD

static void ShowStack_(register_t sp) NO_INLINE;
static void WarningAlarmFunc_(OSAlarm* alarm, OSContext* ctx);
} // namespace db
} // namespace nw4hbm

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace db {
static u32 sWarningTime;
static detail::ConsoleHead* sAssertionConsole;
static bool sDispWarningAuto;
} // namespace db
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace db {

static void Assertion_Printf_(char const* fmt, ...) {
    std::va_list vlist;

    va_start(vlist, fmt);

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
    if (sAssertionConsole) {
        Console_VPrintf(sAssertionConsole, fmt, vlist);
    } else {
        OSVReport(fmt, vlist);
    }
#elif HBM_APP_TYPE == HBM_APP_TYPE_NAND
    if (!sAssertionConsole) {
        OSVReport(fmt, vlist);
    }
#endif // HBM_APP_TYPE

    va_end(vlist);
}

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
static bool ShowMapInfoSubroutine_(u32 address, bool preCRFlag) {
    u8 strBuf[260];

    ensure(MapFile_Exists(), false);
    ensure(0x80000000 > address || address <= 0x82FFFFFF, false);

    if (MapFile_QuerySymbol(address, strBuf, sizeof(strBuf))) {
        if (preCRFlag) {
            Assertion_Printf_("\n");
        }

        Assertion_Printf_("%s\n", strBuf);
        return true;
    }

    return false;
}
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD

static void ShowStack_(register_t sp) {
    u32 i;
    register_t* p;

    Assertion_Printf_("-------------------------------- TRACE\n");
    Assertion_Printf_("Address:   BackChain   LR save\n");

    p = reinterpret_cast<register_t*>(sp);

    for (i = 0; i < 16; i++) {
        if (reinterpret_cast<u32>(p) == 0x00000000) {
            break;
        }

        if (reinterpret_cast<u32>(p) == 0xFFFFFFFF) {
            break;
        }

        if (!(reinterpret_cast<u32>(p) & 0x80000000)) {
            break;
        }

        Assertion_Printf_("%08X:  %08X    %08X ", p, p[0], p[1]);

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
        if (!ShowMapInfoSubroutine_(p[1], false))
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD
        {
            Assertion_Printf_("\n");
        }

        p = reinterpret_cast<register_t*>(*p);
    }
}

WEAK void VPanic(const char* file, int line, const char* fmt, std::va_list vlist) {
    register register_t stackPointer;

    asm { mr stackPointer, r1 } // not OSGetStackPointer?

    stackPointer = *reinterpret_cast<register_t*>(stackPointer);

    OSDisableInterrupts();

    VISetPreRetraceCallback(nullptr);
    VISetPostRetraceCallback(nullptr);

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
    if (sAssertionConsole) {
        detail::DirectPrint_SetupFB(nullptr);
    }
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD

    ShowStack_(stackPointer);

    if (sAssertionConsole) {
        Console_Printf(sAssertionConsole, "%s:%d Panic:", file, line);
#if !defined(NDEBUG)
        Console_VPrintf(sAssertionConsole, fmt, vlist);
#endif // !defined(NDEBUG)
        Console_Printf(sAssertionConsole, "\n");

        Console_ShowLatestLine(sAssertionConsole);
        Console_SetVisible(sAssertionConsole, true);
#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
        Console_DrawDirect(sAssertionConsole);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD
    } else {
        OSReport("%s:%d Panic:", file, line);
        OSVReport(fmt, vlist);
        OSReport("\n");
    }

    PPCHalt();
}

/* --- */

// this is very dumb but it works to make .data match

u16 Console_GetViewHeight(ConsoleHandle console) {
    NW4HBMAssertHeaderPointerNonnull_FileLine(console, "console.h", 434);
    return console->viewLines;
}

bool Console_SetVisible(ConsoleHandle console, bool isVisible) {
    NW4HBMAssertHeaderPointerNonnull_FileLine(console, "console.h", 497);
    bool before = console->isVisible;
    console->isVisible = isVisible;
    return before;
}

s32 Console_SetViewBaseLine(ConsoleHandle console, s32 line) {
    NW4HBMAssertHeaderPointerNonnull_FileLine(console, "console.h", 557);
    s32 before = console->viewTopLine;
    console->viewTopLine = line;
    return before;
}

/* --- */

WEAK void Panic(char const* file, int line, char const* msg, ...) {
    std::va_list vlist;

    va_start(vlist, msg);
    VPanic(file, line, msg, vlist);
    va_end(vlist);
}

WEAK void VWarning(char const* file, int line, char const* fmt, std::va_list vlist) {
    if (sAssertionConsole) {
        Console_Printf(sAssertionConsole, "%s:%d Warning:", file, line);
#if !defined(NDEBUG)
        Console_VPrintf(sAssertionConsole, fmt, vlist);
#endif // !defined(NDEBUG)
        Console_Printf(sAssertionConsole, "\n");

        Console_ShowLatestLine(sAssertionConsole);
        Console_SetVisible(sAssertionConsole, true);
    } else {
        OSReport("%s:%d Warning:", file, line);
        OSVReport(fmt, vlist);
        OSReport("\n");
    }
}

WEAK void Warning(char const* file, int line, char const* msg, ...) {
    static bool sInitializedAlarm = false;
    static OSAlarm sWarningAlarm;

    std::va_list vlist;

    if (!sInitializedAlarm) {
        OSCreateAlarm(&sWarningAlarm);
        sInitializedAlarm = true;
    }

    va_start(vlist, msg);
    VWarning(file, line, msg, vlist);
    va_end(vlist);

    if (sWarningTime) {
        OSCancelAlarm(&sWarningAlarm);
        OSSetAlarm(&sWarningAlarm, sWarningTime, WarningAlarmFunc_);
    }
}

static void WarningAlarmFunc_(OSAlarm* alarm, OSContext* ctx) { Console_SetVisible(sAssertionConsole, false); }

} // namespace db
} // namespace nw4hbm
