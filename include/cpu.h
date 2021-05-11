#ifndef _CPU_H
#define _CPU_H

#include "tree.h"

typedef s32 (*lb_func_t)(void* obj, s32 addr, s8* dst);
typedef s32 (*lh_func_t)(void* obj, s32 addr, s16* dst);
typedef s32 (*lw_func_t)(void* obj, s32 addr, s32* dst);
typedef s32 (*ld_func_t)(void* obj, s32 addr, s64* dst);
typedef s32 (*sb_func_t)(void* obj, s32 addr, s8* src);
typedef s32 (*sh_func_t)(void* obj, s32 addr, s16* src);
typedef s32 (*sw_func_t)(void* obj, s32 addr, s32* src);
typedef s32 (*sd_func_t)(void* obj, s32 addr, s64* src);

typedef struct {
    /* 0x0000 */ u32 unk_0;
    /* 0x0004 */ void *unk_4;
    /* 0x0008 */ s32 unk_8;
    /* 0x000C */ lb_func_t lb;
    /* 0x0010 */ lh_func_t lh;
    /* 0x0014 */ lw_func_t lw;
    /* 0x0018 */ ld_func_t ld;
    /* 0x001C */ sb_func_t sb;
    /* 0x0020 */ sh_func_t sh;
    /* 0x0024 */ sw_func_t sw;
    /* 0x0028 */ sd_func_t sd;
    /* 0x002C */ void *unk_0x2C;
    /* 0x0030 */ char unk_0x30[0x10];
} cpu_dev_t; // size = 0x40

#define SM_BLK_CNT 192
#define LG_BLK_CNT 13
#define TREE_BLK_CNT 125

typedef union {
    u64 d;
    s64 sd;
    u32 w[2];
    s32 sw[2];
    f32 f[2];
    f64 fd;
} reg64_t;

typedef struct {
    reg64_t unk_0x00;
    reg64_t unk_0x08;
    reg64_t unk_0x10;
    reg64_t unk_0x18;
    reg64_t unk_0x20;
} unk_cpu_0x248;

typedef void *(*cpu_execute_func_t)();

typedef struct {
    u32 addr;
    u32 expected;
    u32 replacement;
} cpu_hack_t;

typedef struct {
    /* 0x00000 */ s32 unk_0x00;
    /* 0x00004 */ s32 unk_0x04;
    /* 0x00008 */ reg64_t lo;
    /* 0x00010 */ reg64_t hi;
    /* 0x00018 */ s32 unk_0x18;
    /* 0x0001C */ s32 unk_0x1C;
    /* 0x00020 */ u32 pc;
    /* 0x00024 */ s32 unk_0x24;
    /* 0x00028 */ s32 unk_0x28;
    /* 0x0002C */ recomp_node_t *running_node;
    /* 0x00030 */ s32 unk_0x30;
    /* 0x00034 */ s32 unk_0x34; // timer?
    /* 0x00038 */ s32 unk_0x38;
    /* 0x0003C */ s32 unk_0x3C;
    /* 0x00040 */ char unk_0x40[0x8];
    /* 0x00048 */ reg64_t gpr[32];
    /* 0x00148 */ reg64_t fpr[32];
    /* 0x00248 */ unk_cpu_0x248 unk_0x248[0x30];
    /* 0x009C8 */ u32 fscr[32];
    /* 0x00A48 */ reg64_t cp0[32];
    /* 0x00B48 */ cpu_execute_func_t execute_opcode;
    /* 0x00B4C */ cpu_execute_func_t execute_jump;
    /* 0x00B50 */ cpu_execute_func_t execute_call;
    /* 0x00B54 */ cpu_execute_func_t execute_idle;
    /* 0x00B58 */ cpu_execute_func_t execute_loadstore;
    /* 0x00B5C */ cpu_execute_func_t execute_loadstoref;
    /* 0x00B60 */ cpu_dev_t* devices[0x100];
    /* 0x00F60 */ u8 mem_hi_map[0x10000];
    /* 0x10F60 */ void *sm_blk_code;
    /* 0x10F64 */ void *lg_blk_code;
    /* 0x10F68 */ u32 sm_blk_status[SM_BLK_CNT];
    /* 0x11268 */ u32 lg_blk_status[LG_BLK_CNT];
    /* 0x1129C */ recomp_node_t *recomp_tree_nodes;
    /* 0x112A0 */ u32 tree_blk_status[TREE_BLK_CNT];
    /* 0x11494 */ recomp_tree_t* recomp_tree;
    /* 0x11498 */ char unk_0x11498[0xC00];
    /* 0x12098 */ s32 hack_cnt;
    /* 0x1209C */ cpu_hack_t hacks[4];
    /* 0x120CC */ char unk_0x120CC[0x158];
    /* 0x12224 */ s32 unk_0x12224;
    /* 0x12228 */ s32 unk_0x12228;
    /* 0x1222C */ s32 unk_0x1222C;
    /* 0x12230 */ s32 unk_0x12230;
    /* 0x12234 */ s32 unk_0x12234;
    /* 0x12238 */ s32 unk_0x12238;
    /* 0x1223C */ s32 unk_0x1223C;
    /* 0x12240 */ s32 unk_0x12240;
    /* 0x12244 */ s32 unk_0x12244;
    /* 0x12248 */ s32 unk_0x12248;
    /* 0x1224C */ s32 unk_0x1224C;
    /* 0x12250 */ s32 unk_0x12250;
    /* 0x12254 */ s32 unk_0x12254;
    /* 0x12258 */ s32 unk_0x12258;
    /* 0x1225C */ s32 unk_0x1225C;
    /* 0x12260 */ s32 unk_0x12260;
    /* 0x12264 */ s32 unk_0x12264;
    /* 0x12268 */ s32 unk_0x12268;
    /* 0x1226C */ s32 unk_0x1226C;
    /* 0x12270 */ s32 unk_0x12270;
    /* 0x12274 */ s32 unk_0x12274;
    /* 0x12278 */ s32 unk_0x12278;
    /* 0x1227C */ s32 unk_0x1227C;
    /* 0x12280 */ s32 unk_0x12280;
    /* 0x12284 */ s32 unk_0x12284;
    /* 0x12288 */ s32 unk_0x12288;
    /* 0x1228C */ s32 unk_0x1228C;
    /* 0x12290 */ u32 unk_0x12290;
    /* 0x12294 */ s32 unk_0x12294;
    /* 0x12298 */ char unk_0x12298[0x40];
} cpu_class_t; // size = 0x122D8

struct cpu_blk_req_t;

typedef s32 blockReqHandler(struct cpu_blk_req_t*, s32);

typedef struct cpu_blk_req_t {
    struct cpu_blk_req_t*  done;
    s32             len;
    blockReqHandler* handler;
    s32             dev_addr;
    u32             dst_phys_ram;
} cpu_blk_req_t;

s32 cpuSetGetBlock(cpu_class_t* cpu, cpu_dev_t* dev, void* arg2);
s32 cpuSetDevicePut(cpu_class_t* cpu, cpu_dev_t* dev, sb_func_t sb, sh_func_t sh, sw_func_t sw, sd_func_t sd);
s32 cpuSetDeviceGet(cpu_class_t* cpu, cpu_dev_t* dev, lb_func_t lb, lh_func_t lh, lw_func_t lw, ld_func_t ld);

#endif
