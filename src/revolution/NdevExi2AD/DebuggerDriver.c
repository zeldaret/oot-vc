#include "revolution/ndevexi2ad.h"
#include "revolution/os.h"

static u32 __DBRecvDataSize;
static u32 __DBRecvMail;
static u8 __DBEXIInputFlag;
static OSInterruptHandler __DBDbgCallback;
static OSInterruptHandler __DBMtrCallback;

static u8 __DBReadUSB_CSR(void);
static void __DBWaitForSendMail(void);

void __DBMtrHandler(__OSInterrupt type, OSContext* ctx) {
    __DBEXIInputFlag = true;

    if (__DBMtrCallback != NULL)
        __DBMtrCallback(0, ctx);
}

void __DBIntrHandler(__OSInterrupt type, OSContext* ctx) {
    PI_HW_REGS[PI_INTSR] = PI_INTSR_DEBUG;
    if (__DBDbgCallback != NULL)
        __DBDbgCallback(type, ctx);
}

static void __DBCheckMailBox(void) {
    u8 csr = __DBReadUSB_CSR();
    if (!(csr & 0x8)) {
        u32 mail;
        __DBReadMailbox(&mail);
        if (ODEMUIsValidMail(mail)) {
            __DBRecvMail = mail;
            __DBRecvDataSize = ODEMUGetSize(mail);
            __DBEXIInputFlag = true;
        }
    }
}

static u8 __DBReadUSB_CSR(void) {
    u8 val;
    __DBEXIReadReg(0x34000000, &val, sizeof(val));
    return val;
}

void DBInitComm(u8** flagOut, OSInterruptHandler handler) {
    bool enabled = OSDisableInterrupts();

    *flagOut = &__DBEXIInputFlag;
    __DBMtrCallback = handler;
    __DBEXIInit();

    OSRestoreInterrupts(enabled);
}

// this function is weird, using a Wii compiler
// makes it match but it breaks the other functions
// https://decomp.me/scratch/rRVNR
#ifndef NON_MATCHING
ASM void DBInitInterrupts(void) {
#ifdef __MWERKS__ // clang-format off
    stwu r1, -0x10(r1)
    mflr r0
    lis r3, 2
    stw r0, 0x14(r1)
    addi r3, r3, -0x8000
    bl __OSMaskInterrupts
    li r3, 0x40
    bl __OSMaskInterrupts
    lis r3, __DBMtrHandler@ha
    lis r4, __DBIntrHandler@ha
    addi r3, r3, __DBMtrHandler@l
    stw r3, __DBDbgCallback
    addi r4, r4, __DBIntrHandler@l
    li r3, 0x19
    bl __OSSetInterruptHandler
    li r3, 0x40
    bl __OSUnmaskInterrupts
    lwz r0, 0x14(r1)
    mtlr r0
    addi r1, r1, 0x10
    blr
#endif // clang-format on
}
#else
void DBInitInterrupts(void) {
    __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_2_EXI) | OS_INTR_MASK(OS_INTR_EXI_2_TC));
    __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_PI_DEBUG));
    __DBDbgCallback = __DBMtrHandler;
    __OSSetInterruptHandler(OS_INTR_PI_DEBUG, __DBIntrHandler);
    __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_PI_DEBUG));
}
#endif

u32 DBQueryData(void) {
    __DBEXIInputFlag = false;

    if (__DBRecvDataSize == 0) {
        bool enabled = OSDisableInterrupts();
        __DBCheckMailBox();
        OSRestoreInterrupts(enabled);
    }

    return __DBRecvDataSize;
}

bool DBRead(void* dst, u32 size) {
    bool enabled = OSDisableInterrupts();

    __DBRead(ODEMUGetPc2NngcOffset(__DBRecvMail) + 0x1000, dst, ROUND_UP(size, 4));
    __DBRecvDataSize = 0;
    __DBEXIInputFlag = false;

    OSRestoreInterrupts(enabled);

    return false;
}

bool DBWrite(const void* src, u32 size) {
    static u8 l_byOffsetCounter = 128;

    bool enabled = OSDisableInterrupts();

    u32 ofs, mail;

    __DBWaitForSendMail();

    ofs = 0;
    if (!(++l_byOffsetCounter & 0x1)) {
        ofs = 0;
    } else {
        ofs = 0x800;
    }

    while (__DBWrite(ofs, src, ROUND_UP(size, 4)) == 0) {}

    __DBWaitForSendMail();

    mail = ODEMUGenMailData(l_byOffsetCounter, size);
    while (__DBWriteMailbox(mail) == 0) {}

    __DBWaitForSendMail();

    OSRestoreInterrupts(enabled);

    return false;
}

static void __DBWaitForSendMail(void) {
    while (__DBReadUSB_CSR() & 0x4) {}
}

void DBOpen(void) {}

void DBClose(void) {}
