#ifndef _METROTRK_NUBEVENT_H
#define _METROTRK_NUBEVENT_H

#include "metrotrk/dserror.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    kEventNone,
    kEventWait,
    kEventMessage,
    kEventException,
    kEventBreakpoint,
    kEventSupportRequest
} TRKEventType;

typedef struct TRKEvent {
    /* 0x0 */ TRKEventType type;
    /* 0x4 */ unsigned int id;
    /* 0x8 */ int buffer;
} TRKEvent;

void TRKDestructEvent(TRKEvent* event);
void TRKConstructEvent(TRKEvent* event, TRKEventType type);
DSError TRKPostEvent(TRKEvent* event);
bool TRKGetNextEvent(TRKEvent* event);
DSError TRKInitializeEventQueue(TRKEvent* event);

#ifdef __cplusplus
}
#endif

#endif
