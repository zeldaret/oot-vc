#ifndef _TREE_H
#define _TREE_H

#include "types.h"

typedef struct recomp_node_s recomp_node_t;

typedef void* (*recomp_func_t)();

typedef struct {
    /* 0x0000 */ s32 unk_0x00;
    /* 0x0004 */ void *unk_0x04;
} unk_node_0x18; // size = 0x8

typedef struct {
    s32 unk_0x00;
    s32 unk_0x04;
} unk_node_0x0C;

struct recomp_node_s{
    /* 0x0000 */ u32 unk_0x00;
    /* 0x0004 */ recomp_func_t* recompiled_func;
    /* 0x0008 */ s32 unk_0x08;
    /* 0x000C */ unk_node_0x0C* unk_0x0C;
    /* 0x0010 */ s32 n64_start_addr;
    /* 0x0014 */ s32 n64_end_addr;
    /* 0x0018 */ unk_node_0x18* unk_0x18;
    /* 0x001C */ s32 unk_0x1C;
    /* 0x0020 */ u32 state;
    /* 0x0024 */ u32 checksum;
    /* 0x0028 */ s32 unk_0x28;
    /* 0x002C */ u32 size;
    /* 0x0030 */ s32 alloc_type;
    /* 0x0034 */ s32 unk_0x34;
    /* 0x0038 */ u32 unk_0x38;
    /* 0x003C */ recomp_node_t* parent;
    /* 0x0040 */ recomp_node_t* left;
    /* 0x0044 */ recomp_node_t* right;
}; // size = 0x48

typedef struct {
    /* 0x0000 */ u16 node_cnt;
    /* 0x0004 */ u32 total_size;
    /* 0x0008 */ s32 code_boundary;
    /* 0x000C */ s32 n64_start;
    /* 0x0010 */ s32 n64_end;
    /* 0x0014 */ char unk_0x14[0x20];
    /* 0x0034 */ u32 unk_0x34;
    /* 0x0038 */ char unk_0x38[0x30];
    /* 0x0068 */ recomp_node_t* code_root;
    /* 0x006C */ recomp_node_t* ovl_root;
    /* 0x0070 */ s32 unk_0x70;
    /* 0x0074 */ u32 unk_0x74;
    /* 0x0078 */ u32 unk_0x78;
    /* 0x007C */ recomp_node_t* unk_0x7C;
    /* 0x0080 */ u32 unk_0x80;
} recomp_tree_t; // size = 0x84;

int treeKill(void *cpu);

#endif
