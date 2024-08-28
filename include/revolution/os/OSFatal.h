#ifndef _RVL_SDK_OS_FATAL_H
#define _RVL_SDK_OS_FATAL_H

#include "revolution/gx/GXTypes.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void OSFatal(GXColor textColor, GXColor bgColor, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
