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
#include "revolution/gx.h"
#include "revolution/mem.h"
#include "revolution/nand.h"
#include "revolution/os.h"
#include "revolution/tpl.h"
#include "revolution/vi.h"
#include "string.h"

void fn_8005F1A0(void);
void fn_8005F154(void);
extern s32 fn_8008882C(void**, u32, MEMAllocator*, MEMAllocator*);
extern void fn_800888DC(void**);

char lbl_801C7D00[40];
struct_801C7D28 lbl_801C7D28;

// .rodata
const Quaternion lbl_8016A7C0 = {-GC_FRAME_WIDTH, -GC_FRAME_HEIGHT, GC_FRAME_WIDTH, GC_FRAME_HEIGHT};
const Quaternion lbl_8016A7D0 = {-GC_FRAME_WIDTH, -GC_FRAME_HEIGHT_PAL, GC_FRAME_WIDTH, GC_FRAME_HEIGHT_PAL};

// .sbss
s32 lbl_8025D118;
s32 lbl_8025D114;
u8 lbl_8025D110;
s32 lbl_8025D10C;
s32 lbl_8025D108;
void* lbl_8025D100[2];
s32 lbl_8025D0FC;
s32 lbl_8025D0F8;
char* lbl_8025D0F4;
s32 lbl_8025D0F0;
s32 lbl_8025D0EC;
s32 lbl_8025D0E8;
s32 lbl_8025D0E4;
s32 lbl_8025D0E0;
s32 lbl_8025D0DC;
s32 lbl_8025D0D8;
s64 lbl_8025D0D0;
s32 lbl_8025D0C8[2];
void* lbl_8025D0C4;
s32 lbl_8025D0C0;
s32 lbl_8025D0BC;
u8 lbl_8025D0B8;

// .sdata2
const f32 lbl_8025DEA8 = 0.0f;
const f32 lbl_8025DEAC = 1408.0f;
const f32 lbl_8025DEB0 = 1148.0f;
const f32 lbl_8025DEB4 = 287.0f;
const f32 lbl_8025DEB8 = 320.0f;
const f32 lbl_8025DEBC = -1.0f;
const f32 lbl_8025DEC0 = 960.0f;
const f32 lbl_8025DEC4 = 240.0f;
const f32 lbl_8025DEC8 = 1.0f;
const f32 lbl_8025DECC = 1000.0f;
const f64 lbl_8025DED0 = 4503601774854144.0;
const f64 lbl_8025DED8 = 4503599627370496.0;
const f32 lbl_8025DEE0 = 640.0f;
const f32 lbl_8025DEE4 = 528.0f;
const f32 lbl_8025DEE8 = 480.0f;
const GXColor lbl_8025DEEC = {255, 255, 255, 0};
const f32 lbl_8025DEF0 = 255.9f;
const f32 lbl_8025DEF4 = 250.0f;
const f32 lbl_8025DEF8 = 1.3684211f;
const f32 lbl_8025DEFC = 239.0f;
const f32 lbl_8025DF00 = 339.0f;
const f32 lbl_8025DF04 = 1001.0f;
const f32 lbl_8025DF08 = -243.84001f;
const f32 lbl_8025DF0C = -320.0f;
const f32 lbl_8025DF10 = 500.0f;
const f32 lbl_8025DF14 = -240.0f;

// .data
char lbl_801743B0[] = "html.arc";
char lbl_801743BC[] = "helpRVL.c";
char lbl_801743C8[] = "/tmp/HBMSE.brsar";
char lbl_801743DC[] = "/tmp/opera.arc";

// .sdata
GXColor lbl_8025C850[] = {255, 255, 255, 255};

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

// .sdata
s32 lbl_8025C864 = 0x2E000000;

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
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_RGBA6, 0);
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

void fn_8005E800(s32 param_1, s32 param_2, u16 param_3, u16 param_4, s32 param_5, u32 param_6) {
    Quaternion quaternion1;
    Quaternion quaternion2;
    f32 view[6];
    void* pBuffer;
    GXColor local_54;

    if (param_6 != 0) {
        local_54.r = local_54.g = local_54.b = 0;
        local_54.a = param_6 & 0xFF;
        quaternion1 = lbl_8016A7C0;
        quaternion2 = lbl_8016A7D0;

        GXGetViewportv(view);
        fn_8005E638(local_54);

        if (fn_8007FC84()) {
            GXSetViewport(0.0f, 0.0f, 4.125f, 4.015625f, 0.0f, 1.875f);
            GXSetScissor(0, 0, GC_FRAME_WIDTH, GC_FRAME_HEIGHT_PAL);

            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(quaternion2.x, quaternion2.y, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(quaternion2.x, quaternion2.w, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(quaternion2.z, quaternion2.w, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(quaternion2.z, quaternion2.y, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXEnd();
        } else {
            GXSetViewport(0.0f, 0.0f, 4.125f, 4.015625f, 0.0f, 1.875f);
            GXSetScissor(0, 0, GC_FRAME_WIDTH, GC_FRAME_HEIGHT);

            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            GXPosition3f32(quaternion1.x, quaternion1.y, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(quaternion1.x, quaternion1.w, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(quaternion1.z, quaternion1.w, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(quaternion1.z, quaternion1.y, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXEnd();
        }

        GXSetViewport(view[0], view[1], view[2], view[3], view[4], view[5]);
    }

    GXSetDispCopySrc(0, 0, param_3, param_4);

    if (fn_8007FC84()) {
        GXSetDispCopyDst(0x260, 0x210);
        pBuffer = (void*)((s32)lbl_8025D100[lbl_8025D0FC ^ 1] + ((param_1 + param_2 * 0x260) * 2));
        GXCopyDisp(pBuffer, GX_TRUE);
    } else {
        GXSetDispCopyDst(0x260, 0x1C8);
        pBuffer = (void*)((s32)lbl_8025D100[lbl_8025D0FC ^ 1] + ((param_1 + param_2 * 0x260) * 2));
        GXCopyDisp(pBuffer, GX_TRUE);
    }

    GXDrawDone();

    if (param_5 != 0) {
        lbl_8025D0FC ^= 1;
        VISetNextFrameBuffer(lbl_8025D100[lbl_8025D0FC]);
        VIFlush();
        VIWaitForRetrace();
    }
}

static inline void fn_8005EDFC_UnknownInline(GXColor color) {
    GXTexObj texObj;
    GXColor color2;

    GXClearVtxDesc();
    GXSetVtxAttrFmt(GX_VTXFMT5, GX_VA_POS, GX_POS_XY, GX_RGBA4, 0);
    GXSetVtxAttrFmt(GX_VTXFMT5, GX_VA_TEX0, GX_TEX_ST, GX_RGBA4, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3C, GX_FALSE, 0x7D);
    GXSetNumTevStages(1);
    color2 = color;
    color2.a = lbl_801C7D28.unk0D;
    GXSetTevColor(GX_TEVREG0, color2);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(GX_DISABLE, GX_LEQUAL, GX_DISABLE);
    GXSetCurrentMtx(3);
    TPLGetGXTexObjFromPalette(lbl_801C7D28.pTPLPalette, &texObj, 0);
    GXLoadTexObj(&texObj, GX_TEXMAP0);

    GXBegin(GX_QUADS, GX_VTXFMT5, 4);
    GXPosition2s16(0x20, 0x3C);
    GXTexCoord2s16(0, 1);
    GXPosition2s16(0x20, 0x20);
    GXTexCoord2s16(0, 0);
    GXPosition2s16(0x3C, 0x20);
    GXTexCoord2s16(1, 0);
    GXPosition2s16(0x3C, 0x3C);
    GXTexCoord2s16(1, 1);
    GXEnd();
}

void fn_8005EDFC(void) {
    f32 fTime = OSTicksToMilliseconds(OSGetTick() - lbl_801C7D28.unk08);

    switch (lbl_801C7D28.unk0C) {
        case 0:
            lbl_801C7D28.unk0D = 255.9f * (fTime / 250.0f);
            if (fTime >= 250.0f) {
                lbl_801C7D28.unk08 = OSGetTick();
                lbl_801C7D28.unk0C = 1;
                lbl_801C7D28.unk0D = -1;
            }
            break;
        case 1:
            if (fTime >= 1000.0f) {
                lbl_801C7D28.unk08 = OSGetTick();
                lbl_801C7D28.unk0C = 2;
            }
            break;
        case 2:
            lbl_801C7D28.unk0D = 255.9f * ((250.0f - fTime) / 250.0f);
            if (fTime >= 250.0f) {
                lbl_801C7D28.unk0D = 0;
                lbl_801C7D28.unk04 = 0;
            }
            break;
    }

    fn_8005EDFC_UnknownInline(lbl_8025DEEC);
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

bool fn_8005F1EC(void) { return 0; }

typedef struct struct_801C7CE0 {
    /* 0x00 */ MEMAllocator allocator1;
    /* 0x10 */ MEMAllocator allocator2;
} struct_801C7CE0; // size = 0x20
struct_801C7CE0 lbl_801C7CE0;

inline void test(void** truc, struct_801C7CE0* truc2, s32 size) {
    fn_8008882C(&lbl_8025D0C4, size, &truc2->allocator2, &truc2->allocator1);
}

void fn_8005F1F4(HelpMenu* pHelpMenu) {
    NANDFileInfo sp30;
    void* sp8;
    s32 temp_r14;
    s32 temp_r14_2;
    char* temp_r16;
    struct_801C7D28* truc;
    struct_801C7CE0* truc2;
    char sp10[32] = "HomeButton3/";

    temp_r16 = &sp10[strlen(sp10)];
    truc = &lbl_801C7D28;
    truc2 = &lbl_801C7CE0;

    xlHeapFill8(&truc->unk10, sizeof(struct_801C7D28_10), 0);
    lbl_8025D0C4 = NULL;
    contentInitHandleNAND(4, &truc->handle.handleNAND, &truc2->allocator2);

    if (lbl_8025D0C0 == 0) {
        sp8 = NULL;
        lbl_8025D0C0 = 1;
        strcpy(temp_r16, "Huf8_HomeButtonSe.brsar");
        temp_r14 = fn_8005E2D0(&truc->handle.handleNAND, sp10, &sp8, &truc2->allocator2, &lbl_801C7CE0);
        NANDCreate(lbl_801743C8, 0x30, 0);
        NANDOpen(lbl_801743C8, &sp30, 2);
        NANDWrite(&sp30, sp8, temp_r14);
        NANDClose(&sp30);
        fn_800888DC(&sp8);

        temp_r14_2 = fn_8005F6F4(SYSTEM_HELP(gpSystem), "Opera.arc", &sp8, &truc2->allocator2);
        NANDCreate(lbl_801743DC, 0x30, 0);
        NANDOpen(lbl_801743DC, &sp30, 2);
        NANDWrite(&sp30, sp8, temp_r14_2);
        NANDClose(&sp30);
        fn_800888DC(&sp8);
    }

    strcpy(lbl_801C7D00, "arc:/html/");
    lbl_8025D0F4 = lbl_801C7D00 + strlen(lbl_801C7D00);
    truc->unk10.pTPLPalette2 = NULL;
    strcpy(temp_r16, "LZ77_homeBtn.arc");
    strcpy(lbl_8025D0F4, "index/index_Frameset.html");
    fn_8005E2D0(&truc->handle.handleNAND, sp10, &truc->unk10.pBuffer1, &truc2->allocator2, &lbl_801C7CE0);
    strcpy(temp_r16, "Huf8_SpeakerSe.arc");
    fn_8005E2D0(&truc->handle.handleNAND, sp10, &truc->unk10.pBuffer2, &truc2->allocator2, &lbl_801C7CE0);
    strcpy(temp_r16, "home.csv");
    fn_8005E2D0(&truc->handle.handleNAND, sp10, &truc->unk10.pBuffer3, &truc2->allocator2, &lbl_801C7CE0);
    strcpy(temp_r16, "config.txt");
    fn_8005E2D0(&truc->handle.handleNAND, sp10, &truc->unk10.pBuffer4, &truc2->allocator2, &lbl_801C7CE0);

    truc->unk10.unk24 = fn_8005F1EC;
    truc->unk10.unk28 = 0;
    truc->unk10.unk30 = 0;
    truc->unk10.unk40 = lbl_8025DEF8;
    truc->unk10.unk44 = lbl_8025DEC8;
    truc->unk10.unk3C = lbl_8025DEC8;

    strcpy(temp_r16, "homeBtnIcon.tpl");
    fn_8005E2D0(&truc->handle.handleNAND, sp10, (void**)&truc->pTPLPalette, &truc2->allocator2, &lbl_801C7CE0);
    TPLBind(truc->pTPLPalette);
    truc->unk10.unk38 = 0x80000;
    // fn_8008882C(&truc->unk10.unk20, 0x80000, &truc2->allocator2, &truc2->allocator1);
    test(&lbl_8025D0C4, &lbl_801C7CE0, 0x80000);
    truc->unk10.unk48 = 0;
    fn_80088994(&truc->unk10.pBuffer1);
    fn_80100870(&truc->unk10.pBuffer1);
    test(&lbl_8025D0C4, &lbl_801C7CE0, 0xa0000);
    // fn_8008882C(&lbl_8025D0C4, 0xa0000, &truc2->allocator2, &truc2->allocator1);
    fn_80100CD8(lbl_801743C8, lbl_8025D0C4, 0xa0000);
    fn_80100940();
}

// .data
extern void* lbl_8005FF60;
extern void* lbl_80060224;
extern void* lbl_8005FF68;
extern void* lbl_8005FFA8;
extern void* lbl_8005FFF8;
extern void* lbl_8006019C;
extern void* lbl_80060224;
extern void* lbl_80060224;
extern void* lbl_8005FF58;
extern void* lbl_800601C4;
extern void* lbl_80060200;
void* jumptable_80174490[] = {
    &lbl_8005FF60, &lbl_80060224, &lbl_8005FF68, &lbl_8005FFA8, &lbl_8005FFF8, &lbl_8006019C,
    &lbl_80060224, &lbl_80060224, &lbl_8005FF58, &lbl_800601C4, &lbl_80060200,
};

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
