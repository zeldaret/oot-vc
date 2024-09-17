#include "revolution/ax.h"
#include "revolution/os.h"
#include "string.h"

/**
 * Ring buffers have three entries:
 * 1. For DSP write
 * 2. For DSP read
 * 3. For CPU read/write
 */
#define AUX_RING_MAX 3

/**
 * 96 samples per frame @ 32-bit depth
 */
#define AUX_CHAN_SIZE (AX_SAMPLE_DEPTH_BYTES * AX_SAMPLES_PER_FRAME)

/**
 * Stereo buffer size: (96 samples/frame, 3 channels)
 * DPL2 buffer size:   (96 samples/frame, 4 channels)
 */
#define STEREO_FRAME_SIZE (AUX_CHAN_SIZE * AX_STEREO_MAX)
#define DPL2_FRAME_SIZE (AUX_CHAN_SIZE * AX_DPL2_MAX)

/**
 * Sample indices for stereo
 */
#define STEREO_SAMPLE_L_BEGIN (AUX_CHAN_SIZE * 0)
#define STEREO_SAMPLE_R_BEGIN (AUX_CHAN_SIZE * 1)
#define STEREO_SAMPLE_S_BEGIN (AUX_CHAN_SIZE * 2)

/**
 * Sample indices for DPL2
 */
#define DPL2_SAMPLE_I_BEGIN (AUX_CHAN_SIZE * 3)
#define DPL2_SAMPLE_L_BEGIN (AUX_CHAN_SIZE * 0)
#define DPL2_SAMPLE_R_BEGIN (AUX_CHAN_SIZE * 1)
#define DPL2_SAMPLE_LS_BEGIN (AUX_CHAN_SIZE * 2)
#define DPL2_SAMPLE_RS_BEGIN (AUX_CHAN_SIZE * 2)

static u8 __AXBufferAuxA[AUX_RING_MAX][STEREO_FRAME_SIZE] ATTRIBUTE_ALIGN(32);
static u8 __AXBufferAuxB[AUX_RING_MAX][STEREO_FRAME_SIZE] ATTRIBUTE_ALIGN(32);
static u8 __AXBufferAuxC[AUX_RING_MAX][STEREO_FRAME_SIZE] ATTRIBUTE_ALIGN(32);

static u8 __clearAuxA[AUX_RING_MAX];
static u8 __clearAuxB[AUX_RING_MAX];
static u8 __clearAuxC[AUX_RING_MAX];

static AXAuxCallback __AXCallbackAuxC;
static AXAuxCallback __AXCallbackAuxB;
static AXAuxCallback __AXCallbackAuxA;

static void* __AXContextAuxA;
static void* __AXContextAuxB;
static void* __AXContextAuxC;

static void* __AXAuxADspWrite;
static void* __AXAuxADspRead;

static void* __AXAuxBDspWrite;
static void* __AXAuxBDspRead;

static void* __AXAuxCDspWrite;
static void* __AXAuxCDspRead;

static u32 __AXAuxDspWritePosition;
static u32 __AXAuxDspReadPosition;
static u32 __AXAuxCpuReadWritePosition;

void __AXAuxInit(void) {
    int i;
    u32* bufferA = (u32*)__AXBufferAuxA;
    u32* bufferB = (u32*)__AXBufferAuxB;
    u32* bufferC = (u32*)__AXBufferAuxC;

    __AXCallbackAuxA = NULL;
    __AXCallbackAuxB = NULL;
    __AXCallbackAuxC = NULL;

    __AXContextAuxA = NULL;
    __AXContextAuxB = NULL;
    __AXContextAuxC = NULL;

    __AXAuxDspWritePosition = 0;
    __AXAuxDspReadPosition = 1;
    __AXAuxCpuReadWritePosition = 2;

    for (i = 0; i < (STEREO_FRAME_SIZE / sizeof(u32)); i++) {
        *bufferA++ = 0;
        *bufferB++ = 0;
        *bufferC++ = 0;
    }
}

void __AXAuxQuit(void) {
    __AXCallbackAuxA = NULL;
    __AXCallbackAuxB = NULL;
    __AXCallbackAuxC = NULL;
}

void __AXGetAuxAInput(void** out) {
    if (__AXCallbackAuxA != 0) {
        *out = __AXBufferAuxA[__AXAuxDspWritePosition];
    } else {
        *out = NULL;
    }
}

void __AXGetAuxAOutput(void** out) { *out = __AXBufferAuxA[__AXAuxDspReadPosition]; }

void __AXGetAuxAInputDpl2(void** out) { *out = &__AXBufferAuxC[__AXAuxDspWritePosition][384]; }

void __AXGetAuxAOutputDpl2R(void** out) { *out = &__AXBufferAuxA[__AXAuxDspReadPosition][DPL2_SAMPLE_R_BEGIN]; }

void __AXGetAuxAOutputDpl2Ls(void** out) { *out = &__AXBufferAuxA[__AXAuxDspReadPosition][DPL2_SAMPLE_LS_BEGIN]; }

void __AXGetAuxAOutputDpl2Rs(void** out) { *out = &__AXBufferAuxC[__AXAuxDspReadPosition][384]; }

void __AXGetAuxBInput(void** out) {
    if (__AXCallbackAuxB != 0) {
        *out = __AXBufferAuxB[__AXAuxDspWritePosition];
    } else {
        *out = NULL;
    }
}

void __AXGetAuxBOutput(void** out) { *out = __AXBufferAuxB[__AXAuxDspReadPosition]; }

void __AXGetAuxBInputDpl2(void** out) { *out = &__AXBufferAuxC[__AXAuxDspWritePosition][768]; }

void __AXGetAuxBOutputDpl2R(void** out) { *out = &__AXBufferAuxB[__AXAuxDspReadPosition][DPL2_SAMPLE_R_BEGIN]; }

void __AXGetAuxBOutputDpl2Ls(void** out) { *out = &__AXBufferAuxB[__AXAuxDspReadPosition][DPL2_SAMPLE_LS_BEGIN]; }

void __AXGetAuxBOutputDpl2Rs(void** out) { *out = &__AXBufferAuxC[__AXAuxDspReadPosition][768]; }

void __AXGetAuxCInput(void** out) {
    if (__AXCallbackAuxC != 0) {
        *out = __AXBufferAuxC[__AXAuxDspWritePosition];
    } else {
        *out = NULL;
    }
}

void __AXGetAuxCOutput(void** out) { *out = __AXBufferAuxC[__AXAuxDspReadPosition]; }

void __AXProcessAux(void) {
    __AXAuxADspWrite = __AXBufferAuxA[__AXAuxDspWritePosition];
    __AXAuxADspRead = __AXBufferAuxA[__AXAuxDspReadPosition];

    __AXAuxBDspWrite = __AXBufferAuxB[__AXAuxDspWritePosition];
    __AXAuxBDspRead = __AXBufferAuxB[__AXAuxDspReadPosition];

    __AXAuxCDspWrite = __AXBufferAuxC[__AXAuxDspWritePosition];
    __AXAuxCDspRead = __AXBufferAuxC[__AXAuxDspReadPosition];

    if (__AXCallbackAuxA != 0) {
        if (__AXClMode == AX_OUTPUT_DPL2) {
            void* chans[AX_DPL2_MAX];
            // clang-format off
            chans[AX_DPL2_L]  = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_L_BEGIN];
            chans[AX_DPL2_R]  = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_R_BEGIN];
            chans[AX_DPL2_LS] = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_LS_BEGIN];
            chans[AX_DPL2_RS] = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_RS_BEGIN];
            // clang-format on

            DCInvalidateRange(chans[0], 0x480);
            DCInvalidateRange(chans[3], 0x180);
            __AXCallbackAuxA(chans, __AXContextAuxA);
            DCFlushRangeNoSync(chans[0], 0x480);
            DCFlushRangeNoSync(chans[3], 0x180);
        } else {
            void* chans[AX_STEREO_MAX];
            // clang-format off
            chans[AX_STEREO_L] = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_L_BEGIN];
            chans[AX_STEREO_R] = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_R_BEGIN];
            chans[AX_STEREO_S] = &__AXBufferAuxA[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_S_BEGIN];
            // clang-format on

            DCInvalidateRange(chans[0], 0x480);
            __AXCallbackAuxA(chans, __AXContextAuxA);
            DCFlushRangeNoSync(chans[0], 0x480);
        }
    }

    if (__AXCallbackAuxB != 0) {
        if (__AXClMode == AX_OUTPUT_DPL2) {
            void* chans[AX_DPL2_MAX];
            // clang-format off
            chans[AX_DPL2_L]  = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_L_BEGIN];
            chans[AX_DPL2_R]  = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_R_BEGIN];
            chans[AX_DPL2_LS] = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_LS_BEGIN];
            chans[AX_DPL2_RS] = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][DPL2_SAMPLE_RS_BEGIN];
            // clang-format on

            DCInvalidateRange(chans[0], 0x300);
            DCInvalidateRange(chans[3], 0x180);
            __AXCallbackAuxB(chans, __AXContextAuxB);
            DCFlushRangeNoSync(chans[0], 0x480);
            DCFlushRangeNoSync(chans[3], 0x180);
        } else {
            void* chans[AX_STEREO_MAX];
            // clang-format off
            chans[AX_STEREO_L] = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_L_BEGIN];
            chans[AX_STEREO_R] = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_R_BEGIN];
            chans[AX_STEREO_S] = &__AXBufferAuxB[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_S_BEGIN];
            // clang-format on

            DCInvalidateRange(chans[0], 0x480);
            __AXCallbackAuxB(chans, __AXContextAuxB);
            DCFlushRangeNoSync(chans[0], 0x480);
        }
    }

    if (__AXCallbackAuxC != 0 && __AXClMode != AX_OUTPUT_DPL2) {
        void* chans[AX_STEREO_MAX];
        // clang-format off
        chans[AX_STEREO_L] = &__AXBufferAuxC[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_L_BEGIN];
        chans[AX_STEREO_R] = &__AXBufferAuxC[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_R_BEGIN];
        chans[AX_STEREO_S] = &__AXBufferAuxC[__AXAuxCpuReadWritePosition][STEREO_SAMPLE_S_BEGIN];
        // clang-format on

        DCInvalidateRange(chans[0], 0x480);
        __AXCallbackAuxC(chans, __AXContextAuxC);
        DCFlushRangeNoSync(chans[0], 0x480);
    }

    __AXAuxDspWritePosition++;
    __AXAuxDspWritePosition %= AX_STEREO_MAX;

    __AXAuxDspReadPosition++;
    __AXAuxDspReadPosition %= AX_STEREO_MAX;

    __AXAuxCpuReadWritePosition++;
    __AXAuxCpuReadWritePosition %= AX_STEREO_MAX;
}

void AXRegisterAuxACallback(AXAuxCallback callback, void* context) {
    __AXCallbackAuxA = callback;
    __AXContextAuxA = context;
}

void AXRegisterAuxBCallback(AXAuxCallback callback, void* context) {
    __AXCallbackAuxB = callback;
    __AXContextAuxB = context;
}

void AXRegisterAuxCCallback(AXAuxCallback callback, void* context) {
    __AXCallbackAuxC = callback;
    __AXContextAuxC = context;
}

void AXGetAuxACallback(AXAuxCallback* callback, void** context) {
    *callback = __AXCallbackAuxA;
    *context = __AXContextAuxA;
}

void AXGetAuxBCallback(AXAuxCallback* callback, void** context) {
    *callback = __AXCallbackAuxB;
    *context = __AXContextAuxB;
}

void AXGetAuxCCallback(AXAuxCallback* callback, void** context) {
    *callback = __AXCallbackAuxC;
    *context = __AXContextAuxC;
}
