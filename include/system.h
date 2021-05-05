#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "cpu.h"
#include "ram.h"
#include "rom.h"

typedef struct {
    /* 0x0000 */ char unk_0x00[0x10];
    /* 0x0010 */ cpu_class_t* cpu;
    /* 0x0014 */ char unk_0x14[4];
    /* 0x0018 */ ram_class_t* ram;
    /* 0x001C */ rom_class_t* rom;
    /* 0x0020 */ void *rsp;
    /* 0x0024 */ char unk_0x24[0x30];
    /* 0x0054 */ void* help_menu;
} system_class_t;

extern system_class_t* gSystem;

#endif
