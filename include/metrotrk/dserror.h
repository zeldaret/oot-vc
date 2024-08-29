#ifndef _METROTRK_DSERROR_H
#define _METROTRK_DSERROR_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    kNoError = 0x0000,
    kMsgQueueFull = 0x0100,
    kMsgBufUnavailable = 0x0300,
    kMsgWriteOverflow = 0x0301,
    kMsgReadOverflow = 0x0302,
    kMsgInvalid = 0x500,
    kInvalidMemory = 0x0700,
} DSError;

#ifdef __cplusplus
}
#endif

#endif
