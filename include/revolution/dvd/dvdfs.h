#ifndef _RVL_SDK_DVD_FS_H
#define _RVL_SDK_DVD_FS_H

#include "revolution/dvd/dvd.h"
#include "revolution/os.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern OSThreadQueue __DVDThreadQueue;
extern bool __DVDLongFileNameFlag;

void __DVDFSInit(void);
s32 DVDConvertPathToEntrynum(const char* path);
bool DVDFastOpen(s32 entrynum, DVDFileInfo* info);
bool DVDEntrynumIsDir(s32 entrynum);
bool DVDOpen(const char* path, DVDFileInfo* info);
bool DVDClose(DVDFileInfo* info);
bool DVDGetCurrentDir(char* buffer, u32 maxlen);
bool DVDReadAsyncPrio(DVDFileInfo* info, void* dst, s32 size, s32 offset, DVDAsyncCallback callback, s32 prio);
s32 DVDReadPrio(DVDFileInfo* info, void* dst, s32 size, s32 offset, s32 prio);

#ifdef __cplusplus
}
#endif

#endif
