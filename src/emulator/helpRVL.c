#include "emulator/helpRVL.h"
#include "emulator/cpu.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlFile.h"
#include "emulator/xlFileRVL.h"
#include "emulator/xlHeap.h"
#include "revolution/mem.h"
#include "revolution/os.h"
#include "revolution/gx.h"
#include "revolution/cnt.h"
#include "string.h"
#include "macros.h"

void fn_8005F1A0(void);
void fn_8005F154(void);

struct_801C7D28 lbl_801C7D28;

//! TODO: structs
s32 lbl_801C7D00[0xA];
s32 lbl_801C7CE0[0x8];

extern s64 lbl_8025D0D0;
extern s32 lbl_8025D0EC;
extern s32 lbl_8025D0E8;
extern s32 lbl_8025D0F0;

s32 lbl_8025D118;
s32 lbl_8025D114;
u8 lbl_8025D110;

s32 fn_8005E2D0(HelpMenu* pHelpMenu, char* szPath, void** ppBuffer, void* arg3, void* arg4) {
    CNTFileInfo pFileInfo;
    void* pNANDBuffer;
    s32 var_r31;
    s32 temp_r30;
    u32 var_r29;

    var_r29 = 0;

    if (ARCGetFile(&pFileInfo)) {
        return 0;
    }

    var_r31 = contentGetLengthNAND(&pFileInfo);
    temp_r30 = ALIGN(var_r31, 0x1F);

    if (strstr(szPath, "/LZ77")) {
        var_r29 = 0x10;
    } else if (strstr(szPath, "/Huf8")) {
        var_r29 = 0x20;
    }

    if (var_r29 != 0) {
        pNANDBuffer = 0;
        fn_8008882C(&pNANDBuffer, temp_r30, arg3, arg4);

        if (pNANDBuffer != NULL) {
            contentReadNAND(&pFileInfo, pNANDBuffer, temp_r30, 0);
            var_r31 = fn_800B17A8(pNANDBuffer);
            fn_8008882C(ppBuffer, (var_r31 + 0x1F) & ~0x1F, arg3, arg4);

            if (*ppBuffer != NULL) {
                if (var_r29 == 0x10) {
                    fn_800B17C8(pNANDBuffer);
                } else {
                    fn_800B18DC(pNANDBuffer);
                }
            } else {
                var_r31 = 0;
            }

            fn_800888DC(&pNANDBuffer);
        } else {
            var_r31 = 0;
        }
    } else {
        fn_8008882C(ppBuffer, temp_r30, arg3, arg4);

        if (*ppBuffer != 0) {
            contentReadNAND(&pFileInfo, *ppBuffer, temp_r30, 0);
        } else {
            var_r31 = 0;
        }
    }

    contentCloseNAND(&pFileInfo);
    return var_r31;
}

void fn_8005E45C(void* arg0, GXColor color) {
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGB565, 0);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    fn_800A2400(0);
    GXSetNumTevStages(1);
    fn_800A2420(0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    fn_8009F624(0, 1, 4, 0x3C, 0, 0x7D);
    GXSetColorUpdate(1);
    fn_800A2F0C(1);
    fn_800A2E90(1, 4, 5, 0xF);
    GXSetZMode(0U, GX_ALWAYS, 0);
    fn_800A28FC(7, 0, 1, 7, 0);
    GXSetCullMode(GX_CULL_NONE);
    fn_800A35B8(0);
    fn_800A18D4(arg0, 0);
    GXSetTevColor(GX_TEVREG0, color);
}

void fn_8005E638(GXColor color) {
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    fn_800A2400(0);
    GXSetNumTevStages(1);
    fn_800A2420(0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    fn_800A120C(4, 0, 1, 1, 0, 2, 2);
    GXSetColorUpdate(1);
    fn_800A2F0C(1);
    fn_800A2E90(1, 4, 5, 0xF);
    GXSetZMode(0U, GX_ALWAYS, 0);
    fn_800A28FC(7, 0, 1, 7, 0);
    GXSetCullMode(GX_CULL_NONE);
    fn_800A35B8(0);
    GXSetTevColor(GX_TEVREG0, color);
}

void fn_8005F154(void) {
    lbl_8025D0F0 = 0;
    lbl_8025D0E8 = 1;
    if ((s32) lbl_8025D118 == 7) {
        fn_800887D4(0x1E);
        lbl_8025D110 = 0;
    }
}

void fn_8005F1A0(void) {
    lbl_8025D0F0 = 0;
    lbl_8025D0EC = 1;
    if ((s32) lbl_8025D118 == 7) {
        fn_800887D4(0x1E);
        lbl_8025D110 = 0;
    }
}

s32 fn_8005F1EC(void) {
    return 0;
}

bool fn_8005F5F4(HelpMenu* pHelpMenu, void* pObject, s32 arg2, HelpMenuCallback callback) {
    s32 var_r7;
    u32 temp_r5_2;
    u32 temp_r5;
    s32 temp_r9;
    s32 var_r8;

    temp_r9 = arg2 & 0x8FFFFFFF;

    switch (arg2 & 0x30000000) {
        case 0x0:
            var_r8 = 3;
            break;
        case 0x10000000:
            var_r8 = 7;
            break;
        case 0x20000000:
            var_r8 = 0xF;
            break;
        case 0x30000000:
            var_r8 = 0x1F;
            break;
        default:
            var_r8 = 0;
            break;
    }

    if (arg2 & 0x40000000) {
        temp_r5 = temp_r9 + var_r8;

        if (pHelpMenu->unk04 < temp_r5) {
            return 0;
        }

        var_r7 = pHelpMenu->unk20;
        pHelpMenu->unk04 = (u32)(pHelpMenu->unk04 - temp_r5);
        pHelpMenu->unk20 = (s32)(var_r7 + temp_r5);
    } else {
        temp_r5_2 = temp_r9 + var_r8;

        if (pHelpMenu->unk00 < temp_r5_2) {
            return 0;
        }

        var_r7 = pHelpMenu->unk18;
        pHelpMenu->unk00 = (u32)(pHelpMenu->unk00 - temp_r5_2);
        pHelpMenu->unk18 = (s32)(var_r7 + temp_r5_2);
    }

    while (var_r7 & var_r8) {
        var_r7++;
    }

    pObject = &var_r7;
    pHelpMenu->unk24[pHelpMenu->unk10++] = callback;

    return true;
}

static void inline inlined(s32 arg0) {
    while (arg0 & 0x1F) {
        arg0++;
    }
}

s32 fn_8005F6F4(HelpMenu* pHelpMenu, char* arg1, s32** arg2, MEMAllocator** arg3) {
    s32 size;
    tXL_FILE* spC;
    tXL_FILE* sp8;

    if (xlFileGetSize(&size, arg1)) {
        *arg2 = (s32*)MEMAllocFromAllocator(*arg3, spC->unk_1F);

        if (*arg2 == NULL) {
            return 0;
        }

        while ((s32)*arg2++ & 0x1F) {
            *arg2++;
        }

        if (!xlFileOpen(&sp8, 1, arg1)) {
            return 0;
        }

        if (!xlFileGet(sp8, *arg2, size)) {
            return 0;
        }

        if (xlFileClose(&sp8)) {
            return size;
        }

        return 0;
    }

    return 0;
}

s32 fn_800607B0(HelpMenu* pHelpMenu, s32 arg1) {
    pHelpMenu->unk0C = arg1;
    pHelpMenu->unk08 = 0;
    return 1;
}

static inline bool helpMenuHeapTake(HelpMenu* pHelpMenu) {
    if (!xlHeapTake((void**)&pHelpMenu->unk14, 0x700000 | 0x30000000)) {
        return false;
    }

    if (!xlHeapTake((void**)&pHelpMenu->unk1C, 0x2900000 | 0x70000000)) {
        return false;
    }

    pHelpMenu->unk00 = 0x00700000;
    pHelpMenu->unk18 = (s32)pHelpMenu->unk14;
    pHelpMenu->unk04 = 0x02900000;
    pHelpMenu->unk20 = (s32)pHelpMenu->unk1C;

    return true;
}

static inline bool helpMenuFree(HelpMenu* pHelpMenu) {
    if (!xlHeapFree((void**)&pHelpMenu->unk14)) {
        return false;
    }

    if (!xlHeapFree((void**)&pHelpMenu->unk1C)) {
        return false;
    }

    return true;
}

bool helpMenuEvent(HelpMenu* pHelpMenu, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 0:
            pHelpMenu->unk0C = 1;
            lbl_8025D0EC = 0;
            lbl_8025D0E8 = 0;

            OSSetResetCallback(fn_8005F1A0);
            OSSetPowerCallback(fn_8005F154);

            lbl_801C7D28.pTPLPalette = NULL;
            lbl_801C7D28.unk04 = 0;

            if (xlFileLoad("homeBtnIcon.tpl", (void**)&lbl_801C7D28.pTPLPalette)) {
                TPLBind(lbl_801C7D28.pTPLPalette);
            }

            pHelpMenu->unk10 = 0;

            if (!helpMenuHeapTake(pHelpMenu)) {
                return false;
            }
            break;
        case 1:
            if (!helpMenuFree(pHelpMenu)) {
                return false;
            }
            break;
        case 2:
            lbl_8025D0D0 = OSGetTime();
            pHelpMenu->unk08 = 0;
            break;
        case 3:
        case 5:
        case 6:
            break;
        default:
            return false;
    }

    return true;
}

_XL_OBJECTTYPE gClassHelpMenu = {
    "HelpMenu",
    sizeof(HelpMenu),
    NULL,
    (EventFunc)helpMenuEvent,
};
