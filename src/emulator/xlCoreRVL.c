#include "emulator/xlCoreRVL.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlHeap.h"
#include "emulator/xlList.h"
#include "emulator/xlPostRVL.h"
#include "macros.h"
#include "revolution/demo.h"
#include "revolution/gx.h"
#include "revolution/sc.h"
#include "revolution/vi.h"

static GXRenderModeObj rmodeobj;
static s32 gnCountArgument;
static char** gaszArgument;
static void* DefaultFifo;
static GXFifoObj* DefaultFifoObj;
GXRenderModeObj* rmode;

//! TODO: confirm these
CNTFileInfo gCNTFileInfo;
UnknownContentStruct gUnkContent;

//! TODO: find these function names
u32 fn_800B6F8C();

#define EMU_ROUND_UP(v, x) (((v) + ((x) - 1)) & ~((x) - 1))

static inline u32 getFBTotalSize(f32 aspectRatio) {
    u16 lineCount = GXGetNumXfbLines(rmode->efbHeight, aspectRatio);
    u16 fbWith = EMU_ROUND_UP(rmode->fbWidth, 16);
    return fbWith * lineCount;
}

static void xlCoreInitRenderMode(GXRenderModeObj* mode) {
    u32 nTickLast;

    SCInit();

    nTickLast = OSGetTick();
    while (SCCheckStatus() == 1 && OS_TICKS_TO_MSEC(OSGetTick() - nTickLast) < 3000) {}

    if (mode != NULL) {
        rmode = mode;
        return;
    }

    switch (VIGetTvFormat()) {
        case VI_NTSC:
            rmode = VIGetDTVStatus() && ((fn_800B6F8C() & 0xFF) == 1) ? &GXNtsc480Prog : &GXNtsc480IntDf;
            rmode->viXOrigin -= 32;
            rmode->viWidth += 64;
            break;
        case VI_PAL:
        case VI_MPAL:
        case VI_EURGB60:
            rmode = &GXPal528IntDf;
            rmode->viXOrigin -= 32;
            rmode->viWidth += 64;
            rmode->xfbHeight = rmode->viHeight = 574;
            rmode->viYOrigin = (s32)(574 - rmode->viHeight) / 2;
            break;
        default:
            OSPanic("xlCoreRVL.c", 138, "DEMOInit: invalid TV format\n");
            break;
    }

    rmode->efbHeight = 480;
    GXAdjustForOverscan(rmode, &rmodeobj, 0, 0);
    rmode = &rmodeobj;
}

static inline void __xlCoreInitGX(void) {
    GXSetViewport(0.0f, 0.0f, rmode->fbWidth, rmode->efbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GXSetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GXSetDispCopyDst(rmode->fbWidth, rmode->xfbHeight);
    GXSetDispCopyYScale((f32)rmode->xfbHeight / (f32)rmode->efbHeight);
    GXSetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);

    if (rmode->aa != 0) {
        GXSetPixelFmt(GX_PF_RGBA565_Z16, GX_ZC_LINEAR);
    } else {
        GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

    GXSetDispCopyGamma(GX_GM_1_0);

    VISetNextFrameBuffer(DemoFrameBuffer1);
    DemoCurrentBuffer = DemoFrameBuffer2;
    VIFlush();
    VIWaitForRetrace();

    if (rmode->viTVmode & 1) {
        VIWaitForRetrace();
    }
}

bool xlCoreInitGX(void) {
    __xlCoreInitGX();

    VIConfigure(rmode);

    return true;
}

bool xlCoreBeforeRender(void) {
    if (rmode->field_rendering != 0) {
        GXSetViewportJitter(0.0f, 0.0f, rmode->fbWidth, rmode->efbHeight, 0.0f, 1.0f, VIGetNextField());
    } else {
        GXSetViewport(0.0f, 0.0f, rmode->fbWidth, rmode->efbHeight, 0.0f, 1.0f);
    }

    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    GXSetDrawSync(0);

    return true;
}

s32 xlCoreGetArgumentCount(void) { return gnCountArgument; }

bool xlCoreGetArgument(s32 iArgument, char** pszArgument) {
    if ((iArgument >= 0) && (iArgument < gnCountArgument)) {
        *pszArgument = *(gaszArgument + iArgument);
        return true;
    }

    return false;
}

bool xlCoreHiResolution(void) { return true; }

bool fn_8007FC84(void) {
    switch (VIGetTvFormat()) {
        case VI_PAL:
        case VI_MPAL:
        case VI_EURGB60:
            return true;
        default:
            break;
    }

    return false;
}

void xlExit(void) { OSPanic("xlCoreRVL.c", 524, "xlExit"); }

int main(int nCount, char** aszArgument) {
    s32 nSizeHeap;
    s32 nSize;

    f32 aspectRatio;

    gnCountArgument = nCount;
    gaszArgument = aszArgument;

    OSInit();
    DVDInit();

    if (NANDInit() != 0) {
        xlExit();
    }

    fn_800FEFB8();
    VIInit();
    xlCoreInitRenderMode(NULL);
    VIConfigure(rmode);

#ifdef __MWERKS__ // clang-format off
    asm {
        li      r3, 0x4
        oris    r3, r3, 0x4
        mtspr   GQR2, r3
        li      r3, 0x5
        oris    r3, r3, 0x5
        mtspr   GQR3, r3
        li      r3, 0x6
        oris    r3, r3, 0x6
        mtspr   GQR4, r3
        li      r3, 0x7
        oris    r3, r3, 0x7
        mtspr   GQR5, r3
    }
#endif // clang-format on

    if (!xlPostSetup()) {
        return false;
    }

    if (!xlHeapSetup()) {
        return false;
    }

    if (!xlListSetup()) {
        return false;
    }

    if (!xlObjectSetup()) {
        return false;
    }

    aspectRatio = (f32)rmode->xfbHeight / (f32)rmode->efbHeight;
    nSizeHeap = fn_8007FC84() ? 0xBB800 : 0x87600;
    nSize = getFBTotalSize(aspectRatio) * 2;

    if (nSize < nSizeHeap) {
        nSize = nSizeHeap;
    }

    xlHeapTake(&DemoFrameBuffer1, nSize | 0x70000000);
    xlHeapTake(&DemoFrameBuffer2, nSize | 0x70000000);
    xlHeapFill32(DemoFrameBuffer1, nSize, 0);
    xlHeapFill32(DemoFrameBuffer2, nSize, 0);
    DCStoreRange(DemoFrameBuffer1, nSize);
    DCStoreRange(DemoFrameBuffer2, nSize);

    xlHeapTake(&DefaultFifo, 0x40000 | 0x30000000);
    DefaultFifoObj = GXInit(DefaultFifo, 0x40000);

    __xlCoreInitGX();
    fn_80063C7C();
    xlMain();

    if (!xlObjectReset()) {
        return false;
    }

    if (!xlListReset()) {
        return false;
    }

    if (!xlHeapReset()) {
        return false;
    }

    if (!xlPostReset()) {
        return false;
    }

    OSPanic("xlCoreRVL.c", 603, "CORE DONE!");
    return false;
}
