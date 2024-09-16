#ifndef _RVL_SDK_DVD_H
#define _RVL_SDK_DVD_H

#include "revolution/os/OSAlarm.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// OS sets MSB to signal that the device code was successfully read
#define DVD_DEVICE_CODE_READ (1 << 15)
#define MAKE_DVD_DEVICE_CODE(x) (DVD_DEVICE_CODE_READ | (x))

// Forward declarations
typedef struct DVDCommandBlock;
typedef struct DVDFileInfo;

typedef enum {
    DVD_RESULT_COVER_CLOSED = -4,
    DVD_RESULT_CANCELED,
    DVD_RESULT_M2,
    DVD_RESULT_FATAL,
    DVD_RESULT_OK,
} DVDResult;

typedef enum {
    DVD_STATE_FATAL = -1,
    DVD_STATE_IDLE,
    DVD_STATE_BUSY,
    DVD_STATE_WAITING,
    DVD_STATE_COVER_CLOSED,
    DVD_STATE_NO_DISK,
    DVD_STATE_COVER_OPENED,
    DVD_STATE_WRONG_DISK_ID,
    DVD_STATE_7,
    DVD_STATE_PAUSED,
    DVD_STATE_9,
    DVD_STATE_CANCELED,
    DVD_STATE_DISK_ERROR,
    DVD_STATE_MOTOR_STOPPED,
} DVDAsyncState;

typedef enum {
    DVD_COVER_BUSY,
    DVD_COVER_OPENED,
    DVD_COVER_CLOSED,
} DVDCoverState;

typedef void (*DVDAsyncCallback)(s32 result, struct DVDFileInfo* info);
typedef void (*DVDCommandCallback)(s32 result, struct DVDCommandBlock* block);

typedef void (*DVDCallback)(s32 result, struct DVDFileInfo* fileInfo);
typedef void (*DVDCBCallback)(s32 result, struct DVDCommandBlock* block);
typedef void (*DVDLowCallback)(u32 intType);
typedef void (*DVDDoneReadCallback)(s32, struct DVDFileInfo*);
typedef void (*DVDOptionalCommandChecker)(struct DVDCommandBlock* block, DVDLowCallback callback);

typedef struct DVDDiskID {
    /* 0x0 */ char game[4];
    /* 0x4 */ char company[2];
    /* 0x6 */ u8 disk;
    /* 0x7 */ u8 version;
    /* 0x8 */ u8 strmEnable;
    /* 0x9 */ u8 strmBufSize;
    /* 0xA */ u8 padding[14];
    /* 0x18 */ u32 rvlMagic;
    /* 0x1C */ u32 gcMagic;
} DVDDiskID;

typedef struct DVDCommandBlock {
    /* 0x0 */ struct DVDCommandBlock* next;
    /* 0x4 */ struct DVDCommandBlock* prev;
    /* 0x8 */ u32 command;
    /* 0xC */ volatile s32 state;
    /* 0x10 */ u32 offset;
    /* 0x14 */ u32 length;
    /* 0x18 */ void* addr;
    /* 0x1C */ u32 transferSize;
    /* 0x20 */ u32 transferTotal;
    /* 0x24 */ DVDDiskID* id;
    /* 0x28 */ DVDCommandCallback callback;
    /* 0x2C */ void* userData;
} DVDCommandBlock;

typedef struct DVDDriveInfo {
    /* 0x0 */ u16 revision;
    /* 0x2 */ u16 deviceCode;
    /* 0x4 */ u32 releaseDate;
    char padding[32 - 0x8];
} DVDDriveInfo;

typedef struct DVDFileInfo {
    /* 0x00 */ DVDCommandBlock block;
    /* 0x30 */ u32 offset;
    /* 0x34 */ u32 size;
    /* 0x38 */ DVDAsyncCallback callback;
} DVDFileInfo;

typedef struct DVDDir {
    /* 0x00 */ u32 entryNum;
    /* 0x04 */ u32 location;
    /* 0x08 */ u32 next;
} DVDDir;

typedef struct DVDDirEntry {
    /* 0x00 */ u32 entryNum;
    /* 0x04 */ bool isDir;
    /* 0x08 */ char* name;
} DVDDirEntry;

extern volatile u32 __DVDLayoutFormat;

void DVDInit(void);
bool DVDReadAbsAsyncPrio(DVDCommandBlock* block, void* dst, u32 size, u32 offset, DVDCommandCallback callback,
                         s32 prio);
bool DVDInquiryAsync(DVDCommandBlock* block, DVDDriveInfo* info, DVDCommandCallback callback);
s32 DVDGetCommandBlockStatus(const DVDCommandBlock* block);
s32 DVDGetDriveStatus(void);
void DVDPause(void);
void DVDResume(void);
bool DVDCancelAsync(DVDCommandBlock* block, DVDCommandCallback callback);
s32 DVDCancel(DVDCommandBlock* block);
bool DVDCancelAllAsync(DVDCommandCallback callback);
const DVDDiskID* DVDGetCurrentDiskID(void);
u32 __DVDGetCoverStatus(void);
void __DVDPrepareResetAsync(DVDCommandCallback callback);
void __DVDPrepareReset(void);
bool __DVDTestAlarm(const struct OSAlarm* alarm);
bool __DVDLowBreak(void);
bool __DVDStopMotorAsync(DVDCommandBlock* block, DVDCommandCallback callback);
void __DVDRestartMotor(void);

#ifdef __cplusplus
}
#endif

#endif
