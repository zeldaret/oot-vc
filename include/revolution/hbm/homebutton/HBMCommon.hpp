#ifndef RVL_SDK_HBM_HOMEBUTTON_COMMON_HPP
#define RVL_SDK_HBM_HOMEBUTTON_COMMON_HPP

#include "revolution/hbm/HBMCommon.h" // IWYU pragma: export

/*******************************************************************************
 * headers
 */

#include "revolution/mem/mem_allocator.h"
#include "revolution/types.h" // u32

/*******************************************************************************
 * external variables
 */

// .sdata
extern "C" MEMAllocator* spAllocator;

/*******************************************************************************
 * classes and functions
 */

// library internals
void* HBMAllocMem(u32 length);
void HBMFreeMem(void* ptr);

#endif // RVL_SDK_HBM_HOMEBUTTON_COMMON_HPP
