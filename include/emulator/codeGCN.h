#ifndef _CODEGCN_H
#define _CODEGCN_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// __anon_0x57A1
typedef struct Code {
    /* 0x0 */ s32 nMode;
} Code; // size = 0x4

bool codeEvent(Code* pCode, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassCode;

#ifdef __cplusplus
}
#endif

#endif
