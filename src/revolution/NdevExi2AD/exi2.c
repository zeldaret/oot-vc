#include "revolution/ndevexi2ad/exi2.h"
#include "revolution/exi.h"
#include "revolution/os.h"

// In regular NdevExi2, EXI_CHAN_PARAMS instead points to 0xCC006800 (GC address?).

static bool __EXI2Select(void) {
    u32 temp = EXI_CHAN_PARAMS[EXI_CHAN_2].cpr;
    EXI_CHAN_PARAMS[EXI_CHAN_2].cpr = ((temp & 0x405) | 0xC0);
    return true;
}

static bool __EXI2Deselect(void) {
    u32 temp = EXI_CHAN_PARAMS[EXI_CHAN_2].cpr;
    EXI_CHAN_PARAMS[EXI_CHAN_2].cpr = temp & 0x405;
    return true;
}

static bool __EXI2Sync(void) {
    while (EXI_CHAN_PARAMS[EXI_CHAN_2].cr & 0x1) {}
    return true;
}

bool __EXI2Imm(void* mem, s32 size, u32 type) {
    int i;

    if (type != EXI_READ) {
        u32 imm = 0;

        for (i = 0; i < size; i++) {
            const u8* bmem = (const u8*)mem;
            imm |= bmem[i] << (3 - i) * 8;
        }

        EXI_CHAN_PARAMS[EXI_CHAN_2].data = imm;
    }

    EXI_CHAN_PARAMS[EXI_CHAN_2].cr = type << 2 | 1 | (size - 1) * 16;
    __EXI2Sync();

    if (type == EXI_READ) {
        u32 imm = EXI_CHAN_PARAMS[EXI_CHAN_2].data;

        u8* bmem = (u8*)mem;
        for (i = 0; i < size; i++, bmem++) {
            *bmem = imm >> (3 - i) * 8;
        }
    }

    return true;
}

void __DBEXIInit(void) {
    u32 val0, val1;

    __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_2_EXI) | OS_INTR_MASK(OS_INTR_EXI_2_TC));
    while ((EXI_CHAN_PARAMS[EXI_CHAN_2].cr & 1) == 1U) {}
    EXI_CHAN_PARAMS[EXI_CHAN_2].cpr = 0;

    val0 = 0xB4000000;
    val1 = 0xD4000000;

    __EXI2Select();
    __EXI2Imm(&val0, 4, EXI_WRITE);
    __EXI2Sync();

    __EXI2Imm(&val1, 4, EXI_WRITE);
    __EXI2Sync();
    __EXI2Deselect();
}

bool __DBEXIReadReg(u32 cmd, void* mem, s32 size) {
    bool error = false;
    u32 read_val = 0;

    error = error | !__EXI2Select();
    error = error | !__EXI2Imm(&cmd, sizeof(cmd), EXI_WRITE);
    error = error | !__EXI2Sync();
    error = error | !__EXI2Imm(&read_val, sizeof(read_val), EXI_READ);
    error = error | !__EXI2Sync();
    error = error | !__EXI2Deselect();

    switch (size) {
        case 1:
            *(u8*)mem = read_val >> 24;
            break;
        case 2:
            *(u16*)mem = read_val >> 24 | read_val >> 8 & 0xFF00;
            break;
        default:
            *(u32*)mem = __EXISwap32(read_val);
            break;
    }

    return error == false;
}

bool __DBEXIWriteReg(u32 cmd, const void* mem, s32 size) {
    bool error = false;
    u32 write_val = 0;

    switch (size) {
        case 1:
            write_val = *(u8*)mem << 24;
            break;
        case 2:
            write_val = *(u16*)mem << 24 | (*(u16*)mem & 0xFF00) << 8;
            break;
        default:
            write_val = __EXISwap32(*(u32*)mem);
            break;
    }

    error = error | !__EXI2Select();
    error = error | !__EXI2Imm(&cmd, sizeof(cmd), EXI_WRITE);
    error = error | !__EXI2Sync();
    error = error | !__EXI2Imm(&write_val, sizeof(write_val), EXI_WRITE);
    error = error | !__EXI2Sync();
    error = error | !__EXI2Deselect();

    return error == false;
}

bool __DBEXIReadRam(u32 cmd, void* mem, s32 size) {
    bool error = false;
    u32 read_val;
    u32* lmem = (u32*)mem;

    error = error | !__EXI2Select();
    error = error | !__EXI2Imm(&cmd, sizeof(cmd), EXI_WRITE);
    error = error | !__EXI2Sync();

    for (; size > 0; size -= 4) {
        error = error | !__EXI2Imm(&read_val, sizeof(read_val), EXI_READ);
        error = error | !__EXI2Sync();
        *lmem++ = read_val;
    }

    error = error | !__EXI2Deselect();

    return error == false;
}

bool __DBEXIWriteRam(u32 cmd, const void* mem, s32 size) {
    bool error = false;
    u32* lmem = (u32*)mem;

    error = error | !__EXI2Select();
    error = error | !__EXI2Imm(&cmd, sizeof(cmd), EXI_WRITE);
    error = error | !__EXI2Sync();

    for (; size > 0; size -= 4) {
        u32 write_val = *lmem++;
        error = error | !__EXI2Imm(&write_val, sizeof(write_val), EXI_WRITE);
        error = error | !__EXI2Sync();
    }

    error = error | !__EXI2Deselect();

    return error == false;
}
