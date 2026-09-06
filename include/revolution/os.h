/**
 * References: YAGCD, WiiBrew, Dolphin Emulator
 */

#ifndef _RVL_SDK_PUBLIC_OS_H
#define _RVL_SDK_PUBLIC_OS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "macros.h"
#include "revolution/types.h"

#ifndef ASSERT
#define ASSERT(exp) ((void)0)
#endif

#ifndef ASSERTMSG
#if defined(__STDC_VERSION__) && (199901L <= __STDC_VERSION__) || defined(__MWERKS__) || defined(__SN__)
#define ASSERTMSG(exp, ...) ((void)0)
#else
#define ASSERTMSG(exp, msg) ((void)0)
#endif
#endif

#ifndef ASSERTMSG1
#define ASSERTMSG1(exp, msg, param1) ((void)0)
#endif
#ifndef ASSERTMSG2
#define ASSERTMSG2(exp, msg, param1, param2) ((void)0)
#endif
#ifndef ASSERTMSG3
#define ASSERTMSG3(exp, msg, param1, param2, param3) ((void)0)
#endif
#ifndef ASSERTMSG4
#define ASSERTMSG4(exp, msg, param1, param2, param3, param4) ((void)0)
#endif

#define OS_BUS_CLOCK (u32) __OSBusClock
#define OS_CORE_CLOCK __OSCoreClock
#define OS_TIMER_CLOCK (OS_BUS_CLOCK / 4)

#include "revolution/os/OS.h"
#include "revolution/os/OSAddress.h"
#include "revolution/os/OSAlarm.h"
#include "revolution/os/OSAlloc.h"
#include "revolution/os/OSArena.h"
#include "revolution/os/OSAudioSystem.h"
#include "revolution/os/OSCache.h"
#include "revolution/os/OSContext.h"
#include "revolution/os/OSError.h"
#include "revolution/os/OSExec.h"
#include "revolution/os/OSFastCast.h"
#include "revolution/os/OSFatal.h"
#include "revolution/os/OSFont.h"
#include "revolution/os/OSHardware.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/os/OSIpc.h"
#include "revolution/os/OSLink.h"
#include "revolution/os/OSMemory.h"
#include "revolution/os/OSMessage.h"
#include "revolution/os/OSMutex.h"
#include "revolution/os/OSNet.h"
#include "revolution/os/OSPlayRecord.h"
#include "revolution/os/OSReboot.h"
#include "revolution/os/OSReset.h"
#include "revolution/os/OSRtc.h"
#include "revolution/os/OSStateFlags.h"
#include "revolution/os/OSStateTM.h"
#include "revolution/os/OSSync.h"
#include "revolution/os/OSThread.h"
#include "revolution/os/OSTime.h"
#include "revolution/os/OSUtf.h"
#include "revolution/os/__ppc_eabi_init.h"

#ifdef __cplusplus
}
#endif

#endif
