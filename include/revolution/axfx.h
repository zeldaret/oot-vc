#ifndef _RVL_SDK_PUBLIC_AXFX_H
#define _RVL_SDK_PUBLIC_AXFX_H

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* AXFXAllocFunc(size_t);
typedef void AXFXFreeFunc(void*);

#include "revolution/axfx/AXFXChorus.h"
#include "revolution/axfx/AXFXChorusExp.h"
#include "revolution/axfx/AXFXCommon.h"
#include "revolution/axfx/AXFXDelay.h"
#include "revolution/axfx/AXFXHooks.h"
#include "revolution/axfx/AXFXLfoTable.h"
#include "revolution/axfx/AXFXReverbHi.h"
#include "revolution/axfx/AXFXReverbHiDpl2.h"
#include "revolution/axfx/AXFXReverbHiExp.h"
#include "revolution/axfx/AXFXReverbHiExpDpl2.h"
#include "revolution/axfx/AXFXSrcCoef.h"

#ifdef __cplusplus
}
#endif

#endif
