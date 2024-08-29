#ifndef _METROTRK_MUTEX_TRK_H
#define _METROTRK_MUTEX_TRK_H

#include "metrotrk/dserror.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Unused?
typedef struct TRKMutex {
    char dummy[4];
} TRKMutex;

DSError TRKReleaseMutex(TRKMutex* mutex);
DSError TRKAcquireMutex(TRKMutex* mutex);
DSError TRKInitializeMutex(TRKMutex* mutex);

#ifdef __cplusplus
}
#endif

#endif
