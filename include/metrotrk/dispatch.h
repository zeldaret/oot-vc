#ifndef _METROTRK_DISPATCH_H
#define _METROTRK_DISPATCH_H

#include "metrotrk/dserror.h"
#include "metrotrk/msgbuf.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

DSError TRKInitializeDispatcher(void);
DSError TRKDispatchMessage(TRKMessageBuffer* buf);

#ifdef __cplusplus
}
#endif

#endif
