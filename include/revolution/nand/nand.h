#ifndef _RVL_SDK_NAND_H
#define _RVL_SDK_NAND_H

#include "revolution/fs.h"
#include "revolution/types.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NAND_BANNER_TITLE_MAX 32
#define NAND_BANNER_ICON_MAX_FRAME 8

// Forward declarations
typedef struct NANDCommandBlock;

typedef enum {
    NAND_RESULT_FATAL_ERROR = -128,
    NAND_RESULT_UNKNOWN = -64,

    NAND_RESULT_MAXDEPTH = -16,
    NAND_RESULT_AUTHENTICATION,
    NAND_RESULT_OPENFD,
    NAND_RESULT_NOTEMPTY,
    NAND_RESULT_NOEXISTS,
    NAND_RESULT_MAXFILES,
    NAND_RESULT_MAXFD,
    NAND_RESULT_MAXBLOCKS,
    NAND_RESULT_INVALID,

    NAND_RESULT_EXISTS = -6,
    NAND_RESULT_ECC_CRIT,
    NAND_RESULT_CORRUPT,
    NAND_RESULT_BUSY,
    NAND_RESULT_ALLOC_FAILED,
    NAND_RESULT_ACCESS,

    NAND_RESULT_OK,
} NANDResult;

typedef enum {
    NAND_SEEK_BEG,
    NAND_SEEK_CUR,
    NAND_SEEK_END,
} NANDSeekMode;

typedef enum {
    NAND_ACCESS_NONE,
    NAND_ACCESS_READ,
    NAND_ACCESS_WRITE,
    NAND_ACCESS_RW
} NANDAccessType;

typedef enum {
    NAND_FILE_TYPE_NONE,
    NAND_FILE_TYPE_FILE,
    NAND_FILE_TYPE_DIR,
} NANDFileType;

typedef enum {
    // Read/write by owner
    NAND_PERM_RUSR = (NAND_ACCESS_READ << 4),
    NAND_PERM_WUSR = (NAND_ACCESS_WRITE << 4),
    // Read/write by group
    NAND_PERM_RGRP = (NAND_ACCESS_READ << 2),
    NAND_PERM_WGRP = (NAND_ACCESS_WRITE << 2),
    // Read/write by other
    NAND_PERM_ROTH = (NAND_ACCESS_READ << 0),
    NAND_PERM_WOTH = (NAND_ACCESS_WRITE << 0),
    // Read/write by all
    NAND_PERM_RALL = NAND_PERM_RUSR | NAND_PERM_RGRP | NAND_PERM_ROTH,
    NAND_PERM_WALL = NAND_PERM_WUSR | NAND_PERM_WGRP | NAND_PERM_WOTH,
    NAND_PERM_RWALL = NAND_PERM_RALL | NAND_PERM_WALL
} NANDPermission;

typedef void (*NANDAsyncCallback)(s32 result, struct NANDCommandBlock* block);

typedef struct NANDStatus {
    /* 0x0 */ u32 ownerId;
    /* 0x4 */ u16 groupId;
    /* 0x6 */ u8 attr;
    /* 0x7 */ u8 perm;
} NANDStatus;

typedef struct NANDFileInfo {
    /* 0x0 */ s32 fd;
    /* 0x4 */ s32 tempFd;
    /* 0x8 */ char openPath[FS_MAX_PATH];
    /* 0x48 */ char tempPath[FS_MAX_PATH];
    /* 0x88 */ u8 access;
    /* 0x89 */ u8 stage;
    /* 0x8A */ u8 mark;
    u32 unk_8B;
} NANDFileInfo;

typedef struct NANDCommandBlock {
    /* 0x0 */ void* userData;
    /* 0x4 */ NANDAsyncCallback callback;
    /* 0x8 */ NANDFileInfo* info;
    /* 0xC */ void* bytes;
    /* 0x10 */ void* inodes;
    /* 0x14 */ NANDStatus* status;
    /* 0x18 */ u32 ownerId;
    /* 0x1C */ u16 groupId;
    /* 0x1E */ u8 nextStage;
    /* 0x20 */ u32 attr;
    /* 0x24 */ u32 ownerPerm;
    /* 0x28 */ u32 groupPerm;
    /* 0x2C */ u32 otherPerm;
    /* 0x30 */ u32 dirFileCount;
    /* 0x34 */ char path[FS_MAX_PATH];
    /* 0x74 */ u32* length;
    /* 0x78 */ u32* position;
    /* 0x7C */ s32 state;
    /* 0x80 */ void* buffer;
    /* 0x84 */ u32 bufferSize;
    /* 0x88 */ u8* type;
    /* 0x8C */ u32 uniqueNo;
    /* 0x90 */ u32 reqBlocks;
    /* 0x94 */ u32 reqInodes;
    /* 0x98 */ u32* answer;
    /* 0x9C */ u32 homeBlocks;
    /* 0xA0 */ u32 homeInodes;
    /* 0xA4 */ u32 userBlocks;
    /* 0xA8 */ u32 userInodes;
    /* 0xAC */ u32 workBlocks;
    /* 0xB0 */ u32 workInodes;
    /* 0xB4 */ const char** dir;
} NANDCommandBlock;

typedef struct NANDBanner {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u32 flags;
    /* 0x8 */ u16 iconSpeed;
    u8 reserved[0x20 - /* 0xA */ 0xA];
    /* 0x20 */ wchar_t title[NAND_BANNER_TITLE_MAX];
    /* 0x60 */ wchar_t subtitle[NAND_BANNER_TITLE_MAX];
    /* 0xA0 */ u8 bannerTexture[0x6000];
    /* 0x60A0 */ u8 iconTexture[0x1200][NAND_BANNER_ICON_MAX_FRAME];
} NANDBanner;

s32 NANDCreate(const char* path, u8 perm, u8 attr);
s32 NANDPrivateCreate(const char* path, u8 perm, u8 attr);
s32 NANDPrivateCreateAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDDelete(const char* path);
s32 NANDPrivateDelete(const char* path);
s32 NANDPrivateDeleteAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDRead(NANDFileInfo* info, void* buf, u32 len);
s32 NANDReadAsync(NANDFileInfo* info, void* buf, u32 len, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDWrite(NANDFileInfo* info, const void* buf, u32 len);
s32 NANDWriteAsync(NANDFileInfo* info, const void* buf, u32 len, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDSeek(NANDFileInfo* info, s32 offset, NANDSeekMode whence);
s32 NANDSeekAsync(NANDFileInfo* info, s32 offset, NANDSeekMode whence, NANDAsyncCallback callback,
                  NANDCommandBlock* block);

s32 NANDPrivateCreateDir(const char* path, u8 perm, u8 attr);
s32 NANDPrivateCreateDirAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDMove(const char* from, const char* to);

s32 NANDGetLength(NANDFileInfo* info, u32* length);
s32 NANDGetLengthAsync(NANDFileInfo* info, u32* lengthOut, NANDAsyncCallback callback, NANDCommandBlock* block);

s32 NANDGetStatus(const char* path, NANDStatus* status);
s32 NANDPrivateGetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback,
                              NANDCommandBlock* block);

void NANDSetUserData(NANDCommandBlock* block, void* data);
void* NANDGetUserData(NANDCommandBlock* block);

#ifdef __cplusplus
}
#endif

#endif
