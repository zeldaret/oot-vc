#include "revolution/hbm/nw4hbm/db/assert.hpp"

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/HBMConfig.h"

#include "cstdarg.hpp"

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/db/console.hpp"
#include "revolution/hbm/nw4hbm/db/directPrint.hpp"
#include "revolution/hbm/nw4hbm/db/mapFile.hpp"

#include "revolution/os/OSAlarm.h"
#include "revolution/os/OSContext.h"
#include "revolution/os/OSError.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/os/OSMemory.h"
#include "revolution/os/OSThread.h"
#include "revolution/base/PPCArch.h"
#include "revolution/vi/vi.h"

/*******************************************************************************
 * local function declarations
 */

namespace nw4hbm { namespace db
{
	static void Assertion_Printf_(char const *fmt, ...);

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
	static bool ShowMapInfoSubroutine_(u32 address, bool preCRFlag);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD

	static void ShowStack_(register_t sp) NO_INLINE;

	static OSAlarm &GetWarningAlarm_();
	static void WarningAlarmFunc_(OSAlarm *, OSContext *);
}} // namespace nw4hbm::db

/*******************************************************************************
 * variables
 */

namespace nw4hbm { namespace db
{
	static u32 sWarningTime;
	static detail::ConsoleHead *sAssertionConsole;
	static bool sDispWarningAuto = true;
}} // namespace nw4hbm::db

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace db {

static void Assertion_Printf_(char const *fmt, ...)
{
	std::va_list vlist;

	va_start(vlist, fmt);

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
	if (sAssertionConsole)
		Console_VPrintf(sAssertionConsole, fmt, vlist);
	else
		OSVReport(fmt, vlist);
#elif HBM_APP_TYPE == HBM_APP_TYPE_NAND
	if (!sAssertionConsole)
		OSVReport(fmt, vlist);
#endif // HBM_APP_TYPE

	va_end(vlist);
}

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
static bool ShowMapInfoSubroutine_(u32 address, bool preCRFlag)
{
	ensure(MapFile_Exists(), false);
	ensure(0x80000000 <= address && address <= 0x82ffffff, false);

	{ // 39f796 wants lexical_block
		u8 strBuf[260];
		bool result = MapFile_QuerySymbol(address, strBuf, sizeof strBuf);

		if (result)
		{
			if (preCRFlag)
				Assertion_Printf_("\n");

			Assertion_Printf_("%s\n", strBuf);

			return true;
		}
		else
			{ return false; }
	}
}
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD

static void ShowStack_(register_t sp)
{
	u32 i;
	register_t *p;

	Assertion_Printf_("-------------------------------- TRACE\n");
	Assertion_Printf_("Address:   BackChain   LR save\n");

	p = reinterpret_cast<register_t *>(sp);

	for (i = 0; i < 16; i++)
	{
		if (reinterpret_cast<u32>(p) == 0x00000000)
			break;

		if (reinterpret_cast<u32>(p) == 0xffffffff)
			break;

		if (!(reinterpret_cast<u32>(p) & 0x80000000))
			break;

		// clang-format off
		Assertion_Printf_("%08X:  %08X    %08X ",
		                   p,     p[0],   p[1]);
		// clang-format on

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
		if (!ShowMapInfoSubroutine_(p[1], false))
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD
			Assertion_Printf_("\n");

		p = reinterpret_cast<register_t *>(*p);
	}
}

WEAK void VPanic(char const *file, int line,
                                         char const *fmt, std::va_list vlist)
{
	register register_t stackPointer;

	asm { mr stackPointer, r1 } // not OSGetStackPointer?

	stackPointer = *reinterpret_cast<register_t *>(stackPointer);

	OSDisableInterrupts();

	VISetPreRetraceCallback(nullptr);
	VISetPostRetraceCallback(nullptr);

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
	if (sAssertionConsole)
		detail::DirectPrint_SetupFB(nullptr);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_DVD

	ShowStack_(stackPointer);

	if (sAssertionConsole)
	{
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
	}
	else
	{
		OSReport("%s:%d Panic:", file, line);
		OSVReport(fmt, vlist);
		OSReport("\n");
	}


	PPCHalt();
}

WEAK void Panic(char const *file, int line, char const *msg, ...)
{
	std::va_list vlist;

	va_start(vlist, msg);
	VPanic(file, line, msg, vlist);
	va_end(vlist);
}

WEAK void VWarning(char const *file, int line, char const *fmt,
                        std::va_list vlist)
{
	if (sAssertionConsole)
	{
		Console_Printf(sAssertionConsole, "%s:%d Warning:", file, line);
#if !defined(NDEBUG)
		Console_VPrintf(sAssertionConsole, fmt, vlist);
#endif // !defined(NDEBUG)
		Console_Printf(sAssertionConsole, "\n");

		Console_ShowLatestLine(sAssertionConsole);

		if (sDispWarningAuto)
			Assertion_ShowConsole(sWarningTime);
	}
	else
	{
		OSReport("%s:%d Warning:", file, line);
		OSVReport(fmt, vlist);
		OSReport("\n");
	}
}

WEAK void Warning(char const *file, int line, char const *msg, ...)
{
	std::va_list vlist;

	va_start(vlist, msg);
	VWarning(file, line, msg, vlist);
	va_end(vlist);
}

void Assertion_ShowConsole(u32 time)
{
	// ensure(sAssertionConsole, nullptr);

	OSAlarm &alarm = GetWarningAlarm_();
	OSCancelAlarm(&alarm);

	Console_SetVisible(sAssertionConsole, true);

	if (time)
		OSSetAlarm(&alarm, time, &WarningAlarmFunc_);
}

static OSAlarm &GetWarningAlarm_()
{
	static bool sInitializedAlarm = false;
	static OSAlarm sWarningAlarm;

	if (!sInitializedAlarm)
	{
		OSCreateAlarm(&sWarningAlarm);
		sInitializedAlarm = true;
	}

	return sWarningAlarm;
}

static void WarningAlarmFunc_(OSAlarm *, OSContext *)
{
	if (sAssertionConsole)
		Console_SetVisible(sAssertionConsole, false);
}

}} // namespace nw4hbm::db
