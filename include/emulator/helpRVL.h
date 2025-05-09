#ifndef _HELP_RVL
#define _HELP_RVL

#include "emulator/xlObject.h"
#include "revolution/cnt.h"
#include "revolution/tpl.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*HelpMenuCallback)(void);

typedef struct struct_801C7D38 {
    /* 0x00 */ void* pBuffer1;
    /* 0x04 */ void* pBuffer2;
    /* 0x08 */ void* pBuffer3;
    /* 0x0C */ void* pBuffer4;
    /* 0x10 */ void* unk20;
    /* 0x14 */ void* unk24;
    /* 0x18 */ s32 unk28;
    /* 0x1C */ TPLPalette* pTPLPalette;
    /* 0x20 */ s32 unk30;
    /* 0x24 */ s32 unk34;
    /* 0x28 */ s32 unk38;
    /* 0x2C */ f32 unk3C;
    /* 0x30 */ f32 unk40;
    /* 0x34 */ f32 unk44;
    /* 0x38 */ s32 unk48;
} struct_801C7D38; // size = 0x3C

typedef struct struct_801C7D28 {
    /* 0x00 */ TPLPalette* pTPLPalette;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ u32 unk08;
    /* 0x0C */ s8 unk0C;
    /* 0x0D */ u8 unk0D;
    /* 0x0E */ u8 pad[2];
} struct_801C7D28; // size = 0x10

typedef struct HelpMenu {
    /* 0x00 */ s32 unk00;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 unk0C;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ void* unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ void* unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ HelpMenuCallback unk24[8];
} HelpMenu; // size = 0x28

bool fn_8005F5F4(HelpMenu* pHelpMenu, void* pObject, s32 arg2, HelpMenuCallback callback);
bool helpMenuEvent(HelpMenu* pHelpMenu, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassHelpMenu;

#ifdef __cplusplus
}
#endif

#endif
