#include "revolution/ai.h"
#include "revolution/ax.h"
#include "revolution/dsp.h"
#include "revolution/os.h"

// Output uses a ring buffer
#define OUT_RING_MAX 3
// Each remote output buffer contains 10 frames
// TODO: Not sure if these instances are actually "frames"
#define RMT_FRAME_MAX 10

// Form DSP mail for command list size
#define DSP_MAIL_CLSIZE(size) (0xBABE << 16 | (size))
// DRAM image in DSP memory
// TODO: What does this represent?
#define DRAM_IMAGE_DSP_ADDR 0

// Back again by popular demand!
#define FUNNY_COPY(dst, src, type, n)            \
    do {                                         \
                                                 \
        type* __dst = (type*)dst;                \
        type* __src = (type*)src;                \
        int i;                                   \
                                                 \
        for (i = 0; i < n / sizeof(type); i++) { \
            *__dst = *__src;                     \
            __dst++;                             \
            __src++;                             \
        }                                        \
                                                 \
    } while (0)

AXPROFILE __AXLocalProfile;

static volatile u32 __AXOutFrame;
static volatile u32 __AXAiDmaFrame;
static u32 __AXOutDspReady;
static volatile s64 __AXOsTime;
static AXOutCallback __AXUserFrameCallback;
static volatile bool __AXDSPInitFlag;
static volatile bool __AXDSPDoneFlag;
static bool __AXDebugSteppingMode;
static OSThreadQueue __AXOutThreadQueue;
static u32 __AXOutputBufferMode;
static s32 __AXRmtBuffLen;
static s32 __AXRmtDspPtr;
static s32 __AXRmtCpuPtr;

static s16 __AXRmtOutBuffer[AX_RMT_MAX][AX_SAMPLES_PER_FRAME_RMT * RMT_FRAME_MAX] ATTRIBUTE_ALIGN(32);
// TODO: What is this number?
static u8 __AXOutSBuffer[768] ATTRIBUTE_ALIGN(32);
static u32 __AXOutBuffer[OUT_RING_MAX][AX_SAMPLES_PER_FRAME] ATTRIBUTE_ALIGN(32);

static DSPTask __AXDSPTask;
static u16 __AXDramImage[0x2000];

static void __AXDSPInitCallback(DSPTask* task);
static void __AXDSPResumeCallback(DSPTask* task);
static void __AXDSPDoneCallback(DSPTask* task);
static void __AXDSPRequestCallback(DSPTask* task);

void __AXOutNewFrame(u32 cycles) {
    AXPROFILE* current;
    void* list;
    void* rmt[AX_RMT_MAX];
    s32 ptr;

    __AXLocalProfile.timeBegin = OSGetTime();

    __AXSyncPBs(cycles);

    __AXPrintStudio();

    list = __AXGetCommandListAddress();
    DSP_SEND_MAIL_SYNC(DSP_MAIL_CLSIZE(AX_CL_SIZE));
    DSP_SEND_MAIL_SYNC(list);

    __AXServiceCallbackStack();

    __AXLocalProfile.timeProcessAuxBegin = OSGetTime();
    __AXProcessAux();
    __AXLocalProfile.timeProcessAuxEnd = OSGetTime();

    __AXLocalProfile.timeUserFrameBegin = OSGetTime();
    if (__AXUserFrameCallback != 0) {
        __AXUserFrameCallback();
    }
    __AXLocalProfile.timeUserFrameEnd = OSGetTime();

    rmt[0] = &__AXRmtOutBuffer[0][__AXRmtDspPtr];
    rmt[1] = &__AXRmtOutBuffer[1][__AXRmtDspPtr];
    rmt[2] = &__AXRmtOutBuffer[2][__AXRmtDspPtr];
    rmt[3] = &__AXRmtOutBuffer[3][__AXRmtDspPtr];

    ptr = __AXRmtDspPtr + AX_SAMPLES_PER_FRAME_RMT;
    if (ptr >= __AXRmtBuffLen) {
        ptr = 0;
    }

    if (__AXRmtCpuPtr >= __AXRmtDspPtr && __AXRmtCpuPtr < __AXRmtDspPtr + AX_SAMPLES_PER_FRAME_RMT) {
        __AXRmtCpuPtr = ptr;
    }

    __AXRmtDspPtr = ptr;
    __AXNextFrame(__AXOutSBuffer, __AXOutBuffer[__AXOutFrame], rmt);
    __AXOutFrame++;

    if (__AXOutputBufferMode == 1) {
        __AXOutFrame %= OUT_RING_MAX;
    } else {
        __AXOutFrame &= 1;
        AIInitDMA(__AXOutBuffer[__AXOutFrame], AX_FRAME_SIZE);
    }

    __AXLocalProfile.timeEnd = OSGetTime();
    __AXLocalProfile.numVoices = __AXGetNumVoices();

    current = __AXGetCurrentProfile();
    if (current != 0) {
        FUNNY_COPY(current, &__AXLocalProfile, u8, sizeof(AXPROFILE));
    }
}

void __AXOutAiCallback(void) {
    u32 frame;

    if (__AXOutDspReady == 0) {
        __AXOsTime = OSGetTime();
    }

    if (__AXOutDspReady == 1) {
        __AXOutDspReady = 0;
        __AXOutNewFrame(0);
    } else {
        __AXOutDspReady = 2;
        DSPAssertTask(&__AXDSPTask);
    }

    if (__AXOutputBufferMode == 1) {
        AIInitDMA(__AXOutBuffer[__AXAiDmaFrame], AX_FRAME_SIZE);

        __AXAiDmaFrame++;
        __AXAiDmaFrame %= OUT_RING_MAX;
    }
}

static void __AXDSPInitCallback(DSPTask* task) {
#pragma unused(task)

    __AXDSPInitFlag = true;
}

static void __AXDSPResumeCallback(DSPTask* task) {
    if ((u32)__AXOutDspReady == 2U) {
        __AXOutDspReady = 0U;
        __AXOutNewFrame((u32)(OSGetTime() - __AXOsTime) / 4);
        return;
    }
    __AXOutDspReady = 1U;
}

static void __AXDSPDoneCallback(DSPTask* task) {
#pragma unused(task)

    __AXDSPDoneFlag = true;
    OSWakeupThread(&__AXOutThreadQueue);
}

static void __AXDSPRequestCallback(DSPTask* task) {
#pragma unused(task)
}

void __AXOutInitDSP(void) {
    __AXDSPTask.iramMmemAddr = axDspSlave;
    __AXDSPTask.iramMmemLen = axDspSlaveLength;
    __AXDSPTask.iramDspAddr = 0;

    __AXDSPTask.dramMmemAddr = __AXDramImage;
    __AXDSPTask.dramMmemLen = ARRAY_COUNT(__AXDramImage);
    __AXDSPTask.dramDspAddr = DRAM_IMAGE_DSP_ADDR;

    __AXDSPTask.startVector = axDspInitVector;
    __AXDSPTask.resumeVector = axDspResumeVector;

    __AXDSPTask.initCallback = __AXDSPInitCallback;
    __AXDSPTask.resumeCallback = __AXDSPResumeCallback;
    __AXDSPTask.doneCallback = __AXDSPDoneCallback;
    __AXDSPTask.requestCallback = __AXDSPRequestCallback;

    __AXDSPTask.prio = 0;

    __AXDSPInitFlag = false;
    __AXDSPDoneFlag = false;

    OSInitThreadQueue(&__AXOutThreadQueue);

    if (!DSPCheckInit()) {
        DSPInit();
    }

    DSPAddTask(&__AXDSPTask);
    while (!__AXDSPInitFlag) {
        ;
    }
}

void __AXOutInit(u32 mode) {
    int i;
    u32* dst;
    u32* dst2;
    void* rmt[AX_RMT_MAX];

    __AXOutFrame = 0;
    __AXAiDmaFrame = 0;
    __AXOutputBufferMode = mode;
    __AXDebugSteppingMode = 0;

    for (dst = (u32*)__AXOutBuffer, i = 0; i < sizeof(__AXOutBuffer) / sizeof(u32); i++) {
        *dst++ = 0;
    }
    DCFlushRange(__AXOutBuffer, sizeof(__AXOutBuffer));

    for (dst2 = (u32*)__AXOutSBuffer, i = 0; i < sizeof(__AXOutSBuffer) / sizeof(u32); i++) {
        *dst2++ = 0;
    }
    DCFlushRange(__AXOutSBuffer, sizeof(__AXOutSBuffer));

    for (dst2 = (u32*)__AXRmtOutBuffer, i = 0; i < sizeof(__AXRmtOutBuffer) / sizeof(u32); i++) {
        *dst2++ = 0;
    }
    DCFlushRange(__AXRmtOutBuffer, sizeof(__AXRmtOutBuffer));

    __AXOutInitDSP();
    AIRegisterDMACallback(__AXOutAiCallback);

    rmt[0] = __AXRmtOutBuffer[0];
    rmt[1] = __AXRmtOutBuffer[1];
    rmt[2] = __AXRmtOutBuffer[2];
    rmt[3] = __AXRmtOutBuffer[3];

    __AXRmtCpuPtr = AX_SAMPLES_PER_FRAME_RMT;
    __AXRmtDspPtr = AX_SAMPLES_PER_FRAME_RMT;
    __AXRmtBuffLen = AX_SAMPLES_PER_FRAME_RMT * RMT_FRAME_MAX;

    if (__AXOutputBufferMode == 1) {
        __AXNextFrame(__AXOutSBuffer, __AXOutBuffer[2], rmt);
    } else {
        __AXNextFrame(__AXOutSBuffer, __AXOutBuffer[1], rmt);
    }

    __AXOutDspReady = 1;
    __AXUserFrameCallback = NULL;

    if (__AXOutputBufferMode == 1) {
        AIInitDMA(__AXOutBuffer[__AXAiDmaFrame], AX_FRAME_SIZE);
        __AXAiDmaFrame++;
        __AXAiDmaFrame &= 1;
    } else {
        AIInitDMA(__AXOutBuffer[__AXOutFrame], AX_FRAME_SIZE);
    }

    AIStartDMA();
}

void __AXOutQuit(void) {
    bool interrupts = OSDisableInterrupts();
    __AXUserFrameCallback = NULL;
    DSPCancelTask(&__AXDSPTask);
    OSSleepThread(&__AXOutThreadQueue);
    AIStopDMA();
    OSRestoreInterrupts(interrupts);
}

AXOutCallback AXRegisterCallback(AXOutCallback callback) {
    AXOutCallback old = __AXUserFrameCallback;
    bool enabled = OSDisableInterrupts();

    __AXUserFrameCallback = callback;

    OSRestoreInterrupts(enabled);
    return old;
}

s32 AXRmtGetSamplesLeft(void) {
    s32 samples = __AXRmtDspPtr - (AX_SAMPLES_PER_FRAME_RMT * 2);

    if (samples < 0) {
        samples += __AXRmtBuffLen;
    }

    samples -= __AXRmtCpuPtr;

    if (samples < 0) {
        samples += __AXRmtBuffLen;
    }

    return samples;
}

s32 AXRmtGetSamples(s32 chan, s16* out, s32 num) {
    s32 avail = AXRmtGetSamplesLeft();
    s32 actual = num > avail ? avail : num;
    s32 ptr = __AXRmtCpuPtr;
    int i;

    for (i = 0; i < actual; i++) {
        *out++ = __AXRmtOutBuffer[chan][ptr];

        if (++ptr >= __AXRmtBuffLen) {
            ptr = 0;
        }
    }

    DCInvalidateRange(__AXRmtOutBuffer[chan], __AXRmtBuffLen * sizeof(s16));
    return actual;
}

s32 AXRmtAdvancePtr(s32 num) {
    s32 avail = AXRmtGetSamplesLeft();
    s32 actual = num > avail ? avail : num;

    __AXRmtCpuPtr += actual;

    if (__AXRmtCpuPtr >= __AXRmtBuffLen) {
        __AXRmtCpuPtr -= __AXRmtBuffLen;
    }

    return actual;
}
