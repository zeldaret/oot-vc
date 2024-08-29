#ifndef _RVL_SDK_EXI_BIOS_H
#define _RVL_SDK_EXI_BIOS_H

#include "revolution/exi/EXICommon.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EXIItem {
    /* 0x0 */ u32 dev;
    /* 0x4 */ EXICallback callback;
} EXIItem;

typedef struct EXIData {
    /* 0x0 */ EXICallback exiCallback;
    /* 0x4 */ EXICallback tcCallback;
    /* 0x8 */ EXICallback extCallback;
    /* 0xC */ volatile s32 state;
    /* 0x10 */ s32 bytesRead;
    /* 0x14 */ void* buffer;
    /* 0x18 */ u32 dev;
    /* 0x1C */ u32 id;
    /* 0x20 */ s32 lastInsert;
    /* 0x24 */ s32 numItems;
    /* 0x28 */ EXIItem items[3];
} EXIData;

bool EXIImm(EXIChannel chan, void* buf, s32 len, u32 type, EXICallback callback);
bool EXIImmEx(EXIChannel chan, void* buf, s32 len, u32 type);
bool EXIDma(EXIChannel chan, void* buf, s32 len, u32 type, EXICallback callback);
bool EXISync(EXIChannel chan);
void EXIClearInterrupts(EXIChannel chan, bool exi, bool tc, bool ext);
EXICallback EXISetExiCallback(EXIChannel chan, EXICallback callback);
void EXIProbeReset(void);
bool EXIProbe(EXIChannel chan);
bool EXIAttach(EXIChannel chan, EXICallback callback);
bool EXIDetach(EXIChannel chan);
bool EXISelect(EXIChannel chan, u32 dev, u32 freq);
bool EXIDeselect(EXIChannel chan);
void EXIInit(void);
bool EXILock(EXIChannel chan, u32 dev, EXICallback callback);
bool EXIUnlock(EXIChannel chan);
s32 EXIGetID(EXIChannel chan, u32 dev, u32* out);

#ifdef __cplusplus
}
#endif

#endif
