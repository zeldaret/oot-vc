#include "math.h"
#include "revolution/pad.h"
#include "revolution/types.h"

typedef struct PADClampRegion {
    u8 minTrigger;
    u8 maxTrigger;
    s8 minStick;
    s8 maxStick;
    s8 xyStick;
    s8 minSubstick;
    s8 maxSubstick;
    s8 xySubstick;
    s8 radStick;
    s8 radSubstick;
} PADClampRegion;

// Triggers
#define MIN_TRIGGER 30 // u8 minTrigger;
#define MAX_TRIGGER 180 // u8 maxTrigger;

// Left stick
#define MIN_STICK 15 // s8 minStick;
#define MAX_STICK 72 // s8 maxStick;
#define XY_STICK 40 // s8 xyStick;

// Right stick
#define MIN_SUBSTICK 15 // s8 minSubstick;
#define MAX_SUBSTICK 59 // s8 maxSubstick;
#define XY_SUBSTICK 31 // s8 xySubstick;

// Stick radii
#define RAD_STICK 56 // s8 radStick;
#define RAD_SUBSTICK 44 // s8 radSubstick;

static inline s32 ClampStick_Inline(s8* pn, s8 min) {
    s32 var_r3;

    if ((-min < *pn) && (*pn < min)) {
        return 0;
    }

    var_r3 = *pn + min;

    if (*pn > 0) {
        var_r3 = *pn - min;
    }

    return var_r3;
}

void ClampStick(s8* px, s8* py, s8 max, s8 min) {
    s32 squared;
    s32 root;
    s32 var_r30;
    s32 var_r31;

    var_r31 = ClampStick_Inline(px, min);
    var_r30 = ClampStick_Inline(py, min);

    squared = SQ(var_r31) + SQ(var_r30);
    if ((max * max) < squared) {
        root = (f32)sqrt((f32)squared);

        var_r31 = (var_r31 * max) / root;
        var_r30 = (var_r30 * max) / root;
    }

    *px = var_r31;
    *py = var_r30;
}

inline void ClampTrigger(u8* trigger, u8 min, u8 max) {
    if (*trigger <= min) {
        *trigger = 0;
    } else {
        if (max < *trigger) {
            *trigger = max;
        }
        *trigger -= min;
    }
}

void PADClamp(PADStatus* status) {
    int i;

    for (i = 0; i < PAD_MAX_CONTROLLERS; i++, status++) {
        if (status->err != PAD_ERR_NONE) {
            continue;
        }

        ClampStick(&status->stickX, &status->stickY, RAD_STICK, MIN_STICK);
        ClampStick(&status->substickX, &status->substickY, RAD_SUBSTICK, MIN_SUBSTICK);
        ClampTrigger(&status->triggerLeft, MIN_TRIGGER, MAX_TRIGGER);
        ClampTrigger(&status->triggerRight, MIN_TRIGGER, MAX_TRIGGER);
    }
}
