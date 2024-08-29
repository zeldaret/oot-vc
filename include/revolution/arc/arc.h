/**
 * Modified from decompilation by riidefi in WiiCore
 */

#ifndef _RVL_SDK_ARC_H
#define _RVL_SDK_ARC_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ARC_ENTRY_FILE,
    ARC_ENTRY_FOLDER
} ARCEntryType;

typedef struct ARCNode {
    /* 0x0 */ union {
        struct {
            u32 is_folder : 8;
            u32 name : 24;
        };
        u32 packed_type_name;
    };

    /* 0x4 */ union {
        struct {
            u32 offset;
            u32 size;
        } file;

        struct {
            u32 parent;
            u32 sibling_next;
        } folder;
    };
} ARCNode;

typedef struct ARCHeader {
    /* 0x0 */ u32 magic;

    struct {
        /* 0x4 */ s32 offset;
        /* 0x8 */ s32 size;
    } nodes;

    struct {
        /* 0xC */ s32 offset;
    } files;

    char UNK_0x10[0x10];
} ARCHeader;

typedef struct ARCHandle {
    /* 0x0 */ ARCHeader* header;
    /* 0x4 */ ARCNode* nodes;
    /* 0x8 */ u8* file;
    /* 0xC */ u32 count;
    /* 0x10 */ const char* strings;
    /* 0x14 */ u32 fstSize;
    /* 0x18 */ s32 entrynum;
} ARCHandle;

typedef struct ARCFileInfo {
    /* 0x0 */ ARCHandle* handle;
    /* 0x4 */ u32 offset;
    /* 0x8 */ u32 size;
} ARCFileInfo;

typedef struct ARCEntry {
    /* 0x0 */ ARCHandle* handle;
    /* 0x4 */ u32 path;
    /* 0x8 */ ARCEntryType type;
    /* 0xC */ const char* name;
} ARCEntry;

typedef struct ARCDir {
    /* 0x0 */ ARCHandle* handle;
    /* 0x4 */ u32 path_begin;
    /* 0x8 */ u32 path_it;
    /* 0xC */ u32 path_end;
} ARCDir;

bool ARCGetCurrentDir(ARCHandle* handle, char* string, u32 maxlen);
bool ARCInitHandle(void* bin, ARCHandle* handle);
bool ARCOpen(ARCHandle* handle, const char* path, ARCFileInfo* info);
bool ARCFastOpen(ARCHandle* handle, s32 entrynum, ARCFileInfo* info);
s32 ARCConvertPathToEntrynum(ARCHandle* handle, const char* path);
void* ARCGetStartAddrInMem(ARCFileInfo* info);
s32 ARCGetStartOffset(ARCFileInfo* info);
u32 ARCGetLength(ARCFileInfo* info);
bool ARCClose(ARCFileInfo* info);
bool ARCChangeDir(ARCHandle* info, const char* path);
bool ARCOpenDir(ARCHandle* info, const char* path, ARCDir* dir);
bool ARCReadDir(ARCDir* dir, ARCEntry* entry);
bool ARCCloseDir(ARCDir* dir);

#ifdef __cplusplus
}
#endif

#endif
