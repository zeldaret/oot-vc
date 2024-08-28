#ifndef _RVL_SDK_NWC24_TIME_H
#define _RVL_SDK_NWC24_TIME_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

NWC24Err NWC24iGetUniversalTime(s64* timeOut);
NWC24Err NWC24iGetTimeDifference(s64* diffOut);
NWC24Err NWC24iSetRtcCounter(u32 rtc, u32 flags);
NWC24Err NWC24iSynchronizeRtcCounter(bool forceSave);

#ifdef __cplusplus
}
#endif

#endif
