#ifndef _SOUNDGCN_H
#define _SOUNDGCN_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VI_NTSC_CLOCK 48681812

// __anon_0x221A3
typedef enum SoundRamp {
    SR_NONE = -1,
    SR_DECREASE = 0,
    SR_INCREASE = 1,
} SoundRamp;

// __anon_0x20C8D
typedef enum SoundBeep {
    SOUND_BEEP_ACCEPT = 0,
    SOUND_BEEP_DECLINE = 1,
    SOUND_BEEP_SELECT = 2,
    SOUND_BEEP_COUNT = 3,
} SoundBeep;

typedef enum SoundPlayMode {
    SPM_NONE = -1,
    SPM_PLAY = 0,
    SPM_RAMPQUEUED = 1,
    SPM_RAMPPLAYED = 2,
} SoundPlayMode;

// __anon_0x208BA
typedef struct Sound {
    /* 0x000 */ s32 unk_00;
    /* 0x004 */ void* pSrcData;
    /* 0x008 */ s32 nFrequency;
    /* 0x00C */ s32 nDacrate;
    /* 0x010 */ s32 nSndLen;
    /* 0x014 */ void* apBuffer[16];
    /* 0x01C */ s32 anSizeBuffer[16];
    /* 0x094 */ s32 unk_94;
    /* 0x098 */ s32 nVolumeCurve[257];
    /* 0x49C */ s32 iBufferPlay;
    /* 0x4A0 */ s32 iBufferMake;
    /* 0x4A4 */ SoundPlayMode eMode;
    /* 0x4A8 */ void* pBufferZero;
    /* 0x4AC */ void* pBufferHold;
    /* 0x4B0 */ void* pBufferRampUp;
    /* 0x4B4 */ void* pBufferRampDown;
    /* 0x4B8 */ s32 nSizePlay;
    /* 0x4BC */ s32 nSizeZero;
    /* 0x4C0 */ s32 nSizeHold;
    /* 0x4C4 */ s32 nSizeRamp;
} Sound; // size = 0x4C8

bool soundWipeBuffers(Sound* pSound);
bool soundSetLength(Sound* pSound, s32 nSize);
bool soundSetDACRate(Sound* pSound, s32 nDacRate);
bool soundSetAddress(Sound* pSound, void* pData);
bool soundGetDMABuffer(Sound* pSound, u32* pnSize);
bool soundSetBufferSize(Sound* pSound, s32 nSize);
bool soundLoadBeep(Sound* pSound, SoundBeep iBeep, char* szNameFile);
bool soundPlayBeep(Sound* pSound, SoundBeep iBeep);
bool soundEvent(Sound* pSound, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassAudio;

#ifdef __cplusplus
}
#endif

#endif
