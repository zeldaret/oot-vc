#ifndef _CPU_CLASS_H
#define _CPU_CLASS_H

#include "tree.h"

typedef s32 (*lb_func_t)(void *obj, u32 addr, s8 *dst);
typedef s32 (*lh_func_t)(void *obj, u32 addr, s16 *dst);
typedef s32 (*lw_func_t)(void *obj, u32 addr, s32 *dst);
typedef s32 (*ld_func_t)(void *obj, u32 addr, s64 *dst);
typedef s32 (*sb_func_t)(void *obj, u32 addr, s8 *src);
typedef s32 (*sh_func_t)(void *obj, u32 addr, s16 *src);
typedef s32 (*sw_func_t)(void *obj, u32 addr, s32 *src);
typedef s32 (*sd_func_t)(void *obj, u32 addr, s64 *src);

typedef struct {
    /* 0x0000 */ char unk_00[0xC];
    /* 0x000C */ lb_func_t lb;
    /* 0x0010 */ lh_func_t lh;
    /* 0x0014 */ lw_func_t lw;
    /* 0x0018 */ ld_func_t ld;
    /* 0x001C */ sb_func_t sb;
    /* 0x0020 */ sh_func_t sh;
    /* 0x0024 */ sw_func_t sw;
    /* 0x0028 */ sd_func_t sd;
    /* 0x002C */ char unk_0x2C[0x14];
} cpu_dev_t; // size = 0x40

typedef struct {
    /* 0x00000 */ char unk_0x00[0x34];
    /* 0x00034 */ s32 unk_0x34; // timer?
    /* 0x00038 */ char unk_0x38[0xB28];
    /* 0x00B60 */ cpu_dev_t *devices[0x100];
    /* 0x00F60 */ u8 mem_hi_map[0x10000];
    /* 0x10F60 */ char unk_0x10F60[0x534];
    /* 0x11494 */ recomp_tree_t *recomp_tree;
    /* 0x11498 */ char unk_0x11498[0xE40];
} cpu_class_t; // size = 0x122D8

s32 cpuSetGetBlock(cpu_class_t *cpu, void *arg1, void *arg2);
s32 cpuSetDevicePut(cpu_class_t *cpu, void *arg1, sb_func_t sb, sh_func_t sh, sw_func_t sw, sd_func_t sd);
s32 cpuSetDeviceGet(cpu_class_t *cpu, void *arg1, lb_func_t lb, lh_func_t lh, lw_func_t lw, ld_func_t ld);

#endif
