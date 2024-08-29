#ifndef _METROTRK_MSG_H
#define _METROTRK_MSG_H

#include "metrotrk/dserror.h"
#include "metrotrk/msgbuf.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

DSError TRKMessageSend(TRKMessageBuffer* buf);

#ifdef __cplusplus
}
#endif

#endif
