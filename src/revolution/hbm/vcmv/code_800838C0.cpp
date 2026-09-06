#include "revolution/arc.h"
#include "revolution/gx.h"
#include "revolution/gx/GXTypes.h"
#include "revolution/hbm/vcmv/vcmv.h"
#include "revolution/hbm/vcmv/www.h"
#include "revolution/os.h"
#include "revolution/types.h"

#include <math.h>
#include <string.h>

static char lbl_801CA1C8[256];
static UNKWORD lbl_801CA2C8[16];
static GXTexObj lbl_801CA308[4];
static void* lbl_801CA388[4];
u8 lbl_801CA398[0x240]; // unused?
static volatile UnkStruct_801CA5D8 lbl_801CA5D8;
static ARCHandle lbl_801CA630;

f32 lbl_8025C8E8 = 304.0f;
f32 lbl_8025C8EC = 228.0f;
UNKWORD lbl_8025C8F0 = 0x0F;

char* lbl_8025D228;
char* lbl_8025D22C;
u32* lbl_8025D230;
static u32* lbl_8025D234;
static u32 lbl_8025D238;
u16 lbl_8025D23C;
u16 lbl_8025D23E;
u16 lbl_8025D240;
u16 lbl_8025D242;
static u16 lbl_8025D244;
u16 lbl_8025D246;
f32 lbl_8025D248;
static f32 lbl_8025D24C;
static f32 lbl_8025D250;
static UNKWORD lbl_8025D254;
UNKWORD lbl_8025D258;
static UNKWORD lbl_8025D25C;
UNKWORD lbl_8025D260 = lbl_8025C8F0;
static f32 lbl_8025D264;
static f32 lbl_8025D268;
static bool lbl_8025D26C;
static bool lbl_8025D26D;
bool lbl_8025D26E;
bool lbl_8025D26F;
static bool lbl_8025D270;
static bool lbl_8025D271;
static u32 lbl_8025D274;
static bool lbl_8025D278;
bool lbl_8025D279;
static bool lbl_8025D27A;
static bool lbl_8025D27B;
static u8 lbl_8025D27C;
static u8 lbl_8025D27D;
static u8 lbl_8025D27E;
static u8 lbl_8025D27F;
s8 lbl_8025D280;
static UNKWORD lbl_8025D284;
volatile bool lbl_8025D288;
static volatile bool lbl_8025D289;
static void* lbl_8025D28C;
static u8 lbl_8025D290;
static u8 lbl_8025D291;
static u8 lbl_8025D292;

static u8 lbl_801752F0[] = {
    0x4F, 0x07, 0x24, 0x78, 0x43, 0x1B, 0x38, 0x11, 0x5D, 0x70, 0x2B, 0x7F, 0x66, 0x49, 0x21, 0x55, 0x09, 0x3B, 0x14,
    0x71, 0x5E, 0x2E, 0x68, 0x45, 0x50, 0x02, 0x1F, 0x77, 0x37, 0x59, 0x6D, 0x63, 0x0B, 0x15, 0x4A, 0x2D, 0x3F, 0x01,
    0x54, 0x69, 0x5F, 0x1C, 0x0D, 0x35, 0x26, 0x76, 0x4D, 0x42, 0x03, 0x5B, 0x3A, 0x13, 0x1E, 0x6B, 0x29, 0x53, 0x48,
    0x33, 0x0A, 0x79, 0x3E, 0x60, 0x22, 0x16, 0x2C, 0x6C, 0x51, 0x46, 0x36, 0x00, 0x0C, 0x74, 0x20, 0x64, 0x2A, 0x58,
    0x17, 0x40, 0x34, 0x4C, 0x7A, 0x6F, 0x05, 0x10, 0x61, 0x25, 0x56, 0x39, 0x44, 0x1A, 0x7C, 0x6A, 0x30, 0x08, 0x4E,
    0x5C, 0x73, 0x27, 0x12, 0x41, 0x65, 0x7B, 0x1D, 0x04, 0x32, 0x6E, 0x57, 0x4B, 0x0E, 0x28, 0x7E, 0x3D, 0x75, 0x19,
    0x62, 0x06, 0x31, 0x52, 0x47, 0x23, 0x0F, 0x7D, 0x72, 0x3C, 0x18, 0x5A, 0x67, 0x2F,
};

static inline int UnknownInline1(u32 value) {
    return ((((u8*)&value)[1] & 0xF8) << 8) | ((((u8*)&value)[2] & 0xFC) << 3) | ((((u8*)&value)[3] & 0xF8) >> 3);
}

static inline void UnknownInline2(UNKWORD param1) {
    if (param1 == 0) {
        WWWCloseBrowserWindow(lbl_8025D2C4, param1);
    } else {
        WWWSetBrowserWindowRect(param1, &lbl_801CA2C8);
        WWWSetImageMode(param1, 4);
        WWWSetRenderingMode(param1, lbl_8025D254);
        WWWShowBrowserWindow(param1);

        lbl_8025D258 = param1;
        if (param1 != 0) {
            WWWRaiseBrowserWindow(param1);
        }

        WWWSetFocus(param1);
        WWWSurfaceUpdateScreen(0);
    }
}

static inline void UnknownInline3(const char* param1) {
    s32 sp8;

    if (WWWCreateBrowserWindow(lbl_8025D2C4, &sp8, 0) != 0) {
        OSReport("NO MEMORY\n");
    } else {
        if (param1 != NULL) {
            WWWOpenUrl(sp8, param1);
        }

        UnknownInline2(sp8);
    }
}

static inline void UnknownInline4() {
    lbl_8025D271 = true;
    lbl_8025D27B = false;
    lbl_8025D26D = false;

    do {
        lbl_8025D27C = (lbl_8025D27C + 1) % 4;
    } while (lbl_8025D27C == lbl_8025D27E || lbl_8025D27C == lbl_8025D27D);
}

UNKWORD VCMV_800838C0(UnkStruct_800838C0* param1, f32* param2) {
    s32 temp_r28;
    s32 temp_r27;
    s32 temp_r0;
    s32 temp_r12;
    s32 temp2;
    s32 temp;
    u32* var_r10;
    u32* var_r27;
    u32* var_r29;
    s32 var_r3;
    s32 var_r30;
    s32 var_r5;
    u8 temp_r31_2;
    u32* var_r26;
    u32* var_r25;

    temp_r27 = param1->unk_00 + param1->unk_08;
    temp_r28 = param1->unk_04 + param1->unk_0C;
    temp_r0 = (lbl_8025D240 + 0x7F) / 128;
    temp_r12 = param1->unk_00 / 128;
    temp2 = param1->unk_00 & 0x7F;
    temp = temp_r27 & 0x7F;
    var_r10 = lbl_8025D234 + param1->unk_04 * temp_r0 + temp_r12;
    var_r27 = &lbl_8025D230[param1->unk_04 * lbl_8025D240 + temp_r12 * 128];
    var_r29 = &lbl_8025D230[param1->unk_04 * lbl_8025D240 + temp_r27];

    var_r30 = 0;
    var_r5 = param1->unk_04;
    var_r3 = param1->unk_0C * (temp_r27 / 128 - temp_r12 + 1);

    while (var_r5 < temp_r28) {
        temp_r31_2 = lbl_801752F0[var_r5 & 0x7F];
        var_r26 = var_r27 + temp_r31_2;
        var_r25 = var_r10;

        if (temp_r31_2 < temp2) {
            var_r25++;
            var_r26 += 128;
            var_r3--;
        }

        while (var_r26 < var_r29) {
            if (*var_r25 != *var_r26) {
                *var_r25 = *var_r26;
                var_r30++;
            }

            var_r26 += 128;
            var_r25++;
        }

        if (temp_r31_2 >= temp) {
            var_r3--;
        }

        var_r27 += lbl_8025D240;
        var_r29 += lbl_8025D240;
        var_r10 += temp_r0;
        var_r5++;
    }

    if (var_r3 != 0) {
        *param2 = (f32)var_r30 / (f32)var_r3;
    } else {
        *param2 = 0.0f;
    }

    return var_r3;
}

bool VCMV_80083A60(UnkStruct_800838C0* param1, s32* param2) {
    u8 sp58[0x4D];
    u8 sp8[0x4D];

    s32 temp10;
    s32 var_r31;
    s32 temp_r0;
    s32 temp3;
    s32 i;
    s32 temp_r22;
    s32 temp_r23;
    s32 temp2;
    u32* var_r29;
    u32* var_r28;
    u32* var_r27;
    s32 other_temp_r27;
    s32 other_temp;
    s32 temptemp3;
    s32 index2;
    s32 k;
    u8 val;
    u32* var_r25;
    u32* var_r24;
    s32 var_r23;
    u32 temp9;

    var_r31 = 0x4D;
    temp10 = param1->unk_04 + param1->unk_0C;
    temp_r0 = lbl_8025D242 - temp10;

    if (var_r31 > temp_r0) {
        var_r31 = temp_r0;
    }

    temp3 = param1->unk_04 + var_r31;

    for (i = 0; i < var_r31; i++) {
        sp58[i] = i + 4;
        sp8[i] = 3;
    }

    other_temp_r27 = param1->unk_00 + param1->unk_08;
    temp_r22 = (lbl_8025D240 + 0x7F) / 128;
    temp_r23 = param1->unk_00 / 128;
    temp2 = param1->unk_00 & 0x7F;
    other_temp = other_temp_r27 & 0x7F;
    var_r29 = lbl_8025D234 + temp3 * temp_r22 + temp_r23;
    var_r28 = &lbl_8025D230[temp3 * lbl_8025D240 + temp_r23 * 128];
    var_r27 = &lbl_8025D230[temp3 * lbl_8025D240 + other_temp_r27];

    temptemp3 = temp3;

    while (temptemp3 < temp10 + sp58[var_r31 - 1]) {
        val = lbl_801752F0[temptemp3 & 0x7F];
        var_r25 = var_r28 + val;
        var_r24 = var_r29;

        if (val < temp2) {
            var_r24++;
            var_r25 += 128;
        }

        while (var_r25 < var_r27) {
            var_r23 = 0;
            index2 = 0;

            for (k = 0; k < var_r31; k++) {
                sp8[var_r23] = sp8[index2];
                sp58[var_r23] = sp58[index2];

                temp9 = var_r25[-sp58[var_r23] * lbl_8025D240];

                if (*var_r24 != temp9) {
                    sp8[k]--;

                    if (sp8[k] == 0) {
                        var_r23--;
                    }
                }

                var_r23++;
                index2++;
            }

            var_r31 = var_r23;
            var_r25 += 128;
            var_r24++;
        }

        var_r28 += lbl_8025D240;
        var_r27 += lbl_8025D240;
        var_r29 += temp_r22;
        temptemp3++;
    }

    if (var_r31 == 1) {
        *param2 = sp58[0];
        return true;
    }

    return false;
}

bool VCMV_80083D18(UnkStruct_800838C0* param1, s32* param2) {
    u8 sp58[0x4D];
    u8 sp8[0x4D];
    s32 temp10;
    s32 var_r31;
    s32 temp_r0;
    s32 temp3;
    s32 i;
    s32 temp_r22;
    s32 temp_r23;
    s32 temp2;
    u32* var_r29;
    u32* var_r28;
    u32* var_r27;
    s32 other_temp_r27;
    s32 other_temp;
    s32 temptemp3;
    s32 index2;
    s32 k;
    u8 val;
    u32* var_r25;
    u32* var_r24;
    s32 var_r23;
    u32 temp9;

    var_r31 = 0x4D;
    temp10 = param1->unk_04 + param1->unk_0C;
    temp_r0 = lbl_8025D242 - temp10;

    if (var_r31 > param1->unk_04) {
        var_r31 = param1->unk_04;
    }

    temp3 = param1->unk_04;

    for (i = 0; i < var_r31; i++) {
        sp58[i] = i + 4;
        sp8[i] = 3;
    }

    other_temp_r27 = param1->unk_00 + param1->unk_08;
    temp_r22 = (lbl_8025D240 + 0x7F) / 128;
    temp_r23 = param1->unk_00 / 128;
    temp2 = param1->unk_00 & 0x7F;
    other_temp = other_temp_r27 & 0x7F;
    var_r29 = lbl_8025D234 + temp3 * temp_r22 + temp_r23;
    var_r28 = &lbl_8025D230[temp3 * lbl_8025D240 + temp_r23 * 128];
    var_r27 = &lbl_8025D230[temp3 * lbl_8025D240 + other_temp_r27];

    temptemp3 = temp3;

    while (temptemp3 < temp10 - sp58[var_r31 - 1]) {
        val = lbl_801752F0[temptemp3 & 0x7F];
        var_r25 = var_r28 + val;
        var_r24 = var_r29;

        if (val < temp2) {
            var_r24++;
            var_r25 += 128;
        }

        while (var_r25 < var_r27) {
            var_r23 = 0;
            index2 = 0;

            for (k = 0; k < var_r31; k++) {
                sp8[var_r23] = sp8[index2];
                sp58[var_r23] = sp58[index2];

                temp9 = var_r25[sp58[var_r23] * lbl_8025D240];

                if (*var_r24 != temp9) {
                    sp8[k]--;

                    if (sp8[k] == 0) {
                        var_r23--;
                    }
                }

                var_r23++;
                index2++;
            }

            var_r31 = var_r23;
            var_r25 += 128;
            var_r24++;
        }

        var_r28 += lbl_8025D240;
        var_r27 += lbl_8025D240;
        var_r29 += temp_r22;
        temptemp3++;
    }

    if (var_r31 == 1) {
        *param2 = -sp58[0];
        return true;
    }

    return false;
}

void VCMV_80083FC4(UnkStruct_800838C0* param1, s32 param2) {
    if (!lbl_8025D288) {
        lbl_8025D289 = true;

        lbl_801CA5D8.unk_00[0].unk_04 = -100000;
        lbl_801CA5D8.unk_00[1].unk_04 = -100000;
        lbl_801CA5D8.unk_00[2].unk_04 = -100000;
        lbl_801CA5D8.unk_00[3].unk_04 = -100000;

        lbl_8025D27E = lbl_8025D27D;
        lbl_801CA5D8.unk_00[lbl_8025D27D].unk_00 = (0, lbl_8025D284); //! TODO: fake match
        lbl_801CA5D8.unk_00[lbl_8025D27D].unk_04 = 0;
        lbl_801CA5D8.unk_38 = lbl_8025D27D;
        lbl_801CA5D8.unk_34 = lbl_8025D27D;
        lbl_801CA5D8.unk_30 = lbl_8025D27D;
        lbl_801CA5D8.unk_4C = 0;
        lbl_801CA5D8.unk_50 = 0;
        lbl_801CA5D8.unk_44 = lbl_801CA5D8.unk_48 = 0.0f;
    }

    lbl_801CA5D8.unk_4C += param2;

    if (!lbl_801CA5D8.unk_51) {
        lbl_801CA5D8.unk_51 = true;
        lbl_801CA5D8.unk_20.unk_00 = param1->unk_00;
        lbl_801CA5D8.unk_20.unk_04 = param1->unk_04;
        lbl_801CA5D8.unk_20.unk_08 = param1->unk_08;
        lbl_801CA5D8.unk_20.unk_0C = param1->unk_0C;

        if (param2 < 0) {
            lbl_801CA5D8.unk_20.unk_0C -= param2;
            lbl_801CA5D8.unk_20.unk_04 += param2;
        } else {
            lbl_801CA5D8.unk_20.unk_0C += param2;
        }
    }
}

void VCMV_800840B4(UnkStruct_800838C0* param1, s32 param2) {
    s32 sp8;

    if (lbl_8025D274 < 1 && !lbl_8025D26F) {
        if (lbl_8025D270) {
#if VCMV_REVISION == 1
            lbl_8025D270 = false;

            if (!lbl_8025D278) {
                lbl_8025D278 = true;
            }
#else
            if (lbl_8025D2C2 == 0 || lbl_8025D260 >= 0x0C) {
                lbl_8025D270 = false;

                if (!lbl_8025D278) {
                    lbl_8025D278 = true;
                }
            } else {
                lbl_8025D27B = true;
                return;
            }
#endif
        } else if (lbl_8025D280 != 0) {
            if (param2 == 1 && param1->unk_0C > 0x50) {
                lbl_801CA5D8.unk_52 = false;

                if (lbl_8025D288 && lbl_801CA5D8.unk_51 != 0) {
                    if ((param1->unk_00 != lbl_801CA5D8.unk_20.unk_00 ||
                         param1->unk_08 != lbl_801CA5D8.unk_20.unk_08)) {
                        lbl_801CA5D8.unk_52 = true;
                        goto end; //! TODO: fake match?
                    }
                }

                if (lbl_8025D280 == 1) {
                    if (VCMV_80083A60(param1, &sp8)) {
                        VCMV_80083FC4(param1, sp8);
                    }
                } else if (lbl_8025D280 == -1 && VCMV_80083D18(param1, &sp8)) {
                    VCMV_80083FC4(param1, sp8);
                }
            }
        }

    end:
        UnknownInline4();

#if VCMV_REVISION == 1
        if (lbl_8025D2B4) {
            lbl_8025D2B4 = false;
            VCMV_80083070(0);
        }
#endif
    }
}

void VCMV_80084268() {
    void* temp_r31;
    u8* var_r3;
    u32 temp_r4;
    u8* var_r5;
    s32 var_r6;
    u16* var_r7;
    u32* var_r8;
    u32* var_r9;
    u32* var_r10;
    u32* var_r11;
    s32 var_r12;

    temp_r31 = lbl_801CA388[lbl_8025D27C];

    lbl_8025D271 = false;
    lbl_8025D274 = 0;
    var_r3 = (u8*)lbl_8025D230;
    temp_r4 = lbl_8025D240 * 4;
    var_r5 = (u8*)temp_r31;

    for (var_r6 = 0; var_r6 < lbl_8025D242; var_r6 += 4) {
        var_r7 = (u16*)var_r5;
        var_r8 = (u32*)var_r3;
        var_r9 = (u32*)((u8*)var_r8 + temp_r4);
        var_r10 = (u32*)((u8*)var_r9 + temp_r4);
        var_r11 = (u32*)((u8*)var_r10 + temp_r4);

        for (var_r12 = 0; var_r12 < lbl_8025D240; var_r12 += 4) {
            *var_r7++ = UnknownInline1(*var_r8++);
            *var_r7++ = UnknownInline1(*var_r8++);
            *var_r7++ = UnknownInline1(*var_r8++);
            *var_r7++ = UnknownInline1(*var_r8++);

            *var_r7++ = UnknownInline1(*var_r9++);
            *var_r7++ = UnknownInline1(*var_r9++);
            *var_r7++ = UnknownInline1(*var_r9++);
            *var_r7++ = UnknownInline1(*var_r9++);

            *var_r7++ = UnknownInline1(*var_r10++);
            *var_r7++ = UnknownInline1(*var_r10++);
            *var_r7++ = UnknownInline1(*var_r10++);
            *var_r7++ = UnknownInline1(*var_r10++);

            *var_r7++ = UnknownInline1(*var_r11++);
            *var_r7++ = UnknownInline1(*var_r11++);
            *var_r7++ = UnknownInline1(*var_r11++);
            *var_r7++ = UnknownInline1(*var_r11++);
        }

        var_r5 += lbl_8025D240 * 8;
        var_r3 += temp_r4 * 4;
    }

    DCStoreRange(temp_r31, lbl_8025D240 * lbl_8025D242 * 2);
    GXInitTexObj(&lbl_801CA308[lbl_8025D27C], temp_r31, lbl_8025D240, lbl_8025D242, GX_TF_RGB565, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&lbl_801CA308[lbl_8025D27C], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                    GX_ANISO_1);
    lbl_8025D27A = true;
}

UNKWORD VCMV_8008461C(void* param1, UNKWORD param2, UNKWORD param3, UnkStruct_8008461C_Arg4* param4) {
    void* temp_r31;
    UNKWORD var_r30;
    char* var_r29;
    UNKWORD var_r29_2;
    UNKWORD temp_r3;
    UNKWORD temp_r3_2;

    switch (param3) {
        case 0x06:
            lbl_8025D2E8 = lbl_8025D2D8 - 5;
            lbl_8025D26D = true;
            lbl_8025D26E = true;
            lbl_8025D268 = 0.0f;
            break;
        case 0x05:
            strncpy(lbl_801CA1C8, param4->unk_00, sizeof(lbl_801CA1C8) - 1);
            lbl_8025D22C = lbl_801CA1C8;
            lbl_801CA5D8.unk_51 = 0;

            if (lbl_8025D2A4) {
                lbl_8025D2A4 = false;
                if (strcmp(lbl_8025D22C, lbl_8025D228) == 0) {
                    VCMV_80083070(0x05);
                } else {
                    VCMV_80083070(0x01);
                }
            }

            lbl_8025D26E = true;
            lbl_8025D26F = true;
            lbl_8025D270 = false;
            break;
        case 0x08:
            lbl_8025D268 = 1.0f / (2.0f - lbl_8025D268);
            break;
        case 0x07: {
            lbl_8025D2E8 = lbl_8025D2D8 - 5;
            lbl_8025D26E = false;

            if (lbl_8025D26F) {
                lbl_8025D26F = false;
                lbl_8025D270 = true;
            }

            if (lbl_8025D2C2 != 0) {
                lbl_8025D2C2--;

#if VCMV_REVISION == 1
                if (!lbl_8025D2C2) {
                    VCMV_80083070(2);
                }
#else
                if (lbl_8025D2C2 == 1) {
                    VCMV_80083070(2);
                }
#endif
            }

            UnkStruct_800838C0 sp18 = {0x000, 0x000, 0x260, 0x1C8};
            VCMV_800840B4(&sp18, 1);
            break;
        }
        case 0x01:
            return 0;
        case 0x02:
            UnknownInline2(param2);
            return 0;
        case 0x23:
            OSReport("!!WWW OUT OF MEMORY!!\n");
            lbl_8025D2BE = true;
            VCMV_80083070(3);
            return 0;
        case 0x2E:
            if (lbl_8025D25C == 0) {
                if (stricmp(param4->unk_00, "GOGI Previous Page") == 0) {
                    WWWPrevPage(param2);
                } else if (stricmp(param4->unk_00, "GOGI Next Page") == 0) {
                    WWWNextPage(param2);
                }

                return 0;
            }

            break;
        case 0x2C:
            var_r29 = param4->unk_00 + 5;

            while (*var_r29 == 0x2F) {
                var_r29++;
            }

            if (strcmp(param4->unk_04, "arc") == 0) {
                temp_r3_2 = WWWProtocolSetMimeType(param4->unk_08, "text/html");
                if (temp_r3_2 != 0) {
                    return temp_r3_2;
                }

                if (lbl_8025D28C != 0) {
                    ARCFileInfo sp8;

                    if (ARCOpen(&lbl_801CA630, var_r29, &sp8) != 0) {
                        temp_r31 = ARCGetStartAddrInMem(&sp8);

                        var_r29_2 = 0U;
                        while (var_r29_2 < sp8.size) {
                            var_r30 = sp8.size - var_r29_2;

                            if (var_r30 > 0x7FFF) {
                                var_r30 = 0x7FFF;
                            }

                            temp_r3 = WWWProtocolWrite(param4->unk_08, (u8*)temp_r31 + var_r29_2, var_r30);
                            if (temp_r3 != 0) {
                                ARCClose(&sp8);
                                return temp_r3;
                            }

                            var_r29_2 += var_r30;
                        }

                        ARCClose(&sp8);
                        return WWWProtocolFinished(param4->unk_08);
                    }

                    WWWProtocolFailed(param4->unk_08);
                    return 1;
                }
                break;
            }
            break;
        case 0x34:
            lbl_8025D26C = 1;
            break;
        default:
            break;
    }

    return 0;
}

void VCMV_80084A18(void* param1) {
    lbl_8025D28C = param1;

    if (param1 != 0) {
        ARCInitHandle(param1, &lbl_801CA630);
    }
}

void VCMV_80084A34(UNKWORD param1, UNKWORD param2) {
    lbl_8025D23C = param1;
    lbl_8025D23E = param2;
}

void VCMV_80084A40(u16 param1) {
    lbl_8025D244 = param1;
    lbl_8025D248 = (lbl_8025D240 - param1) * 0.5f;
    lbl_8025D246 = lbl_8025D240 - (s32)(param1 * 0.5f);
}

bool VCMV_80084AAC(s32 param1, s32 param2) {
    bool var_r31;
    bool var_r30;
    s32 var_r29;
    s32 temp_r26;
    u8 var_r6;

    lbl_8025D240 = ALIGN_NEXT(param1, 4);
    lbl_8025D242 = ALIGN_NEXT(param2, 4);
    lbl_8025C8E8 = lbl_8025D240 * 0.5f;
    lbl_8025C8EC = lbl_8025D242 * 0.5f;
    lbl_8025D24C = 0.5f / lbl_8025C8E8;
    lbl_8025D250 = 0.5f / lbl_8025C8EC;
    lbl_8025D248 = (lbl_8025D240 - lbl_8025D244) * 0.5f;
    lbl_8025D246 = lbl_8025D240 - (s32)(lbl_8025D244 * 0.5f);
    lbl_8025D27E = 5;

    var_r31 = 1;
    var_r30 = 0;

    if (lbl_8025D27C < 5 && lbl_801CA388[lbl_8025D27C] != NULL) {
        lbl_8025D27D = lbl_8025D27C;
        var_r30 = 1;
        var_r6 = lbl_8025D27C;
    } else {
        lbl_8025D27C = 0;
        lbl_8025D27D = 0;
        var_r6 = 0;
    }

    if (!lbl_8025D279) {
        lbl_8025D27E = var_r6;
        lbl_8025D279 = true;
    }

    temp_r26 = lbl_8025D240 * lbl_8025D242 * 2;

    for (var_r29 = 0; var_r29 < 4; var_r29++) {
        // why??
        var_r31 &= VCMV_80087E34(&lbl_801CA388[var_r29], temp_r26, lbl_8025D2CC, lbl_8025D2C8);

        if (var_r31 == 0) {
            goto loop_at_end;
        }

        GXInitTexObj(&lbl_801CA308[var_r29], lbl_801CA388[var_r29], lbl_8025D240, lbl_8025D242, GX_TF_RGB565, GX_CLAMP,
                     GX_CLAMP, 0);
        GXInitTexObjLOD(&lbl_801CA308[var_r29], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    }

    if (var_r30 == 0) {
        memset(lbl_801CA388[lbl_8025D27D], 0, temp_r26);
        DCFlushRange(lbl_801CA388[lbl_8025D27D], temp_r26);
        GXInitTexObj(&lbl_801CA308[lbl_8025D27D], lbl_801CA388[lbl_8025D27D], lbl_8025D240, lbl_8025D242, GX_TF_RGB565,
                     GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(&lbl_801CA308[lbl_8025D27D], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                        GX_ANISO_1);
    }

    lbl_8025D238 = lbl_8025D240 * lbl_8025D242 * 4;
    var_r31 &= VCMV_80087E34((void**)&lbl_8025D230, lbl_8025D238, lbl_8025D2CC, lbl_8025D2C8);

    if (var_r31 != 0) {
        u32 size;

#if VCMV_REVISION == 1
        size = lbl_8025D238 / 118;
#else
        size = ALIGN_NEXT(lbl_8025D242 * ((lbl_8025D240 + 0x7F) / 128) * 4, 32);
#endif

        var_r31 &= VCMV_80087E34((void**)&lbl_8025D234, size, lbl_8025D2CC, lbl_8025D2C8);

        if (var_r31 != 0) {
            return true;
        }

        VCMV_80087EE4((void**)&lbl_8025D230);
    }

loop_at_end:
    while (var_r29-- != 0) {
        VCMV_80087EE4(&lbl_801CA388[var_r29]);
    }

    return false;
}

void VCMV_80084DF4(void) {
    s32 var_r30;

    VCMV_80087EE4((void**)&lbl_8025D234);
    VCMV_80087EE4((void**)&lbl_8025D230);

    for (var_r30 = 0; var_r30 < 4; var_r30++) {
        VCMV_80087EE4(&lbl_801CA388[var_r30]);
    }
}

void VCMV_80084E54(void) {
    Mtx44 sp38;
    Mtx sp8;

    GXSetViewport(0.0f, 0.0f, lbl_8025D23C, lbl_8025D23E, 0.0f, 1.0f);
    GXSetScissor(0, 0, lbl_8025D240, lbl_8025D242);
    C_MTXOrtho(sp38, lbl_8025C8EC, -lbl_8025C8EC, -lbl_8025C8E8, lbl_8025C8E8, 0.0f, -1.0f);
    GXSetProjection(sp38, GX_ORTHOGRAPHIC);
    PSMTXIdentity(sp8);
    GXLoadPosMtxImm(sp8, 3);

    lbl_801CA2C8[0] = 0;
    lbl_801CA2C8[1] = 0;
    lbl_801CA2C8[2] = lbl_8025D240;
    lbl_801CA2C8[3] = lbl_8025D242;

    lbl_8025D270 = false;
    lbl_8025D26F = false;
}

void VCMV_80084F34(s32 param1) {
    lbl_8025D254 = param1;
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_CLR_RGBA, GX_RGBA4, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_CLR_RGBA, GX_RGBA4, 8);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    GXSetAlphaUpdate(1);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetNumChans(0);

    GXColor spC = {255, 255, 255, 255};
    GXSetCopyClear(spC, 0x00FFFFFF);

    UnknownInline4();
    lbl_8025D25C = 1;
    UnknownInline3(lbl_8025D22C);
}

void VCMV_80085164(void) {
    for (int i = 0; i < 0x14; i++) {
        if (WWWRunSlice(lbl_8025D2C4) != 0) {
            break;
        }
    }

    if (lbl_8025D26C) {
        lbl_8025D26C = false;
    }

    WWWSurfaceUpdateScreen(0);

#if VCMV_REVISION == 2
    if (lbl_8025D27B && lbl_8025D260 >= 0x0C) {
        lbl_8025D270 = false;
        lbl_8025D2C2 = 0;
        lbl_8025D278 = true;
        UnknownInline4();
    }
#endif

    if (lbl_8025D271) {
        if (lbl_8025D288) {
            do {
                do {
                    lbl_8025D27C = (lbl_8025D27C + 1) % 4;
                } while (lbl_8025D27C == lbl_801CA5D8.unk_30 || lbl_8025D27C == lbl_801CA5D8.unk_34);
            } while (lbl_8025D27C == lbl_801CA5D8.unk_38);
        }

        VCMV_80084268();

        lbl_8025D284 = lbl_8025D2FC;
        lbl_8025D27F = lbl_8025D27C;

        if (lbl_8025D289) {
            lbl_8025D289 = false;
            lbl_8025D288 = true;
            lbl_801CA5D8.unk_3C = lbl_8025D2D8;
        }

        u8 temp = lbl_8025D27C;
        if (lbl_8025D288) {
            lbl_801CA5D8.unk_00[temp].unk_00 = *(volatile s32*)&lbl_8025D2FC; //! TODO: fake match?
            lbl_801CA5D8.unk_00[temp].unk_04 = lbl_801CA5D8.unk_4C;

            s32 var_r4_3 = -0x2710;
            s32 var_r8 = 0x2710;
            s32 temp_r3_4 = lbl_801CA5D8.unk_20.unk_0C >> 1;

            for (int var_r9 = 0; var_r9 < 4; var_r9++) {
                UNKWORD temp_r10 = lbl_801CA5D8.unk_00[var_r9].unk_04;

                if (temp_r10 <= lbl_801CA5D8.unk_50 && temp_r10 >= lbl_801CA5D8.unk_50 - temp_r3_4 &&
                    (var_r8 > temp_r10 || (var_r8 == temp_r10 && var_r9 == lbl_8025D27C))) {
                    var_r8 = temp_r10;
                    lbl_801CA5D8.unk_30 = var_r9;
                }

                if (temp_r10 >= lbl_801CA5D8.unk_50 && temp_r10 <= lbl_801CA5D8.unk_50 + temp_r3_4 &&
                    (var_r4_3 < temp_r10 || (var_r4_3 == temp_r10 && var_r9 == lbl_8025D27C))) {
                    var_r4_3 = temp_r10;
                    lbl_801CA5D8.unk_34 = var_r9;
                }
            }

            s32 var_r5 = 0x05F5E100;
            s32 unused =
                lbl_801CA5D8.unk_00[lbl_801CA5D8.unk_30].unk_04 + lbl_801CA5D8.unk_00[lbl_801CA5D8.unk_34].unk_04;
            for (int i = 0; i < 4; i++) {
                if (i != lbl_801CA5D8.unk_30 && i != lbl_801CA5D8.unk_34) {
                    s32 temp_r0_2 = lbl_801CA5D8.unk_50 - lbl_801CA5D8.unk_00[i].unk_04;
                    s32 temp_r0_3 = temp_r0_2 * temp_r0_2;

                    if (var_r5 > temp_r0_3) {
                        var_r5 = temp_r0_3;
                        lbl_801CA5D8.unk_38 = i;
                    }
                }
            }
        }

        UnkStruct_800838C0 sp10 = {0x00, 0x00, lbl_8025D240, lbl_8025D242};
        f32 sp8;
        VCMV_800838C0(&sp10, &sp8);
    }

    BOOL status = OSDisableInterrupts();
    if (lbl_8025D27A) {
        lbl_8025D27A = false;

        if (lbl_8025D278) {
            lbl_8025D278 = false;
            lbl_8025D27E = lbl_8025D27D;
            lbl_8025D279 = true;
        }

        lbl_8025D27D = lbl_8025D27C;
    }
    OSRestoreInterrupts(status);
}

void VCMV_800855BC(GXTexObj* param1, GXColor param2) {
    GXClearVtxDesc();
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_CLR_RGBA, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_CLR_RGBA, GX_F32, 0);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    GXSetColorUpdate(GX_ENABLE);
    GXSetAlphaUpdate(GX_ENABLE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetZMode(GX_DISABLE, GX_ALWAYS, GX_DISABLE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXLoadTexObj(param1, GX_TEXMAP0);
    GXSetTevColor(GX_TEVREG0, param2);
}

void VCMV_80085798(const UnkStack20* param1) {
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXPosition3f32(param1->x1, param1->y1, 0.0f);
    GXPosition2f32(0.0f, 0.0f);

    GXPosition3f32(param1->x2, param1->y2, 0.0f);
    GXPosition2f32(0.0f, 1.0f);

    GXPosition3f32(param1->x3, param1->y3, 0.0f);
    GXPosition2f32(1.0f, 1.0f);

    GXPosition3f32(param1->x4, param1->y4, 0.0f);
    GXPosition2f32(1.0f, 0.0f);

    GXEnd();
}

void VCMV_80085874(void) {
    Mtx44 sp48;

    if (lbl_8025D290 != lbl_8025D27E || lbl_8025D291 != lbl_8025D27D || lbl_8025D292 != lbl_8025D27C) {
        lbl_8025D290 = lbl_8025D27E;
        lbl_8025D291 = lbl_8025D27D;
        lbl_8025D292 = lbl_8025D27C;
    }

    f32 w = lbl_8025C8E8;
    f32 h = lbl_8025C8EC;
    C_MTXOrtho(sp48, h, -h, -w, w, 0.0f, -1.0f);
    GXSetProjection(sp48, GX_ORTHOGRAPHIC);
    GXSetNumTexGens(1);
    GXSetNumChans(0);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GXColor sp24 = {255, 255, 255, 255};

    if (lbl_8025D288) {
        if (lbl_8025D2D8 - lbl_8025D2E8 > 0xA) {
            lbl_8025D280 = 0;
        }

        lbl_801CA5D8.unk_40 = lbl_801CA5D8.unk_4C;
        f32 temp_f5 = lbl_801CA5D8.unk_40 - lbl_801CA5D8.unk_44;

        if (temp_f5 > 1.0f) {
            if (!(0.5f * (lbl_801CA5D8.unk_48 * (lbl_801CA5D8.unk_48 / 2.0f + 1.0f)) < temp_f5)) {
                f32 temp = sqrt(0.25f + 2.0f * temp_f5 / 2.0f);
                lbl_801CA5D8.unk_48 = 2.0f * (temp - 0.5f);
            } else {
                lbl_801CA5D8.unk_48 += 2.0f;

                if (lbl_801CA5D8.unk_48 > 40.0f) {
                    lbl_801CA5D8.unk_48 = 40.0f;
                }
            }
        } else if (temp_f5 < -1.0f) {
            if (!(-0.5f * (lbl_801CA5D8.unk_48 * (lbl_801CA5D8.unk_48 / 2.0f - 1.0f)) > temp_f5)) {
                f32 temp2 = sqrt(0.25f - 2.0f * temp_f5 / 2.0f);
                lbl_801CA5D8.unk_48 = 2.0f * (0.5f - temp2);
            } else {
                lbl_801CA5D8.unk_48 -= 2.0f;

                if (lbl_801CA5D8.unk_48 < -40.0f) {
                    lbl_801CA5D8.unk_48 = -40.0f;
                }
            }
        } else {
            lbl_801CA5D8.unk_48 = 0.0f;
            lbl_801CA5D8.unk_44 = lbl_801CA5D8.unk_40;
            lbl_8025D280 = 0;
            lbl_8025D288 = false;
            goto block_21;
        }

        lbl_801CA5D8.unk_44 += lbl_801CA5D8.unk_48;
        lbl_801CA5D8.unk_50 = lbl_801CA5D8.unk_44 + 0.5f;

        s32 temp_r29 = lbl_801CA5D8.unk_20.unk_00 + lbl_801CA5D8.unk_20.unk_08;
        s32 temp_r28 = lbl_801CA5D8.unk_20.unk_04 + lbl_801CA5D8.unk_20.unk_0C;

        f32 temp_f31 = lbl_8025D24C * lbl_801CA5D8.unk_20.unk_00;
        f32 temp_f30 = lbl_8025D24C * temp_r29;
        f32 temp_f29 = lbl_8025D250 * lbl_801CA5D8.unk_20.unk_04;
        f32 temp_f28 = lbl_8025D250 * temp_r28;

        f32 temp_f27 = -lbl_8025C8E8 + lbl_801CA5D8.unk_20.unk_00;
        f32 temp_f26 = -lbl_8025C8E8 + temp_r29;
        f32 temp_f25 = lbl_8025C8EC - lbl_801CA5D8.unk_20.unk_04;
        f32 temp_f24 = lbl_8025C8EC - temp_r28;

        s32 temp_r6 = lbl_801CA5D8.unk_50 - lbl_801CA5D8.unk_00[lbl_801CA5D8.unk_30].unk_04;
        s32 temp_r0_2 = lbl_801CA5D8.unk_20.unk_04 + temp_r6;
        f32 temp_f23 = temp_r0_2 * lbl_8025D250;
        f32 temp_f22 = temp_f24 + temp_r6;

        s32 temp_r3_3 = lbl_801CA5D8.unk_20.unk_0C - temp_r6;
        s32 temp_r3_2 = lbl_801CA5D8.unk_50 + temp_r3_3 - lbl_801CA5D8.unk_00[lbl_801CA5D8.unk_34].unk_04;
        s32 temp_r3 = lbl_801CA5D8.unk_20.unk_04 + temp_r3_2;
        s32 temp_r0 = temp_r3 + temp_r6;
        f32 temp_f21 = temp_r3 * lbl_8025D250;
        f32 temp_f20 = temp_r0 * lbl_8025D250;

        if (temp_f23 < temp_f29 || temp_f21 < temp_f29 || temp_f20 > temp_f28) {
        block_21:
            VCMV_800855BC(&lbl_801CA308[lbl_8025D27F], sp24);
            {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                GXPosition3f32(-lbl_8025C8E8, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(0.0f, 0.0f);

                GXPosition3f32(-lbl_8025C8E8, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(0.0f, 1.0f);

                GXPosition3f32(lbl_8025C8E8, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(1.0f, 1.0f);

                GXPosition3f32(lbl_8025C8E8, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(1.0f, 0.0f);

                GXEnd();
            }

            lbl_8025D27D = lbl_8025D27F;
        } else {
            VCMV_800855BC(&lbl_801CA308[lbl_801CA5D8.unk_30], sp24);
            {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                GXPosition3f32(temp_f27, temp_f22, 0.0f);
                GXTexCoord2f32(temp_f31, temp_f28);

                GXPosition3f32(temp_f27, temp_f25, 0.0f);
                GXTexCoord2f32(temp_f31, temp_f23);

                GXPosition3f32(temp_f26, temp_f25, 0.0f);
                GXTexCoord2f32(temp_f30, temp_f23);

                GXPosition3f32(temp_f26, temp_f22, 0.0f);
                GXTexCoord2f32(temp_f30, temp_f28);

                GXEnd();
            }

            if (lbl_801CA5D8.unk_34 != lbl_801CA5D8.unk_30) {
                VCMV_800855BC(&lbl_801CA308[lbl_801CA5D8.unk_34], sp24);
            }

            {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32(temp_f27, temp_f24, 0.0f);
                GXTexCoord2f32(temp_f31, temp_f20);

                GXPosition3f32(temp_f27, temp_f22, 0.0f);
                GXTexCoord2f32(temp_f31, temp_f21);

                GXPosition3f32(temp_f26, temp_f22, 0.0f);
                GXTexCoord2f32(temp_f30, temp_f21);

                GXPosition3f32(temp_f26, temp_f24, 0.0f);
                GXTexCoord2f32(temp_f30, temp_f20);

                GXEnd();
            }

            if (lbl_8025D27F != lbl_801CA5D8.unk_34) {
                VCMV_800855BC(&lbl_801CA308[lbl_8025D27F], sp24);
            }

            if (lbl_801CA5D8.unk_20.unk_04 != 0) {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                GXPosition3f32(-lbl_8025C8E8, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(0.0f, 0.0f);

                GXPosition3f32(-lbl_8025C8E8, temp_f25, 0.0f);
                GXTexCoord2f32(0.0f, temp_f29);

                GXPosition3f32(lbl_8025C8E8, temp_f25, 0.0f);
                GXTexCoord2f32(1.0f, temp_f29);

                GXPosition3f32(lbl_8025C8E8, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(1.0f, 0.0f);

                GXEnd();
            }

            if (lbl_801CA5D8.unk_20.unk_00 != 0) {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                GXPosition3f32(-lbl_8025C8E8, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(0.0f, 0.0f);

                GXPosition3f32(-lbl_8025C8E8, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(0.0f, 1.0f);

                GXPosition3f32(temp_f27, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(temp_f31, 1.0f);

                GXPosition3f32(temp_f27, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(temp_f31, 0.0f);

                GXEnd();
            }

            if (temp_r29 < lbl_8025D240) {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                GXPosition3f32(temp_f26, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(temp_f30, 0.0f);

                GXPosition3f32(temp_f26, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(temp_f30, 1.0f);

                GXPosition3f32(lbl_8025C8E8, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(1.0f, 1.0f);

                GXPosition3f32(lbl_8025C8E8, lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(1.0f, 0.0f);

                GXEnd();
            }

            if (temp_r28 < lbl_8025D242) {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                GXPosition3f32(-lbl_8025C8E8, temp_f24, 0.0f);
                GXTexCoord2f32(0.0f, temp_f28);

                GXPosition3f32(-lbl_8025C8E8, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(0.0f, 1.0f);

                GXPosition3f32(lbl_8025C8E8, -lbl_8025C8EC, 0.0f);
                GXTexCoord2f32(1.0f, 1.0f);

                GXPosition3f32(lbl_8025C8E8, temp_f24, 0.0f);
                GXTexCoord2f32(1.0f, temp_f28);

                GXEnd();
            }
        }
    } else {
        if (lbl_8025D279) {
            lbl_8025D279 = false;
            lbl_8025D260 = 0;

            UNKWORD temp_r28_2 = VCMV_80086E18();
            VCMV_80086E20();

            lbl_8025C8F0 = 0x0F;

            if (temp_r28_2 == 1) {
                lbl_8025D264 = 1.0f / lbl_8025C8F0;
            } else if (temp_r28_2 == 2) {
                lbl_8025D264 = -1.0f / lbl_8025C8F0;
            } else {
                lbl_8025D264 = 0.0f;
            }

            VCMV_80086E28();
        }

        f32 temp_f8 = lbl_8025D260 * lbl_8025D264;
        f32 temp_f11 = temp_f8 - lbl_8025C8F0 * lbl_8025D264;

        temp_f8 *= 0.15f;
        temp_f11 *= 0.15f;

        f32 sp38[4];
        sp38[0] = (temp_f11 - 0.5f) * lbl_8025D240;
        sp38[1] = lbl_8025C8EC;
        sp38[2] = (temp_f11 + 0.5f) * lbl_8025D240;
        sp38[3] = -lbl_8025C8EC;

        f32 sp28[4];
        sp28[0] = (temp_f8 - 0.5f) * lbl_8025D240;
        sp28[1] = lbl_8025C8EC;
        sp28[2] = (temp_f8 + 0.5f) * lbl_8025D240;
        sp28[3] = -lbl_8025C8EC;

        if (lbl_801CA388[lbl_8025D27D] != NULL) {
            VCMV_800855BC(&lbl_801CA308[lbl_8025D27D], sp24);
            {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                GXPosition3f32(sp38[0], sp38[1], 0.0f);
                GXTexCoord2f32(0.0f, 0.0f);

                GXPosition3f32(sp38[0], sp38[3], 0.0f);
                GXTexCoord2f32(0.0f, 1.0f);

                GXPosition3f32(sp38[2], sp38[3], 0.0f);
                GXTexCoord2f32(1.0f, 1.0f);

                GXPosition3f32(sp38[2], sp38[1], 0.0f);
                GXTexCoord2f32(1.0f, 0.0f);

                GXEnd();
            }
        }

        if (lbl_8025D260 < lbl_8025C8F0) {
            s32 alpha = 255 - (u8)(lbl_8025D260 * 255 / lbl_8025C8F0);

            if (lbl_801CA388[lbl_8025D27E] != NULL) {
                GXColor sp20 = {255, 255, 255, alpha};
                VCMV_800855BC(&lbl_801CA308[lbl_8025D27E], sp20);
                {
                    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

                    GXPosition3f32(sp28[0], sp28[1], 0.0f);
                    GXTexCoord2f32(0.0f, 0.0f);

                    GXPosition3f32(sp28[0], sp28[3], 0.0f);
                    GXTexCoord2f32(0.0f, 1.0f);

                    GXPosition3f32(sp28[2], sp28[3], 0.0f);
                    GXTexCoord2f32(1.0f, 1.0f);

                    GXPosition3f32(sp28[2], sp28[1], 0.0f);
                    GXTexCoord2f32(1.0f, 0.0f);

                    GXEnd();
                }
            }

            lbl_8025D260++;
        } else {
            lbl_8025D27E = 5;
            lbl_8025D264 = 0.0f;
        }
    }
}

void VCMV_80086670(void) {
    u32 temp_r30 = lbl_8025D258;
    char* temp_r31 = lbl_8025D228;

    UnknownInline3(temp_r31);

    if (lbl_8025D258 != temp_r30) {
        WWWCloseBrowserWindow(lbl_8025D2C4, temp_r30);
    }

    lbl_8025D25C = 0;
}
