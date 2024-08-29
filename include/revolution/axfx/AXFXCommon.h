#ifndef _RVL_SDK_AXFX_COMMON_H
#define _RVL_SDK_AXFX_COMMON_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AXFX_BUS {
    /* 0x0 */ s32* left;
    /* 0x4 */ s32* right;
    /* 0x8 */ s32* surround;
} AXFX_BUS;

typedef struct AXFX_BUFFERUPDATE {
    /* 0x0 */ s32* left;
    /* 0x4 */ s32* right;
    /* 0x8 */ s32* surround;
} AXFX_BUFFERUPDATE;

typedef struct AXFX_BUFFERUPDATE_DPL2 {
    /* 0x0 */ s32* left;
    /* 0x4 */ s32* right;
    /* 0x8 */ s32* left_surround;
    /* 0xC */ s32* right_surround;
} AXFX_BUFFERUPDATE_DPL2;

#ifdef __cplusplus
}
#endif

#endif
