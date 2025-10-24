#include "emulator/helpRVL.h"
#include "emulator/controller.h"
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
#include "revolution/ai.h"
#include "revolution/ax.h"
#include "revolution/demo.h"
#include "revolution/gx.h"
#include "revolution/hbm.h"
#include "revolution/mem.h"
#include "revolution/mtx.h"
#include "revolution/nand.h"
#include "revolution/os.h"
#include "revolution/tpl.h"
#include "revolution/vi.h"
#include "string.h"

//! TODO: move to the proper headers when documented properly
extern char* VCMVRun(void*, char*, u8);
extern s32 fn_8008882C(void**, u32, MEMAllocator*, MEMAllocator*);
extern void* fn_80083140(void);
extern void fn_800888DC(void**);
extern HBMControllerData lbl_801CA670;

static void helpMenuResetCallback(void);
static void helpMenuPowerCallback(void);

typedef struct Rect {
    /* 0x0 */ f32 x0;
    /* 0x4 */ f32 y0;
    /* 0x8 */ f32 x1;
    /* 0xC */ f32 y1;
} Rect; // size = 0x10

static MEMAllocator sMemAllocator1 = {0};
static MEMAllocator sMemAllocator2 = {0}; // MyAllocMem2?
static char sWebsitePath[40] = {0};
static struct_801C7D28 lbl_801C7D28 = {0};
static HBMDataInfo hbmInfo = {0};
static CNTHandleNAND sHandleNAND;
static GXTexObj sTexObj;

const Rect lbl_8016A7C0 = {-GC_FRAME_WIDTH, -GC_FRAME_HEIGHT, GC_FRAME_WIDTH, GC_FRAME_HEIGHT};
const Rect lbl_8016A7D0 = {-GC_FRAME_WIDTH, -GC_FRAME_HEIGHT_PAL, GC_FRAME_WIDTH, GC_FRAME_HEIGHT_PAL};

HbmVcmvState nHbmVcmvState;
s32 lbl_8025D114;
u8 lbl_8025D110;
static GXRenderModeObj* sRenderMode;
AIDMACallback lbl_8025D108;
void* lbl_8025D100[2];
s32 lbl_8025D0FC;
s32* lbl_8025D0F8;
char* lbl_8025D0F4;
bool lbl_8025D0F0;
bool sResetBtnPressed;
bool sPowerBtnPressed;
MEMiHeapHead* lbl_8025D0E4;
MEMiHeapHead* lbl_8025D0E0;
s32 lbl_8025D0DC;
s32 lbl_8025D0D8;
s64 lbl_8025D0D0;
s32 lbl_8025D0CC;
void* lbl_8025D0C8;
void* lbl_8025D0C4;
s32 lbl_8025D0C0;
char* lbl_8025D0BC;
u8 lbl_8025D0B8;

GXColor lbl_8025C850[] = {255, 255, 255, 255};

static s32 helpMenuReadNAND(CNTHandleNAND* pHandle, char* szPath, void** ppBuffer, MEMAllocator* arg3, void* arg4) {
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

static void helpMenuSetupTexturedQuad(GXTexObj* pTexObj, GXColor color) {
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
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

static void helpMenuSetupColoredQuad(GXColor color) {
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
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

#pragma inline_max_auto_size(100)

void helpMenuSetupRender(GXTexObj* pTexObj) {
    Rect rect = {0};
    GXColor color;
    f32 fWidth;
    f32 fHeight;
    Mtx44 matrix44;
    f32 x1 = 0.0f;
    f32 y1 = 0.0f;
    f32 y0 = 0.0f;
    f32 x0 = 0.0f;

    rect.x1 = sRenderMode->fbWidth / 2;
    rect.y1 = sRenderMode->xfbHeight / 2;

    GXInvalidateVtxCache();
    GXInvalidateTexAll();

    if (fn_8007FC84()) {
        fWidth = ((sRenderMode->viWidth - 704) * 320) / 1408.0f;
        fHeight = ((sRenderMode->viHeight - 574) * 287) / 1148.0f;
        C_MTXOrtho(matrix44, -fHeight, fHeight + 287.0f, -fWidth, fWidth + 320.0f, 0.0f, -1.0f);
    } else {
        fWidth = ((sRenderMode->viWidth - 704) * 320) / 1408.0f;
        fHeight = ((sRenderMode->viHeight - 480) * 240) / 960.0f;
        C_MTXOrtho(matrix44, -fHeight, fHeight + 240.0f, -fWidth, fWidth + 320.0f, 0.0f, -1.0f);
    }

    GXSetViewport(0.0f, 0.0f, sRenderMode->fbWidth, sRenderMode->efbHeight, 0.0f, 1.0f);
    GXSetProjection(matrix44, GX_ORTHOGRAPHIC);

    GXSetFog(GX_FOG_NONE, lbl_8025C850[0], 0.0f, 0.0f, 0.0f, 1000.0f);
    GXFlush();

    color.a = color.r = color.g = color.b = 0xFF;
    helpMenuSetupTexturedQuad(pTexObj, color);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    x0 = rect.x0;
    y0 = rect.y0;
    x1 = rect.x1;
    y1 = rect.y1;
    GXPosition3f32(x0, y0, 0.0f);
    GXTexCoord2u8(0, 0);
    GXPosition3f32(x0, y1, 0.0f);
    GXTexCoord2u8(0, 1);
    GXPosition3f32(x1, y1, 0.0f);
    GXTexCoord2u8(1, 1);
    GXPosition3f32(x1, y0, 0.0f);
    GXTexCoord2u8(1, 0);
    GXEnd();
}

#pragma inline_max_auto_size(10)

static void helpMenu_8005E800(s32 param_1, s32 param_2, u16 param_3, u16 param_4, s32 param_5, u32 param_6) {
    f32 view[6];
    void* pBuffer;
    Rect rect;
    Rect rectPAL;

    if (param_6 != 0) {
        GXColor local_54;
        local_54.r = local_54.g = local_54.b = 0;
        local_54.a = param_6;
        rect = lbl_8016A7C0;
        rectPAL = lbl_8016A7D0;

        GXGetViewportv(view);
        helpMenuSetupColoredQuad(local_54);

        if (fn_8007FC84()) {
            f32 x1;
            f32 y1;
            f32 y0;
            f32 x0;

            GXSetViewport(0.0f, 0.0f, GC_FRAME_WIDTH, GC_FRAME_HEIGHT_PAL, 0.0f, 1.0f);
            GXSetScissor(0, 0, GC_FRAME_WIDTH, GC_FRAME_HEIGHT_PAL);

            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            x0 = rectPAL.x0;
            y0 = rectPAL.y0;
            x1 = rectPAL.x1;
            y1 = rectPAL.y1;
            GXPosition3f32(x0, y0, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(x0, y1, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(x1, y1, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(x1, y0, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXEnd();
        } else {
            f32 x1;
            f32 y1;
            f32 y0;
            f32 x0;

            GXSetViewport(0.0f, 0.0f, GC_FRAME_WIDTH, GC_FRAME_HEIGHT, 0.0f, 1.0f);
            GXSetScissor(0, 0, GC_FRAME_WIDTH, GC_FRAME_HEIGHT);

            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            x0 = rect.x0;
            y0 = rect.y0;
            x1 = rect.x1;
            y1 = rect.y1;
            GXPosition3f32(x0, y0, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(x0, y1, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(x1, y1, 0.0f);
            GXColor1u32(0xFFFFFFFF);
            GXPosition3f32(x1, y0, 0.0f);
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

bool helpMenuUpdate_UnknownInline(void) {
    return helpMenuAllocateFile(SYSTEM_HELP(gpSystem), "html.arc", &lbl_8025D0F8, &sMemAllocator2);
}

static void helpMenuRunManualViewer(void) {
    GXRenderModeObj sp8;
    s32 var_r31;

    var_r31 = MB(20);

    if (lbl_8025D0BC == NULL) {
        lbl_8025D0BC = sWebsitePath;
    }

    sp8 = *sRenderMode;
    sp8.viWidth = 0x29E;

    if (fn_8007FC84()) {
        sp8.fbWidth = 0x260;
        sp8.viHeight = 0x210;
        sp8.xfbHeight = 0x210;
        sp8.efbHeight = 0x210;
    } else {
        sp8.fbWidth = 0x260;
        sp8.viHeight = 0x1C8;
        sp8.xfbHeight = 0x1C8;
        sp8.efbHeight = 0x1C8;
    }

    if (fn_8007FC84()) {
        sp8.viXOrigin = (720 - sp8.viWidth) / 2;
        sp8.viYOrigin = 23;
    } else {
        switch (VIGetTvFormat()) {
            case VI_TV_FMT_NTSC:
                sp8.viXOrigin = (720 - sp8.viWidth) / 2;
                sp8.viYOrigin = 12;
                break;
            case VI_TV_FMT_MPAL:
                sp8.viXOrigin = (720 - sp8.viWidth) / 2;
                sp8.viYOrigin = 12;
                break;
            case VI_TV_FMT_EURGB60:
                sp8.viXOrigin = (720 - sp8.viWidth) / 2;
                sp8.viYOrigin = 12;
                break;
            default:
                break;
        }
    }

    VIConfigure(&sp8);
    VIFlush();
    GXSetDispCopyYScale((f32)sp8.xfbHeight / (f32)sp8.efbHeight);
    VIWaitForRetrace();
    VIWaitForRetrace();

    if (fn_8007FC84()) {
        fn_80088668(0x260, 0x210);
        fn_8008866C(0x260, 0x210);
    } else {
        fn_80088668(0x260, 0x1C8);
        fn_8008866C(0x260, 0x1C8);
    }

    fn_80088670(0xC);

    for (; var_r31 > 0; var_r31 -= MB(1)) {
        if (fn_80088678(var_r31) != 0) {
            break;
        }
    }

    if (var_r31 <= 0) {
        OSPanic("helpRVL.c", 938, ".");
    }

    fn_800887CC(sWebsitePath);
    lbl_8025D0BC = VCMVRun(helpMenu_8005E800, lbl_8025D0BC, lbl_8025D0B8);

    VIConfigure(sRenderMode);
    VIFlush();
    GXSetDispCopyYScale((f32)sRenderMode->xfbHeight / (f32)sRenderMode->efbHeight);
    VIWaitForRetrace();
    VIWaitForRetrace();
}

static inline void helpMenu_8005EDFC_UnknownInline(GXColor color) {
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

const GXColor lbl_8025DEEC = {255, 255, 255, 0};

static void helpMenu_8005EDFC(void) {
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
                lbl_801C7D28.unk04 = false;
            }
            break;
    }

    helpMenu_8005EDFC_UnknownInline(lbl_8025DEEC);
}

static void helpMenuPowerCallback(void) {
    lbl_8025D0F0 = false;
    sPowerBtnPressed = true;

    if (nHbmVcmvState == E_ManualViewerRunning) {
        fn_800887D4(0x1E);
        lbl_8025D110 = 0;
    }
}

static void helpMenuResetCallback(void) {
    lbl_8025D0F0 = false;
    sResetBtnPressed = true;

    if (nHbmVcmvState == E_ManualViewerRunning) {
        fn_800887D4(0x1E);
        lbl_8025D110 = 0;
    }
}

static bool helpMenuSoundCallback(s32 event, s32 num) { return false; }

void helpMenuInit_UnknownInline1(NANDFileInfo* pFileInfo, void** ppBuffer, char* szPath) {
    s32 nLength = helpMenuReadNAND(&sHandleNAND, szPath, ppBuffer, &sMemAllocator2, &sMemAllocator1);
    NANDCreate("/tmp/HBMSE.brsar", 0x30, 0);
    NANDOpen("/tmp/HBMSE.brsar", pFileInfo, 2);
    NANDWrite(pFileInfo, *ppBuffer, nLength);
    NANDClose(pFileInfo);
    fn_800888DC(ppBuffer);
}

void helpMenuInit_UnknownInline2(NANDFileInfo* pFileInfo, void** ppBuffer, char* szPath) {
    s32 nLength = helpMenuAllocateFile(SYSTEM_HELP(gpSystem), szPath, (s32**)ppBuffer, &sMemAllocator2);
    NANDCreate("/tmp/opera.arc", 0x30, 0);
    NANDOpen("/tmp/opera.arc", pFileInfo, 2);
    NANDWrite(pFileInfo, *ppBuffer, nLength);
    NANDClose(pFileInfo);
    fn_800888DC(ppBuffer);
}

static void helpMenuInit(HelpMenu* pHelpMenu) {
    NANDFileInfo sp30;
    void* sp8;
    char* temp_r16;
    char sp10[32] = "HomeButton3/";
    u8 language;

    temp_r16 = &sp10[strlen(sp10)];

    xlHeapFill8(&hbmInfo, sizeof(HBMDataInfo), 0);
    lbl_8025D0C4 = NULL;
    contentInitHandleNAND(4, &sHandleNAND, &sMemAllocator2);

    if (lbl_8025D0C0 == 0) {
        sp8 = NULL;
        lbl_8025D0C0 = 1;
        strcpy(temp_r16, "Huf8_HomeButtonSe.brsar");
        helpMenuInit_UnknownInline1(&sp30, &sp8, sp10);
        helpMenuInit_UnknownInline2(&sp30, &sp8, "Opera.arc");
    }

    strcpy(sWebsitePath, "arc:/html/");
    lbl_8025D0F4 = sWebsitePath + strlen(sWebsitePath);

#if VERSION == OOT_J
    hbmInfo.region = SC_LANG_JP;
    strcpy(temp_r16, "LZ77_homeBtn.arc");
    strcpy(lbl_8025D0F4, "index/index_Frameset.html");
#else
    language = SCGetLanguage();
    hbmInfo.region = language;

    switch (language) {
#if VERSION == OOT_E
        case SC_LANG_DE:
            strcpy(temp_r16, "LZ77_homeBtn_GER.arc");
            strcpy(lbl_8025D0F4, "startup_noe.html");
            break;
#endif
        case SC_LANG_FR:
            strcpy(temp_r16, "LZ77_homeBtn_FRA.arc");
            strcpy(lbl_8025D0F4, "startup_fra.html");
            break;
        case SC_LANG_SP:
            strcpy(temp_r16, "LZ77_homeBtn_SPA.arc");
            strcpy(lbl_8025D0F4, "startup_esp.html");
            break;
#if VERSION == OOT_E
        case SC_LANG_IT:
            strcpy(temp_r16, "LZ77_homeBtn_ITA.arc");
            strcpy(lbl_8025D0F4, "startup_ita.html");
            break;
        case SC_LANG_NL:
            strcpy(temp_r16, "LZ77_homeBtn_NED.arc");
            strcpy(lbl_8025D0F4, "startup_hol.html");
            break;
#endif
        case SC_LANG_EN:
        default:
            strcpy(temp_r16, "LZ77_homeBtn_ENG.arc");
            strcpy(lbl_8025D0F4, "startup.html");
            OSReport("SC_LANG_ENGLISH\n");
            break;
    }
#endif

    helpMenuReadNAND(&sHandleNAND, sp10, &hbmInfo.layoutBuf, &sMemAllocator2, &sMemAllocator1);

    strcpy(temp_r16, "Huf8_SpeakerSe.arc");
    helpMenuReadNAND(&sHandleNAND, sp10, &hbmInfo.spkSeBuf, &sMemAllocator2, &sMemAllocator1);

    strcpy(temp_r16, "home.csv");
    helpMenuReadNAND(&sHandleNAND, sp10, &hbmInfo.msgBuf, &sMemAllocator2, &sMemAllocator1);

    strcpy(temp_r16, "config.txt");
    helpMenuReadNAND(&sHandleNAND, sp10, &hbmInfo.configBuf, &sMemAllocator2, &sMemAllocator1);

    hbmInfo.sound_callback = helpMenuSoundCallback;
    hbmInfo.backFlag = 0;
    hbmInfo.cursor = 0;
    hbmInfo.adjust.x = 1.3684211f;
    hbmInfo.adjust.y = 1.0f;
    hbmInfo.frameDelta = 1.0f;

    strcpy(temp_r16, "homeBtnIcon.tpl");
    helpMenuReadNAND(&sHandleNAND, sp10, (void**)&lbl_801C7D28, &sMemAllocator2, &sMemAllocator1);
    TPLBind(lbl_801C7D28.pTPLPalette);

    hbmInfo.memSize = 0x80000;
    fn_8008882C(&hbmInfo.mem, 0x80000, &sMemAllocator2, &sMemAllocator1);

    hbmInfo.pAllocator = NULL;
    fn_80088994(&hbmInfo);
    HBMCreate(&hbmInfo);

    fn_8008882C(&lbl_8025D0C4, 0xA0000, &sMemAllocator2, &sMemAllocator1);
    HBMCreateSound("/tmp/HBMSE.brsar", lbl_8025D0C4, 0xA0000);

    HBMInit();
}

bool helpMenuAllocate(HelpMenu* pHelpMenu, void* pObject, s32 nByteCount, HelpMenuCallback callback) {
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

static bool helpMenuAllocateFile(HelpMenu* pHelpMenu, char* szFileName, s32** arg2, MEMAllocator* arg3) {
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

static inline bool helpSetupAllocator(HelpMenu* pHelpMenu) {
    lbl_8025D0E4 = MEMCreateExpHeapEx(pHelpMenu->unk14, 0x700000, 0x0);
    if (lbl_8025D0E4 == NULL) {
        return false;
    }
    MEMInitAllocatorForExpHeap(&sMemAllocator1, lbl_8025D0E4, 0x20);

    lbl_8025D0E0 = MEMCreateExpHeapEx(pHelpMenu->unk1C, 0x2900000, 0x0);
    if (lbl_8025D0E0 == NULL) {
        return false;
    }
    MEMInitAllocatorForExpHeap(&sMemAllocator2, lbl_8025D0E0, 0x20);

    return true;
}

static inline bool helpMenuDestroyHeap(HelpMenu* pHelpMenu) {
    s32 i;
    void* pMVar5;

    MEMDestroyExpHeap(lbl_8025D0E4);
    pMVar5 = MEMDestroyExpHeap(lbl_8025D0E0);

    for (i = 0; i < pHelpMenu->unk10; i++) {
        if (pHelpMenu->unk24[i] != NULL && !pHelpMenu->unk24[i]()) {
            return false;
        }
    }

    return true;
}

static inline void helpMenuUnknownControllerInline() {
    u8 i;

    for (i = 0; i < KPAD_MAX_CONTROLLERS; i++) {
        if (lbl_801CA670.wiiCon[i].kpad != NULL) {
            if (lbl_801CA670.wiiCon[i].use_devtype == 0) {
                if (lbl_801CA670.wiiCon[i].kpad->trig & 0x800) {
                    lbl_8025D0B8 = i;
                }
            } else if ((lbl_801CA670.wiiCon[i].kpad->ex_status).cl.trig & 0x10) {
                lbl_8025D0B8 = i;
            }
        }
    }
}

static inline void helpCopyScreen() {
    s32 iVar9;
    void* pCurrentFrameBuffer;

    lbl_8025D0DC = rmode->fbWidth;
    lbl_8025D0D8 = rmode->efbHeight;

    if (lbl_8025D0C8 == NULL) {
        s32 result = ((u32)((lbl_8025D0DC / 2) * lbl_8025D0D8));
        lbl_8025D0C8 = MEMAllocFromAllocator(&sMemAllocator2, result / 2 * 2);
    }

    GXSetTexCopySrc(0, 0, (u16)lbl_8025D0DC, (u16)lbl_8025D0D8);
    GXSetTexCopyDst(lbl_8025D0DC / 2, lbl_8025D0D8 / 2, GX_TF_RGB565, GX_ENABLE);
    GXCopyTex(lbl_8025D0C8, 0);
    GXDrawDone();

    iVar9 = (lbl_8025D0DC / 2) * lbl_8025D0D8;
    DCInvalidateRange(lbl_8025D0C8, iVar9 / 2 * 2);
    GXInitTexObj(&sTexObj, lbl_8025D0C8, lbl_8025D0DC / 2, lbl_8025D0D8 / 2, GX_TF_RGB565, GX_CLAMP, GX_CLAMP,
                 GX_DISABLE);
    GXInitTexObjLOD(&sTexObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);

    lbl_8025D100[0] = DemoFrameBuffer1;
    lbl_8025D100[1] = DemoFrameBuffer2;
    pCurrentFrameBuffer = VIGetCurrentFrameBuffer();
    sRenderMode = rmode;
    lbl_8025D0FC = (u32)(((s32)DemoFrameBuffer1 - (s32)pCurrentFrameBuffer) |
                         ((s32)pCurrentFrameBuffer - (s32)DemoFrameBuffer1)) >>
                   0x1F;
    GXSetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GXSetDispCopyDst(sRenderMode->fbWidth, sRenderMode->xfbHeight);
}

s32 helpMenuUpdate(HelpMenu* pHelpMenu) {
    Mtx44 matrix44_4;
    Mtx matrix;
    Mtx44 matrix44;
    s32 i;
    bool bVar8 = false;

    if (lbl_801C7D28.unk04 && lbl_801C7D28.pTPLPalette != NULL) {
        GXSetViewport(0.0f, 0.0f, GC_FRAME_WIDTH, GC_FRAME_HEIGHT, 0.0f, 1.0f);
        GXSetScissor(0, 0, GC_FRAME_WIDTH, GC_FRAME_HEIGHT);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);
        C_MTXOrtho(matrix44, 0.0f, 239.0f, 0.0f, 339.0f, 0.0f, 1001.0f);
        GXSetProjection(matrix44, GX_ORTHOGRAPHIC);
        PSMTXIdentity(matrix);
        GXLoadPosMtxImm(matrix, 3);
        helpMenu_8005EDFC();
    }

    if (pHelpMenu->unk08) {
        pHelpMenu->unk08 = false;

        if (!fn_800631B8(SYSTEM_CONTROLLER(gpSystem), 0)) {
            return false;
        }

        if (!helpSetupAllocator(pHelpMenu)) {
            return false;
        }

        helpCopyScreen();

        if (lbl_8025D0C8 != NULL) {
            helpMenuSetupRender(&sTexObj);
        }

        lbl_8025D0FC ^= 1;
        GXCopyDisp(lbl_8025D100[lbl_8025D0FC], GX_TRUE);
        GXDrawDone();
        VIConfigure(sRenderMode);
        VISetNextFrameBuffer(lbl_8025D100[lbl_8025D0FC]);
        VIFlush();
        VIWaitForRetrace();
        fn_80088654(&sMemAllocator1, &sMemAllocator2);
        AIStopDMA();
        lbl_8025D108 = AIRegisterDMACallback(NULL);
        AXInit();
        fn_800B1AB8();
        AXSetMode(0);
        fn_800B1B84(1);
        helpMenuInit(fn_80083140());
        HBMSetAdjustFlag(false);

        for (i = 0; i < PAD_MAX_CONTROLLERS; i++) {
            fn_800CB958(i);
        }

        lbl_801C7D28.unk04 = false;
        nHbmVcmvState = E_GameToHbmTransition1;

        while (!bVar8) {
            fn_80088934();

            if (sResetBtnPressed || sPowerBtnPressed) {
                switch (nHbmVcmvState) {
                    case E_GameToHbmTransition2:
                    case E_Unknown_04:
                        if (!lbl_8025D0F0) {
                            lbl_8025D0F0 = true;
                            HBMStartBlackOut();
                        }
                        break;
                    default:
                        nHbmVcmvState = E_BlackOutTransition;
                        break;
                }
            }

            switch (nHbmVcmvState) {
                case E_InGamePlusBanIcon:
                    break;
                case E_ManualViewerStopped:
                case_8:
                    bVar8 = true;
                    break;
                case E_Unknown_00:
                    lbl_8025D114 = 0;
                    nHbmVcmvState = E_GameToHbmTransition1;
                case E_GameToHbmTransition1:
                    lbl_8025D114++;
                    lbl_8025D110 = 0xFF - lbl_8025D114 * 0xFF / 10;
                    if (lbl_8025D114 < 10) {
                        break;
                    }
                    lbl_8025D114 = -3;
                    nHbmVcmvState = E_GameToHbmTransition2;
                case E_GameToHbmTransition2:
                    lbl_8025D114++;
                    lbl_8025D110 = lbl_8025D114 * 0xFF / 10;
                    if (lbl_8025D114 < 10) {
                        if (lbl_8025D114 <= 0) {
                            lbl_8025D110 = 0;
                        } else {
                            lbl_8025D110 = 0xFF;
                            lbl_8025D114 = 0x0;
                            nHbmVcmvState = E_Unknown_04;
                        }
                    }
                case E_Unknown_04:
                case_4:
                    helpMenuUnknownControllerInline();
                    HBMUpdateSound();
                    if (HBMCalc(&lbl_801CA670) == HBM_SELECT_NULL) {
                        break;
                    }

                    switch (HBMGetSelectBtnNum()) {
                        case HBM_SELECT_BTN1:
                            VISetBlack(true);
                            VIFlush();
                            fn_8000A830(gpSystem, 0x1004, NULL);
                            OSReturnToMenu();
                        case HBM_SELECT_BTN2:
                            lbl_8025D110 = 0;
                            sResetBtnPressed = true;
                            goto case_9;
                        case HBM_SELECT_BTN3:
                            nHbmVcmvState = E_ManualViewerInit;
                            fn_80088660();
                            helpMenuUpdate_UnknownInline();
                            fn_800887C4(lbl_8025D0F8);

                            OSDisableInterrupts();
                            if (!sResetBtnPressed && !sPowerBtnPressed) {
                                nHbmVcmvState = E_ManualViewerRunning;
                                helpMenuRunManualViewer();
                            }
                            fn_80088674();
                            fn_8008876C();
                            fn_800888DC((void**)&lbl_8025D0F8);
                            fn_80088664();
                            OSEnableInterrupts();
                            if (sResetBtnPressed || sPowerBtnPressed) {
                                goto case_9;
                            }
                            lbl_8025D114 = -3;
                            nHbmVcmvState = E_ManualViewerStopped;
                            goto case_8;
                        case HBM_SELECT_HOMEBTN:
                            nHbmVcmvState = E_ManualViewerStopped;
                            goto case_8;
                    }
                case E_HbmToGameTransition:
                    lbl_8025D114++;
                    if (lbl_8025D114 > 0xA) {
                        lbl_8025D114 = 0xA;
                    }
                    lbl_8025D110 = 0xFF;
                    goto case_4;
                case_9:
                    nHbmVcmvState = E_BlackOutTransition;
                case E_BlackOutTransition:
                    if (lbl_8025D110 != 0) {
                        lbl_8025D110 = (lbl_8025D110 - 4) & 0xF8;
                    } else if (sPowerBtnPressed || sResetBtnPressed) {
                        bVar8 = true;
                    }
                    break;
                case E_Unknown_10:
                    if (lbl_8025D110 < 0xFF) {
                        lbl_8025D110 = (lbl_8025D110 + 4) | 7;
                    } else {
                        nHbmVcmvState = E_Unknown_00;
                    }
                    break;
                default:
                    break;
            }

            if (lbl_8025D0C8 != NULL) {
                helpMenuSetupRender(&sTexObj);
            }

            if (fn_8007FC84()) {
                C_MTXOrtho(matrix44_4, 240.0f, -243.84001f, -320.0f, 320.0f, 0.0f, 500.0f);
            } else {
                C_MTXOrtho(matrix44_4, 240.0f, -240.0f, -320.0f, 320.0f, 0.0f, 500.0f);
            }

            GXSetProjection(matrix44_4, GX_ORTHOGRAPHIC);
            GXSetCullMode(GX_CULL_NONE);
            GXClearVtxDesc();
            GXSetVtxAttrFmt(GX_VTXFMT4, GX_VA_POS, GX_POS_XY, GX_F32, 0);
            GXSetVtxAttrFmt(GX_VTXFMT4, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            GXSetNumChans(1);
            GXSetNumTexGens(0);
            GXSetNumTevStages(1);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GXSetTevOp(GX_TEVSTAGE0, GX_BLEND);
            GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
            GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
            GXSetCurrentMtx(3);
            HBMDraw();
            lbl_8025D0FC ^= 1;
            GXCopyDisp(lbl_8025D100[lbl_8025D0FC], GX_TRUE);
            GXDrawDone();
            VIConfigure(sRenderMode);
            VISetNextFrameBuffer(lbl_8025D100[lbl_8025D0FC]);
            VIFlush();
            VIWaitForRetrace();
        }

        if (lbl_8025D0C8 != NULL) {
            MEMFreeToAllocator(&sMemAllocator2, lbl_8025D0C8);
        }

        lbl_8025D0C8 = NULL;

        HBMDeleteSound();
        HBMDelete();
        fn_80083154();
        AXQuit();
        fn_800B1B80();
        AIStopDMA();
        AIRegisterDMACallback(lbl_8025D108);
        fn_800888DC(&hbmInfo.mem);
        contentReleaseHandleNAND(&sHandleNAND);

        if (!fn_800631B8(SYSTEM_CONTROLLER(gpSystem), 1)) {
            return false;
        }

        if (!xlCoreInitGX()) {
            return false;
        }

        frameDrawReset(SYSTEM_FRAME(gpSystem), 0x5FFED);

        if (!helpMenuDestroyHeap(pHelpMenu)) {
            return false;
        }

        lbl_8025D0D0 = OSGetTime();
    }

    if (sPowerBtnPressed) {
        VISetBlack(true);
        VIFlush();
        VIWaitForRetrace();
        fn_8000A830(gpSystem, 0x1004, NULL);
        OSShutdownSystem();
    }

    if (sResetBtnPressed) {
        VISetBlack(true);
        VIFlush();
        VIWaitForRetrace();

        if (!fn_8000A8A8(gpSystem)) {
            return false;
        }
    }

    return true;
}

bool helpMenu_800607B0(HelpMenu* pHelpMenu, bool arg1) {
    pHelpMenu->unk0C = arg1;
    pHelpMenu->unk08 = false;
    return true;
}

bool helpMenu_800607C4(HelpMenu* pHelpMenu, bool arg1) {
    if (!pHelpMenu->unk0C) {
        lbl_801C7D28.unk0C = 0;
        lbl_801C7D28.unk04 = true;
        lbl_801C7D28.unk08 = OSGetTick();
        lbl_801C7D28.unk0D = 0;
    } else {
        if (OSGetTime() - lbl_8025D0D0 < OSMillisecondsToTicks(250)) {
            arg1 = false;
        }

        pHelpMenu->unk08 = arg1 && !lbl_801C7D28.unk04;
    }

    return true;
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
            pHelpMenu->unk0C = true;
            sResetBtnPressed = false;
            sPowerBtnPressed = false;

            OSSetResetCallback(helpMenuResetCallback);
            OSSetPowerCallback(helpMenuPowerCallback);

            lbl_801C7D28.pTPLPalette = NULL;
            lbl_801C7D28.unk04 = false;

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
            pHelpMenu->unk08 = false;
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
