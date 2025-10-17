#ifndef _RVL_SDK_AX_PB_H
#define _RVL_SDK_AX_PB_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * One frame contains eight bytes:
 * - One for the header
 * - Seven for the audio samples
 */
#define AX_ADPCM_FRAME_SIZE 8
#define AX_ADPCM_SAMPLE_BYTES_PER_FRAME (AX_ADPCM_FRAME_SIZE - 1)

// Two audio samples per byte (each nibble)
#define AX_ADPCM_SAMPLES_PER_BYTE 2

// Amount of audio samples in a frame
#define AX_ADPCM_SAMPLES_PER_FRAME (AX_ADPCM_SAMPLE_BYTES_PER_FRAME * AX_ADPCM_SAMPLES_PER_BYTE)

// Amount of nibbles in a frame
#define AX_ADPCM_NIBBLES_PER_FRAME (AX_ADPCM_FRAME_SIZE * 2)

typedef enum {
    AX_VOICE_NORMAL,
    AX_VOICE_STREAM
} AXVOICETYPE;

typedef enum {
    AX_VOICE_STOP,
    AX_VOICE_RUN
} AXVOICESTATE;

typedef enum {
    AX_SAMPLE_FORMAT_DSP_ADPCM = 0,
    AX_SAMPLE_FORMAT_PCM_S16 = 10,
    AX_SAMPLE_FORMAT_PCM_S8 = 25,
} AXSAMPLETYPE;

// For rmtIIR union I think? From NW4R asserts, but fits well in __AXSyncPBs
typedef enum {
    AX_PB_LPF_ON = 1,
    AX_PB_BIQUAD_ON,
};

typedef enum {
    AX_SRC_TYPE_NONE,
    AX_SRC_TYPE_LINEAR,
    AX_SRC_TYPE_4TAP_8K,
    AX_SRC_TYPE_4TAP_12K,
    AX_SRC_TYPE_4TAP_16K,
    AX_SRC_TYPE_4TAP_AUTO
} AXPBSRCTYPE;

typedef enum {
    AX_MIXER_CTRL_L = (1 << 0),
    AX_MIXER_CTRL_R = (1 << 1),
    AX_MIXER_CTRL_DELTA = (1 << 2),
    AX_MIXER_CTRL_S = (1 << 3),
    AX_MIXER_CTRL_DELTA_S = (1 << 4),

    AX_MIXER_CTRL_AL = (1 << 16),
    AX_MIXER_CTRL_AR = (1 << 17),
    AX_MIXER_CTRL_DELTA_A = (1 << 18),
    AX_MIXER_CTRL_AS = (1 << 19),
    AX_MIXER_CTRL_DELTA_AS = (1 << 20),

    AX_MIXER_CTRL_BL = (1 << 21),
    AX_MIXER_CTRL_BR = (1 << 22),
    AX_MIXER_CTRL_DELTA_B = (1 << 23),
    AX_MIXER_CTRL_BS = (1 << 24),
    AX_MIXER_CTRL_DELTA_BS = (1 << 25),

    AX_MIXER_CTRL_CL = (1 << 26),
    AX_MIXER_CTRL_CR = (1 << 27),
    AX_MIXER_CTRL_DELTA_C = (1 << 28),
    AX_MIXER_CTRL_CS = (1 << 29),
    AX_MIXER_CTRL_DELTA_CS = (1 << 30)
};

typedef enum {
    AX_MIXER_CTRL_RMT_M0 = (1 << 0),
    AX_MIXER_CTRL_RMT_DELTA_M0 = (1 << 1),
    AX_MIXER_CTRL_RMT_A0 = (1 << 2),
    AX_MIXER_CTRL_RMT_DELTA_A0 = (1 << 3),

    AX_MIXER_CTRL_RMT_M1 = (1 << 4),
    AX_MIXER_CTRL_RMT_DELTA_M1 = (1 << 5),
    AX_MIXER_CTRL_RMT_A1 = (1 << 6),
    AX_MIXER_CTRL_RMT_DELTA_A1 = (1 << 7),

    AX_MIXER_CTRL_RMT_M2 = (1 << 8),
    AX_MIXER_CTRL_RMT_DELTA_M2 = (1 << 9),
    AX_MIXER_CTRL_RMT_A2 = (1 << 10),
    AX_MIXER_CTRL_RMT_DELTA_A2 = (1 << 11),

    AX_MIXER_CTRL_RMT_M3 = (1 << 12),
    AX_MIXER_CTRL_RMT_DELTA_M3 = (1 << 13),
    AX_MIXER_CTRL_RMT_A3 = (1 << 14),
    AX_MIXER_CTRL_RMT_DELTA_A3 = (1 << 15)
};

typedef struct _AXPBMIX {
    /* 0x00 */ u16 vL;
    /* 0x02 */ u16 vDeltaL;
    /* 0x04 */ u16 vR;
    /* 0x06 */ u16 vDeltaR;
    /* 0x08 */ u16 vAuxAL;
    /* 0x0A */ u16 vDeltaAuxAL;
    /* 0x0C */ u16 vAuxAR;
    /* 0x0E */ u16 vDeltaAuxAR;
    /* 0x10 */ u16 vAuxBL;
    /* 0x12 */ u16 vDeltaAuxBL;
    /* 0x14 */ u16 vAuxBR;
    /* 0x16 */ u16 vDeltaAuxBR;
    /* 0x18 */ u16 vAuxCL;
    /* 0x1A */ u16 vDeltaAuxCL;
    /* 0x1C */ u16 vAuxCR;
    /* 0x1E */ u16 vDeltaAuxCR;
    /* 0x20 */ u16 vS;
    /* 0x22 */ u16 vDeltaS;
    /* 0x24 */ u16 vAuxAS;
    /* 0x26 */ u16 vDeltaAuxAS;
    /* 0x28 */ u16 vAuxBS;
    /* 0x2A */ u16 vDeltaAuxBS;
    /* 0x2C */ u16 vAuxCS;
    /* 0x2E */ u16 vDeltaAuxCS;
} AXPBMIX; // size = 0x30

typedef struct _AXPBITD {
    /* 0x00 */ u16 flag;
    /* 0x02 */ u16 bufferHi;
    /* 0x04 */ u16 bufferLo;
    /* 0x06 */ u16 shiftL;
    /* 0x08 */ u16 shiftR;
    /* 0x0A */ u16 targetShiftL;
    /* 0x0C */ u16 targetShiftR;
} AXPBITD; // size = 0xE

typedef struct _AXPBDPOP {
    /* 0x00 */ s16 aL;
    /* 0x02 */ s16 aAuxAL;
    /* 0x04 */ s16 aAuxBL;
    /* 0x06 */ s16 aAuxCL;
    /* 0x08 */ s16 aR;
    /* 0x0A */ s16 aAuxAR;
    /* 0x0C */ s16 aAuxBR;
    /* 0x0E */ s16 aAuxCR;
    /* 0x10 */ s16 aS;
    /* 0x12 */ s16 aAuxAS;
    /* 0x14 */ s16 aAuxBS;
    /* 0x16 */ s16 aAuxCS;
} AXPBDPOP; // size = 0x18

typedef struct _AXPBVE {
    /* 0x0 */ u16 currentVolume;
    /* 0x2 */ s16 currentDelta;
} AXPBVE;

typedef struct _AXPBADDR {
    /* 0x0 */ u16 loopFlag;
    /* 0x2 */ u16 format;
    /* 0x4 */ u16 loopAddressHi;
    /* 0x6 */ u16 loopAddressLo;
    /* 0x8 */ u16 endAddressHi;
    /* 0xA */ u16 endAddressLo;
    /* 0xC */ u16 currentAddressHi;
    /* 0xE */ u16 currentAddressLo;
} AXPBADDR;

typedef struct _AXPBADPCM {
    /* 0x0 */ u16 a[8][2];
    /* 0x20 */ u16 gain;
    /* 0x22 */ u16 pred_scale;
    /* 0x24 */ u16 yn1;
    /* 0x26 */ u16 yn2;
} AXPBADPCM;

typedef struct _AXPBSRC {
    /* 0x0 */ u16 ratioHi;
    /* 0x2 */ u16 ratioLo;
    /* 0x4 */ u16 currentAddressFrac;
    /* 0x6 */ u16 last_samples[4];
} AXPBSRC;

typedef struct _AXPBADPCMLOOP {
    /* 0x0 */ u16 loop_pred_scale;
    /* 0x2 */ u16 loop_yn1;
    /* 0x4 */ u16 loop_yn2;
} AXPBADPCMLOOP; // size = 0x6

typedef struct _AXPBLPF {
    /* 0x0 */ u16 on;
    /* 0x2 */ u16 yn1;
    /* 0x4 */ u16 a0;
    /* 0x6 */ u16 b0;
} AXPBLPF;

typedef struct _AXPBBIQUAD {
    /* 0x0 */ u16 on;
    /* 0x2 */ u16 xn1;
    /* 0x4 */ u16 xn2;
    /* 0x6 */ u16 yn1;
    /* 0x8 */ u16 yn2;
    /* 0xA */ u16 b0;
    /* 0xC */ u16 b1;
    /* 0xE */ u16 b2;
    /* 0x10 */ u16 a1;
    /* 0x12 */ u16 a2;
} AXPBBIQUAD;

typedef struct _AXPBRMTMIX {
    /* 0x0 */ u16 vMain0;
    /* 0x2 */ u16 vDeltaMain0;
    /* 0x4 */ u16 vAux0;
    /* 0x6 */ u16 vDeltaAux0;
    /* 0x8 */ u16 vMain1;
    /* 0xA */ u16 vDeltaMain1;
    /* 0xC */ u16 vAux1;
    /* 0xE */ u16 vDeltaAux1;
    /* 0x10 */ u16 vMain2;
    /* 0x12 */ u16 vDeltaMain2;
    /* 0x14 */ u16 vAux2;
    /* 0x16 */ u16 vDeltaAux2;
    /* 0x18 */ u16 vMain3;
    /* 0x1A */ u16 vDeltaMain3;
    /* 0x1C */ u16 vAux3;
    /* 0x1E */ u16 vDeltaAux3;
} AXPBRMTMIX;

typedef struct _AXPBRMTDPOP {
    /* 0x0 */ s16 aMain0;
    /* 0x2 */ s16 aMain1;
    /* 0x4 */ s16 aMain2;
    /* 0x6 */ s16 aMain3;
    /* 0x8 */ s16 aAux0;
    /* 0xA */ s16 aAux1;
    /* 0xC */ s16 aAux2;
    /* 0xE */ s16 aAux3;
} AXPBRMTDPOP;

typedef struct _AXPBRMTSRC {
    /* 0x0 */ u16 currentAddressFrac;
    /* 0x2 */ u16 last_samples[4];
} AXPBRMTSRC;

typedef union __AXPBRMTIIR {
    AXPBLPF lpf;
    AXPBBIQUAD biquad;
} AXPBRMTIIR;

typedef struct _struct_052 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u16 unk8;
} struct_052;

typedef struct _struct_0CC {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
} struct_0CC;

typedef struct _AXPB {
    /* 0x000 */ u16 nextHi;
    /* 0x002 */ u16 nextLo;
    /* 0x004 */ u16 currHi;
    /* 0x006 */ u16 currLo;
    /* 0x008 */ u16 srcSelect;
    /* 0x00A */ u16 coefSelect;
    /* 0x00C */ u32 mixerCtrl;
    /* 0x010 */ u16 state;
    /* 0x012 */ u16 type;
    /* 0x014 */ AXPBMIX mix;
    /* 0x044 */ AXPBITD itd;
    /* 0x052 */ struct_052 unk052; // TODO
    /* 0x05C */ AXPBDPOP dpop;
    /* 0x074 */ AXPBVE ve;
    /* 0x078 */ AXPBADDR addr;
    /* 0x088 */ AXPBADPCM adpcm;
    /* 0x0B0 */ AXPBSRC src;
    /* 0x0BE */ AXPBADPCMLOOP adpcmLoop;
    /* 0x0C4 */ AXPBLPF lpf;
    /* 0x0CC */ struct_0CC unk0CC; // TODO (biquad?)
    /* 0x0D4 */ u16 remote;
    /* 0x0D6 */ u16 rmtMixerCtrl;
    /* 0x0D8 */ AXPBRMTMIX rmtMix;
    /* 0x0F8 */ AXPBRMTDPOP rmtDpop;
    /* 0x108 */ AXPBRMTSRC rmtSrc;
    /* 0x112 */ u8 pad3[0xE]; // TODO
} AXPB; // size = 0x120

#ifdef __cplusplus
}
#endif

#endif
