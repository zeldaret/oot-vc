#ifndef _RVL_SDK_MTX_MTXVEC_H
#define _RVL_SDK_MTX_MTXVEC_H

#include "revolution/mtx/mtx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void PSMTXMultVec(const Mtx mtx, const Vec* vec, Vec* out);

#ifdef __cplusplus
}
#endif

#endif
