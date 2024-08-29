#ifndef _RVL_SDK_NWC24_CONFIG_H
#define _RVL_SDK_NWC24_CONFIG_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NWC24_IDCS_INITIAL,
    NWC24_IDCS_GENERATED,
    NWC24_IDCS_REGISTERED
} NWC24IDCreationStage;

typedef struct NWC24Config {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u32 version;
    /* 0x8 */ u64 userId;
    /* 0x10 */ u32 createCount;
    /* 0x14 */ NWC24IDCreationStage createStage;
    /* 0x18 */ char acctDomain[64];
    /* 0x58 */ char password[32];
    /* 0x78 */ char mailchkId[36];
    /* 0x9C */ char acctUrl[128];
    /* 0x11C */ char mailchkUrl[128];
    /* 0x19C */ char mailrecvUrl[128];
    /* 0x21C */ char maildeleteUrl[128];
    /* 0x29C */ char mailsendUrl[128];
    char UNK_0x31C[0x3F8 - 0x31C];
    /* 0x3F8 */ bool allowTitleBoot;
    /* 0x3FC */ u32 checksum;
} NWC24Config;

NWC24Err NWC24GetMyUserId(u64* idOut);
NWC24Err NWC24GenerateNewUserId(u64* idOut);
NWC24Err NWC24iConfigOpen(void);
NWC24Err NWC24iConfigReload(void);
NWC24Err NWC24iConfigFlush(void);
const char* NWC24GetAccountDomain(void);
const char* NWC24GetMBoxDir(void);
u32 NWC24GetAppId(void);
u16 NWC24GetGroupId(void);

#ifdef __cplusplus
}
#endif

#endif
