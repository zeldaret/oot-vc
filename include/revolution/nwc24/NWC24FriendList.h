#ifndef _RVL_SDK_NWC24_FRIEND_LIST_H
#define _RVL_SDK_NWC24_FRIEND_LIST_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24_FRIEND_LIST_CAPACITY 100

typedef struct NWC24FLHeader {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u32 version;
    /* 0x8 */ u32 capacity;
    /* 0xC */ u32 size;
    char UNK_0x10[0x40 - 0x10];
    /* 0x40 */ u64 friendCodes[NWC24_FRIEND_LIST_CAPACITY];
} NWC24FLHeader;

NWC24Err NWC24iOpenFriendList(void);

#ifdef __cplusplus
}
#endif

#endif
