#ifndef _RVL_SDK_CNT_H
#define _RVL_SDK_CNT_H

#include "revolution/arc.h"
#include "revolution/mem.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef s32 CNTResult;

typedef struct CNTHandleNAND {
    /* 0x00 */ ARCHandle arcHandle;
    /* 0x1C */ s32 fd;
    /* 0x20 */ MEMAllocator* memAlloc;
} CNTHandleNAND; // size = 0x24

typedef struct CNTHandle {
    /* 0x00 */ CNTHandleNAND handleNAND;
    /* 0x24 */ u8 type;
} CNTHandle; // size = 0x28

typedef struct CNTFileInfoNAND {
    /* 0x00 */ CNTHandleNAND* handle;
    /* 0x04 */ u32 offset;
    /* 0x08 */ u32 length;
    /* 0x0C */ s32 position;
} CNTFileInfoNAND; // size = 0x10

s32 contentInitHandleNAND(s32 contentNum, CNTHandleNAND* handle, MEMAllocator* memAlloc);
CNTResult contentOpenNAND(CNTHandleNAND* handle, const char* path, CNTFileInfoNAND* info);
CNTResult contentFastOpenNAND(CNTHandleNAND* handle, s32 entrynum, CNTFileInfoNAND* info);
s32 contentConvertPathToEntrynumNAND(CNTHandleNAND* info, const char* path);
u32 contentGetLengthNAND(CNTFileInfoNAND* info);
CNTResult contentSeekNAND(CNTFileInfoNAND* info, u32 offset, s32 whence);
CNTResult contentReadNAND(CNTFileInfoNAND* info, void* dst, u32 len, s32 offset);
CNTResult contentCloseNAND(CNTFileInfoNAND* info);
bool contentOpenDirNAND(CNTHandleNAND* handle, const char* path, ARCDir* dir);
CNTResult contentReleaseHandleNAND(CNTHandleNAND* handle);

#ifdef __cplusplus
}
#endif

#endif
