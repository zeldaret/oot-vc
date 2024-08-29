#ifndef _RVL_SDK_NWC24_SECRET_FRIEND_LIST_H
#define _RVL_SDK_NWC24_SECRET_FRIEND_LIST_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NWC24SecretFLHeader {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u32 version;
    u8 UNK_0x8[0x800 - 0x8];
} NWC24SecretFLHeader;

NWC24Err NWC24iOpenSecretFriendList(void);

#ifdef __cplusplus
}
#endif

#endif
