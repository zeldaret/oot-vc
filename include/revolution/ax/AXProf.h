#ifndef _RVL_SDK_AX_PROF_H
#define _RVL_SDK_AX_PROF_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _AXPROFILE {
    /* 0x0 */ s64 timeBegin;
    /* 0x8 */ s64 timeProcessAuxBegin;
    /* 0x10 */ s64 timeProcessAuxEnd;
    /* 0x18 */ s64 timeUserFrameBegin;
    /* 0x20 */ s64 timeUserFrameEnd;
    /* 0x28 */ s64 timeEnd;
    /* 0x30 */ s32 numVoices;
    UNKWORD WORD_0x34;
} AXPROFILE;

AXPROFILE* __AXGetCurrentProfile(void);

#ifdef __cplusplus
}
#endif

#endif
