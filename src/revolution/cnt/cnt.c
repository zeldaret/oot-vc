#include "revolution/cnt.h"
#include "revolution/esp.h"
#include "revolution/ipc.h"
#include "revolution/nand.h"

s32 ESP_InitLib(void) {
    s32 ret = 0;

    if (__esFd < 0) {
        __esFd = IOS_Open("/dev/es", 0);

        if (__esFd < 0) {
            ret = __esFd;
        }
    }

    return ret;
}

s32 ESP_OpenContentFile(s32 fd) {
    u8 WORK[256] ATTRIBUTE_ALIGN(32);

    IPCIOVector* vecWork = (IPCIOVector*)(WORK + 0xD8);
    s32* fdWork = (s32*)(WORK + 0x00);

    if (__esFd < 0) {
        return -1017;
    }

    *fdWork = fd;
    vecWork->base = fdWork;
    vecWork->length = 4;
    return IOS_Ioctlv(__esFd, ES_IOCTLV_OPEN_CONTENT_FILE, 1, 0, vecWork);
}

s32 ESP_CloseContentFile(s32 fd) {
    u8 WORK[256] ATTRIBUTE_ALIGN(32);

    IPCIOVector* vecWork = (IPCIOVector*)(WORK + 0xD8);
    s32* fdWork = (s32*)(WORK + 0x00);

    if (__esFd < 0 || fd < 0) {
        return -1017;
    }

    *fdWork = fd;
    vecWork->base = fdWork;
    vecWork->length = 4;
    return IOS_Ioctlv(__esFd, ES_IOCTLV_CLOSE_CONTENT_FILE, 1, 0, vecWork);
}

static inline CNTResult contentConvertErrorCode(s32 error) {
    int i;

    // clang-format off
    const s32 errorMap[] = {
        0,     0,
        -1001, -5063,
        -1002, -5063,
        -1003, -5063,
        -1004, -5002,
        -1005, -5063,
        -1006, -5063,
        -1007, -5063,
        -1008, -5002,
        -1009, -5003,
        -1010, -5063,
        -1011, -5063,
        -1012, -5063,
        -1013, -5063,
        -1014, -5063,
        -1015, -5063,
        -1016, -5000,
        -1017, -5009,
        -1018, -5063,
        -1019, -5063,
        -1020, -5063,
        -1021, -5063,
        -1022, -5063,
        -1023, -5063,
        -1024, -5008,
        -1025, -5063,
        -1026, -5010,
        -1027, -5063,
        -1028, -5063,
        -1029, -5063,
        -1030, -5063,
        -1031, -5063,
        -1032, -5063,
        -1033, -5063,
        -1034, -5063,
        -1035, -5063,
        -1036, -5063,
        -1037, -5063,
        -1038, -5063,
        IPC_RESULT_OK,                  NAND_RESULT_OK,
        IPC_RESULT_ACCESS,              NAND_RESULT_ACCESS,
        IPC_RESULT_CORRUPT,             NAND_RESULT_CORRUPT,
        IPC_RESULT_ECC_CRIT,            NAND_RESULT_ECC_CRIT,
        IPC_RESULT_EXISTS,              NAND_RESULT_EXISTS,
        -116,                           NAND_RESULT_AUTHENTICATION,
        IPC_RESULT_INVALID,             NAND_RESULT_INVALID,
        IPC_RESULT_MAXBLOCKS,           NAND_RESULT_MAXBLOCKS,
        IPC_RESULT_MAXFD,               NAND_RESULT_MAXFD,
        IPC_RESULT_MAXFILES,            NAND_RESULT_MAXFILES,
        IPC_RESULT_NOEXISTS,            NAND_RESULT_NOEXISTS,
        IPC_RESULT_NOTEMPTY,            NAND_RESULT_NOTEMPTY,
        -104,                           NAND_RESULT_UNKNOWN,
        IPC_RESULT_OPENFD,              NAND_RESULT_OPENFD,
        -117,                           NAND_RESULT_UNKNOWN,
        IPC_RESULT_BUSY,                NAND_RESULT_BUSY,
        IPC_RESULT_ACCESS_INTERNAL,     NAND_RESULT_ACCESS,
        IPC_RESULT_EXISTS_INTERNAL,     NAND_RESULT_EXISTS,
        -3,                             NAND_RESULT_UNKNOWN,
        IPC_RESULT_INVALID_INTERNAL,    NAND_RESULT_INVALID,
        -5,                             NAND_RESULT_UNKNOWN,
        IPC_RESULT_NOEXISTS_INTERNAL,   NAND_RESULT_NOEXISTS,
        -7,                             NAND_RESULT_UNKNOWN,
        IPC_RESULT_BUSY_INTERNAL,       NAND_RESULT_BUSY,
        -9,                             NAND_RESULT_UNKNOWN,
        -10,                            NAND_RESULT_UNKNOWN,
        -11,                            NAND_RESULT_UNKNOWN,
        IPC_RESULT_ECC_CRIT_INTERNAL,   NAND_RESULT_ECC_CRIT,
        -13,                            NAND_RESULT_UNKNOWN,
        -14,                            NAND_RESULT_UNKNOWN,
        -15,                            NAND_RESULT_UNKNOWN,
        -16,                            NAND_RESULT_UNKNOWN,
        -17,                            NAND_RESULT_UNKNOWN,
        -18,                            NAND_RESULT_UNKNOWN,
        -19,                            NAND_RESULT_UNKNOWN,
        -20,                            NAND_RESULT_UNKNOWN,
        -21,                            NAND_RESULT_UNKNOWN,
        IPC_RESULT_ALLOC_FAILED,        NAND_RESULT_ALLOC_FAILED,
        -23,                            NAND_RESULT_UNKNOWN,
    };
    // clang-format on

    i = 0;

    if (error >= 0) {
        return error;
    }

    for (; i < ARRAY_COUNT(errorMap); i += 2) {
        if (error == errorMap[i]) {
            return errorMap[i + 1];
        }
    }

    OSReport("CAUTION!  Unexpected error code [%d] was found.\n", error);
    return -5063;
}

void contentInit() {
    ESP_InitLib();
}

s32 contentInitHandleNAND(s32 contentNum, CNTHandleNAND* handle, MEMAllocator* memAlloc) {
    int error;
    int fd;
    ARCHandle arcHandle;
    void* buffer;
    int len;

    fd = ESP_OpenContentFile(contentNum);
    if (fd < 0) {
        return -5002;
    }

    buffer = MEMAllocFromAllocator(memAlloc, sizeof(ARCHeader));
    if (buffer == NULL) {
        return -5001;
    }

    if (ESP_ReadContentFile(fd, buffer, sizeof(ARCHeader)) < 0) {
        MEMFreeToAllocator(memAlloc, buffer);
        return -5003;
    }

    len = OSRoundUp32B(((ARCHeader*)buffer)->files.offset);
    MEMFreeToAllocator(memAlloc, buffer);
    if (ESP_SeekContentFile(fd, 0, 0) < 0) {
        return -5004;
    }

    buffer = MEMAllocFromAllocator(memAlloc, OSRoundUp32B(len));
    if (buffer == NULL) {
        return -5001;
    }

    if (ESP_ReadContentFile(fd, buffer, len) < 0) {
        MEMFreeToAllocator(memAlloc, buffer);
        return -5003;
    }

    ARCInitHandle(buffer, &arcHandle);

    handle->arcHandle = arcHandle;
    handle->fd = fd;
    handle->memAlloc = memAlloc;

    return 0;
}

CNTResult contentOpenNAND(CNTHandleNAND* handle, const char* path, CNTFileInfoNAND* info) {
    ARCFileInfo arcInfo;

    if (!ARCOpen(&handle->arcHandle, path, &arcInfo)) {
        return -5002;
    }

    info->handle = handle;
    info->offset = arcInfo.offset;
    info->length = arcInfo.size;
    info->position = 0;

    return 0;
}

CNTResult contentFastOpenNAND(CNTHandleNAND* handle, s32 entrynum, CNTFileInfoNAND* info) {
    ARCFileInfo arcInfo;

    if (!ARCFastOpen(&handle->arcHandle, entrynum, &arcInfo)) {
        return -5002;
    }

    info->handle = handle;
    info->offset = arcInfo.offset;
    info->length = arcInfo.size;
    info->position = 0;

    return 0;
}

s32 contentConvertPathToEntrynumNAND(CNTHandleNAND* info, const char* path) {
    return ARCConvertPathToEntrynum(&info->arcHandle, path);
}

u32 contentGetLengthNAND(CNTFileInfoNAND* info) { return info->length; }

CNTResult contentSeekNAND(CNTFileInfoNAND* info, u32 offset, s32 whence) {
    u32 position;

    switch (whence) {
        case 0:
            position = offset;
            break;
        case 1:
            position = info->position + offset;
            break;
        case 2:
            position = info->length + offset;
            break;
        default:
            return -5127;
    }

    if (position > info->length) {
        return -5009;
    }

    info->position = position;

    return 0;
}

CNTResult contentReadNAND(CNTFileInfoNAND* info, void* dst, u32 len, s32 offset) {
    if (info->position + offset > info->length) {
        return -5009;
    }

    if (ESP_SeekContentFile(info->handle->fd, info->offset + info->position + offset, IPC_SEEK_BEG) < 0) {
        return -5004;
    }

    return contentConvertErrorCode(ESP_ReadContentFile(info->handle->fd, dst, len));
}

CNTResult contentCloseNAND(CNTFileInfoNAND* info) { return 0; }

CNTResult contentReleaseHandleNAND(CNTHandleNAND* handle) {
    int error;

    MEMFreeToAllocator(handle->memAlloc, (void*)handle->arcHandle.header);
    error = ESP_CloseContentFile(handle->fd);
    return contentConvertErrorCode(error);
}

bool contentOpenDirNAND(CNTHandleNAND* handle, const char* path, ARCDir* dir) {
    return ARCOpenDir(&handle->arcHandle, path, dir);
}
