#ifndef _XL_CORE_RVL_H
#define _XL_CORE_RVL_H

#include "macros.h"
#include "revolution/arc.h"
#include "revolution/cnt.h"
#include "revolution/gx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

bool xlCoreInitGX(void) NO_INLINE;
bool xlCoreBeforeRender(void);
bool xlCoreHiResolution(void);
bool fn_8007FC84(void);
bool xlCoreGetArgument(s32 iArgument, char** pszArgument);
s32 xlCoreGetArgumentCount(void);
bool xlCoreReset(void);
void xlExit(void);

extern GXRenderModeObj* rmode;

#ifdef __cplusplus
}
#endif

#endif
