#ifndef _STRINGTABLE_H
#define _STRINGTABLE_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENCODING_NAME_LENGTH 18

typedef enum StringTableID {
    TID_NONE = 0,
    TID_ERRORS = 0x2842C987,
    TID_COMMENTS = 0x3D2C2A7C
} StringTableID;

typedef enum StringID {
    SID_NONE = 0,
    SID_ERROR_BLANK = 0xA4E943CC,
    SID_ERROR_CHOICE_OK = 0xF6C5198F,
    SID_ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU = 0xBB628DD1,
    SID_ERROR_DATA_CORRUPT = 0x14A5B6E2,
    SID_ERROR_DVD_STATE_FATAL_ERROR = 0xEBB077F2, // Brawl Demo only
    SID_ERROR_DVD_STATE_NO_DISK = 0x3A1F93A8, // Brawl Demo only
    SID_ERROR_DVD_STATE_RETRY = 0x25D38BC6, // Brawl Demo only
    SID_ERROR_DVD_STATE_WRONG_DISK = 0x7527596C, // Brawl Demo only
    SID_ERROR_FATAL = 0x27A3CCC2,
    SID_ERROR_GAMECUBE_CONTROLLER_CONNECTED = 0xD6E8882D,
    SID_ERROR_INS_INNODE = 0xE136B8C3,
    SID_ERROR_INS_SPACE = 0xFBB27B1C,
    SID_ERROR_INS_SPACE_PLURAL = 0x92FBDC3E, // Brawl Demo only
    SID_ERROR_MAX_BLOCKS = 0xC0192EA3,
    SID_ERROR_MAX_FILES = 0x5D3A795B,
    SID_ERROR_NEED_CLASSIC = 0x2F8DCDD7,
    SID_ERROR_NO_CONTROLLER = 0x1786067E,
    SID_ERROR_NWC24_CORRUPTED_FILE = 0x19C2F27E, // Brawl Demo only
    SID_ERROR_NWC24_ERROR_BUFFER = 0xB958D7EE, // Brawl Demo only
    SID_ERROR_NWC24_FATAL_ERROR = 0xECC5F9D8, // Brawl Demo only
    SID_ERROR_NWC24_SYSTEM_MENU_UPDATE_REQUIRED = 0x0522BEC0, // Brawl Demo only
    SID_ERROR_NWC24_UNAVAILABLE = 0xE1347C92, // Brawl Demo only
    SID_ERROR_REMOTE_BATTERY = 0x80E449BF,
    SID_ERROR_REMOTE_COMMUNICATION = 0xB35ABA8B,
    SID_ERROR_SYS_CORRUPT = 0x57D16861,
    SID_ERROR_TIME_UP_OF_THE_TRIAL_VER = 0xA2B7EF38, // Brawl Demo only
    SID_COMMENT_GAME_NAME = 0x30690AFB,
    SID_COMMENT_EMPTY = 0x30690AFD
} StringID;

// clang-format off

typedef enum StringIndex {
    SI_NONE = -1,
    SI_ERROR_INS_SPACE = 0, // "There is not enough available space in the Wii system memory. Create %ld block(s) of free space by either moving files to an SD Card or deleting files in the Data Management Screen."
    SI_ERROR_CHOICE_PRESS_A_TO_RETURN_TO_MENU = 1, // "Press the A Button to return to the Wii Menu."
    SI_ERROR_INS_INNODE = 2, // "There is not enough available space in the Wii system memory. Either move files to an SD Card or delete files on the Data Management Screen."
    SI_ERROR_SYS_CORRUPT = 3, // "The Wii system memory has been damaged. Refer to the Wii operations manual for further instructions."
    SI_ERROR_DATA_CORRUPT = 4, // "This file cannot be used because the data is corrupted."
    SI_ERROR_MAX_BLOCKS = 5, // "There is no more available space in Wii system memory. Refer to the Wii operations manual for further information."
    SI_ERROR_MAX_FILES = 6, // "There is no more available space in Wii system memory."
    SI_ERROR_NO_CONTROLLER = 7, // "You will need the Classic Controller."
    SI_ERROR_NEED_CLASSIC = 8, // "Connect Classic Controller to the P1 Wii Remote or press the A Button to return to the Wii Menu."
    SI_ERROR_REMOTE_BATTERY = 9, // "The battery charge is running low."
    SI_ERROR_REMOTE_COMMUNICATION = 10, // "Communications with the Wii Remote have been interrupted."
    SI_ERROR_BLANK = 11, // ""
    SI_NULL = 12,
} StringIndex;

// clang-format on

typedef struct STEntry {
    /* 0x00 */ u32 nStringID;
    /* 0x04 */ s32 nTextOffset1;
    /* 0x08 */ s32 nTextOffset2;
    /* 0x0C */ s16 nTextSize1;
    /* 0x0E */ s16 nTextSize2;
} STEntry; // size = 0x10

typedef struct STHeader {
    /* 0x00 */ s32 magic;
    /* 0x04 */ StringTableID eTableID;
    /* 0x08 */ u16 nEntries;
    /* 0x0A */ char szEncoding[ENCODING_NAME_LENGTH];
    /* 0x1C */ char unk1C[2];
    /* 0x1E */ u8 nSizeEntry;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ u8* entries;
} STHeader; // size = 0x24

typedef struct StringTable {
    /* 0x00 */ STHeader header;
    /* 0x24 */ char* szStrings;
} StringTable; // size = 0x28

STEntry* tableGetEntry(StringTable* pStringTable, StringID eStringID);
char* tableGetString(void* pSTBuffer, StringID eStringID);

#ifdef __cplusplus
}
#endif

#endif
