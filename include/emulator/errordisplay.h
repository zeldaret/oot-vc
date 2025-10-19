#ifndef _ERRORDISPLAY_H
#define _ERRORDISPLAY_H

#include "emulator/banner.h"
#include "emulator/stringtable.h"
#include "revolution/gx.h"
#include "revolution/sc.h"
#include "revolution/types.h"
#include "versions.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FADE_TIMER_MAX 15

#define FLAG_COLOR_WHITE (0 << 0)
#define FLAG_COLOR_YELLOW (1 << 0)
#define FLAG_RESET_FADE_TIMER (1 << 1)

typedef enum ErrorIndex {
    // (nothing)
    ERROR_NONE = -1,
    // "There is not enough available space in the Wii system memory. Create %ld block(s) of free space by either moving
    // files to an SD Card or deleting files in the Data Management Screen."
    ERROR_INS_SPACE,
    // "Press the A Button to return to the Wii Menu."
    ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU,
    // "There is not enough available space in the Wii system memory. Either move files to an SD Card or delete files on
    // the Data Management Screen."
    ERROR_INS_INNODE,
    // "The Wii system memory has been damaged. Refer to the Wii operations manual for further instructions."
    ERROR_SYS_CORRUPT,
    // "This file cannot be used because the data is corrupted."
    ERROR_DATA_CORRUPT,
    // "There is no more available space in Wii system memory. Refer to the Wii operations manual for further
    // information."
    ERROR_MAX_BLOCKS,
    // "There is no more available space in Wii system memory."
    ERROR_MAX_FILES,
    // "You will need the Classic Controller."
    ERROR_NO_CONTROLLER,
    // "Connect Classic Controller to the P1 Wii Remote or press the A Button to return to the Wii Menu."
    ERROR_NEED_CLASSIC,
    // "The battery charge is running low."
    ERROR_REMOTE_BATTERY,
    // "Communications with the Wii Remote have been interrupted."
    ERROR_REMOTE_COMMUNICATION,
#if VERSION >= OOT_J
    // ""
    ERROR_BLANK,
#endif
    // (nothing)
    ERROR_NULL,
    ERROR_MAX = ERROR_NULL
} ErrorIndex;

typedef struct DisplayFiles {
    /* 0x00 */ SCLanguage eLanguage;
    /* 0x04 */ char* szErrorsFilename;
    /* 0x08 */ char* szSaveCommentsFilename;
} DisplayFiles; // size = 0xC

typedef struct EDString {
    /* 0x00 */ struct ErrorDisplay* apStringDraw[ERROR_MAX];
    /* 0x30 */ ErrorIndex iString;
    /* 0x34 */ s32 iAction;
} EDString; // size = 0x38

typedef s32 (*ErrorCallback)(EDString*);
typedef struct EDStringInfo {
    /* 0x00 */ StringID eStringID;
    /* 0x04 */ s32 nLines;
    /* 0x08 */ char* szString;
    /* 0x0C */ s32 unk0C;
    /* 0x10 */ s32 unk10;
} EDStringInfo; // size = 0x10

#if VERSION == SM64_J || VERSION == SM64_U
typedef struct EDMessage {
    /* 0x00 */ StringID eStringID;
    /* 0x14 */ s16 nFlags; // bitfield
    /* 0x06 */ s16 nFadeInTimer;
    /* 0x08 */ s32 nShiftY; // Y position relative to nStartY
    EDStringInfo* pStringInfo;
} EDMessage; // size = 0xC

typedef struct EDAction {
    /* 0x00 */ EDMessage message;
    s32 unk_04;
    s32 unk_05;
    /* 0x0C */ ErrorCallback callback;
} EDAction; // size = 0x10

typedef struct ErrorDisplay {
    struct {
        EDMessage message;
        s32 unk_01;
        s32 nShiftY;
    };
    EDAction action[2];
    s32 nAction;
    ErrorCallback callback;
    s16 nStartY;
    s16 unk36; // unused?
    s32 unk38;
    s32 unk3C;
} ErrorDisplay; // size = 0x64
#else
typedef struct EDMessage {
    /* 0x00 */ EDStringInfo* pStringInfo;
    /* 0x04 */ s16 nFlags; // bitfield
    /* 0x06 */ s16 nFadeInTimer;
    /* 0x08 */ s32 nShiftY; // Y position relative to nStartY
} EDMessage; // size = 0xC

typedef struct EDAction {
    /* 0x00 */ EDMessage message;
    /* 0x0C */ ErrorCallback callback;
} EDAction; // size = 0x10

typedef struct ErrorDisplay {
    /* 0x00 */ EDMessage message;
    /* 0x0C */ EDAction action[2];
    /* 0x2C */ s32 nAction;
    /* 0x30 */ ErrorCallback callback;
    /* 0x34 */ s16 nStartY;
    /* 0x36 */ s16 unk36; // unused?
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
} ErrorDisplay; // size = 0x40
#endif

typedef struct struct_80174988 {
    NANDResult result;
    ErrorIndex eStringIndex;
} struct_80174988;

GXRenderModeObj* DEMOGetRenderModeObj(void);
void* OSAllocFromHeap(s32 handle, s32 size);
void OSFreeToHeap(s32 handle, void* p);
void errorDisplayInit(void);
bool errorDisplayShow(ErrorIndex iString);

extern ErrorDisplay sStringDraw[];
extern struct_80174988 lbl_80174988[17];
extern bool lbl_8025D130;
extern s32 lbl_8025D12C;

#ifdef __cplusplus
}
#endif

#endif
