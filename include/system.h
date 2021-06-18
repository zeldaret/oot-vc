#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "cpu.h"
#include "ram.h"
#include "rom.h"

typedef struct {
    /* 0x0000 */ s32 unk_0x00;
    /* 0x0004 */ char unk_0x04[0x8];
    /* 0x000C */ rom_id_t rom_id;
    /* 0x0010 */ cpu_class_t* cpu;
    /* 0x0014 */ char unk_0x14[4];
    /* 0x0018 */ ram_class_t* ram;
    /* 0x001C */ rom_class_t* rom;
    /* 0x0020 */ void *rsp;
    /* 0x0024 */ char unk_0x24[0x10];
    /* 0x0034 */ void *video;
    /* 0x0038 */ char unk_0x38[0x1C];
    /* 0x0054 */ void* help_menu;
    /* 0x0058 */ void *unk_0x0058;
    /* 0x005C */ u32 *frame;
} system_class_t;

extern system_class_t* gSystem;

s32 systemCheckInterrupts(system_class_t *sys);

#endif
