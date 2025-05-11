#ifndef REVOSDK_KPAD_H
#define REVOSDK_KPAD_H

#include "revolution/mtx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KPAD_MAX_CONTROLLERS 4

typedef union KPADEXStatus {
    struct {
        /* 0x00 */ Vec2 stick;
        /* 0x08 */ Vec acc;
        /* 0x14 */ f32 acc_value;
        /* 0x18 */ f32 acc_speed;
    } fs; // size = 0x1C

    struct {
        /* 0x00 */ u32 hold;
        /* 0x04 */ u32 trig;
        /* 0x08 */ u32 release;
        /* 0x0C */ Vec2 lstick;
        /* 0x14 */ Vec2 rstick;
        /* 0x1C */ f32 ltrigger;
        /* 0x20 */ f32 rtrigger;
    } cl; // size = 0x24

    struct {
        /* 0x00 */ Vec2 stick;
        /* 0x08 */ Vec2 substick;
        /* 0x10 */ f32 ltrigger;
        /* 0x14 */ f32 rtrigger;
    } gc; // size = 0x18
} KPADEXStatus;

typedef struct KPADStatus {
    /* 0x00 */ u32 hold;
    /* 0x04 */ u32 trig;
    /* 0x08 */ u32 release;
    /* 0x0C */ Vec acc;
    /* 0x18 */ f32 acc_value;
    /* 0x1C */ f32 acc_speed;
    /* 0x20 */ Vec2 pos;
    /* 0x28 */ Vec2 vec;
    /* 0x30 */ f32 speed;
    /* 0x34 */ Vec2 horizon;
    /* 0x3C */ Vec2 hori_vec;
    /* 0x44 */ f32 hori_speed;
    /* 0x48 */ f32 dist;
    /* 0x4C */ f32 dist_vec;
    /* 0x50 */ f32 dist_speed;
    /* 0x54 */ Vec2 acc_vertical;
    /* 0x5C */ u32 dev_type;
    /* 0x60 */ KPADEXStatus ex_status;
    /* 0x84 */ s8 dpd_valid_fg;
    /* 0x85 */ s8 wpad_err;
} KPADStatus; // size = 0x88

s32 fn_800CB890(s32);

#ifdef __cplusplus
}
#endif

#endif
