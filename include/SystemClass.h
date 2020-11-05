#ifndef _SYSTEM_CLASS_H
#define _SYSTEM_CLASS_H

#include "CPUClass.h"
#include "RAMClass.h"

typedef struct {
    /* 0x0000 */ char unk_0x00[0x10];
    /* 0x0010 */ cpu_class_t *cpu;
    /* 0x0014 */ char unk_0x14[4];
    /* 0x0018 */ ram_class_t *ram;
} system_class_t;

extern system_class_t *gSystem;

#endif
