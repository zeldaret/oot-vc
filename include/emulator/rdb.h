#ifndef _RDB_H
#define _RDB_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Rdb {
    /* 0x000 */ s32 nHackCount;
    /* 0x004 */ char szString[256];
    /* 0x104 */ s32 nIndexString;
    /* 0x108 */ s32 nAddress;
} Rdb; // size = 0x10C

bool rdbEvent(Rdb* pRDB, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassRdb;

#ifdef __cplusplus
}
#endif

#endif
