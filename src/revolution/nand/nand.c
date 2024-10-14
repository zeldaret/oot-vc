#include "revolution/nand.h"
#include "revolution/fs.h"
#include "stdio.h"
#include "string.h"

static void nandComposePerm(u8* out, u32 ownerPerm, u32 groupPerm, u32 otherPerm) NO_INLINE;
static void nandSplitPerm(u8 perm, u32* ownerPerm, u32* groupPerm, u32* otherPerm) NO_INLINE;
static void nandGetStatusCallback(s32 result, void* arg);
static void nandGetFileStatusAsyncCallback(s32 result, void* arg);
static bool nandInspectPermission(u8 perm);

static s32 nandCreate(const char* path, u8 perm, u8 attr, NANDCommandBlock* block, bool async, bool priv) NO_INLINE {
    char absPath[64];
    u32 ownerPerm, groupPerm, otherPerm;

    MEMCLR(&absPath);

    ownerPerm = 0;
    groupPerm = 0;
    otherPerm = 0;

    nandGenerateAbsPath(absPath, path);

    if (!priv && nandIsPrivatePath(absPath)) {
        return IPC_RESULT_ACCESS;
    }

    nandSplitPerm(perm, &ownerPerm, &groupPerm, &otherPerm);

    if (async) {
        return ISFS_CreateFileAsync(absPath, attr, ownerPerm, groupPerm, otherPerm, nandCallback, block);
    } else {
        return ISFS_CreateFile(absPath, attr, ownerPerm, groupPerm, otherPerm);
    }
}

s32 NANDCreate(const char* path, u8 perm, u8 attr) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCreate(path, perm, attr, NULL, false, false));
}

s32 NANDPrivateCreate(const char* path, u8 perm, u8 attr) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCreate(path, perm, attr, NULL, false, true));
}

s32 NANDPrivateCreateAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandCreate(path, perm, attr, block, true, true));
}

static s32 nandDelete(const char* path, NANDCommandBlock* block, bool async, bool priv) {
    char absPath[64];

    MEMCLR(&absPath);
    nandGenerateAbsPath(absPath, path);

    if (!priv && nandIsPrivatePath(absPath)) {
        return IPC_RESULT_ACCESS;
    }

    if (async) {
        return ISFS_DeleteAsync(absPath, nandCallback, block);
    } else {
        return ISFS_Delete(absPath);
    }
}

s32 NANDDelete(const char* path) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandDelete(path, NULL, false, false));
}

// s32 NANDPrivateDelete(const char* path) {
//     if (!nandIsInitialized()) {
//         return NAND_RESULT_FATAL_ERROR;
//     }

//     return nandConvertErrorCode(nandDelete(path, NULL, false, true));
// }

s32 NANDPrivateDeleteAsync(const char* path, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandDelete(path, block, true, true));
}

s32 NANDRead(NANDFileInfo* info, void* buf, u32 len) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_Read(info->fd, buf, len));
}

s32 NANDReadAsync(NANDFileInfo* info, void* buf, u32 len, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(ISFS_ReadAsync(info->fd, buf, len, nandCallback, block));
}

s32 NANDWrite(NANDFileInfo* info, const void* buf, u32 len) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(ISFS_Write(info->fd, buf, len));
}

s32 NANDWriteAsync(NANDFileInfo* info, const void* buf, u32 len, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(ISFS_WriteAsync(info->fd, buf, len, nandCallback, block));
}

static s32 nandSeek(s32 fd, s32 offset, NANDSeekMode whence, NANDCommandBlock* block, bool async) {
    IPCSeekMode mode = (IPCSeekMode)-1;

    switch (whence) {
        case NAND_SEEK_BEG:
            mode = IPC_SEEK_BEG;
            break;
        case NAND_SEEK_CUR:
            mode = IPC_SEEK_CUR;
            break;
        case NAND_SEEK_END:
            mode = IPC_SEEK_END;
            break;
    }

    if (async) {
        return ISFS_SeekAsync(fd, offset, mode, nandCallback, block);
    } else {
        return ISFS_Seek(fd, offset, mode);
    }
}

s32 NANDSeek(NANDFileInfo* info, s32 offset, NANDSeekMode whence) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandSeek(info->fd, offset, whence, NULL, false));
}

s32 NANDSeekAsync(NANDFileInfo* info, s32 offset, NANDSeekMode whence, NANDAsyncCallback callback,
                  NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandSeek(info->fd, offset, whence, block, true));
}

static s32 nandReadDir(const char* path, char* filesOut, u32* fileCountOut) {
    char absPath[64];

    MEMCLR(&absPath);
    nandGenerateAbsPath(absPath, path);

    if (nandIsPrivatePath(absPath)) {
        return IPC_RESULT_ACCESS;
    }

    return ISFS_ReadDir(absPath, filesOut, fileCountOut);
}

s32 NANDReadDir(const char* path, char* filesOut, u32* fileCountOut) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandReadDir(path, filesOut, fileCountOut));
}

static s32 nandCreateDir(const char* path, u8 perm, u8 attr, NANDCommandBlock* block, bool async, bool priv) {
    char absPath[64];
    u32 ownerPerm, groupPerm, otherPerm;

    MEMCLR(&absPath);
    nandGenerateAbsPath(absPath, path);

    if (!priv && nandIsPrivatePath(absPath)) {
        return IPC_RESULT_ACCESS;
    }

    ownerPerm = 0;
    groupPerm = 0;
    otherPerm = 0;
    nandSplitPerm(perm, &ownerPerm, &groupPerm, &otherPerm);

    if (async) {
        return ISFS_CreateDirAsync(absPath, attr, ownerPerm, groupPerm, otherPerm, nandCallback, block);
    } else {
        return ISFS_CreateDir(absPath, attr, ownerPerm, groupPerm, otherPerm);
    }
}

s32 NANDCreateDir(const char* path, u8 perm, u8 attr) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandCreateDir(path, perm, attr, NULL, false, false));
}

s32 NANDPrivateCreateDirAsync(const char* path, u8 perm, u8 attr, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(nandCreateDir(path, perm, attr, block, true, true));
}

static s32 nandGetFileStatus(s32 fd, u32* lengthOut, u32* positionOut) {
    FSFileStats stats;
    s32 result;

    result = ISFS_GetFileStats(fd, &stats);
    if (result == IPC_RESULT_OK) {
        if (lengthOut != 0) {
            *lengthOut = stats.length;
        }

        if (positionOut != 0) {
            *positionOut = stats.position;
        }
    }

    return result;
}

s32 NANDGetLength(NANDFileInfo* info, u32* length) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetFileStatus(info->fd, length, NULL));
}

static void nandComposePerm(u8* out, u32 ownerPerm, u32 groupPerm, u32 otherPerm) {
    u32 perm = 0;

    if (ownerPerm & NAND_ACCESS_READ) {
        perm |= NAND_PERM_RUSR;
    }

    if (ownerPerm & NAND_ACCESS_WRITE) {
        perm |= NAND_PERM_WUSR;
    }

    if (groupPerm & NAND_ACCESS_READ) {
        perm |= NAND_PERM_RGRP;
    }

    if (groupPerm & NAND_ACCESS_WRITE) {
        perm |= NAND_PERM_WGRP;
    }

    if (otherPerm & NAND_ACCESS_READ) {
        perm |= NAND_PERM_ROTH;
    }

    if (otherPerm & NAND_ACCESS_WRITE) {
        perm |= NAND_PERM_WOTH;
    }

    *out = perm;
}

static void nandSplitPerm(u8 perm, u32* ownerPerm, u32* groupPerm, u32* otherPerm) {
    *ownerPerm = 0;
    *groupPerm = 0;
    *otherPerm = 0;

    if (perm & NAND_PERM_RUSR) {
        *ownerPerm |= NAND_ACCESS_READ;
    }

    if (perm & NAND_PERM_WUSR) {
        *ownerPerm |= NAND_ACCESS_WRITE;
    }

    if (perm & NAND_PERM_RGRP) {
        *groupPerm |= NAND_ACCESS_READ;
    }

    if (perm & NAND_PERM_WGRP) {
        *groupPerm |= NAND_ACCESS_WRITE;
    }

    if (perm & NAND_PERM_ROTH) {
        *otherPerm |= NAND_ACCESS_READ;
    }

    if (perm & NAND_PERM_WOTH) {
        *otherPerm |= NAND_ACCESS_WRITE;
    }
}

static s32 nandGetStatus(const char* path, NANDStatus* status, NANDCommandBlock* block, bool async, bool priv) {
    s32 result;
    u32 attr;
    u32 ownerPerm, groupPerm, otherPerm;
    char absPath[64];

    MEMCLR(&absPath);
    nandGenerateAbsPath(absPath, path);

    if (!priv && nandIsUnderPrivatePath(absPath)) {
        return IPC_RESULT_ACCESS;
    }

    if (async) {
        return ISFS_GetAttrAsync(absPath, &status->ownerId, &status->groupId, &block->attr, &block->ownerPerm,
                                 &block->groupPerm, &block->otherPerm, nandGetStatusCallback, block);
    } else {
        attr = 0;
        ownerPerm = 0;
        groupPerm = 0;
        otherPerm = 0;

        result = ISFS_GetAttr(absPath, &status->ownerId, &status->groupId, &attr, &ownerPerm, &groupPerm, &otherPerm);

        if (result == IPC_RESULT_OK) {
            nandComposePerm(&status->perm, ownerPerm, groupPerm, otherPerm);
            status->attr = attr;
        }

        return result;
    }
}

static void nandGetStatusCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;
    NANDStatus* status;

    if (result == IPC_RESULT_OK) {
        status = block->status;
        status->attr = block->attr;
        nandComposePerm(&status->perm, block->ownerPerm, block->groupPerm, block->otherPerm);
    }

    block->callback(nandConvertErrorCode(result), block);
}

s32 NANDGetStatus(const char* path, NANDStatus* status) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    return nandConvertErrorCode(nandGetStatus(path, status, NULL, false, false));
}

s32 NANDPrivateGetStatusAsync(const char* path, NANDStatus* status, NANDAsyncCallback callback,
                              NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->status = status;
    return nandConvertErrorCode(nandGetStatus(path, status, block, true, true));
}

void NANDSetUserData(NANDCommandBlock* block, void* data) { block->userData = data; }

void* NANDGetUserData(NANDCommandBlock* block) { return block->userData; }

static bool nandInspectPermission(u8 perm) { return perm & NAND_PERM_RUSR; }
