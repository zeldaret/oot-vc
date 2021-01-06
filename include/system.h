#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "cpu.h"
#include "ram.h"

typedef struct {
    /* 0x0000 */ char unk_0x00[0x10];
    /* 0x0010 */ cpu_class_t* cpu;
    /* 0x0014 */ char unk_0x14[4];
    /* 0x0018 */ ram_class_t* ram;
    /* 0x001C */ char unk_0x1C[0x38];
    /* 0x0054 */ void* help_menu;
} system_class_t;

extern system_class_t* gSystem;

#endif
