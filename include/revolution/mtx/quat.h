#ifndef _RVL_SDK_MTX_QUAT_H
#define _RVL_SDK_MTX_QUAT_H

#include "revolution/mtx/mtx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void PSQUATMultiply(const Quaternion* a, const Quaternion* b, Quaternion* prod);
void PSQUATNormalize(const Quaternion* in, Quaternion* out);
void C_QUATMtx(Quaternion* quat, const Mtx mtx);
void C_QUATSlerp(const Quaternion* a, const Quaternion* b, Quaternion* out, f32 t);

#ifdef __cplusplus
}
#endif

#endif
