#ifndef _RVL_SDK_DVD_FATAL_H
#define _RVL_SDK_DVD_FATAL_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void __DVDShowFatalMessage(void);
bool DVDSetAutoFatalMessaging(bool enable);
bool __DVDGetAutoFatalMessaging(void);
void __DVDPrintFatalMessage(void);

#ifdef __cplusplus
}
#endif

#endif
