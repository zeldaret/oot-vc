#ifndef _RVL_SDK_AXFX_HOOKS_H
#define _RVL_SDK_AXFX_HOOKS_H

#include "revolution/types.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*AXFXAllocHook)(size_t size);
typedef void (*AXFXFreeHook)(void* block);

extern AXFXAllocHook __AXFXAlloc;
extern AXFXFreeHook __AXFXFree;

void AXFXSetHooks(AXFXAllocHook alloc, AXFXFreeHook free);
void AXFXGetHooks(AXFXAllocHook* alloc, AXFXFreeHook* free);

#ifdef __cplusplus
}
#endif

#endif
