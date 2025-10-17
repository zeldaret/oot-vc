#include "mem_funcs.h"
#include "revolution/base.h"
#include "revolution/gx.h"
#include "revolution/gx/GXRegs.h"
#include "revolution/os.h"

static GXFifoObjImpl CPUFifo;
static GXFifoObjImpl GPFifo;

static GXBool CPGPLinked;
static OSThread* __GXCurrentThread;
static bool GXOverflowSuspendInProgress;
static GXBreakPtCallback BreakPointCB;
volatile void* __GXCurrentBP;
static u32 __GXOverflowCount;
static GXBool GPFifoReady;
static GXBool CPUFifoReady;

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
    __GXWriteFifoIntEnable(GX_FALSE, GX_TRUE);
    __GXWriteFifoIntReset(GX_TRUE, GX_FALSE);
    GXOverflowSuspendInProgress = true;
    OSSuspendThread(__GXCurrentThread);
}

static inline void GXUnderflowHandler(void) {
    OSResumeThread(__GXCurrentThread);
    GXOverflowSuspendInProgress = false;
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
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

GXBool fn_8009E5AC(void) {
    s32 temp_r3;
    s32 temp_r6;
    bool var_r0;
    u32 var_r7 = 0;

    if (!CPUFifoReady || !GPFifoReady) {
        return GX_FALSE;
    }

    if (CPUFifo.base == GPFifo.base) {
        var_r7++;
    }
    if (CPUFifo.end == GPFifo.end) {
        var_r7++;
    }
    if (var_r7 == 2) {
        return GX_TRUE;
    }

    temp_r3 = (s32)CPUFifo.end - (s32)GPFifo.base;
    temp_r6 = (s32)GPFifo.end - (s32)CPUFifo.base;
    var_r0 = (temp_r3 > 0 && temp_r6 > 0) || (temp_r3 < 0 && temp_r6 < 0);
    if (var_r0) {
        OSReport("CPUFifo: %08X - %08X\n", (u32)CPUFifo.base, (u32)CPUFifo.end);
        OSReport("GP Fifo: %08X - %08X\n", (u32)GPFifo.base, (u32)GPFifo.end);
    }
    return GX_FALSE;
}

void GXSetCPUFifo(GXFifoObj* fifo) {
    bool interrupts = OSDisableInterrupts();

    if (fifo == NULL) {
        CPUFifoReady = GX_FALSE;
        CPGPLinked = GX_FALSE;
        CPUFifo.bindGP = GX_FALSE;
        CPUFifo.bindCPU = GX_FALSE;
        OSRestoreInterrupts(interrupts);
        return;
    }

    CPUFifo = *(GXFifoObjImpl*)fifo;
    CPUFifoReady = GX_TRUE;
    CPUFifo.bindCPU = GX_TRUE;

    if (fn_8009E5AC()) {
        u32 reg;

        CPGPLinked = GX_TRUE;
        CPUFifo.bindGP = GX_TRUE;

        GX_SET_PI_REG(3, TOPHYSICAL(CPUFifo.base));
        GX_SET_PI_REG(4, TOPHYSICAL(CPUFifo.end));
        reg = 0;
        GX_SET_REG(reg, TOPHYSICAL(CPUFifo.writePtr) >> 5, 3, 26);
        GX_SET_PI_REG(5, reg);

        __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    } else {
        u32 reg;

        CPUFifo.bindGP = GX_FALSE;

        if (CPGPLinked) {
            __GXFifoLink(GX_FALSE);
            CPGPLinked = GX_FALSE;
        }
        __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);

        GX_SET_PI_REG(3, TOPHYSICAL(CPUFifo.base));
        GX_SET_PI_REG(4, TOPHYSICAL(CPUFifo.end));
        reg = 0;
        GX_SET_REG(reg, TOPHYSICAL(CPUFifo.writePtr) >> 5, 3, 26);
        GX_SET_PI_REG(5, reg);
    }

    PPCSync();

    OSRestoreInterrupts(interrupts);
}

void GXSetGPFifo(GXFifoObj* fifo) {
    int interrupts = OSDisableInterrupts();
    u32 reg;

    __GXFifoReadDisable();
    __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);

    if (fifo == NULL) {
        GPFifoReady = GX_FALSE;
        CPGPLinked = GX_FALSE;
        GX_SET_REG(gx->cpEnable, 0, 27, 27);
        GX_SET_CP_REG(1, gx->cpEnable);
        GPFifo.bindCPU = GX_FALSE;
        GPFifo.bindGP = GX_FALSE;
        OSRestoreInterrupts(interrupts);
        return;
    }

    GPFifo = *(GXFifoObjImpl*)fifo;
    GPFifoReady = GX_TRUE;
    GPFifo.bindGP = GX_TRUE;

    GX_SET_CP_REG(16, (u16)GPFifo.base);
    GX_SET_CP_REG(18, (u16)GPFifo.end);
    GX_SET_CP_REG(24, (u16)GPFifo.rwDistance);
    GX_SET_CP_REG(26, (u16)GPFifo.writePtr);
    GX_SET_CP_REG(28, (u16)GPFifo.readPtr);
    GX_SET_CP_REG(20, (u16)GPFifo.highWatermark);
    GX_SET_CP_REG(22, (u16)GPFifo.lowWatermark);

    GX_SET_CP_REG(17, TOPHYSICAL(GPFifo.base) >> 16);
    GX_SET_CP_REG(19, TOPHYSICAL(GPFifo.end) >> 16);
    GX_SET_CP_REG(25, GPFifo.rwDistance >> 16);
    GX_SET_CP_REG(27, TOPHYSICAL(GPFifo.writePtr) >> 16);
    GX_SET_CP_REG(29, TOPHYSICAL(GPFifo.readPtr) >> 16);
    GX_SET_CP_REG(21, (GPFifo.highWatermark) >> 16);
    GX_SET_CP_REG(23, (GPFifo.lowWatermark) >> 16);

    PPCSync();

    if (fn_8009E5AC()) {
        CPGPLinked = GX_TRUE;
        GPFifo.bindCPU = GX_TRUE;
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    } else {
        CPGPLinked = GX_FALSE;
        GPFifo.bindCPU = GX_FALSE;
        __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);
        __GXFifoLink(GX_FALSE);
    }

    reg = gx->cpEnable;
    GX_SET_REG(reg, 0, 30, 30);
    GX_SET_REG(reg, 0, 26, 26);
    GX_SET_CP_REG(1, reg);

    reg = gx->cpEnable;
    GX_SET_CP_REG(1, reg);

    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXFifoReadEnable();
    OSRestoreInterrupts(interrupts);
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
    CPUFifoReady = GX_FALSE;
    GPFifoReady = GX_FALSE;
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
