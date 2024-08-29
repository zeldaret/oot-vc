#ifndef _RVL_SDK_SI_SIBIOS_H
#define _RVL_SDK_SI_SIBIOS_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SI_CHAN_0,
    SI_CHAN_1,
    SI_CHAN_2,
    SI_CHAN_3,

    SI_MAX_CHAN,
    SI_CHAN_NONE = -1,
} SIChannel;

typedef enum {
    SI_TYPE_0,
    SI_TYPE_1,
    SI_TYPE_2,
    SI_TYPE_3,

    SI_MAX_TYPE
} SIType;

typedef enum {
    SI_ERROR_NOREP = (1 << 3),
    SI_ERROR_BUSY = (1 << 7),
    SI_WIRELESS_LITE = (1 << 18),
    SI_WIRELESS_CONT = (1 << 19),
} SIStatus;

typedef void (*SICallback)(s32 chan, u32 status, OSContext* context);
typedef void (*SITypeAndStatusCallback)(s32 chan, u32 type);

typedef struct SIMain {
    s32 chan; // at 0x0
    u32 poll; // at 0x4
    u32 inSize; // at 0x8
    void* inAddr; // at 0xC
    SICallback callback; // at 0x10
} SIMain;

typedef struct SIPacket {
    s32 chan; // at 0x0
    void* outAddr; // at 0x4
    u32 outSize; // at 0x8
    void* inAddr; // at 0xC
    u32 inSize; // at 0x10
    SICallback callback; // at 0x14
    s64 fire; // at 0x18
} SIPacket;

void SIInit(void);
u32 SIGetStatus(s32 chan);
u32 SISetXY(u32 lines, u32 times);
bool SITransfer(s32 chan, void* outAddr, u32 outSize, void* inAddr, u32 inSize, SICallback callback, s64 wait);
u32 SIGetType(s32 chan);

#ifdef __cplusplus
}
#endif

#endif
