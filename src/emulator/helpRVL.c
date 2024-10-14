#include "emulator/helpRVL.h"
#include "emulator/cpu.h"
#include "emulator/frame.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlCoreRVL.h"
#include "emulator/xlFile.h"
#include "emulator/xlFileRVL.h"
#include "emulator/xlHeap.h"
#include "macros.h"
#include "math.h"
#include "revolution/tpl.h"
#include "revolution/nand.h"
#include "revolution/gx.h"
#include "revolution/mem.h"
#include "revolution/os.h"
#include "revolution/vi.h"
#include "string.h"

void fn_8005F1A0(void);
void fn_8005F154(void);

struct_801C7D28 lbl_801C7D28;



extern GXColorU32 lbl_8016A7C0;
extern GXColorU32 lbl_8016A7D0;

s32 lbl_8025D118;
s32 lbl_8025D114;
u8 lbl_8025D110;
s64 lbl_8025D100;
s32 lbl_8025D0FC;
s32 lbl_8025D0F0;
s32 lbl_8025D0EC;
s32 lbl_8025D0E8;
s64 lbl_8025D0D0;

s32 fn_8005E2D0(CNTHandleNAND* pHandle, char* szPath, void** ppBuffer, MEMAllocator* arg3, void* arg4) {
    CNTFileInfoNAND fileInfo;
    void* pNANDBuffer;
    s32 var_r31;
    s32 temp_r30;
    u32 var_r29;

    var_r29 = 0;

    if (contentOpenNAND(pHandle, szPath, &fileInfo)) {
        return 0;
    }

    var_r31 = contentGetLengthNAND(&fileInfo);
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
            contentReadNAND(&fileInfo, pNANDBuffer, temp_r30, 0);
            var_r31 = fn_800B17A8(pNANDBuffer);
            fn_8008882C(ppBuffer, ALIGN(var_r31, 0x1F), arg3, arg4);

            if (*ppBuffer != NULL) {
                if (var_r29 == 0x10) {
                    fn_800B17C8(pNANDBuffer, *ppBuffer);
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
            contentReadNAND(&fileInfo, *ppBuffer, temp_r30, 0);
        } else {
            var_r31 = 0;
        }
    }

    contentCloseNAND(&fileInfo);
    return var_r31;
}

void fn_8005E45C(GXTexObj* pTexObj, GXColor color) {
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGB565, 0);
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
    GXLoadTexObj(pTexObj, GX_TEXMAP0);
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
    GXSetNumIndStages(0);
    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetColorUpdate(GX_ENABLE);
    GXSetAlphaUpdate(GX_ENABLE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
    GXSetZMode(GX_DISABLE, GX_ALWAYS, GX_DISABLE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetTevColor(GX_TEVREG0, color);
}

GXColor lbl_8025DEEE = {255, 255, 255, 0};
void fn_8005EDFC(void) {
    GXTexObj sp10;
    s32 spC;
    u8 spB;
    u8 spA;
    u8 sp9;
    u8 sp8;
    f32 temp_f3;
    GXColor color2;

    temp_f3 = OSTicksToMilliseconds(OSGetTick() - lbl_801C7D28.unk08);

    switch (lbl_801C7D28.unk0C) {
        case 0:
            lbl_801C7D28.unk0D = 255.9f * (temp_f3 / 250.0f);
            if (temp_f3 >= 250.0f) {
                lbl_801C7D28.unk08 = OSGetTick();
                lbl_801C7D28.unk0C = 1;
                lbl_801C7D28.unk0D = -1;
            }
            break;
        case 1:
            if (temp_f3 >= 1000.0f) {
                lbl_801C7D28.unk08 = OSGetTick();
                lbl_801C7D28.unk0C = 2;
            }
            break;
        case 2:
            lbl_801C7D28.unk0D = 255.9f * ((250.0f - temp_f3) / 250.0f);
            if (temp_f3 >= 250.0f) {
                lbl_801C7D28.unk0D = 0;
                lbl_801C7D28.unk04 = 0;
            }
            break;
    }

    GXClearVtxDesc();
    GXSetVtxAttrFmt(GX_VTXFMT5, GX_VA_POS, GX_COMPCNT_NULL, GX_RGBA4, 0);
    GXSetVtxAttrFmt(GX_VTXFMT5, GX_VA_TEX0, GX_TEX_ST, GX_RGBA4, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    GXSetNumTevStages(1);
    color2.r = lbl_8025DEEE.r;
    color2.g = lbl_8025DEEE.g;
    color2.b = lbl_8025DEEE.b;
    color2.a = lbl_801C7D28.unk0D;
    GXSetTevColor(GX_TEVREG0, color2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(0U, GX_LEQUAL, 0);
    GXSetCurrentMtx(3);
    TPLGetGXTexObjFromPalette(lbl_801C7D28.pTPLPalette, &sp10, 0);
    GXLoadTexObj(&sp10, GX_TEXMAP0);

    GXBegin(GX_QUADS, GX_VTXFMT5, 4);
    GXPosition2s16(0x20, 0x3C);
    GXPosition2s16(0, 1);
    GXPosition2s16(0x20, 0x20);
    GXPosition2s16(0, 0);
    GXPosition2s16(0x3C, 0x20);
    GXPosition2s16(1, 0);
    GXPosition2s16(0x3C, 0x3C);
    GXPosition2s16(1,  1);
    GXEnd();
}

void fn_8005F154(void) {
    lbl_8025D0F0 = 0;
    lbl_8025D0E8 = 1;
    if ((s32)lbl_8025D118 == 7) {
        fn_800887D4(0x1E);
        lbl_8025D110 = 0;
    }
}

void fn_8005F1A0(void) {
    lbl_8025D0F0 = 0;
    lbl_8025D0EC = 1;
    if ((s32)lbl_8025D118 == 7) {
        fn_800887D4(0x1E);
        lbl_8025D110 = 0;
    }
}

s32 fn_8005F1EC(void) { return 0; }

/*
extern s32 lbl_8025D0C0;
extern s32* lbl_8025D0C4;
extern void* lbl_801C7CE0;
char lbl_8016A7E0[] = "HomeButton3/";

typedef struct struct_801C7D00 {
    /* 0x00 */ MEMAllocator allocator;
    /* 0x10 *
} struct_801C7D00; // size = 0x28

struct_801C7D00 lbl_801C7D00;

void fn_8005F1F4(HelpMenu* pHelpMenu) {
    s32 spBC;
    NANDFileInfo sp30;
    void* sp8;
    u32 temp_r14;
    u32 temp_r14_2;
    void** temp_r14_3;

    char* temp_r16;
    char sp10[120];
    s32 i;

    for (i = 0; i < ARRAY_COUNT(lbl_8016A7E0); i++) {
        sp10[i] = lbl_8016A7E0[i];
    }

    temp_r16 = &sp10[strlen((char*)sp10)];

    xlHeapFill8(lbl_801C7D28.unk10, sizeof(lbl_801C7D28.unk10), 0);
    lbl_8025D0C4 = NULL;
    contentInitHandleNAND(4, &lbl_801C7D28.handle.handleNAND, &lbl_801C7D00.allocator);

    if (lbl_8025D0C0 == 0) {
        sp8 = NULL;
        lbl_8025D0C0 = 1;
        strcpy(temp_r16, "Huf8_HomeButtonSe.brsar");
        temp_r14 = fn_8005E2D0(&lbl_801C7D28.handle.handleNAND, (char*)sp10, &sp8, &lbl_801C7D00.allocator, lbl_801C7CE0);
        NANDCreate("/tmp/HBMSE.brsar", 0x30, 0);
        NANDOpen("/tmp/HBMSE.brsar", &sp30, 2);
        NANDWrite(&sp30, sp8, temp_r14);
        NANDClose(&sp30);
        fn_800888DC(&sp8);
        temp_r14_2 = fn_8005F6F4(SYSTEM_HELP(gpSystem), "Opera.arc", &sp8, &lbl_801C7D00.allocator);
        NANDCreate("/tmp/opera.arc", 0x30, 0);
        NANDOpen("/tmp/opera.arc", &sp30, 2);
        NANDWrite(&sp30, sp8, temp_r14_2);
        NANDClose(&sp30);
        fn_800888DC(sp8);
    }

    strcpy(&lbl_801C7CE0 + 0x20, "arc:/html/");
    // temp_r14_3 = &lbl_801C7CE0 + 0x58;
    // lbl_8025D0F4 = &lbl_801C7CE0 + 0x20 + strlen(&lbl_801C7CE0 + 0x20);
    // temp_r14_3->unk1C = 0;
    // strcpy(temp_r16, "LZ77_homeBtn.arc");
    // strcpy(lbl_8025D0F4, "index/index_Frameset.html");
    // fn_8005E2D0(&lbl_801C7D28.handle, &sp10, temp_r14_3, &lbl_801C7D00.allocator, &lbl_801C7CE0);
    // strcpy(temp_r16, "Huf8_SpeakerSe.arc");
    // fn_8005E2D0(&lbl_801C7D28.handle, &sp10, temp_r14_3 + 4, &lbl_801C7D00.allocator, &lbl_801C7CE0);
    // strcpy(temp_r16, "home.csv");
    // fn_8005E2D0(&lbl_801C7D28.handle, &sp10, temp_r14_3 + 8, &lbl_801C7D00.allocator, &lbl_801C7CE0);
    // strcpy(temp_r16, "config.txt");
    // fn_8005E2D0(&lbl_801C7D28.handle, &sp10, temp_r14_3 + 0xC, &lbl_801C7D00.allocator, &lbl_801C7CE0);
    // temp_r14_3->unk14 = &fn_8005F1EC;
    // temp_r14_3->unk18 = 0;
    // temp_r14_3->unk20 = 0;
    // temp_r14_3->unk30 = (f32) lbl_8025DEF8;
    // temp_r14_3->unk34 = (f32) lbl_8025DEC8;
    // temp_r14_3->unk2C = (f32) lbl_8025DEC8;
    // strcpy(temp_r16, "homeBtnIcon.tpl", lbl_8025DEF8);
    // fn_8005E2D0(&lbl_801C7D28.handle, &sp10, &lbl_801C7CE0 + 0x48, &lbl_801C7D00.allocator, &lbl_801C7CE0);
    // TPLBind(lbl_801C7CE0.unk48);
    // temp_r14_3->unk28 = 0x80000;
    // fn_8008882C(temp_r14_3 + 0x10, 0x80000, &lbl_801C7D00.allocator, &lbl_801C7CE0);
    // temp_r14_3->unk38 = 0;
    fn_80088994(temp_r14_3);
    fn_80100870(temp_r14_3);
    // fn_8008882C(&lbl_8025D0C4, 0xA0000, &lbl_801C7D00.allocator, &lbl_801C7CE0);
    // fn_80100CD8("/tmp/HBMSE.brsar", lbl_8025D0C4, 0xA0000);
    fn_80100940();
}
*/

bool fn_8005F5F4(HelpMenu* pHelpMenu, void* pObject, s32 nByteCount, HelpMenuCallback callback) {
    u32 nSize;
    s32 var_r7;
    s32 nSizeExtra;
    s32 nHeapSize;

    nHeapSize = nByteCount & ~0x70000000;

    switch (nByteCount & 0x30000000) {
        case 0x0:
            nSizeExtra = 3;
            break;
        case 0x10000000:
            nSizeExtra = 7;
            break;
        case 0x20000000:
            nSizeExtra = 15;
            break;
        case 0x30000000:
            nSizeExtra = 31;
            break;
        default:
            nSizeExtra = 0;
            break;
    }

    if (nByteCount & 0x40000000) {
        nSize = nHeapSize + nSizeExtra;

        if (pHelpMenu->unk04 < nSize) {
            return false;
        }

        var_r7 = pHelpMenu->unk20;
        pHelpMenu->unk04 = pHelpMenu->unk04 - nSize;
        pHelpMenu->unk20 = var_r7 + nSize;
    } else {
        nSize = nHeapSize + nSizeExtra;

        if (pHelpMenu->unk00 < nSize) {
            return false;
        }

        var_r7 = pHelpMenu->unk18;
        pHelpMenu->unk00 = pHelpMenu->unk00 - nSize;
        pHelpMenu->unk18 = var_r7 + nSize;
    }

    while (var_r7 & nSizeExtra) {
        var_r7++;
    }

    *(s32*)pObject = var_r7;
    pHelpMenu->unk24[pHelpMenu->unk10++] = callback;

    return true;
}

bool fn_8005F6F4(HelpMenu* pHelpMenu, char* szFileName, s32** arg2, MEMAllocator* arg3) {
    s32 nSize;
    tXL_FILE* pFile;

    if (xlFileGetSize(&nSize, szFileName)) {
        *arg2 = (s32*)MEMAllocFromAllocator(arg3, nSize + 0x1F);

        if (*arg2 == NULL) {
            return false;
        }

        while ((s32)*arg2 & 0x1F) {
            *arg2 = (s32*)((s32)*arg2 + 1);
        }

        if (!xlFileOpen(&pFile, XLFT_BINARY, szFileName)) {
            return false;
        }

        if (!xlFileGet(pFile, *arg2, nSize)) {
            return false;
        }

        if (xlFileClose(&pFile)) {
            return nSize;
        }

        return false;
    }

    return false;
}

s32 fn_800607B0(HelpMenu* pHelpMenu, s32 arg1) {
    pHelpMenu->unk0C = arg1;
    pHelpMenu->unk08 = 0;
    return 1;
}

s32 fn_800607C4(HelpMenu* pHelpMenu, s32 arg1) {
    if (pHelpMenu->unk0C == 0) {
        lbl_801C7D28.unk0C = 0;
        lbl_801C7D28.unk04 = 1;
        lbl_801C7D28.unk08 = OSGetTick();
        lbl_801C7D28.unk0D = 0;
    } else {
        if (OSGetTime() - lbl_8025D0D0 < OSMillisecondsToTicks(250)) {
            arg1 = 0;
        }

        pHelpMenu->unk08 = arg1 != 0 && lbl_801C7D28.unk04 == 0;
    }

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
