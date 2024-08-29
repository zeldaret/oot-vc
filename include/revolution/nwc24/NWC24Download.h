#ifndef _RVL_SDK_NWC24_DOWNLOAD_H
#define _RVL_SDK_NWC24_DOWNLOAD_H

#include "macros.h"
#include "revolution/fs.h"
#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24_DL_TASK_MAX 120
#define NWC24_DL_SUBTASK_MAX 32

typedef enum {
    NWC24_DLTYPE_MULTIPART_V1,
    NWC24_DLTYPE_OCTETSTREAM_V1,
    NWC24_DLTYPE_MULTIPART_V2,
    NWC24_DLTYPE_OCTETSTREAM_V2
} NWC24DlType;

typedef enum {
    NWC24_DL_STTYPE_NONE,
    NWC24_DL_STTYPE_INCREMENT,
    NWC24_DL_STTYPE_TIME_HOUR,
    NWC24_DL_STTYPE_TIME_DAYOFWEEK,
    NWC24_DL_STTYPE_TIME_DAY
} NWC24DlSubTaskType;

typedef enum {
    NWC24_DL_STFLAG_TRAILING_FILENAME = (1 << 0),
    NWC24_DL_STFLAG_TRAILING_URL = (1 << 1),
    NWC24_DL_STFLAG_INTELLIGENT_UPDATE = (1 << 8),
    NWC24_DL_STFLAG_RETICENT_UPDATE = (1 << 9)
} NWC24DlSubTaskFlags;

typedef struct NWC24DlEntry {
    /* 0x0 */ u32 app;
    /* 0x4 */ u32 nextTime;
    /* 0x8 */ u32 lastAccess;
    /* 0xC */ u8 flags;
    char UNK_0xD[0x10 - 0xD];
} NWC24DlEntry;

#pragma pack(push, 1)
typedef struct NWC24DlHeader {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u32 version;
    char UNK_0x8[0x10 - 0x8];
    /* 0x10 */ u16 maxSubTasks;
    /* 0x12 */ u16 privateTasks;
    /* 0x14 */ u16 maxTasks;
    char UNK_0x16[0x80 - 0x16];
    /* 0x80 */ NWC24DlEntry entries[NWC24_DL_TASK_MAX];
} NWC24DlHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct NWC24DlTask {
    /* 0x0 */ u16 id;
    /* 0x2 */ u8 type;
    /* 0x3 */ u8 priority;
    /* 0x4 */ u32 flags;
    /* 0x8 */ u32 appId;
    /* 0xC */ u32 titleIdHi;
    /* 0x10 */ u32 titleIdLo;
    /* 0x14 */ u16 groupId;
    char UNK_0x16[0x2];
    /* 0x18 */ s16 count;
    /* 0x1A */ s16 errorCount;
    /* 0x1C */ u16 interval;
    /* 0x1E */ u16 margin;
    /* 0x20 */ u32 lastError;
    /* 0x24 */ u8 subTaskCounter;
    /* 0x25 */ u8 subTaskType;
    /* 0x26 */ u8 subTaskFlags;
    char UNK_0x27[0x1];
    /* 0x28 */ u32 subTaskMask;
    /* 0x2C */ u32 serverInterval;
    /* 0x30 */ u32 lastUpdate;
    /* 0x34 */ u32 lastUpdateSubTask[NWC24_DL_SUBTASK_MAX];
    /* 0xB4 */ char url[236];
    /* 0x1A0 */ char fileName[FS_MAX_PATH];
    char UNK_0x1E0[0x1F8 - 0x1E0];
    /* 0x1F8 */ u32 userParam;
    /* 0x1FC */ u8 optFlags;
    /* 0x1FD */ u8 rootCaId;
    char UNK_0x1FE[0x200 - 0x1FE];
} NWC24DlTask;
#pragma pack(pop)

NWC24Err NWC24CheckDlTask(NWC24DlTask* task);
NWC24Err NWC24DeleteDlTaskForced(NWC24DlTask* task);
NWC24Err NWC24GetDlTask(NWC24DlTask* task, u16 i);
NWC24Err NWC24iOpenDlTaskList(void);
NWC24Err NWC24iCloseDlTaskList(void);
NWC24DlHeader* NWC24iGetCachedDlHeader(void);
NWC24Err NWC24iCheckHeaderConsistency(NWC24DlHeader* header, bool clear) DECOMP_DONT_INLINE;
NWC24Err NWC24iLoadDlHeader(void);

#ifdef __cplusplus
}
#endif

#endif
