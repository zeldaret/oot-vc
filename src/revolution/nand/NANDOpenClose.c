#include "macros.h"
#include "revolution/nand.h"
#include "revolution/os.h"
#include "stdio.h"

static void nandOpenCallback(s32 result, void* arg);
static void nandSafeCloseCallback(s32 result, void* arg);

static s32 nandOpen(const char* path, u8 mode, NANDCommandBlock* block, bool async, bool priv) {
    IPCOpenMode ipcMode;
    char absPath[64];

    MEMCLR(&absPath);
    ipcMode = IPC_OPEN_NONE;
    nandGenerateAbsPath(absPath, path);

    if (!priv && nandIsPrivatePath(absPath)) {
        return IPC_RESULT_ACCESS;
    }

    switch (mode) {
        case NAND_ACCESS_RW:
            ipcMode = IPC_OPEN_RW;
            break;
        case NAND_ACCESS_READ:
            ipcMode = IPC_OPEN_READ;
            break;
        case NAND_ACCESS_WRITE:
            ipcMode = IPC_OPEN_WRITE;
            break;
    }

    if (async) {
        return ISFS_OpenAsync(absPath, ipcMode, nandOpenCallback, block);
    } else {
        return ISFS_Open(absPath, ipcMode);
    }
}

s32 NANDOpen(const char* path, NANDFileInfo* info, u8 mode) {
    s32 result;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    result = nandOpen(path, mode, NULL, false, false);
    if (result >= 0) {
        info->fd = result;
        return NAND_RESULT_OK;
    }

    return nandConvertErrorCode(result);
}

s32 NANDPrivateOpen(const char* path, NANDFileInfo* info, u8 mode) {
    s32 result;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    result = nandOpen(path, mode, NULL, false, true);
    if (result >= 0) {
        info->fd = result;
        return NAND_RESULT_OK;
    }

    return nandConvertErrorCode(result);
}

s32 NANDOpenAsync(const char* path, NANDFileInfo* info, u8 mode, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->info = info;
    return nandConvertErrorCode(nandOpen(path, mode, block, true, false));
}

s32 NANDPrivateOpenAsync(const char* path, NANDFileInfo* info, u8 mode, NANDAsyncCallback callback,
                         NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    block->info = info;
    return nandConvertErrorCode(nandOpen(path, mode, block, true, true));
}

static void nandOpenCallback(s32 result, void* arg) {
    NANDCommandBlock* block = (NANDCommandBlock*)arg;

    if (result >= 0) {
        block->info->fd = result;
        block->callback(NAND_RESULT_OK, block);
    } else {
        block->callback(nandConvertErrorCode(result), block);
    }
}

s32 NANDClose(NANDFileInfo* info) {
    s32 result;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    result = ISFS_Close(info->fd);

    return nandConvertErrorCode(result);
}

s32 NANDCloseAsync(NANDFileInfo* info, NANDAsyncCallback callback, NANDCommandBlock* block) {
    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    block->callback = callback;
    return nandConvertErrorCode(ISFS_CloseAsync(info->fd, nandCallback, block));
}

s32 lbl_8025DA18;
s32 fn_800B3958(const char* path, NANDFileInfo* info, u8 access, void* buffer, s32 len, bool private);

s32 NANDSafeOpen(const char* path, NANDFileInfo* info, u8 access, void* buffer, s32 len) {
    return fn_800B3958(path, info, access, buffer, len, false);
}

s32 fn_800B3958(const char* path, NANDFileInfo* info, u8 access, void* buffer, s32 len, bool private) {
    s32 result;
    u32 sp1C;
    u16 sp8;
    u32 attr;
    u32 ownerPerm;
    u32 groupPerm;
    u32 otherPerm;
    char name[13];
    char dirpath[FS_MAX_PATH];
    bool interrupts;
    u8 temp;
    s32 temp2;
    s32 temp_r30;

    s32 result3;
    s32 result2;
    s32 error;
    s32 fd;
    s32 fd2;

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    result = ISFS_CreateDir("/tmp/sys", 0, 3, 3, 3);
    if (result != 0 && result != -105) {
        return nandConvertErrorCode(result);
    }

    nandGenerateAbsPath(info->openPath, path);
    if (!private && nandIsPrivatePath(info->openPath)) {
        return -1;
    }

    if (access == 1) {
        result = ISFS_Open(info->openPath, 1);
        if (result >= 0) {
            info->fd = result;
            info->access = access;
            return 0;
        }
        return nandConvertErrorCode(result);
    }
    temp = access + 0xFE;
    if ((temp) <= 1) {
        MEMCLR(&name);

        result = ISFS_GetAttr(info->openPath, &sp1C, &sp8, &attr, &ownerPerm, &groupPerm, &otherPerm);
        if (result != 0) {
            return nandConvertErrorCode(result);
        }

        info->tempFd = ISFS_Open(info->openPath, 1);
        if (info->tempFd < 0) {
            return nandConvertErrorCode(info->tempFd);
        }

        interrupts = OSDisableInterrupts();
        temp_r30 = lbl_8025DA18;
        lbl_8025DA18 = temp_r30 + 1;
        OSRestoreInterrupts(interrupts);

        sprintf(dirpath, "%s/%08x", "/tmp/sys", temp_r30);
        result = ISFS_CreateDir(dirpath, 0, 3, 0, 0);
        if (result != 0) {
            return nandConvertErrorCode(result);
        }

        nandGetRelativeName(name, info->openPath);
        sprintf(info->tempPath, "%s/%08x/%s", "/tmp/sys", temp_r30, name);
        result = ISFS_CreateFile(info->tempPath, attr, ownerPerm, groupPerm, otherPerm);
        if (result != 0) {
            return nandConvertErrorCode(result);
        }

        if (access == 2) {
            info->fd = ISFS_Open(info->tempPath, 2);
        } else if (access == 3) {
            info->fd = ISFS_Open(info->tempPath, 3);
        }
        if (info->fd < 0) {
            return nandConvertErrorCode(info->fd);
        }

        // most remaining issues are in this inline
        fd = info->fd;
        fd2 = info->tempFd;
        while (true) {
            result3 = ISFS_Read(fd2, buffer, len);
            error = result3;
            if (result3 == 0) {
                temp2 = 0;
                break;
            } else if (result3 < 0) {
                temp2 = error;
                break;
            } else {
                result2 = ISFS_Write(fd, buffer, result3);
                if (result2 < 0) {
                    temp2 = result2;
                    break;
                }
            }
        }
        if (temp2 != 0) {
            return nandConvertErrorCode(temp2);
        }

        result = ISFS_Seek(info->fd, 0, 0);
        if (result != 0) {
            return nandConvertErrorCode(result);
        }

        info->access = access;
        return nandConvertErrorCode(0);
    }

    return -8;
}

s32 NANDSafeClose(NANDFileInfo* info) {
    s32 result;
    u8 temp;
    char path[FS_MAX_PATH];

    MEMCLR(&path);

    if (!nandIsInitialized()) {
        return NAND_RESULT_FATAL_ERROR;
    }

    if (info->access == 1) {
        return nandConvertErrorCode(ISFS_Close(info->fd));
    }

    temp = info->access + 0xFE;
    if (temp <= 1) {
        result = ISFS_Close(info->fd);
        if (result != 0) {
            return nandConvertErrorCode(result);
        }

        result = ISFS_Close(info->tempFd);
        if (result != 0) {
            return nandConvertErrorCode(result);
        }

        result = ISFS_Rename(info->tempPath, info->openPath);
        if (result != 0) {
            return nandConvertErrorCode(result);
        }

        nandGetParentDirectory(path, info->tempPath);
        return nandConvertErrorCode(ISFS_Delete(path));
    }

    OSReport("Illegal NANDFileInfo.\n");
    return NAND_RESULT_FATAL_ERROR;
}
