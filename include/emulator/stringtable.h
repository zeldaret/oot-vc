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
    SID_ERROR_DVD_STATE_FATAL_ERROR = 0xEBB077F2,
    SID_ERROR_DVD_STATE_NO_DISK = 0x3A1F93A8,
    SID_ERROR_DVD_STATE_RETRY = 0x25D38BC6,
    SID_ERROR_DVD_STATE_WRONG_DISK = 0x7527596C,
    SID_ERROR_FATAL = 0x27A3CCC2,
    SID_ERROR_GAMECUBE_CONTROLLER_CONNECTED = 0xD6E8882D,
    SID_ERROR_INS_INNODE = 0xE136B8C3,
    SID_ERROR_INS_SPACE = 0xFBB27B1C,
    SID_ERROR_INS_SPACE_PLURAL = 0x92FBDC3E,
    SID_ERROR_MAX_BLOCKS = 0xC0192EA3,
    SID_ERROR_MAX_FILES = 0x5D3A795B,
    SID_ERROR_NEED_CLASSIC = 0x2F8DCDD7,
    SID_ERROR_NO_CONTROLLER = 0x1786067E,
    SID_ERROR_NWC24_CORRUPTED_FILE = 0x19C2F27E,
    SID_ERROR_NWC24_ERROR_BUFFER = 0xB958D7EE,
    SID_ERROR_NWC24_FATAL_ERROR = 0xECC5F9D8,
    SID_ERROR_NWC24_SYSTEM_MENU_UPDATE_REQUIRED = 0x0522BEC0,
    SID_ERROR_NWC24_UNAVAILABLE = 0xE1347C92,
    SID_ERROR_REMOTE_BATTERY = 0x80E449BF,
    SID_ERROR_REMOTE_COMMUNICATION = 0xB35ABA8B,
    SID_ERROR_SYS_CORRUPT = 0x57D16861,
    SID_ERROR_TIME_UP_OF_THE_TRIAL_VER = 0xA2B7EF38,
    SID_COMMENT_GAME_NAME = 0x30690AFB,
    SID_COMMENT_EMPTY = 0x30690AFD,
    SID_ERROR_PHOTO_ALREADY_POSTED = 0x4016043D,
    SID_ERROR_PHOTO_CANT_POST = 0xB93DC9EA,
    SID_ERROR_PHOTO_POSTED = 0x24B1085E,
    SID_ERROR_SYS_CORRUPT_REDOWNLOAD = 0x3D24B5C1,
    SID_ERROR_SYS_MEM_CANT_ACCESS = 0xE46954BD,
    SID_ERROR_SYS_MEM_CANT_ACCESS_2 = 0xEE30A5E0,
} StringID;

typedef struct STEntry {
    /* 0x00 */ u32 nStringID;
    /* 0x04 */ s32 nTextOffset1;
    /* 0x08 */ s32 nTextOffset2; // same as `nTextOffset1`
    /* 0x0C */ s16 nTextSize1;
    /* 0x0E */ s16 nTextSize2; // same as `nTextSize1`
} STEntry; // size = 0x10

typedef struct STHeader {
    /* 0x00 */ s32 magic;
    /* 0x04 */ StringTableID eTableID;
    /* 0x08 */ u16 nEntries;
    /* 0x0A */ char szEncoding[ENCODING_NAME_LENGTH];
    /* 0x1C */ char unk1C[2];
    /* 0x1E */ u8 nSizeEntry;
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
