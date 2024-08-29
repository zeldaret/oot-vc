#ifndef _RVL_SDK_IPC_CLT_H
#define _RVL_SDK_IPC_CLT_H

#include "revolution/os.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IPC_RESULT_FATAL_ERROR = -119,
    IPC_RESULT_BUSY,
    IPC_RESULT_NOTEMPTY = -115,
    IPC_RESULT_ECC_CRIT,
    IPC_RESULT_OPENFD = -111,
    IPC_RESULT_MAXFD = -109,
    IPC_RESULT_MAXBLOCKS,
    IPC_RESULT_MAXFILES,
    IPC_RESULT_NOEXISTS,
    IPC_RESULT_EXISTS,
    IPC_RESULT_CORRUPT = -103,
    IPC_RESULT_ACCESS,
    IPC_RESULT_INVALID,

    IPC_RESULT_ALLOC_FAILED = -22,
    IPC_RESULT_ECC_CRIT_INTERNAL = -12,
    IPC_RESULT_BUSY_INTERNAL = -8,
    IPC_RESULT_NOEXISTS_INTERNAL = -6,
    IPC_RESULT_CONN_MAX_INTERNAL = -5,
    IPC_RESULT_INVALID_INTERNAL = -4,
    IPC_RESULT_EXISTS_INTERNAL = -2,
    IPC_RESULT_ACCESS_INTERNAL = -1,

    IPC_RESULT_OK = 0
} IPCResult;

typedef enum {
    IPC_REQ_NONE,
    IPC_REQ_OPEN,
    IPC_REQ_CLOSE,
    IPC_REQ_READ,
    IPC_REQ_WRITE,
    IPC_REQ_SEEK,
    IPC_REQ_IOCTL,
    IPC_REQ_IOCTLV
} IPCRequestType;

typedef enum {
    IPC_OPEN_NONE = 0,
    IPC_OPEN_READ = (1 << 0),
    IPC_OPEN_WRITE = (1 << 1),
    IPC_OPEN_RW = IPC_OPEN_READ | IPC_OPEN_WRITE
} IPCOpenMode;

typedef enum {
    IPC_SEEK_BEG,
    IPC_SEEK_CUR,
    IPC_SEEK_END,
} IPCSeekMode;

typedef s32 (*IPCAsyncCallback)(s32 result, void* arg);

typedef struct IPCIOVector {
    /* 0x0 */ void* base;
    /* 0x4 */ u32 length;
} IPCIOVector;

typedef struct IPCOpenArgs {
    /* 0x0 */ const char* path;
    /* 0x4 */ IPCOpenMode mode;
} IPCOpenArgs;

typedef struct IPCReadWriteArgs {
    /* 0x0 */ void* data;
    /* 0x4 */ u32 length;
} IPCReadWriteArgs;

typedef struct IPCSeekArgs {
    /* 0x0 */ s32 offset;
    /* 0x4 */ IPCSeekMode mode;
} IPCSeekArgs;

typedef struct IPCIoctlArgs {
    /* 0x0 */ s32 type;
    /* 0x4 */ void* in;
    /* 0x8 */ s32 inSize;
    /* 0xC */ void* out;
    /* 0x10 */ s32 outSize;
} IPCIoctlArgs;

typedef struct IPCIoctlvArgs {
    /* 0x0 */ s32 type;
    /* 0x4 */ u32 inCount;
    /* 0x8 */ u32 outCount;
    /* 0xC */ IPCIOVector* vectors;
} IPCIoctlvArgs;

typedef struct IPCRequest {
    /* 0x0 */ IPCRequestType type;
    /* 0x4 */ s32 ret;
    /* 0x8 */ s32 fd;
    /* 0xC */ union {
        IPCOpenArgs open;
        IPCReadWriteArgs rw;
        IPCSeekArgs seek;
        IPCIoctlArgs ioctl;
        IPCIoctlvArgs ioctlv;
    };
} IPCRequest;

typedef struct IPCRequestEx {
    /* 0x0 */ IPCRequest base;
    /* 0x20 */ IPCAsyncCallback callback;
    /* 0x24 */ void* callbackArg;
    /* 0x28 */ bool reboot;
    /* 0x2C */ OSThreadQueue queue;
    char padding[64 - 0x34];
} IPCRequestEx;

s32 IPCCltInit(void);
s32 IOS_OpenAsync(const char* path, IPCOpenMode mode, IPCAsyncCallback callback, void* callbackArg);
s32 IOS_Open(const char* path, IPCOpenMode mode);
s32 IOS_CloseAsync(s32 fd, IPCAsyncCallback callback, void* callbackArg);
s32 IOS_Close(s32 fd);
s32 IOS_ReadAsync(s32 fd, void* buf, s32 len, IPCAsyncCallback callback, void* callbackArg);
s32 IOS_Read(s32 fd, void* buf, s32 len);
s32 IOS_WriteAsync(s32 fd, const void* buf, s32 len, IPCAsyncCallback callback, void* callbackArg);
s32 IOS_Write(s32 fd, const void* buf, s32 len);
s32 IOS_SeekAsync(s32 fd, s32 offset, IPCSeekMode mode, IPCAsyncCallback callback, void* callbackArg);
s32 IOS_Seek(s32 fd, s32 offset, IPCSeekMode mode);
s32 IOS_IoctlAsync(s32 fd, s32 type, void* in, s32 inSize, void* out, s32 outSize, IPCAsyncCallback callback,
                   void* callbackArg);
s32 IOS_Ioctl(s32 fd, s32 type, void* in, s32 inSize, void* out, s32 outSize);
s32 IOS_IoctlvAsync(s32 fd, s32 type, s32 inCount, s32 outCount, IPCIOVector* vectors, IPCAsyncCallback callback,
                    void* callbackArg);
s32 IOS_Ioctlv(s32 fd, s32 type, s32 inCount, s32 outCount, IPCIOVector* vectors);
s32 IOS_IoctlvReboot(s32 fd, s32 type, s32 inCount, s32 outCount, IPCIOVector* vectors);

#ifdef __cplusplus
}
#endif

#endif
