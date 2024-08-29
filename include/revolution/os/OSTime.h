#ifndef _RVL_SDK_OS_TIME_H
#define _RVL_SDK_OS_TIME_H

#include "revolution/os/OSHardware.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Time base frequency = 1/4 bus clock
#define OS_TIME_SPEED (OS_BUS_CLOCK_SPEED / 4)

// OS time -> Real time
#define OS_TICKS_TO_SEC(x) ((x) / (OS_TIME_SPEED))
#define OS_TICKS_TO_MSEC(x) ((x) / (OS_TIME_SPEED / 1000))
#define OS_TICKS_TO_USEC(x) (((x) * 8) / (OS_TIME_SPEED / 125000))
#define OS_TICKS_TO_NSEC(x) (((x) * 8000) / (OS_TIME_SPEED / 125000))

// Real time -> OS time
#define OS_SEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED))
#define OS_MSEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 1000))
#define OS_USEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 125000) / 8)
#define OS_NSEC_TO_TICKS(x) ((x) * (OS_TIME_SPEED / 125000) / 8000)

// Interpret as signed to find tick delta
#define OS_TICKS_DELTA(x, y) ((s32)x - (s32)y)

#define OSTicksToCycles(ticks) (((ticks) * ((OS_CORE_CLOCK * 2) / OS_TIME_SPEED)) / 2)
#define OSTicksToSeconds(ticks) ((ticks) / OS_TIME_SPEED)
#define OSTicksToMilliseconds(ticks) ((ticks) / (OS_TIME_SPEED / 1000))
#define OSTicksToMicroseconds(ticks) (((ticks) * 8) / (OS_TIME_SPEED / 125000))
#define OSTicksToNanoseconds(ticks) (((ticks) * 8000) / (OS_TIME_SPEED / 125000))
#define OSSecondsToTicks(sec) ((sec) * OS_TIME_SPEED)
#define OSMillisecondsToTicks(msec) ((msec) * (OS_TIME_SPEED / 1000))
#define OSMicrosecondsToTicks(usec) (((usec) * (OS_TIME_SPEED / 125000)) / 8)
#define OSNanosecondsToTicks(nsec) (((nsec) * (OS_TIME_SPEED / 125000)) / 8000)

#define OSDiffTick(tick1, tick0) ((s32)(tick1) - (s32)(tick0))

#define OSRoundUp32B(v) (((u32)(v) + 31) & ~31)
#define OSRoundDown32B(x) (((u32)(x)) & ~(0x1F))

typedef struct OSCalendarTime {
    /* 0x0 */ s32 sec;
    /* 0x4 */ s32 min;
    /* 0x8 */ s32 hour;
    /* 0xC */ s32 mday;
    /* 0x10 */ s32 month;
    /* 0x14 */ s32 year;
    /* 0x18 */ s32 wday;
    /* 0x1C */ s32 yday;
    /* 0x20 */ s32 msec;
    /* 0x24 */ s32 usec;
} OSCalendarTime;

s64 OSGetTime(void);
u32 OSGetTick(void);

s64 __OSGetSystemTime(void);
s64 __OSTimeToSystemTime(s64 time);

void OSTicksToCalendarTime(s64 time, OSCalendarTime* cal);
s64 OSCalendarTimeToTicks(const OSCalendarTime* cal);

#ifdef __cplusplus
}
#endif

#endif
