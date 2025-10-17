#ifndef _RVL_SDK_AX_VPB_H
#define _RVL_SDK_AX_VPB_H

#include "revolution/ax/AXPB.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AX_SAMPLE_RATE 32000
#define AX_VOICE_MAX 64

typedef enum {
    AX_PBSYNC_SELECT = (1 << 0),
    AX_PBSYNC_MIXER_CTRL = (1 << 1),
    AX_PBSYNC_STATE = (1 << 2),
    AX_PBSYNC_TYPE = (1 << 3),
    AX_PBSYNC_MIX = (1 << 4),
    AX_PBSYNC_ITD = (1 << 5),
    AX_PBSYNC_ITD_SHIFT = (1 << 6),

    // TODO
    AX_PBSYNC_UNK7 = (1 << 7),

    AX_PBSYNC_DPOP = (1 << 8),
    AX_PBSYNC_VE = (1 << 9),
    AX_PBSYNC_VE_DELTA = (1 << 10),
    AX_PBSYNC_ADDR = (1 << 11),
    AX_PBSYNC_LOOP_FLAG = (1 << 12),
    AX_PBSYNC_LOOP_ADDR = (1 << 13),
    AX_PBSYNC_END_ADDR = (1 << 14),
    AX_PBSYNC_CURR_ADDR = (1 << 15),
    AX_PBSYNC_ADPCM = (1 << 16),
    AX_PBSYNC_SRC = (1 << 17),
    AX_PBSYNC_SRC_RATIO = (1 << 18),
    AX_PBSYNC_ADPCM_LOOP = (1 << 19),
    AX_PBSYNC_LPF = (1 << 20),
    AX_PBSYNC_LPF_COEFS = (1 << 21),

    // TODO (AX_PBSYNC_BIQUAD/AX_PBSYNC_BIQUAD_COEFS?)
    AX_PBSYNC_UNK22 = (1 << 22),
    AX_PBSYNC_UNK23 = (1 << 23),

    AX_PBSYNC_REMOTE = (1 << 24),
    AX_PBSYNC_RMT_MIXER_CTRL = (1 << 25),
    AX_PBSYNC_RMTMIX = (1 << 26),
    AX_PBSYNC_RMTDPOP = (1 << 27),
    AX_PBSYNC_RMTSRC = (1 << 28),
    AX_PBSYNC_ALL = (1 << 31),
};

typedef void (*AXVoiceCallback)(void* vpb);

typedef struct _AXPBU {
    u16 UNK_0x0[0x80];
} AXPBU;

typedef struct _AXITD {
    u8 UNK_0x0[0x40];
} AXITD;

typedef struct _AXVPB {
    /* 0x000 */ void* next;
    /* 0x004 */ void* prev;
    /* 0x008 */ void* next1;
    /* 0x00C */ u32 priority;
    /* 0x010 */ AXVoiceCallback callback;
    /* 0x014 */ u32 userContext;
    /* 0x018 */ u32 index;
    /* 0x01C */ u32 sync;
    /* 0x020 */ u32 depop;
    /* 0x024 */ u32 updateMS;
    /* 0x028 */ u32 updateCounter;
    /* 0x02C */ u32 updateTotal;
    /* 0x030 */ u16* updateWrite;
    /* 0x034 */ AXPBU updateData;
    /* 0x134 */ void* itdBuffer;
    /* 0x138 */ AXPB pb;
} AXVPB; // size = 0x258

s32 __AXGetNumVoices(void);
void __AXServiceVPB(AXVPB* vpb);
void __AXDumpVPB(AXVPB* vpb);
void __AXSyncPBs(u32 baseCycles);
AXPB* __AXGetPBs(void);
void __AXSetPBDefault(AXVPB* vpb);
void __AXVPBInit(void);

void AXSetVoiceSrcType(AXVPB* vpb, u32 type);
void AXSetVoiceState(AXVPB* vpb, u16 state);
void AXSetVoiceType(AXVPB* vpb, u16 type);
void AXSetVoiceMix(AXVPB* vpb, AXPBMIX* mix);
void AXSetVoiceVe(AXVPB* vpb, AXPBVE* ve);
void AXSetVoiceAddr(AXVPB* vpb, AXPBADDR* addr);
void AXSetVoiceAdpcm(AXVPB* vpb, AXPBADPCM* adpcm);
void AXSetVoiceSrc(AXVPB* vpb, AXPBSRC* src_);
void AXSetVoiceSrcRatio(AXVPB* vpb, float ratio);
void AXSetVoiceAdpcmLoop(AXVPB* vpb, AXPBADPCMLOOP* adpcmloop);
void AXSetVoiceLpf(AXVPB* vpb, AXPBLPF* lpf);
void AXSetVoiceLpfCoefs(AXVPB* vpb, u16 a0, u16 b0);
void AXGetLpfCoefs(u16 freq, u16* a, u16* b);
void AXSetVoiceRmtOn(AXVPB* vpb, u16 on);
void AXSetVoiceRmtMix(AXVPB* vpb, AXPBRMTMIX* mix);
void AXSetMaxDspCycles(u32 num);

#ifdef __cplusplus
}
#endif

#endif
