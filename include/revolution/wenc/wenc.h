#ifndef _RVL_SDK_WENC_H
#define _RVL_SDK_WENC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WENC_FLAG_USER_INFO = (1 << 0),
} WENCFlag;

// somehow required to match RemoteSpeaker class
#pragma pack(push, 1)

typedef struct WENCInfo {
    /* 0x0 */ s32 xn;
    /* 0x4 */ s32 dl;
    /* 0x8 */ s32 qn;
    /* 0xC */ s32 dn;
    /* 0x10 */ s32 dlh;
    /* 0x14 */ s32 dlq;
    /* 0x18 */ u8 padding[8];
} WENCInfo;

#pragma pack(pop)

s32 WENCGetEncodeData(WENCInfo* info, u32 flag, const s16* pcmData, s32 samples, u8* adpcmData);

#ifdef __cplusplus
}
#endif

#endif
