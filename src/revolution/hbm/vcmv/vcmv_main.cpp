#include "revolution/gx/GXFrameBuf.h"
#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/hbm/vcmv/www.h"
#include "revolution/kpad/KPAD.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/os/OSAlarm.h"
#include "revolution/os/OSError.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/os/OSTime.h"
#include "revolution/types.h"
#include "revolution/vi/vi.h"
#include "revolution/wpad/WPAD.h"

#include <string.h>

extern "C" void fn_800CAFB8(int index, KPADStatus* pStatus, UNKWORD);
extern "C" void fn_800C8158();

static s32 lbl_8025D2B8;
static bool lbl_8025D2BC;
static bool lbl_8025D2BD;
volatile bool lbl_8025D2BE;
volatile bool lbl_8025D2BF;
bool lbl_8025D2C0;
bool lbl_8025D2C1;
u8 lbl_8025D2C2;
UNKWORD lbl_8025D2C4;
MEMAllocator* lbl_8025D2C8;
MEMAllocator* lbl_8025D2CC;
static Callback_8025D2D0 lbl_8025D2D0;
u8 lbl_8025D2D4;
volatile UNKWORD lbl_8025D2D8;
bool lbl_8025D2DC;
UNKWORD lbl_8025D2E0;
static s32 lbl_8025D2E4;
UNKWORD lbl_8025D2E8;
static s32 lbl_8025D2EC;
static bool lbl_8025D2F0;
static u8 lbl_8025D2F1;
static u8 lbl_8025D2F2;
static s32 lbl_8025D2F4;
static s32 lbl_8025D2F8;
UNKWORD lbl_8025D2FC;
static BOOL lbl_8025D300;
static s32 lbl_8025D304;

HBMControllerData lbl_801CA670;
VCMV_VOLATILE UnkStruct_801CA6B0 lbl_801CA6B0[4];
static KPADStatus lbl_801CA800[4][10];
static OSThread lbl_801CBD40;

static bool lbl_8025C900 = true;

static inline void UnknownInline1(void* block) {
    if (!((uintptr_t)block & 0x30000000)) {
        MEMFreeToAllocator(lbl_8025D2C8, block);
    } else {
        MEMFreeToAllocator(lbl_8025D2CC, block);
    }
}

static inline bool UnknownInline2(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    if (*param1 == NULL) {
        *param1 = MEMAllocFromAllocator(allocator1, size);

        if (*param1 == NULL) {
            *param1 = MEMAllocFromAllocator(allocator2, size);
        }

        if (*param1 == NULL) {
            return false;
        }
    }

    return true;
}

static inline bool UnknownInline3(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    if (!UnknownInline2(param1, size, allocator1, allocator2)) {
        OSReport("AllocIfNecessary size=%p failed\n ", size);
        return false;
    }

    return true;
}

static inline f32 UnknownInline4(f32 param1, f32 param2, UNKWORD param3, UNKWORD param4, f32 param5) {
    if (param3 >= param4) {
        return param2;
    }

    f32 temp = (param2 - param1) * 3.0f / param5;
    return param1 + (temp * param3 * param3 * (param4 - param3 * 0.6666667f));
}

void VCMV_80086E38(VCMV_VOLATILE UnkStruct_801CA6B0* param1) {
    s32 var_r6;
    s32 var_r5;

    // fixes .sdata2 ordering
    (void)3.0f;
    (void)0.6666667f;
    (void)4503601774854144.0;

    var_r6 = lbl_8025D2D8 - param1->unk_40;
    var_r5 = lbl_8025D2D8 - param1->unk_44;

    if (param1->unk_08 & 0x02) {
        param1->unk_40 = lbl_8025D2D8;

        if (var_r5 < 7) {
            param1->unk_40 -= 7 - var_r5;
        }

        var_r6 = lbl_8025D2D8 - param1->unk_40;
    } else if (param1->unk_0C & 2) {
        param1->unk_44 = lbl_8025D2D8;

        if (var_r6 < 7) {
            param1->unk_44 -= 7 - var_r6;
        }

        var_r5 = lbl_8025D2D8 - param1->unk_44;
    }

    param1->unk_38 = param1->unk_20;

    if (param1->unk_04 & 0x02) {
        UNKWORD unused1 = param1->unk_40;
        UNKWORD unused2 = lbl_8025D2D8;
        param1->unk_34 = UnknownInline4(param1->unk_1C, lbl_8025D248, var_r6, 7, 343.0f);
    } else {
        param1->unk_34 = UnknownInline4(lbl_8025D248, param1->unk_1C, var_r5, 7, 343.0f);
    }
}

void VCMV_80086FDC(VCMV_VOLATILE UnkStruct_801CA6B0* param1, KPADStatus* param2) {
    param1->unk_1C = param2->pos.x * lbl_8025C8E8;
    param1->unk_20 = param2->pos.y * lbl_8025C8EC;
    param1->unk_14 = param1->unk_1C + lbl_8025C8E8;
    param1->unk_18 = param1->unk_20 + lbl_8025C8EC;
    param1->unk_2C = param2->horizon.x;
    param1->unk_30 = param2->horizon.y;

    param1->unk_04 = 0x00;

    if (param2->hold & 0x800) {
        param1->unk_04 |= 0x01;
    }

    if (param2->hold & 0x08) {
        param1->unk_04 |= 0x08;
    }

    if (param2->hold & 0x04) {
        param1->unk_04 |= 0x04;
    }

    if (param2->hold & 0x1000) {
        param1->unk_04 |= 0x10;
    }

    if (param2->hold & 0x10) {
        param1->unk_04 |= 0x20;
    }

    if (param2->hold & 0x8000) {
        param1->unk_04 |= 0x40;
    }
}

#pragma push
#pragma gen_fsel on // it won't generate the fsel otherwise
void VCMV_800870E8(VCMV_VOLATILE UnkStruct_801CA6B0* param1, KPADStatus* param2) {
    f32 temp_f0;
    f32 temp_f2;
    f32 temp_f2_2;
    f32 var_f5;

    temp_f2 = param1->unk_24 * param2->ex_status.cl.lstick.x + param1->unk_28 * -param2->ex_status.cl.lstick.y;

    var_f5 = 1.0f;
    if (temp_f2 > 0.0f) {
        var_f5 += 0.02f * temp_f2 + var_f5;
        var_f5 = 2.5f - var_f5 >= 0.0f ? var_f5 : 2.5f;
    }

    param1->unk_24 = param2->ex_status.cl.lstick.x * var_f5;
    param1->unk_28 = -param2->ex_status.cl.lstick.y * var_f5;

#if VCMV_REVISION == 1
    param1->unk_1C = param1->unk_24 * 4.0f + param1->unk_1C;
    param1->unk_20 = param1->unk_28 * 4.0f + param1->unk_20;
    param1->unk_34 = param1->unk_1C;
    param1->unk_38 = param1->unk_20;
    param1->unk_14 = param1->unk_1C + lbl_8025C8E8;
#else
    //! TODO: it was likely like above block but volatile probably makes it more funny to match
    temp_f2_2 = param1->unk_24 * 4.0f + param1->unk_1C;
    temp_f0 = param1->unk_28 * 4.0f + param1->unk_20;

    param1->unk_34 = temp_f2_2;
    param1->unk_1C = temp_f2_2;
    param1->unk_20 = temp_f0;
    param1->unk_38 = temp_f0;
    param1->unk_14 = temp_f2_2 + lbl_8025C8E8;
#endif

    param1->unk_18 = param1->unk_20 + lbl_8025C8EC;
    param1->unk_04 = 0;
    param1->unk_30 = -0.2f;
    param1->unk_2C = 0.97899997f;

    if (param2->ex_status.cl.hold & 0x10) {
        param1->unk_04 |= 0x01;
    }

    if (param2->ex_status.cl.hold & 0x01) {
        param1->unk_04 |= 0x08;
    }

    if (param2->ex_status.cl.hold & 0x4000) {
        param1->unk_04 |= 0x04;
    }

    if (param2->ex_status.cl.hold & 0x1000) {
        param1->unk_04 |= 0x10;
    }

    if (param2->ex_status.cl.hold & 0x400) {
        param1->unk_04 |= 0x20;
    }

    if (param2->ex_status.cl.hold & 0x800) {
        param1->unk_04 |= 0x40;
    }
}
#pragma pop

#if VCMV_REVISION == 2
//! TODO: fake match
#define FLOAT_CAST_HACK *(volatile f32*)&
#else
#define FLOAT_CAST_HACK
#endif

void VCMV_8008725C(int index) {
    WPADDeviceType sp8;
    WPADResult temp_r31;
    u32 temp_r26;
    u32 var_r31;
    VCMV_VOLATILE UnkStruct_801CA6B0* temp_r29;
    KPADStatus* temp_r28;

    temp_r29 = &lbl_801CA6B0[index];
    temp_r28 = &lbl_801CA800[index][0];

    temp_r29->unk_3C = temp_r28->speed;
    temp_r31 = WPADProbe(index, &sp8);
    temp_r26 = temp_r28->dev_type;
    fn_800CAFB8(index, temp_r28, 0xA);

    switch (temp_r31) {
        case WPAD_ERR_OK:
        case WPAD_ERR_COMMUNICATION_ERROR:
        case WPAD_ERR_TRANSFER:
            if (temp_r28->dev_type == WPAD_DEV_INITIALIZING || temp_r28->dev_type == WPAD_DEV_252) {
                temp_r28->dev_type = sp8;
            }

            if (temp_r28->dev_type == WPAD_DEV_INITIALIZING || temp_r28->dev_type == WPAD_DEV_252) {
                temp_r28->dev_type = temp_r26;
            }
            break;
        default:
            temp_r29->unk_00 = 0x00;
            lbl_801CA670.wiiCon[index].kpad = nullptr;
            return;
    }

    var_r31 = index == lbl_8025D2D4 ? 0x5DC : 0x1F4;

    switch (temp_r28->dev_type) {
        case WPAD_DEV_CORE:
        case WPAD_DEV_FS:
            VCMV_80086FDC(temp_r29, temp_r28);

            if (temp_r28->hold != 0 || temp_r28->speed != temp_r29->unk_3C && temp_r28->speed > 0.007f) {
                temp_r29->unk_00 += 0x32;
            }

            temp_r29->unk_50 = true;
            lbl_801CA670.wiiCon[index].use_devtype = WPAD_DEV_CORE;
            break;
        case WPAD_DEV_CLASSIC: {
            s32 value1 = lbl_8025D2D8 - temp_r29->unk_48;
            s32 value2 = lbl_8025D2D8 - temp_r29->unk_4C;

            // note: x and y not explicitely comparing against 0.0f?
            if (temp_r28->ex_status.cl.hold != 0 || temp_r28->ex_status.cl.lstick.x ||
                temp_r28->ex_status.cl.lstick.y) {
                VCMV_800870E8(temp_r29, temp_r28);
                temp_r29->unk_50 = false;
                lbl_801CA670.wiiCon[index].use_devtype = WPAD_DEV_CLASSIC;
                temp_r29->unk_48 = lbl_8025D2D8;
                temp_r29->unk_00 += 0x32;
            } else if (temp_r28->hold != 0 ||
                       temp_r28->speed != temp_r29->unk_3C &&
                           (temp_r28->speed > 0.015f || value1 > 0x0A && temp_r28->speed > 0.005f)) {
                VCMV_80086FDC(temp_r29, temp_r28);
                lbl_801CA670.wiiCon[index].use_devtype = WPAD_DEV_CORE;
                temp_r29->unk_4C = lbl_8025D2D8;
                temp_r29->unk_00 += 0x32;
            } else if (value1 > value2) {
                VCMV_80086FDC(temp_r29, temp_r28);
                temp_r29->unk_50 = true;
                lbl_801CA670.wiiCon[index].use_devtype = WPAD_DEV_CORE;
            } else {
                VCMV_800870E8(temp_r29, temp_r28);
                temp_r29->unk_50 = false;
                lbl_801CA670.wiiCon[index].use_devtype = WPAD_DEV_CLASSIC;
            }

            break;
        }
        case WPAD_DEV_NONE:
        default:
            temp_r29->unk_00 = 0x00;
            lbl_801CA670.wiiCon[index].kpad = nullptr;
            return;
    }

    if (temp_r29->unk_1C < -lbl_8025C8E8) {
        temp_r29->unk_1C = -lbl_8025C8E8;
    } else if (temp_r29->unk_1C > lbl_8025C8E8) {
        temp_r29->unk_1C = FLOAT_CAST_HACK lbl_8025C8E8;
    }

    if (temp_r29->unk_20 < -lbl_8025C8EC) {
        temp_r29->unk_20 = -lbl_8025C8EC;
    } else if (temp_r29->unk_20 > lbl_8025C8EC) {
        temp_r29->unk_20 = FLOAT_CAST_HACK lbl_8025C8EC;
    }

    lbl_801CA670.wiiCon[index].kpad = temp_r28;
    lbl_801CA670.wiiCon[index].pos.x = (temp_r29->unk_1C / lbl_8025C8E8);
    lbl_801CA670.wiiCon[index].pos.y = (temp_r29->unk_20 / lbl_8025C8EC);

    if (temp_r29->unk_00 != 0) {
        temp_r29->unk_00 = (temp_r29->unk_00 - 1);

        if (temp_r29->unk_00 > var_r31) {
            temp_r29->unk_00 = var_r31;
        }
    }

    UNKWORD unused1 = temp_r29->unk_08;
    UNKWORD unused2 = temp_r29->unk_0C;
    UNKWORD unused3 = temp_r29->unk_10;

    BOOL status = OSDisableInterrupts();
    temp_r29->unk_08 |= temp_r29->unk_04 & ~temp_r29->unk_10;
    temp_r29->unk_0C |= temp_r29->unk_10 & ~temp_r29->unk_04;
    temp_r29->unk_10 = temp_r29->unk_04;
    OSRestoreInterrupts(status);

    VCMV_80086E38(temp_r29);
}

void VCMV_80087654(void) {
    u32 var_r4;

    if (lbl_801CA6B0[lbl_8025D2D4].unk_00 == 0) {
        var_r4 = 0;

        for (int i = 0; i < 4; i++) {
            if (var_r4 < lbl_801CA6B0[i].unk_00) {
                var_r4 = lbl_801CA6B0[i].unk_00;
                lbl_8025D2D4 = i;
            }
        }
    }

    for (int var_r31 = 0; var_r31 < 4; var_r31++) {
        if (var_r31 != lbl_8025D2D4) {
            VCMV_8008725C(var_r31);
            VCMV_8008345C(var_r31);
        }
    }

    VCMV_8008725C(lbl_8025D2D4);
    VCMV_8008345C(lbl_8025D2D4);
}

void VCMV_80087734(s32 param1, s32 param2, s32 param3, s32 param4, VCMV_VOLATILE UnkStruct_801CA6B0* param5) {
    s32 temp_r0;
    bool var_r4;

    temp_r0 = lbl_8025D240 - 2;

    if (param2 > temp_r0) {
        param2 = temp_r0;
    } else if (param2 < 2) {
        param2 = 2;
    }

    temp_r0 = lbl_8025D242 - 2;
    if (param3 > temp_r0) {
        param3 = temp_r0;
    } else if (param3 < 2) {
        param3 = 2;
    }

    var_r4 = false;
    if (param2 == lbl_8025D2F8 && param3 == lbl_8025D2FC) {
        var_r4 = true;
    }

    if (param1 == 0) {
        if (lbl_8025D2F4 != 0 || !var_r4) {
            if (lbl_8025D2F0 && lbl_8025D300) {
                lbl_8025D280 = 0;

                if (lbl_8025D2D8 - lbl_8025D2E8 > 5) {
                    temp_r0 = param3 - lbl_8025D2EC;

                    if (temp_r0 * temp_r0 > 0xF) {
                        lbl_8025D2EC = param3;
                        lbl_8025D2E8 = lbl_8025D2D8;
                    }
                }
            }

            if (lbl_8025D300 && !(param5->unk_04 & 3)) {
                lbl_8025D300 = false;
                WWWSurfaceMouseEvt(2, param2, param3, 1, 0, 0);
            }

            goto block_29;
        }

    } else if (param1 == 1) {
#if VCMV_REVISION == 1
        if (!(lbl_8025D279 | lbl_8025D26E | lbl_8025D2F1 | lbl_8025D2F2) && (!lbl_8025D300 || !var_r4)) {
            lbl_8025D300 = true;
            goto block_29;
        }
#else
        if (!lbl_8025D300 || !var_r4) {
            lbl_8025D300 = true;
            goto block_29;
        }
#endif

    } else if (param1 == 2) {
#if VCMV_REVISION == 1
        if (lbl_8025D300 || !var_r4) {
            lbl_8025D300 = false;
            goto block_29;
        }
#else
        if (lbl_8025D300) {
            lbl_8025D300 = false;
            goto block_29;
        }
#endif
    } else {
    block_29:
        WWWSurfaceMouseEvt(param1, param2, param3, param4, 0, 0);
        lbl_8025D2F4 = param1;
        lbl_8025D2F8 = param2;
        lbl_8025D2FC = param3;
        lbl_8025D304 = param4;
    }

    return;
}

void VCMV_80087918(s32 param1, s32 param2, s32 param3, VCMV_VOLATILE UnkStruct_801CA6B0* param4) {
    s32 var_r31 = param2;
    s32 var_r30 = param3;

    if (param2 > lbl_8025D240 - 0x14) {
        var_r31 = lbl_8025D240 - 0x14;
    } else if (param2 < 2) {
        var_r31 = 2;
    }

    if (param3 > lbl_8025D242 - 2) {
        var_r30 = lbl_8025D242 - 2;
    } else if (param3 < 2) {
        var_r30 = 2;
    }

    if (var_r31 != param2 || var_r30 != param3) {
        VCMV_80087734(0, var_r31, var_r30, 4, param4);
    }

    WWWSurfaceWheelEvt(0, var_r31, var_r30, param1, 0);
}

void VCMV_800879E8(void) {
    static s32 lbl_8025D308 = 0;

    u8 value = 0;

#if VCMV_REVISION == 2
    if (lbl_8025D279) {
        if (lbl_8025C900) {
            lbl_8025C900 = false;
            VCMV_80087734(0, 0xA, 0xA, 4, &lbl_801CA6B0[lbl_8025D2D4]);
            VCMV_80087734(0, 0xA, 0xE4, 4, &lbl_801CA6B0[lbl_8025D2D4]);
        }
    } else {
        lbl_8025C900 = true;
    }

    if (lbl_8025D308 == lbl_8025D2D8) {
        return;
    }

    value = lbl_8025D279 | lbl_8025D26F | lbl_8025D2F1 | lbl_8025D2F2;
#endif

    if (value == 0) {
#if VCMV_REVISION == 1
        lbl_8025D308++;
#else
        lbl_8025D308 = lbl_8025D2D8;
#endif

        if (!lbl_8025D26F) {
            for (int var_r28 = 0; var_r28 < ARRAY_COUNT(lbl_801CA6B0); var_r28++) {
                VCMV_VOLATILE UnkStruct_801CA6B0* var_r29 = &lbl_801CA6B0[var_r28];

                if (var_r29->unk_00 != 0 && lbl_8025D2D4 == var_r28) {
#if VCMV_REVISION == 1
                    if (var_r29->unk_04 & 2) {
                        s32 temp_r3 = lbl_8025D2D8 - var_r29->unk_40;

                        if (temp_r3 >= 7) {
                            if (temp_r3 - 7 < 0x2C && !((temp_r3 - 7) % 4)) {
                                VCMV_80087734(2, lbl_8025D246, var_r29->unk_18, 1, var_r29);
                                VCMV_80087734(1, lbl_8025D246, var_r29->unk_18, 1, var_r29);
                            } else {
                                VCMV_80087734(0, lbl_8025D246, var_r29->unk_18, 1, var_r29);
                            }
                        }
                    } else if (var_r29->unk_0C & 2) {
                        VCMV_80087734(2, lbl_8025D246, var_r29->unk_18, 1, var_r29);
                    } else if (var_r29->unk_08 & 1) {
                        if (var_r29->unk_14 < lbl_8025D240 - 0x10) {
                            lbl_8025D2F0 = false;
                            VCMV_80087734(0, var_r29->unk_14, var_r29->unk_18, 4, var_r29);
                            VCMV_80087734(1, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                            VCMV_80087734(2, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                        } else {
                            lbl_8025D2F0 = true;
                            lbl_8025D2EC = var_r29->unk_18;
                            VCMV_80087734(0, var_r29->unk_14, var_r29->unk_18, 4, var_r29);
                            VCMV_80087734(1, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                        }
                    } else if (var_r29->unk_0C & 1) {
                        VCMV_80087734(2, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                    } else {
                        VCMV_80087734(0, var_r29->unk_14, var_r29->unk_18, 4, var_r29);
                    }

                    if (lbl_8025D279) {
                        if (lbl_8025C900) {
                            lbl_8025C900 = false;
                            VCMV_80087734(0, 0xA, 0xA, 4, var_r29);
                        }
                    } else {
                        lbl_8025C900 = true;
                    }
#else
                    if (var_r29->unk_08 & 1) {
                        if (var_r29->unk_14 < lbl_8025D240 - 0x10) {
                            lbl_8025D2F0 = false;
                            VCMV_80087734(0, var_r29->unk_14, var_r29->unk_18, 4, var_r29);
                            VCMV_80087734(1, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                            VCMV_80087734(2, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                        } else {
                            lbl_8025D2F0 = true;
                            lbl_8025D2EC = var_r29->unk_18;
                            VCMV_80087734(0, var_r29->unk_14, var_r29->unk_18, 4, var_r29);
                            VCMV_80087734(1, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                        }
                    } else if (var_r29->unk_0C & 1) {
                        VCMV_80087734(2, var_r29->unk_14, var_r29->unk_18, 1, var_r29);
                    } else {
                        VCMV_80087734(0, var_r29->unk_14, var_r29->unk_18, 4, var_r29);
                    }
#endif

                    if (lbl_8025D260 == lbl_8025C8F0 && !(lbl_8025D279 | lbl_8025D26E | lbl_8025D2F1 | lbl_8025D2F2)) {
                        if (var_r29->unk_08 & 0x20) {
                            VCMV_80083070(5);
                            lbl_8025D2F1 = true;
                        } else if (var_r29->unk_08 & 0x10) {
                            lbl_8025D2F2 = true;
                        }

                        if (var_r29->unk_08 & 4) {
                            lbl_8025D2E8 = lbl_8025D2D8;
                            lbl_8025D280 = 1;
                            VCMV_80087918(1, var_r29->unk_14, var_r29->unk_18, var_r29);
                            lbl_8025D2E4 = lbl_8025D2D8 + 0x10;
                        } else if (var_r29->unk_04 & 0x04) {
                            if (lbl_8025D2E4 < lbl_8025D2D8) {
                                lbl_8025D2E8 = lbl_8025D2D8;
                                lbl_8025D280 = 1;
                                VCMV_80087918(1, var_r29->unk_14, var_r29->unk_18, var_r29);
                                lbl_8025D2E4 = lbl_8025D2D8;
                            }
                        } else if (var_r29->unk_08 & 8) {
                            lbl_8025D2E8 = lbl_8025D2D8;
                            lbl_8025D280 = -1;
                            VCMV_80087918(-1, var_r29->unk_14, var_r29->unk_18, var_r29);
                            lbl_8025D2E4 = lbl_8025D2D8 + 0x10;
                        } else if (var_r29->unk_04 & 0x08 && lbl_8025D2E4 < lbl_8025D2D8) {
                            lbl_8025D2E8 = lbl_8025D2D8;
                            lbl_8025D280 = -1;
                            VCMV_80087918(-1, var_r29->unk_14, var_r29->unk_18, var_r29);
                            lbl_8025D2E4 = lbl_8025D2D8;
                        }
                    }

#if VCMV_REVISION == 1
                    var_r29->unk_0C = var_r29->unk_08 &= 0x40;
#else
                    var_r29->unk_0C &= 0x40;
                    var_r29->unk_08 &= 0x40;
#endif
                }
            }
        }
    }

    for (int var_r28_2 = 0; var_r28_2 < ARRAY_COUNT(lbl_801CA6B0); var_r28_2++) {
        VCMV_VOLATILE UnkStruct_801CA6B0* var_r30 = &lbl_801CA6B0[var_r28_2];

        if (lbl_8025D2D4 == var_r28_2) {
            if (var_r30->unk_08 & 0x40) {
                VCMV_80083070(3);
                lbl_8025D2BE = true;

                if (lbl_8025D2DC) {
                    lbl_8025D2DC = false;
                    lbl_8025D2E0 = lbl_8025D2D8;
                    WPADControlMotor(lbl_8025D2D4, WPAD_MOTOR_STOP);
                }
            }

#if VCMV_REVISION == 2
            var_r30->unk_0C &= ~0x40;
            var_r30->unk_08 &= ~0x40;
#endif
        } else {
            if (var_r30->unk_08 & 0x01 || var_r30->unk_08 & 0x02) {
                WPADControlMotor(lbl_8025D2D4, WPAD_MOTOR_STOP);
                lbl_8025D224 = 0x14;
                lbl_8025D2D4 = var_r28_2;

                if (var_r30->unk_50 != 0) {
                    lbl_8025D2BF = true;
                }
            }

            var_r30->unk_08 = 0;
            var_r30->unk_0C = 0;
        }
    }
}

static void* lbl_8025D30C;
static UNKWORD lbl_8025D310;
static UNKWORD lbl_8025D314;
static UNKWORD lbl_8025D318[2]; //! TODO: real?
static OSThreadQueue lbl_8025D320;
static bool lbl_8025D328;
static volatile UNKWORD lbl_8025D32C;
static volatile UNKWORD lbl_8025D330;
static volatile UNKWORD lbl_8025D334;

bool VCMV_80087E34(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    return UnknownInline3(param1, size, allocator1, allocator2);
}

void VCMV_80087EE4(void** param1) {
    if (*param1 != NULL) {
        UnknownInline1(*param1);
        *param1 = NULL;
    }
}

void VCMV_80087F3C(void) {
    const char* sp8[6];
    s32 temp_r3;

    if (WWWSurfaceInit(lbl_8025D240, lbl_8025D242, lbl_8025D240 * 4, 0, lbl_8025D230) != 0) {
        OSPanic("vcmv_main.cpp", VCMV_REVISION == 1 ? 811 : 838, "Failed to initialize WWW");
    }

    if (WWWSurfaceSetFlushCallback((void*)VCMV_800840B4, 0) != 0) {
        OSPanic("vcmv_main.cpp", VCMV_REVISION == 1 ? 814 : 841, "Failed to init flush callback for WWW");
    }

    VCMV_800889E8();

    sp8[0] = "Wii NTLG PGothic";
    sp8[1] = "Wii NTLG PGothic";
    sp8[4] = "Wii NTLG PGothic";
    sp8[3] = "Wii NTLG PGothic";
    sp8[2] = "Wii NTLG PGothic";
    sp8[5] = "Wii NTLG PGothic";

    const char* temp = "/flash/tmp/opera.arc/opera";
    temp_r3 = WWWCreateBrowser(&lbl_8025D2C4, (void*)VCMV_8008461C, sp8, temp);

    if (temp_r3 != 0) {
        OSReport("Failed to init Opera: %d, %s\n", temp_r3, temp_r3 == -1 ? "OOM" : "Failure");
        WWWSurfaceShutdown();
    } else {
        if (!lbl_8025D2C0) {
            VCMV_800867E4();
        }

        WWWAddProtocol("arc");
        lbl_8025D2C0 = true;
    }
}

void VCMV_80088090(OSAlarm* alarm, OSContext* ctx) { OSWakeupThread(&lbl_8025D320); }

void* VCMV_80088098(void* arg) {
    OSAlarm sp40;
    Mtx sp10;
    GXColor sp8;
    f32 var_f1;
    u8 var_r30;

#if VCMV_REVISION == 1
    fn_800C8158();
#endif

    memset((void*)lbl_801CA6B0, 0, sizeof(lbl_801CA6B0));
    lbl_8025D2D8 = 0x64;
    lbl_8025D2E0 = 0;
    lbl_8025D2DC = false;

    for (int i = 0; i < ARRAY_COUNT(lbl_801CA6B0); i++) {
        lbl_801CA6B0[i].unk_44 = lbl_8025D2D8 - 7;
        lbl_801CA6B0[i].unk_00 = i == lbl_8025D2D4 ? 0x7D0 : 0x1F4;
        lbl_801CA6B0[i].unk_2C = 1.0f;
    }

    lbl_8025D328 = true;
    OSInitThreadQueue(&lbl_8025D320);

    s64 period = OSMicrosecondsToTicks(1000000 / ((u32)VIGetTvFormat() == VI_TV_FMT_PAL ? 50 : 60));
    OSCreateAlarm(&sp40);
    OSSetPeriodicAlarm(&sp40, OSGetTime(), period, VCMV_80088090);
    PSMTXIdentity(sp10);
    GXLoadPosMtxImm(sp10, 0);
    VISetBlack(false);

    var_r30 = 0;

    while (lbl_8025D32C != 0) {
        lbl_8025D2D8++;

        if (lbl_8025D2BE) {
            lbl_8025D32C--;

            if (lbl_8025D32C >= lbl_8025D330) {
                var_f1 = 1.0f;
            } else {
                var_f1 = (f32)lbl_8025D32C / (f32)lbl_8025D330;
            }

            if (lbl_8025D32C >= lbl_8025D334) {
                var_r30 = 0;
            } else {
                var_r30 = 255 - (lbl_8025D32C * 255 / lbl_8025D334);
            }

            HBMSetSoundVolume(var_f1);

            *(u32*)&sp8 = 0;
            GXSetCopyClear(sp8, 0xFFFFFF);
        } else if (lbl_8025D2BF) {
            lbl_8025D2BF = false;
            lbl_8025D2DC = true;
            lbl_8025D2E0 = lbl_8025D2D8;
            WPADControlMotor(lbl_8025D2D4, WPAD_MOTOR_RUMBLE);
        }

        if (lbl_8025D2DC && lbl_8025D2D8 - lbl_8025D2E0 >= 2) {
            lbl_8025D2DC = false;
            lbl_8025D2E0 = lbl_8025D2D8;
            WPADControlMotor(lbl_8025D2D4, WPAD_MOTOR_STOP);
        }

        GXInvalidateVtxCache();
        GXInvalidateTexAll();

        if (lbl_8025D23C > 0x280) {
            VCMV_80085874();
            VCMV_80087654();
            lbl_8025D2D0(0, 0, 0x280, lbl_8025D23E, 0, var_r30);
            lbl_8025D2D0(0x280, 0, lbl_8025D23C - 0x280, lbl_8025D23E, 1, var_r30);
        } else {
            VCMV_80085874();
            VCMV_80087654();
            lbl_8025D2D0(0, 0, lbl_8025D23C, lbl_8025D23E, 1, var_r30);
        }

        HBMUpdateSoundArchivePlayer();
    }

    HBMStopSound();
    OSCancelAlarm(&sp40);
    lbl_8025D328 = false;
    return NULL;
}

char* VCMV_8008844C(Callback_8025D2D0 param1, char* param2, u8 param3) {
    void* var_r28 = NULL;

    lbl_8025D32C = 0x32;
    lbl_8025D330 = 0x1E;
    lbl_8025D334 = 0x1E;
    lbl_8025D2BE = false;
    OSEnableInterrupts();
    lbl_8025D2D4 = param3;
    lbl_8025D2D0 = param1;
    lbl_8025D22C = param2;

    if (lbl_8025D228 == 0) {
        lbl_8025D228 = param2;
    }

    lbl_8025D2B8 = 6;

    if (lbl_8025D2C1) {
        lbl_8025D2C2 = 2;
        VCMV_80087F3C();
        VCMV_80084E54();
        VCMV_80083158();
        VCMV_80084F34(lbl_8025D2B8);
        lbl_8025D2BC = true;
        lbl_8025D2A4 = false;
        VCMV_80085164();

        UnknownInline3(&var_r28, 0x4000, lbl_8025D2C8, lbl_8025D2CC);

        if (var_r28 != NULL) {
            OSCreateThread(&lbl_801CBD40, VCMV_80088098, 0, (u8*)var_r28 + 0x4000, 0x4000, 0x0E, 1);
            OSResumeThread(&lbl_801CBD40);

            do {
                if (!lbl_8025D2BE) {
                    VCMV_80085164();
                    VCMV_800879E8();
                }

                if (!lbl_8025D288) {
                    if (lbl_8025D2F1) {
                        VCMV_80086670();
                    } else if (lbl_8025D2F2) {
                        WWWPrevPage(lbl_8025D258);
                    }

                    lbl_8025D2F1 = false;
                    lbl_8025D2F2 = false;
                }

                if (!lbl_8025D2BD) {
                    lbl_8025D2BD = true;
                    VCMV_80086670();
                }
            } while (!OSIsThreadTerminated(&lbl_801CBD40));

            if (var_r28 != NULL) {
                UnknownInline1(var_r28);
            }

            WWWSurfaceShutdown();
        }
    }

    return lbl_8025D22C;
}

void VCMV_80088654(MEMAllocator* param1, MEMAllocator* param2) {
    lbl_8025D2C8 = param1;
    lbl_8025D2CC = param2;
}

void VCMV_80088660(void) { VCMV_800891B4(); }

void VCMV_80088664(void) { VCMV_80089224(); }

void VCMV_80088668(UNKWORD param1, UNKWORD param2) { VCMV_80084A34(param1, param2); }

bool VCMV_8008866C(s32 param1, s32 param2) { return VCMV_80084AAC(param1, param2); }

void VCMV_80088670(u16 param1) { VCMV_80084A40(param1); }

void VCMV_80088674(void) { VCMV_80084DF4(); }

bool VCMV_80088678(u32 param1) {
    UnknownInline3(&lbl_8025D30C, param1, lbl_8025D2CC, lbl_8025D2C8);

    if (lbl_8025D30C == NULL) {
        return false;
    }

    WWWGetBrowserAllocationFunctions(lbl_8025D30C, param1, &lbl_8025D310, &lbl_8025D314, lbl_8025D318);
    WWWSetAllocationFunctions(lbl_8025D310, lbl_8025D314, lbl_8025D318[0], lbl_8025D310, lbl_8025D314, lbl_8025D310,
                              lbl_8025D314);
    return true;
}

void VCMV_8008876C(void) {
    WWWShutdownBrowserAllocationFunctions();

    if (lbl_8025D30C != NULL) {
        UnknownInline1(lbl_8025D30C);
        lbl_8025D30C = NULL;
    }
}

void VCMV_800887C4(void* param1) { VCMV_80084A18(param1); }

char* VCMVRun(Callback_8025D2D0 param1, char* param2, u8 param3) { return VCMV_8008844C(param1, param2, param3); }

void VCMV_800887CC(char* param1) { lbl_8025D228 = param1; }

void VCMV_800887D4(UNKWORD param1) {
    lbl_8025D2BE = true;

    if (lbl_8025D32C > param1) {
        if (param1 >= 0x1E) {
            VCMV_80083070(3);
        }

        lbl_8025D334 = param1;
        lbl_8025D330 = param1;
        lbl_8025D32C = param1;
    }
}

bool VCMV_8008882C(void** param1, u32 size, MEMAllocator* allocator1, MEMAllocator* allocator2) {
    return UnknownInline3(param1, size, allocator1, allocator2);
}

void VCMV_800888DC(void** param1) {
    if (*param1 != NULL) {
        UnknownInline1(*param1);
        *param1 = NULL;
    }
}

void VCMV_80088934(void) {
    static UNKWORD lbl_8025D338;

    if (lbl_8025D2D8 == lbl_8025D338) {
        lbl_8025D2D8++;
    }

    lbl_8025D338 = lbl_8025D2D8;

    for (int var_r31 = 0; var_r31 < 4; var_r31++) {
        VCMV_8008725C(var_r31);
    }
}

void VCMV_80088994(void** param1) { VCMV_8008315C(param1); }

BOOL NETGetUniversalCalendar(OSCalendarTime* time) {
    OSTicksToCalendarTime(OSGetTick(), time);
    return true;
}
