#ifndef _RVL_SDK_SI_SISAMPLINGRATE_H
#define _RVL_SDK_SI_SISAMPLINGRATE_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SI_MAX_SAMPLE_RATE 11

void SISetSamplingRate(u32 msec);
void SIRefreshSamplingRate(void);

#ifdef __cplusplus
}
#endif

#endif
