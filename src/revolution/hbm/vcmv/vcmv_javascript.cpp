#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/hbm/vcmv/www.h"
#include "revolution/types.h"

#include <string.h>

static UnkStruct_801CA650 lbl_801CA650;

static UnkStruct_8025D298* lbl_8025D298;
static s32 lbl_8025D29C;
static s32 lbl_8025D2A0;
bool lbl_8025D2A4;
static UNKWORD lbl_8025D2A8;
static const char* lbl_8025D2AC;
static UnkStruct_CommonFuncArg* lbl_8025D2B0;
bool lbl_8025D2B4;

static const char* lbl_8025C8F8[] = {"vcJavaScriptExt"};

static inline BOOL UnknownInline1(const char* param1, UNKWORD* param2, void* param3, UNKWORD param4,
                                  bool do_force = false) {
    s32 sp8;

    if (strcmp(param1, lbl_8025D2AC) != 0) {
        return false;
    }

    //! TODO: fake match
    if (do_force) {
        *param2 = lbl_8025D298->unk_00(lbl_8025D2A8, 0, 0, param3, param3, "", 0, &sp8);
    } else {
        *param2 = lbl_8025D298->unk_00(param4, 0, 0, param3, param3, "", 0, &sp8);
    }

    if (*param2 >= 0) {
        lbl_8025D2B0->unk_00 = 0;
        lbl_8025D2B0->value_int = sp8;
    }

    return true;
}

bool VCMV_800867DC() { return true; }

void VCMV_800867E4(void) {
    lbl_801CA650.unk_00 = lbl_8025C8F8;
    lbl_801CA650.unk_04 = 0;
    lbl_801CA650.unk_08 = (void*)VCMV_80086864;
    lbl_801CA650.unk_0C = 0;
    lbl_801CA650.unk_10 = 0;
    lbl_801CA650.unk_14 = 0;
    lbl_801CA650.unk_18 = 0;
    lbl_801CA650.unk_1C = (void*)VCMV_800867DC;
    WWWAddJSPlugin("vcJavaScriptExt.dll", &lbl_801CA650, &lbl_8025D298, (void*)VCMV_800867DC, (void*)VCMV_80086864);
    lbl_8025D29C = 0;
}

s32 VCMV_80086864(UNKWORD param1, const char* param2, UnkStruct_CommonFuncArg* param3) {
    s32 sp8;

    if (strcmp(param2, "vcJavaScriptExt") == 0) {
        if (lbl_8025D298->unk_00(param1, 0, 0, NULL, (void*)VCMV_8008691C, "", 0, &sp8) < 0) {
            return 0x0A;
        }

        param3->unk_00 = 0;
        param3->value_int = sp8;
        return 0x07;
    }

    return 0x08;
}

s32 VCMV_8008691C(UNKWORD param1, s32 param2, int param3, UNKWORD param4, UnkStruct_CommonFuncArg* param5) {
    s32* sp8;

    if (param3 != 0) {
        return 0x13;
    }

    if (lbl_8025D298->unk_04(param2, (void*)VCMV_800869A8, 0, 0, &sp8) < 0) {
        return 0x12;
    }

    param5->unk_00 = 0;
    param5->ptr = sp8;
    *sp8 = 0;
    return 0x10;
}

s32 VCMV_800869A8(s32 param1, const char* param2, UnkStruct_CommonFuncArg* param3) {
    UNKWORD var_r31;
    u8 var_r6;

    var_r6 = 0;
    lbl_8025D2AC = param2;
    lbl_8025D2A8 = param1;
    lbl_8025D2B0 = param3;

    switch (*param2) {
        case 0x46:
            var_r6 |= UnknownInline1("FadeIn", &var_r31, (void*)VCMV_80086CCC, param1, false);
            var_r6 |= UnknownInline1("FocusSound", &var_r31, (void*)VCMV_80086D08, lbl_8025D2A8, true);
            break;
        case 0x4C:
            var_r6 |= UnknownInline1("LeftScroll", &var_r31, (void*)VCMV_80086D60, param1, false);
            break;
        case 0x52:
            var_r6 |= UnknownInline1("RightScroll", &var_r31, (void*)VCMV_80086DB4, param1, false);
            break;
        case 0x53:
            var_r6 |= UnknownInline1("SelectSound", &var_r31, (void*)VCMV_80086E08, param1, false);
            break;
        default:
            break;
    }

    if (var_r6 == 0) {
        return 0x08;
    }

    if (var_r31 < 0) {
        return 0x0A;
    }

    return 0x07;
}

s32 VCMV_80086CCC(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4) {
    lbl_8025D2A0 = 0;

    if (param3 == 1) {
        lbl_8025D2A0 = param4->value_double;
    }

    lbl_8025D29C = 0;
    return 0x11;
}

s32 VCMV_80086D08(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4) {
    if (!lbl_8025D279 && lbl_8025D260 == lbl_8025C8F0 && !lbl_8025D26E) {
        lbl_8025D2B4 = true;
        VCMV_80083070(0);
    }

    return 0x11;
}

s32 VCMV_80086D60(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4) {
    lbl_8025D2A0 = 0;

    if (param3 == 1) {
        lbl_8025D2A0 = param4->value_double;
    }

    lbl_8025D29C = 1;
    VCMV_80083070(1);
    return 0x11;
}

s32 VCMV_80086DB4(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4) {
    lbl_8025D2A0 = 0;

    if (param3 == 1) {
        lbl_8025D2A0 = param4->value_double;
    }

    lbl_8025D29C = 2;
    VCMV_80083070(1);
    return 0x11;
}

s32 VCMV_80086E08(UNKWORD param1, UNKWORD param2, s32 param3, UnkStruct_CommonFuncArg* param4) {
    lbl_8025D2A4 = true;
    return 0x11;
}

UNKWORD VCMV_80086E18(void) { return lbl_8025D29C; }

UNKWORD VCMV_80086E20(void) { return lbl_8025D2A0; }

void VCMV_80086E28(void) {
    lbl_8025D29C = 0;
    lbl_8025D2A0 = 0;
}
