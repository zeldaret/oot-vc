#ifndef _RVL_SDK_NWC24_SYSTEM_H
#define _RVL_SDK_NWC24_SYSTEM_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

NWC24Err NWC24EnableLedNotification(bool enable);
NWC24Err NWC24iPrepareShutdown(void);
NWC24Err NWC24iRequestShutdown(u32 event, NWC24Err* resultOut);
bool NWC24Shutdown(bool final, u32 event);

#ifdef __cplusplus
}
#endif

#endif
