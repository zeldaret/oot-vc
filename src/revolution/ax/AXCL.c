#include "macros.h"
#include "revolution/ax.h"
#include "revolution/os.h"
#include "stdint.h"

// Compression parameters.
// Later versions of AX have these as static vars
#define THRESHOLD 32768
#define RELEASE_FRAMES 10

// Space is allocated for two commands lists
#define LIST_MAX 2

#define LIST_WRITE_16(x)             \
    do {                             \
        __AXWriteToCommandList((x)); \
    } while (0)

#define LIST_WRITE_32(x)                   \
    do {                                   \
        __AXWriteToCommandList((x) >> 16); \
        __AXWriteToCommandList((x));       \
    } while (0)

/**
 * Command list opcodes (from Dolphin Emulator)
 */
typedef enum {
    COMMAND_SETUP,
    COMMAND_ADD_TO_LR,
    COMMAND_SUB_TO_LR,
    COMMAND_ADD_SUB_TO_LR,
    COMMAND_PROCESS,
    COMMAND_5 = 5,
    COMMAND_MIX_AUXA = 6,
    COMMAND_MIX_AUXB = 7,
    COMMAND_MIX_AUXC = 8,
    COMMAND_UPL_AUXA_MIX_LRSC,
    COMMAND_UPL_AUXB_MIX_LRSC,
    COMMAND_COMPRESSOR = 11,
    COMMAND_OUTPUT = 12,
    COMMAND_OUTPUT_DPL2,
    COMMAND_WM_OUTPUT,
    COMMAND_END = 15,
};

static u32 __AXCommandListPosition;

static u16* __AXClWrite;
u32 __AXClMode;

static u32 __AXCommandListCycles;
static u8 __AXCommandList[AX_CL_SIZE * LIST_MAX * 6];

static bool __AXCompressor;

static volatile u16 __AXMasterVolume;
static u16 __AXAuxAVolume;
static u16 __AXAuxBVolume;

u32 __AXGetCommandListCycles(void) { return __AXCommandListCycles; }

void* __AXGetCommandListAddress(void) {
    void* list = __AXCommandList + __AXCommandListPosition * AX_CL_SIZE * 6;

    __AXCommandListPosition++;
    __AXCommandListPosition %= LIST_MAX;
    __AXClWrite = (u16*)(__AXCommandList + __AXCommandListPosition * AX_CL_SIZE * 6);

    return list;
}

static inline void __AXWriteToCommandList(u16 cmd) {
    *__AXClWrite = cmd;
    __AXClWrite++;
}

void __AXNextFrame(void* surround, void* lr, void* rmt) {
    void* imm;
    void* backup;

    __AXCommandListCycles = 7811;
    backup = __AXClWrite;
    imm = __AXGetStudio();

    LIST_WRITE_16(COMMAND_SETUP);
    LIST_WRITE_32((uintptr_t)imm);
    __AXCommandListCycles += 4246;

    switch (__AXClMode) {
        case AX_OUTPUT_STEREO:
            LIST_WRITE_16(COMMAND_ADD_TO_LR);
            LIST_WRITE_32((uintptr_t)surround);
            __AXCommandListCycles += 733;
            break;
        case AX_OUTPUT_SURROUND:
            LIST_WRITE_16(COMMAND_SUB_TO_LR);
            LIST_WRITE_32((uintptr_t)surround);
            __AXCommandListCycles += 829;
            break;
        case AX_OUTPUT_DPL2:
            LIST_WRITE_16(COMMAND_ADD_SUB_TO_LR);
            LIST_WRITE_32((uintptr_t)surround);
            __AXCommandListCycles += 925;
            break;
    }

    imm = __AXGetPBs();
    LIST_WRITE_16(COMMAND_PROCESS);
    LIST_WRITE_32((uintptr_t)imm);
    LIST_WRITE_16(COMMAND_5);

    if (__AXClMode == AX_OUTPUT_DPL2) {
        __AXGetAuxAInput(&imm);
        if (imm != NULL) {
            LIST_WRITE_16(COMMAND_UPL_AUXA_MIX_LRSC);
            LIST_WRITE_16(__AXMasterVolume);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxAInputDpl2(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxAOutput(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxAOutputDpl2R(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxAOutputDpl2Ls(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxAOutputDpl2Rs(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXCommandListCycles += 1747;
        }

        __AXGetAuxBInput(&imm);
        if (imm != NULL) {
            LIST_WRITE_16(COMMAND_UPL_AUXB_MIX_LRSC);
            LIST_WRITE_16(__AXAuxAVolume);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxBInputDpl2(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxBOutput(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxBOutputDpl2R(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxBOutputDpl2Ls(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxBOutputDpl2Rs(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXCommandListCycles += 1747;
        }
    } else {
        __AXGetAuxAInput(&imm);
        if (imm != NULL) {
            LIST_WRITE_16(COMMAND_MIX_AUXA);
            LIST_WRITE_16(__AXMasterVolume);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxAOutput(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXCommandListCycles += 1249;
        }

        __AXGetAuxBInput(&imm);
        if (imm != NULL) {
            LIST_WRITE_16(COMMAND_MIX_AUXB);
            LIST_WRITE_16(__AXAuxAVolume);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxBOutput(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXCommandListCycles += 1249;
        }

        __AXGetAuxCInput(&imm);
        if (imm != NULL) {
            LIST_WRITE_16(COMMAND_MIX_AUXC);
            LIST_WRITE_16(__AXAuxBVolume);
            LIST_WRITE_32((uintptr_t)imm);

            __AXGetAuxCOutput(&imm);
            LIST_WRITE_32((uintptr_t)imm);

            __AXCommandListCycles += 1249;
        }
    }

    if (__AXCompressor) {
        LIST_WRITE_16(COMMAND_COMPRESSOR);
        LIST_WRITE_16(THRESHOLD);
        LIST_WRITE_16(RELEASE_FRAMES);
        LIST_WRITE_32((uintptr_t)__AXCompressorTable);
        __AXCommandListCycles += 1850;
    }

    LIST_WRITE_16(COMMAND_WM_OUTPUT);
    LIST_WRITE_32(((u32*)rmt)[0]);
    LIST_WRITE_32(((u32*)rmt)[1]);
    LIST_WRITE_32(((u32*)rmt)[2]);
    LIST_WRITE_32(((u32*)rmt)[3]);
    __AXCommandListCycles += 409;

    if (__AXClMode == AX_OUTPUT_DPL2) {
        LIST_WRITE_16(COMMAND_OUTPUT_DPL2);
        LIST_WRITE_32((uintptr_t)surround);
        LIST_WRITE_32((uintptr_t)lr);
        __AXCommandListCycles += 687;
    } else {
        LIST_WRITE_16(COMMAND_OUTPUT);
        LIST_WRITE_32((uintptr_t)surround);
        LIST_WRITE_32((uintptr_t)lr);
        __AXCommandListCycles += 665;
    }

    LIST_WRITE_16(COMMAND_END);
    __AXCommandListCycles += 30;

    DCFlushRange(backup, AX_CL_SIZE * 6);
}

void __AXClInit(void) {
    __AXClMode = AX_OUTPUT_STEREO;
    __AXCommandListPosition = 0;
    __AXClWrite = (u16*)__AXCommandList;
    __AXCompressor = true;

    __AXAuxBVolume = __AXAuxAVolume = __AXMasterVolume = AX_MAX_VOLUME;
}

void __AXClQuit(void) {}

void AXSetMode(u32 mode) { __AXClMode = mode; }

u16 AXGetMasterVolume(void) { return __AXMasterVolume; }

u16 AXGetAuxAReturnVolume(void) { return __AXAuxAVolume; }

u16 AXGetAuxBReturnVolume(void) { return __AXAuxBVolume; }

void AXSetMasterVolume(u16 volume) { __AXMasterVolume = volume; }

void AXSetAuxAReturnVolume(u16 volume) { __AXAuxAVolume = volume; }

void AXSetAuxBReturnVolume(u16 volume) { __AXAuxBVolume = volume; }
