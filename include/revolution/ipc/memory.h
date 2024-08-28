#ifndef _RVL_SDK_IPC_MEMORY_H
#define _RVL_SDK_IPC_MEMORY_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 iosCreateHeap(void* base, u32 size);
void* iosAllocAligned(s32 handle, u32 size, u32 align);
s32 iosFree(s32 handle, void* block);

#ifdef __cplusplus
}
#endif

#endif
