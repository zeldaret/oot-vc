#ifndef _RVL_SDK_OS_FONT_H
#define _RVL_SDK_OS_FONT_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OS_FONT_ENCODE_ANSI,
    OS_FONT_ENCODE_SJIS,
    OS_FONT_ENCODE_2,
    OS_FONT_ENCODE_UTF8,
    OS_FONT_ENCODE_UTF16,
    OS_FONT_ENCODE_UTF32,
    OS_FONT_ENCODE_MAX
} OSFontEncode;

typedef struct OSFontHeader {
    /* 0x0 */ u16 type;
    /* 0x2 */ u16 firstChar;
    /* 0x4 */ u16 lastChar;
    /* 0x6 */ u16 invalidChar;
    /* 0x8 */ u16 ascent;
    /* 0xA */ u16 descent;
    /* 0xC */ u16 width;
    /* 0xE */ u16 leading;
    /* 0x10 */ u16 cellWidth;
    /* 0x12 */ u16 cellHeight;
    /* 0x14 */ u32 sheetSize;
    /* 0x18 */ u16 sheetFormat;
    /* 0x1A */ u16 sheetNumCol;
    /* 0x1C */ u16 sheetNumRow;
    /* 0x1E */ u16 sheetWidth;
    /* 0x20 */ u16 sheetHeight;
    /* 0x22 */ u16 widthTableOfs;
    /* 0x24 */ u32 sheetImageOfs;
    /* 0x28 */ u32 sheetFullSize;
    /* 0x2C */ u8 c0;
    /* 0x2D */ u8 c1;
    /* 0x2E */ u8 c2;
    /* 0x2F */ u8 c3;
} OSFontHeader;

u16 OSGetFontEncode(void);
u16 OSSetFontEncode(u16 encode);
u32 OSLoadFont(OSFontHeader* font, void* dst);
const char* OSGetFontTexel(const char* str, void* dst, s32 xOfs, s32 arg3, u32* widthOut);
bool OSInitFont(OSFontHeader* font);
const char* OSGetFontTexture(const char* str, void** texOut, u32* xOut, u32* yOut, u32* widthOut);
const char* OSGetFontWidth(const char* str, u32* widthOut);

#ifdef __cplusplus
}
#endif

#endif
