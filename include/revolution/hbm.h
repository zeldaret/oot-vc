#ifndef _REVOLUTION_HBM_H
#define _REVOLUTION_HBM_H

#include "revolution/kpad/KPAD.h"
#include "revolution/mtx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HBMKPadData {
    /* 0x00 */ KPADStatus* kpad;
    /* 0x04 */ Vec2 pos;
    /* 0x0C */ u32 use_devtype;
} HBMKPadData; // size = 0x10

typedef struct HBMControllerData {
    /* 0x00 */ HBMKPadData wiiCon[KPAD_MAX_CONTROLLERS];
} HBMControllerData; // size = 0x40

#ifdef __cplusplus
}
#endif

#endif
