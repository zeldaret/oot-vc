#ifndef _CPU_CLASS_H
#define _CPU_CLASS_H

#include "tree.h"

typedef struct {
    /* 0x00000 */ char unk_0x00[0x34];
    /* 0x00034 */ s32 unk_0x34; // timer?
    /* 0x00038 */ char unk_0x38[0x1145C];
    /* 0x11494 */ recomp_tree_t *recomp_tree;
    /* 0x11498 */ char unk_0x11498[0xE40];
} cpu_class_t; // size = 0x122D8

#endif