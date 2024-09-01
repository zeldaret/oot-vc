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

//! TODO: confirm this
typedef struct UnknownContentStruct {
    /* 0x00 */ CNTFileInfo fileInfo;
    /* 0x10 */ ARCHandle* pARCHandle;
    /* 0x14 */ u8 unk_08[0x14];
} UnknownContentStruct; // size = 0x28

bool xlCoreInitGX(void) NO_INLINE;
bool xlCoreBeforeRender(void);
bool xlCoreHiResolution(void);
bool fn_8007FC84(void);
bool xlCoreGetArgument(s32 iArgument, char** pszArgument);
s32 xlCoreGetArgumentCount(void);
bool xlCoreReset(void);
void xlExit(void);

extern GXRenderModeObj* rmode;
extern UnknownContentStruct gUnkContent;
extern CNTFileInfo gCNTFileInfo;

#ifdef __cplusplus
}
#endif

#endif
