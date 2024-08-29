#ifndef _RVL_SDK_CARD_H
#define _RVL_SDK_CARD_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CARDFileInfo {
    /* 0x0 */ s32 chan;
    /* 0x4 */ s32 fileNo;
    /* 0x8 */ s32 offset;
    /* 0xC */ s32 length;
    /* 0x10 */ u16 iBlock;
    /* 0x12 */ u16 padding;
} CARDFileInfo;

#ifdef __cplusplus
}
#endif

#endif
