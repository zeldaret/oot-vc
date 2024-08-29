#ifndef _METROTRK_MSGHNDLR_H
#define _METROTRK_MSGHNDLR_H

#include "metrotrk/dserror.h"
#include "metrotrk/msgbuf.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

DSError TRKDoConnect(TRKMessageBuffer* buf);
DSError TRKDoDisconnect(TRKMessageBuffer* buf);
DSError TRKDoReset(TRKMessageBuffer* buf);
DSError TRKDoOverride(TRKMessageBuffer* buf);
DSError TRKDoVersions(TRKMessageBuffer* buf);
DSError TRKDoSupportMask(TRKMessageBuffer* buf);
DSError TRKDoReadMemory(TRKMessageBuffer* buf);
DSError TRKDoWriteMemory(TRKMessageBuffer* buf);
DSError TRKDoReadRegisters(TRKMessageBuffer* buf);
DSError TRKDoWriteRegisters(TRKMessageBuffer* buf);
DSError TRKDoContinue(TRKMessageBuffer* buf);
DSError TRKDoStep(TRKMessageBuffer* buf);
DSError TRKDoStop(TRKMessageBuffer* buf);
DSError TRKDoSetOption(TRKMessageBuffer* buf);

#ifdef __cplusplus
}
#endif

#endif
