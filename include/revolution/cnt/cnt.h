#ifndef _RVL_SDK_CNT_H
#define _RVL_SDK_CNT_H

#include "revolution/arc.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CNTHandle {
    /* 0x0 */ ARCHandle arcHandle;
    /* 0x1C */ s32 fd;
} CNTHandle;

typedef struct CNTFileInfo {
    /* 0x0 */ CNTHandle* handle;
    /* 0x4 */ u32 offset;
    /* 0x8 */ u32 length;
    /* 0xC */ u32 position;
} CNTFileInfo;

s32 contentFastOpenNAND(CNTHandle* handle, s32 entrynum, CNTFileInfo* info);
s32 contentConvertPathToEntrynumNAND(CNTHandle* handle, const char* path);
u32 contentGetLengthNAND(const CNTFileInfo* info);
s32 contentReadNAND(CNTFileInfo* info, void* dst, u32 len, s32 offset);
s32 contentCloseNAND(CNTFileInfo* info);

#ifdef __cplusplus
}
#endif

#endif
