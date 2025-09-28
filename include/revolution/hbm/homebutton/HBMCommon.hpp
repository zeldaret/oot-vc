#ifndef RVL_SDK_HBM_HOMEBUTTON_COMMON_HPP
#define RVL_SDK_HBM_HOMEBUTTON_COMMON_HPP

#include "revolution/hbm/HBMCommon.h" // IWYU pragma: export

#include "revolution/mem/mem_allocator.h"
#include "revolution/types.h" // u32

// .sdata
extern "C" MEMAllocator* spAllocator;

// library internals
void* HBMAllocMem(u32 length);
void HBMFreeMem(void* ptr);

#endif // RVL_SDK_HBM_HOMEBUTTON_COMMON_HPP
