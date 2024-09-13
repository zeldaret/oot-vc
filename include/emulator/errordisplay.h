#ifndef _CODE_800633F8_H
#define _CODE_800633F8_H

#include "emulator/banner.h"
#include "emulator/stringtable.h"
#include "revolution/gx.h"
#include "revolution/sc.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FADE_TIMER_MAX 15

#define FLAG_COLOR_WHITE (0 << 0)
#define FLAG_COLOR_YELLOW (1 << 0)
#define FLAG_RESET_FADE_TIMER (1 << 1)

typedef enum ErrorIndex {
    // (nothing)
    ERROR_INDEX_NONE = -1,
    // "There is not enough available space in the Wii system memory. Create %ld block(s) of free space by either moving
    // files to an SD Card or deleting files in the Data Management Screen."
    ERROR_INDEX_INS_SPACE = 0,
    // "Press the A Button to return to the Wii Menu."
    ERROR_INDEX_CHOICE_PRESS_A_TO_RETURN_TO_MENU = 1,
    // "There is not enough available space in the Wii system memory. Either move files to an SD Card or delete files on
    // the Data Management Screen."
    ERROR_INDEX_INS_INNODE = 2,
    // "The Wii system memory has been damaged. Refer to the Wii operations manual for further instructions."
    ERROR_INDEX_SYS_CORRUPT = 3,
    // "This file cannot be used because the data is corrupted."
    ERROR_INDEX_DATA_CORRUPT = 4,
    // "There is no more available space in Wii system memory. Refer to the Wii operations manual for further
    // information."
    ERROR_INDEX_MAX_BLOCKS = 5,
    // "There is no more available space in Wii system memory."
    ERROR_INDEX_MAX_FILES = 6,
    // "You will need the Classic Controller."
    ERROR_INDEX_NO_CONTROLLER = 7,
    // "Connect Classic Controller to the P1 Wii Remote or press the A Button to return to the Wii Menu."
    ERROR_INDEX_NEED_CLASSIC = 8,
    // "The battery charge is running low."
    ERROR_INDEX_REMOTE_BATTERY = 9,
    // "Communications with the Wii Remote have been interrupted."
    ERROR_INDEX_REMOTE_COMMUNICATION = 10,
    // ""
    ERROR_INDEX_BLANK = 11,
    // (nothing)
    ERROR_INDEX_NULL = 12,
} ErrorIndex;

typedef struct DisplayFiles {
    /* 0x00 */ SCLanguage eLanguage;
    /* 0x04 */ char* szErrorsFilename;
    /* 0x08 */ char* szSaveCommentsFilename;
} DisplayFiles; // size = 0xC

typedef struct EDString {
    /* 0x00 */ struct ErrorDisplay* apStringDraw[ERROR_INDEX_NULL];
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

typedef struct struct_80174988 {
    NANDResult result;
    ErrorIndex eStringIndex;
} struct_80174988;

GXRenderModeObj* DEMOGetRenderModeObj(void);
void* OSAllocFromHeap(s32 handle, s32 size);
void OSFreeToHeap(s32 handle, void* p);
void errordisplayInit(void);
bool errordisplayShow(ErrorIndex iString);

extern ErrorDisplay sStringDraw[12];
extern struct_80174988 lbl_80174988[17];
extern bool lbl_8025D130;
extern s32 lbl_8025D12C;

#ifdef __cplusplus
}
#endif

#endif
