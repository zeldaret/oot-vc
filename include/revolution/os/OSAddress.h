#ifndef _RVL_SDK_OS_ADDRESS_H
#define _RVL_SDK_OS_ADDRESS_H

#include "macros.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Upper words of the masks, since UIMM is only 16 bits
#define OS_CACHED_REGION_PREFIX 0x8000
#define OS_UNCACHED_REGION_PREFIX 0xC000
#define OS_PHYSICAL_MASK 0x3FFF

#define OS_BASE_CACHED (OS_CACHED_REGION_PREFIX << 16)
#define OS_BASE_UNCACHED (OS_UNCACHED_REGION_PREFIX << 16)

u8 GameChoice AT_ADDRESS(OS_BASE_CACHED | 0x30E3);
u16 __OSWirelessPadFixMode AT_ADDRESS(OS_BASE_CACHED | 0x30E0);
u32 __OSBusClock AT_ADDRESS(OS_BASE_CACHED | 0x00F8);
u32 __OSCoreClock AT_ADDRESS(OS_BASE_CACHED | 0x00FC);

static inline void* OSPhysicalToCached(u32 ofs) { return (void*)(ofs + 0x80000000); }

static inline void* OSPhysicalToUncached(u32 ofs) { return (void*)(ofs + 0xC0000000); }

static inline void* OSCachedToPhysical(const void* ofs) { return (u8*)ofs - 0x80000000; }

static inline void* OSUncachedToPhysical(const void* ofs) { return ((void*)((u8*)ofs - OS_BASE_UNCACHED)); }

#ifdef __cplusplus
}
#endif

#endif
