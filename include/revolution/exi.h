/**
 * References: YAGCD, publicly available patents
 */

#ifndef _RVL_SDK_PUBLIC_EXI_H
#define _RVL_SDK_PUBLIC_EXI_H

#ifdef __cplusplus
extern "C" {
#endif

#define EXI_FREQ_1M 0
#define EXI_FREQ_2M 1
#define EXI_FREQ_4M 2
#define EXI_FREQ_8M 3
#define EXI_FREQ_16M 4
#define EXI_FREQ_32M 5

#include "revolution/exi/EXIBios.h"
#include "revolution/exi/EXICommon.h"
#include "revolution/exi/EXIHardware.h"
#include "revolution/exi/EXIUart.h"

#ifdef __cplusplus
}
#endif

#endif
