#ifndef _RVL_SDK_OS_ALARM_H
#define _RVL_SDK_OS_ALARM_H

#include "revolution/os/OSContext.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OSAlarm OSAlarm;
typedef void (*OSAlarmHandler)(struct OSAlarm* alarm, struct OSContext* ctx);

struct OSAlarm {
    /* 0x0 */ OSAlarmHandler handler;
    /* 0x4 */ u32 tag;
    s64 end;
    /* 0x10 */ struct OSAlarm* prev;
    /* 0x14 */ struct OSAlarm* next;
    /* 0x18 */ s64 period;
    /* 0x20 */ s64 start;
    /* 0x28 */ void* userData;
};

typedef struct OSAlarmQueue {
    /* 0x0 */ OSAlarm* head;
    /* 0x4 */ OSAlarm* tail;
} OSAlarmQueue;

void __OSInitAlarm(void);
void OSCreateAlarm(OSAlarm* alarm);
void OSSetAlarm(OSAlarm* alarm, s64 tick, OSAlarmHandler handler);
void OSSetPeriodicAlarm(OSAlarm* alarm, s64 tick, s64 period, OSAlarmHandler handler);
void OSCancelAlarm(OSAlarm* alarm);
void OSSetAlarmTag(OSAlarm* alarm, u32 tag);
void OSSetAlarmUserData(OSAlarm* alarm, void* userData);
void* OSGetAlarmUserData(const OSAlarm* alarm);

#define OSGetAlarmUserDataAny(type_, alarm_) ((type_)(OSGetAlarmUserData(alarm_)))

#define OSSetAlarmUserDataAny(alarm_, data_) OSSetAlarmUserData(alarm_, (void*)(data_))

#ifdef __cplusplus
}
#endif

#endif
