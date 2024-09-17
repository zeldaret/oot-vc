#include "revolution/demo.h"
#include "revolution/gx.h"
#include "revolution/mem.h"
#include "revolution/mtx.h"
#include "revolution/os.h"
#include "stdarg.h"

static OSFontHeader* FontData;
static void* LastSheet;
static s16 FontSize;
static s16 FontSpace;

void DEMOSetupScrnSpc(s32 width, s32 height, f32 depth) {
    Mtx44 pMtx;
    Mtx mMtx;
    f32 top;

    // fixes float ordering
    (void)0.0f;
    (void)1.0f;
    (void)4503599627370496.0;

    if (DEMOGetRenderModeObj()->field_rendering && !VIGetNextField()) {
        top = -0.667f;
    } else {
        top = 0.0f;
    }

    C_MTXOrtho(pMtx, top, (f32)height, 0.0f, (f32)width, 0.0f, -depth);
    GXSetProjection(pMtx, GX_ORTHOGRAPHIC);
    PSMTXIdentity(mMtx);
    GXLoadPosMtxImm(mMtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

OSFontHeader* DEMOInitROMFont(void) {
    switch (OSGetFontEncode()) {
        case OS_FONT_ENCODE_SJIS:
            if (DemoUseMEMHeap) {
                FontData = MEMAllocFromAllocator(&DemoAllocator1, OS_FONT_SIZE_SJIS);
            } else {
                FontData = OSAllocFromHeap(__OSCurrHeap, OS_FONT_SIZE_SJIS);
            }
            break;
        case OS_FONT_ENCODE_ANSI:
            if (DemoUseMEMHeap) {
                FontData = MEMAllocFromAllocator(&DemoAllocator1, OS_FONT_SIZE_ANSI);
            } else {
                FontData = OSAllocFromHeap(__OSCurrHeap, OS_FONT_SIZE_ANSI);
            }
            break;
        default:
            if (DemoUseMEMHeap) {
                FontData = MEMAllocFromAllocator(&DemoAllocator1, OS_FONT_SIZE_UTF);
            } else {
                FontData = OSAllocFromHeap(__OSCurrHeap, OS_FONT_SIZE_UTF);
            }
            break;
    }

    if (FontData == NULL) {
        OSPanic("DEMOPuts.c", 465, "Ins. memory to load ROM font.");
    }

    if (!OSInitFont(FontData)) {
        OSPanic("DEMOPuts.c", 469, "ROM font is available in boot ROM ver 0.8 or later.");
    }

    FontSize = FontData->cellWidth * 16;
    FontSpace = -16;
    return FontData;
}

void DEMOGetROMFontSize(s16* size, s16* space) {
    if (size) {
        *size = FontSize / 16;
    }

    if (space) {
        *space = FontSpace / 16;
    }
}

static void DrawFontChar(int x, int y, int z, int xChar, int yChar) {
    s16 posLeft = x;
    s16 posRight = posLeft + FontSize;
    s16 posTop = y - (FontData->ascent * FontSize / FontData->cellWidth);
    s16 posBottom = y + (FontData->descent * FontSize / FontData->cellWidth);
    s16 texLeft = xChar;
    s16 texRight = xChar + FontData->cellWidth;
    s16 texTop = yChar;
    s16 texBottom = yChar + FontData->cellHeight;

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3s16(posLeft, posTop, z);
    GXTexCoord2s16(texLeft, texTop);
    GXPosition3s16(posRight, posTop, z);
    GXTexCoord2s16(texRight, texTop);
    GXPosition3s16(posRight, posBottom, z);
    GXTexCoord2s16(texRight, texBottom);
    GXPosition3s16(posLeft, posBottom, z);
    GXTexCoord2s16(texLeft, texBottom);
    GXEnd();
}

static inline void LoadSheet(void* image, GXTexMapID texMapID) {
    Mtx mtx;
    GXTexObj texObj;

    if (LastSheet == image) {
        return;
    }
    LastSheet = image;

    GXInitTexObj(&texObj, image, FontData->sheetWidth, FontData->sheetHeight, (GXTexFmt)FontData->sheetFormat, GX_CLAMP,
                 GX_CLAMP, GX_FALSE);

    GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_DISABLE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, texMapID);
    PSMTXScale(mtx, 1.0f / FontData->sheetWidth, 1.0f / FontData->sheetHeight, 1.0f);
    GXLoadTexMtxImm(mtx, GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
}

static s32 DEMORFPuts(s16 x, s16 y, s16 z, const char* string) {
    u32 cx;
    void* image;
    u32 xChar;
    u32 yChar;
    s32 width;

    LastSheet = 0;

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 4);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 0);

    x *= 16;
    y *= 16;
    z *= 16;

    width = 0;
    while (*string) {
        if (*string == '\n') {
            width = 0;
            y += FontData->leading * FontSize / FontData->cellWidth;
            ++string;
            continue;
        }

        if (*string == '\t') {
            width += 8 * (FontSize + FontSpace);
            width -= width % (8 * (FontSize + FontSpace));
            ++string;
            continue;
        }

        string = OSGetFontTexture(string, &image, &xChar, &yChar, &cx);

        LoadSheet(image, GX_TEXMAP0);
        DrawFontChar(x + width, y, z, xChar, yChar);
        width += (s32)(FontSize * cx) / FontData->cellWidth + FontSpace;
    }

    return (width + 15) / 16;
}

void DEMOPrintf(s16 x, s16 y, s16 z, char* fmt, ...) {
    va_list vlist;
    char buf[256];

    va_start(vlist, fmt);
    vsprintf(buf, fmt, vlist);
    DEMORFPuts(x, y, z, buf);
    va_end(vlist);
}

s32 DEMOGetRFTextWidth(const char* string) {
    s32 cx;
    s32 width;
    s32 maxWidth;

    maxWidth = 0;
    width = 0;

    while (*string) {
        if (*string == '\n') {
            if (maxWidth < width) {
                maxWidth = width;
            }
            width = 0;
        }
        string = OSGetFontWidth(string, (u32*)&cx);
        width += FontSize * cx / FontData->cellWidth + FontSpace;
    }

    if (maxWidth < width) {
        maxWidth = width;
    }

    return (maxWidth + 15) / 16;
}

s32 DEMOGetRFTextHeight(const char* string) {
    s32 height = 1;

    while (*string) {
        if (*string == '\n') {
            ++height;
        }
        ++string;
    }

    height *= FontData->leading * FontSize / FontData->cellWidth;
    return (height + 15) / 16;
}

static char unused1[] = "%08x%08x%08x%08x%08x%08x\n";
static char unused2[] = "\nwidth %d\n";
