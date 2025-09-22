#ifndef _RVL_SDK_GX_MISC_H
#define _RVL_SDK_GX_MISC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GXDrawSyncCallback)(u16 token);
typedef void (*GXDrawDoneCallback)(void);

void GXSetMisc(GXMiscToken token, u32 val);
void GXFlush(void);
void GXResetWriteGatherPipe(void);

void GXAbortFrame(void);

void GXDrawDone(void);
void GXPixModeSync(void);

void GXPokeZ(u16 x, u16 y, u32 z);

GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback);

#ifdef __cplusplus
}
#endif

#endif
