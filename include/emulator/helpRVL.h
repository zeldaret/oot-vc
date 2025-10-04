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

typedef struct struct_801C7D28 {
    /* 0x00 */ TPLPalette* pTPLPalette;
    /* 0x04 */ bool unk04;
    /* 0x08 */ u32 unk08;
    /* 0x0C */ s8 unk0C;
    /* 0x0D */ u8 unk0D;
} struct_801C7D28; // size = 0x10

typedef struct HelpMenu {
    /* 0x00 */ s32 unk00;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ bool unk08;
    /* 0x0C */ bool unk0C;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ void* unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ void* unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ HelpMenuCallback unk24[8];
} HelpMenu; // size = 0x28

bool helpMenuSetupSize(HelpMenu* pHelpMenu, void* pObject, s32 nByteCount, HelpMenuCallback callback);
s32 helpMenuUpdate(HelpMenu* pHelpMenu);
bool helpMenu_800607B0(HelpMenu* pHelpMenu, bool arg1);
bool helpMenu_800607C4(HelpMenu* pHelpMenu, bool arg1);
bool helpMenuEvent(HelpMenu* pHelpMenu, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassHelpMenu;

#ifdef __cplusplus
}
#endif

#endif
