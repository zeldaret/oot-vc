#include "mem_funcs.h"
#include "revolution/base.h"
#include "revolution/gx.h"
#include "revolution/gx/GXRegs.h"
#include "revolution/os.h"

static GXFifoObjImpl CPUFifo;
static GXFifoObjImpl GPFifo;

static OSThread* __GXCurrentThread;
static bool GXOverflowSuspendInProgress;
static GXBreakPtCallback BreakPointCB;
volatile void* __GXCurrentBP;
static u32 __GXOverflowCount;
static GXBool GPFifoReady;
static GXBool CPGPLinked;

#define TOPHYSICAL(a) (((u32)a) & 0x3FFFFFFF)

void GXInitFifoPtrs(GXFifoObj* fifo, void* readPtr, void* writePtr);
void GXInitFifoLimits(GXFifoObj* fifo, u32 highWatermark, u32 lowWatermark);

static void __GXFifoLink(GXBool en);
static void __GXWriteFifoIntEnable(GXBool hi, GXBool lo);
static void __GXWriteFifoIntReset(GXBool highWatermarkClr, GXBool lowWatermarkClr);
static void __GXFifoReadEnable(void);
static void __GXFifoReadDisable(void);

static inline void GXOverflowHandler(void) {
    __GXOverflowCount += 1;
    __GXWriteFifoIntEnable(0, 1);
    __GXWriteFifoIntReset(1, 0);
    GXOverflowSuspendInProgress = true;
    OSSuspendThread(__GXCurrentThread);
}

static inline void GXUnderflowHandler(void) {
    OSResumeThread(__GXCurrentThread);
    GXOverflowSuspendInProgress = false;
    __GXWriteFifoIntReset(1, 1);
    __GXWriteFifoIntEnable(1, 0);
}

static inline void GXBreakPointHandler(OSContext* context) {
    OSContext bpContext;
    GX_SET_REG(gx->cpEnable, 0, 26, 26);
    GX_SET_CP_REG(1, gx->cpEnable);

    if (BreakPointCB) {
        OSClearContext(&bpContext);
        OSSetCurrentContext(&bpContext);
        BreakPointCB();
        OSClearContext(&bpContext);
        OSSetCurrentContext(context);
    }
}

void GXCPInterruptHandler(s16 p1, OSContext* context) {

    gx->cpStatus = GX_GET_CP_REG(0);

    if ((gx->cpEnable >> 3 & 1) && (gx->cpStatus >> 1 & 1)) {
        GXUnderflowHandler();
    }

    if ((gx->cpEnable >> 2 & 1) && (gx->cpStatus >> 0 & 1)) {
        GXOverflowHandler();
    }

    if ((gx->cpEnable >> 5 & 1) && (gx->cpStatus >> 4 & 1)) {
        GXBreakPointHandler(context);
    }
}

void GXInitFifoBase(GXFifoObj* fifo, void* base, u32 size) {
    GXFifoObjImpl* pFifo = (GXFifoObjImpl*)fifo;
    pFifo->base = base;
    pFifo->end = (void*)((u32)base + size - 4);
    pFifo->size = size;
    pFifo->rwDistance = 0;
    GXInitFifoLimits(fifo, size - 0x4000, OSRoundDown32B(size / 2));
    GXInitFifoPtrs(fifo, base, base);
}

void GXInitFifoPtrs(GXFifoObj* fifo, void* readPtr, void* writePtr) {
    GXFifoObjImpl* pFifo = (GXFifoObjImpl*)fifo;
    int interrupts = OSDisableInterrupts();
    pFifo->readPtr = readPtr;
    pFifo->writePtr = writePtr;
    pFifo->rwDistance = (u32)writePtr - (u32)readPtr;
    if (pFifo->rwDistance < 0) {
        pFifo->rwDistance += pFifo->size;
    }
    OSRestoreInterrupts(interrupts);
}

void GXInitFifoLimits(GXFifoObj* fifo, u32 highWatermark, u32 lowWatermark) {
    GXFifoObjImpl* pFifo = (GXFifoObjImpl*)fifo;
    pFifo->highWatermark = highWatermark;
    pFifo->lowWatermark = lowWatermark;
}

GXBool __GXIsGPFifoReady(void) { return GPFifoReady; }

GXBreakPtCallback GXSetBreakPtCallback(GXBreakPtCallback cb) {
    GXBreakPtCallback oldCallback = BreakPointCB;
    int interrupts = OSDisableInterrupts();
    BreakPointCB = cb;
    OSRestoreInterrupts(interrupts);
    return oldCallback;
}

void __GXFifoInit(void) {
    __OSSetInterruptHandler(OS_INTR_PI_CP, GXCPInterruptHandler);
    __OSUnmaskInterrupts(0x4000);
    __GXCurrentThread = OSGetCurrentThread();
    GXOverflowSuspendInProgress = 0;
    memset(&CPUFifo, 0, sizeof(GXFifoObjImpl));
    memset(&GPFifo, 0, sizeof(GXFifoObjImpl));
    CPGPLinked = false;
    GPFifoReady = false;
}

static void __GXFifoLink(GXBool en) {
    FAST_FLAG_SET(gx->cpEnable, (en ? 1 : 0), 4, 1);
    GX_CP_REG_WRITE_U16(1, (u16)gx->cpEnable);
}

static void __GXWriteFifoIntEnable(GXBool hi, GXBool lo) {
    FAST_FLAG_SET(gx->cpEnable, hi, 2, 1);
    FAST_FLAG_SET(gx->cpEnable, lo, 3, 1);
    GX_CP_REG_WRITE_U16(1, gx->cpEnable);
}

static void __GXWriteFifoIntReset(GXBool highWatermarkClr, GXBool lowWatermarkClr) {
    FAST_FLAG_SET(gx->cpClr, highWatermarkClr, 0, 1);
    FAST_FLAG_SET(gx->cpClr, lowWatermarkClr, 1, 1);
    GX_CP_REG_WRITE_U16(2, (u16)gx->cpClr);
}

static void __GXFifoReadEnable(void) {
    FAST_FLAG_SET(gx->cpEnable, 1, 0, 1);
    GX_CP_REG_WRITE_U16(1, (u16)gx->cpEnable);
}

static void __GXFifoReadDisable(void) {
    FAST_FLAG_SET(gx->cpEnable, 0, 0, 1);
    GX_CP_REG_WRITE_U16(1, (u16)gx->cpEnable);
}

void __GXCleanGPFifo(void) {
    bool enabled;

    if (!GPFifoReady) {
        return;
    }

    enabled = OSDisableInterrupts();
    __GXFifoReadDisable();
    __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);

    GPFifo.readPtr = GPFifo.writePtr;
    GPFifo.rwDistance = 0;

    GX_CP_REG_WRITE_U16(0x18, GPFifo.rwDistance);
    GX_CP_REG_WRITE_U16(0x1A, TOPHYSICAL(GPFifo.writePtr));
    GX_CP_REG_WRITE_U16(0x1C, TOPHYSICAL(GPFifo.readPtr));

    GX_CP_REG_WRITE_U16(0x19, GPFifo.rwDistance >> 16);
    GX_CP_REG_WRITE_U16(0x1B, TOPHYSICAL(GPFifo.writePtr) >> 16);
    GX_CP_REG_WRITE_U16(0x1D, TOPHYSICAL(GPFifo.readPtr) >> 16);

    PPCSync();

    if (CPGPLinked) {
        u32 reg = 0;
        CPUFifo.readPtr = GPFifo.readPtr;
        CPUFifo.writePtr = GPFifo.writePtr;
        CPUFifo.rwDistance = GPFifo.rwDistance;

        FAST_FLAG_SET(reg, (GX_PHY_ADDR(TOPHYSICAL(CPUFifo.writePtr)) >> 5), 5, 24);
        GX_PI_REG_WRITE_U32(0x14, reg);

        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    }

    FAST_FLAG_SET(gx->cpEnable, 0, 1, 1);
    FAST_FLAG_SET(gx->cpEnable, 0, 5, 1);
    GX_CP_REG_WRITE_U16(1, gx->cpEnable);
    __GXCurrentBP = 0;
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXFifoReadEnable();
    OSRestoreInterrupts(enabled);
}
