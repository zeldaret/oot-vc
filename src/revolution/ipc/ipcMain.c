#include "revolution/ipc.h"
#include "revolution/os.h"

static u8 Initialized = false;
static void* IPCCurrentBufferLo = NULL;
static void* IPCCurrentBufferHi = NULL;
static void* IPCBufferLo = NULL;
static void* IPCBufferHi = NULL;

void IPCInit(void) {
    if (!Initialized) {
        IPCBufferHi = __OSGetIPCBufferHi();
        IPCBufferLo = __OSGetIPCBufferLo();
        IPCCurrentBufferHi = IPCBufferHi;
        IPCCurrentBufferLo = IPCBufferLo;
        Initialized = true;
    }
}

u32 IPCReadReg(s32 index) { return IPC_HW_REGS[index]; }

void IPCWriteReg(s32 index, u32 value) { IPC_HW_REGS[index] = value; }

void* IPCGetBufferHi(void) { return IPCCurrentBufferHi; }

void* IPCGetBufferLo(void) { return IPCCurrentBufferLo; }

void IPCSetBufferLo(void* lo) { IPCCurrentBufferLo = lo; }
