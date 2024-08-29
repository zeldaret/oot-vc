#ifndef _RVL_SDK_NWC24_MANAGE_H
#define _RVL_SDK_NWC24_MANAGE_H

#include "macros.h"
#include "revolution/nwc24/NWC24Config.h"
#include "revolution/nwc24/NWC24Download.h"
#include "revolution/nwc24/NWC24FriendList.h"
#include "revolution/nwc24/NWC24SecretFList.h"
#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NWC24_IO_BUFFER_SIZE 512

#define ROUND_UP(x, align) (((x) + (align) - 1) & (-(align)))
#define WORK_SIZE(x) (ROUND_UP(sizeof(x), 0x100))

typedef struct NWC24Work {
    /* 0x0 */ char stringWork[1024];
    char WORK_0x400[0x800 - 0x400];
    /* 0x800 */ char pathWork[128];
    char WORK_0x880[0x900 - 0x880];
    /* 0x900 */ u8 readBuffer[NWC24_IO_BUFFER_SIZE];
    /* 0xB00 */ u8 writeBuffer[NWC24_IO_BUFFER_SIZE];
    u8 config[WORK_SIZE(NWC24Config) /* 0xD00 */];
    char WORK_0x1100[128];
    char WORK_0x1180[128];
    char WORK_0x1200[128];
    char WORK_0x1280[128];
    /* 0x1300 */ u8 base64Work[256];
    char WORK_0x1400[0x2400 - 0x1400];
    u8 flHeader[WORK_SIZE(NWC24FLHeader) /* 0x2800 */];
    u8 secretFlHeader[WORK_SIZE(NWC24SecretFLHeader) /* 0x2800 */];
    u8 dlHeader[WORK_SIZE(NWC24DlHeader) /* 0x3000 */];
    u8 dlTask[WORK_SIZE(NWC24DlTask) /* 0x3800 */];
} NWC24Work;
#undef WORK_SIZE

extern NWC24Work* NWC24WorkP;

void NWC24iRegister(void);
NWC24Err NWC24OpenLib(NWC24Work* work);
NWC24Err NWC24CloseLib(void);
bool NWC24IsMsgLibOpened(void);
bool NWC24IsMsgLibOpenedByTool(void);
bool NWC24IsMsgLibOpenBlocking(void);
NWC24Err NWC24BlockOpenMsgLib(bool block);
NWC24Err NWC24iSetNewMsgArrived(u32 flags);
void NWC24iSetErrorCode(u32 code);

#ifdef __cplusplus
}
#endif

#endif
