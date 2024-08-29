#include "emulator/soundRVL.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlFileRVL.h"
#include "emulator/xlHeap.h"
#include "emulator/xlPostRVL.h"
#include "macros.h"
#include "math.h"
#include "revolution/ai.h"
#include "stdlib.h"

_XL_OBJECTTYPE gClassAudio = {
    "AUDIO",
    sizeof(Sound),
    NULL,
    (EventFunc)soundEvent,
};

bool soundWipeBuffers(Sound* pSound) {
    s32 iBuffer;

    if (pSound->pBufferZero != NULL && !xlHeapFree(&pSound->pBufferZero)) {
        return false;
    }

    if (pSound->pBufferHold != NULL && !xlHeapFree(&pSound->pBufferHold)) {
        return false;
    }

    if (pSound->pBufferRampUp != NULL && !xlHeapFree(&pSound->pBufferRampUp)) {
        return false;
    }

    if (pSound->pBufferRampDown != NULL && !xlHeapFree(&pSound->pBufferRampDown)) {
        return false;
    }

    for (iBuffer = 0; iBuffer < 16; iBuffer++) {
        if (pSound->apBuffer[iBuffer] != NULL && !xlHeapFree(&pSound->apBuffer[iBuffer])) {
            return false;
        }
    }

    return true;
}

static bool soundMakeRamp(Sound* pSound, s32 iBuffer, SoundRamp eRamp) {
    s32 bFlag;
    s32 iData;
    s16* anData;
    s16 nData0;
    s16 nData1;
    s16 nGoal0;
    s16 nGoal1;
    s16 nStep0;
    s16 nStep1;
    s16 nLast0;
    s16 nLast1;

    int offset;

    if (eRamp == SR_DECREASE) {
        anData = pSound->pBufferRampDown;
        offset = (pSound->anSizeBuffer[iBuffer] >> 1) - 2;
        nData0 = ((s16*)pSound->apBuffer[iBuffer])[offset];
        nData1 = ((s16*)pSound->apBuffer[iBuffer])[offset + 1];

        if (nData0 == 0) {
            nStep0 = 0;
        } else if ((nStep0 = 0.5f + (f32)nData0 / (pSound->nSizeRamp >> 2)) == 0) {
            nStep0 = nData0 > 0 ? 1 : -1;
        }

        if (nData1 == 0) {
            nStep1 = 0;
        } else if ((nStep1 = 0.5f + (f32)nData1 / (pSound->nSizeRamp >> 2)) == 0) {
            nStep1 = nData1 > 0 ? 1 : -1;
        }

        nStep0 = -nStep0;
        nStep1 = -nStep1;
        nGoal0 = nGoal1 = 0;
    } else if (eRamp == SR_INCREASE) {
        anData = pSound->pBufferRampUp;
        nGoal0 = ((s16*)pSound->apBuffer[iBuffer])[0];
        nGoal1 = ((s16*)pSound->apBuffer[iBuffer])[1];

        if (nGoal0 == 0) {
            nStep0 = 0;
        } else if ((nStep0 = 0.5f + (f32)nGoal0 / (pSound->nSizeRamp >> 2)) == 0) {
            nStep0 = nGoal0 > 0 ? 1 : -1;
        }

        if (nGoal1 == 0) {
            nStep1 = 0;
        } else if ((nStep1 = 0.5f + (f32)nGoal1 / (pSound->nSizeRamp >> 2)) == 0) {
            nStep1 = nGoal1 > 0 ? 1 : -1;
        }

        nData0 = nData1 = 0;
    } else {
        return false;
    }

    do {
        bFlag = 1;

        if (nStep0 != 0) {
            nLast0 = nData0 + (((u32)nStep0) * (pSound->nSizeRamp >> 2));
            if ((nData0 < 0 && nLast0 < 0) || (nData0 > 0 && nLast0 > 0) ||
                (nData0 == 0 && (nGoal0 < 0 ? nLast0 > nGoal0 : nLast0 < nGoal0))) {
                nStep0 <<= 1;
                bFlag = 0;
            }
        }

        if (nStep1 != 0) {
            nLast1 = nData1 + (((u32)nStep1) * (pSound->nSizeRamp >> 2));
            if ((nData1 < 0 && nLast1 < 0) || (nData1 > 0 && nLast1 > 0) ||
                (nData1 == 0 && (nGoal1 < 0 ? nLast1 > nGoal1 : nLast1 < nGoal1))) {
                nStep1 <<= 1;
                bFlag = 0;
            }
        }
    } while (bFlag == 0);

    for (iData = 0; iData < (pSound->nSizeRamp >> 1); iData += 2) {
        anData[iData + 0] = nData0;
        anData[iData + 1] = nData1;

        if (nData0 != nGoal0) {
            if ((nData0 < nGoal0 && nData0 + nStep0 > nGoal0) || (nData0 > nGoal0 != 0 && nData0 + nStep0 < nGoal0)) {
                nData0 = nGoal0;
            } else {
                nData0 += nStep0;
            }
        }

        if (nData1 != nGoal1) {
            if ((nData1 < nGoal1 && nData1 + nStep1 > nGoal1) || (nData1 > nGoal1 != 0 && nData1 + nStep1 < nGoal1)) {
                nData1 = nGoal1;
            } else {
                nData1 += nStep1;
            }
        }
    }

    DCStoreRange(anData, pSound->nSizeRamp);
    return true;
}

static inline bool soundMakeZero(Sound* pSound) {
    int iData;

    for (iData = 0; iData < (pSound->nSizeZero >> 1); iData++) {
        ((s16*)pSound->pBufferZero)[iData] = 0;
    }

    DCStoreRange(pSound->pBufferZero, pSound->nSizeZero);
    return true;
}

bool soundPlayBuffer(Sound* pSound) {
    void* pData;
    s32 iBuffer;
    s32 nSize;

    iBuffer = pSound->iBufferPlay;

    if ((iBuffer = pSound->iBufferPlay) != pSound->iBufferMake) {
        if (pSound->eMode == SPM_RAMPPLAYED) {
            nSize = pSound->nSizeRamp;
            pData = pSound->pBufferRampUp;
        } else {
            nSize = pSound->anSizeBuffer[iBuffer];
            pData = pSound->apBuffer[iBuffer];
            pSound->iBufferPlay = (iBuffer + 1) % 16;
        }
        pSound->eMode = SPM_PLAY;
        //! TODO: fake match on the following condition?
    } else if ((*((volatile SoundPlayMode*)&pSound->eMode) == SPM_RAMPQUEUED) || (pSound->eMode == SPM_RAMPPLAYED)) {
        pData = pSound->pBufferZero;
        nSize = pSound->nSizeZero;
        pSound->eMode = SPM_RAMPPLAYED;
    } else {
        pData = pSound->pBufferRampDown;
        nSize = pSound->nSizeRamp;
        pSound->eMode = SPM_RAMPQUEUED;
    }

    AIInitDMA((u32*)pData, nSize);
    return true;
}

static void soundCallbackDMA(void) { soundPlayBuffer(SYSTEM_SOUND(gpSystem)); }

bool soundMakeBuffer(Sound* pSound) {
    u32 nSamples;
    s16* curBufP;
    u32 sampleStep;
    u32 sample;
    s32 j;
    s32 nSize;
    s32 samp;
    s32 iBuffer;
    s32 vol;
    s32 bFlag;
    s32 bPlay;
    s32 volAdjust;

    s32 temp_r6;
    u32 temp_r3;

    iBuffer = pSound->iBufferMake;
    bPlay = 0;
    nSize = pSound->nSndLen;
    nSamples = ((nSize * 32000) + 16000) / pSound->nFrequency;
    temp_r3 = ((nSamples >> 2) & ~7);
    temp_r6 = pSound->anSizeBuffer[iBuffer] = nSamples & (~0x1F);
    curBufP = pSound->apBuffer[iBuffer];
    vol = 0x10000;
    sample = 0;

    for (j = 0; j < temp_r3 * 2; j += 2) {
        samp = sample >> 16;

        if (j + 1 < (pSound->nSizePlay >> 1)) {
            volAdjust = pSound->nVolumeCurve[vol >> 8];
            curBufP[j + 1] = (volAdjust * ((s16*)pSound->pSrcData)[2 * samp + 0]) >> 16;
            curBufP[j + 0] = (volAdjust * ((s16*)pSound->pSrcData)[2 * samp + 1]) >> 16;
        }

        // required to match
        vol = (u32)vol;

        sampleStep = ((nSize << 14) & 0xFFFF0000);
        sampleStep /= temp_r3;
        sample += sampleStep;
    }

    DCStoreRange(curBufP, temp_r6);
    pSound->iBufferMake = (iBuffer + 1) % 16;
    bFlag = OSDisableInterrupts();

    if (AIGetDMAEnableFlag() == 0 || pSound->eMode == SPM_RAMPQUEUED) {
        bPlay = 1;
        if (soundPlayBuffer(pSound) == 0) {
            return false;
        }
    }

    OSRestoreInterrupts(bFlag);

    if (pSound->eMode != SPM_PLAY && !soundMakeRamp(pSound, iBuffer, SR_INCREASE)) {
        return false;
    }

    if (!soundMakeRamp(pSound, iBuffer, SR_DECREASE)) {
        return false;
    }

    if (bPlay && AIGetDMAEnableFlag() == 0) {
        AIStartDMA();
    }

    return true;
}

bool soundSetLength(Sound* pSound, s32 nSize) {
    pSound->nSndLen = nSize;

    if (!soundMakeBuffer(pSound)) {
        return false;
    }

    return true;
}

bool soundSetDACRate(Sound* pSound, s32 nDacRate) {
    pSound->nDacrate = nDacRate;
    pSound->nFrequency = VI_NTSC_CLOCK / (nDacRate + 1);
    return true;
}

bool soundSetAddress(Sound* pSound, void* pData) {
    pSound->pSrcData = pData;
    return true;
}

bool soundGetDMABuffer(Sound* pSound, u32* pnSize) {
    *pnSize = AIGetDMABytesLeft();
    return true;
}

bool soundSetBufferSize(Sound* pSound, s32 nSize) {
    int iBuffer;

    if (nSize % 32 != 0) {
        nSize = (nSize + 0x1F) & ~0x1F;
    }

    pSound->nSizePlay = nSize;
    pSound->nSizeHold = pSound->nSizeZero = nSize >> 4;
    pSound->nSizeRamp = nSize >> 2;

    if (!soundWipeBuffers(pSound)) {
        return false;
    }

    for (iBuffer = 0; iBuffer < 16; iBuffer++) {
        if (!xlHeapTake(&pSound->apBuffer[iBuffer], pSound->nSizePlay | 0x30000000)) {
            return false;
        }
    }

    if (!xlHeapTake(&pSound->pBufferZero, pSound->nSizeZero | 0x30000000)) {
        return false;
    }

    soundMakeZero(pSound);

    if (!xlHeapTake(&pSound->pBufferHold, pSound->nSizeHold | 0x30000000)) {
        return false;
    }

    if (!xlHeapTake(&pSound->pBufferRampUp, pSound->nSizeRamp | 0x30000000)) {
        return false;
    }

    if (!xlHeapTake(&pSound->pBufferRampDown, pSound->nSizeRamp | 0x30000000)) {
        return false;
    }

    return true;
}

static inline void InitVolumeCurve(Sound* pSound) {
    s32 i;
    f64 value;

    // fixes float ordering issue
    (void)20.0;
    (void)65536.0f;
    (void)256.0;

    pSound->nVolumeCurve[0] = 0;
    for (i = 1; i < ARRAY_COUNT(pSound->nVolumeCurve); i++) {
        value = pow(10, (20.0f * log10f(SQ(256 - i) / 65536.0f)) / 20.0f);
        pSound->nVolumeCurve[i] = (256.0f * (f32) - ((value * 256.0) - 256.0));
    }
}

bool soundEvent(Sound* pSound, s32 nEvent, void* pArgument) {
    s32 var_r0;
    char* sp8;

    switch (nEvent) {
        case 0:
            AIInit(NULL);
            AISetDSPSampleRate(AI_DSP_32KHZ);
            AIStopDMA();
            AIRegisterDMACallback(soundCallbackDMA);
            break;
        case 1:
            AIStopDMA();
            fn_800ABA60();
            break;
        case 2:
            pSound->eMode = SPM_NONE;
            pSound->iBufferMake = 0;
            pSound->iBufferPlay = 0;
            pSound->unk_00 = 0;

            if (simulatorGetArgument(SAT_VIBRATION, &sp8)) {
                var_r0 = atoi(sp8) << 8;
            } else {
                var_r0 = 0x10000;
            }

            pSound->unk_94 = var_r0;

            pSound->apBuffer[0] = NULL;
            pSound->apBuffer[1] = NULL;
            pSound->apBuffer[2] = NULL;
            pSound->apBuffer[3] = NULL;
            pSound->apBuffer[4] = NULL;
            pSound->apBuffer[5] = NULL;
            pSound->apBuffer[6] = NULL;
            pSound->apBuffer[7] = NULL;
            pSound->apBuffer[8] = NULL;
            pSound->apBuffer[9] = NULL;
            pSound->apBuffer[10] = NULL;
            pSound->apBuffer[11] = NULL;
            pSound->apBuffer[12] = NULL;
            pSound->apBuffer[13] = NULL;
            pSound->apBuffer[14] = NULL;
            pSound->apBuffer[15] = NULL;
            pSound->apBuffer[16] = NULL;

            pSound->pBufferZero = NULL;
            pSound->pBufferHold = NULL;
            pSound->pBufferRampUp = NULL;
            pSound->pBufferRampDown = NULL;

            InitVolumeCurve(pSound);
            break;
        case 3:
            if (!soundWipeBuffers(pSound)) {
                return false;
            }
            break;
        case 0x1003:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return false;
    }

    return true;
}
