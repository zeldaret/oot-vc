#ifndef _RAM_CLASS_H
#define _RAM_CLASS_H

#include "types.h"

typedef struct {
    /* 0x0000 */char unk_0x00[4];
    /* 0x0004 */ u8 *dram;
    /* 0x0008 */ size_t dram_size;
    /* 0x000C */ u32 RDRAM_CONFIG_REG;
    /* 0x0010 */ u32 RDRAM_DEVICE_ID_REG;
    /* 0x0014 */ u32 RDRAM_DELAY_REG;
    /* 0x0018 */ u32 RDRAM_MODE_REG;
    /* 0x001C */ u32 RDRAM_REF_INTERVAL_REG;
    /* 0x0020 */ u32 RDRAM_REF_ROW_REG;
    /* 0x0024 */ u32 RDRAM_RAS_INTERVAL_REG;
    /* 0x0028 */ u32 RDRAM_MIN_INTERVAL_REG;
    /* 0x002C */ u32 RDRAM_ADDR_SELECT_REG;
    /* 0x0030 */ u32 RDRAM_DEVICE_MANUF_REG;
    /* 0x0034 */ u32 RI_MODE_REG;
    /* 0x0038 */ u32 RI_CONFIG_REG;
    /* 0x003C */ u32 RI_SELECT_REG;
    /* 0x0040 */ u32 RI_REFRESH_REG;
    /* 0x0044 */ u32 RI_LATENCY_REG;
} ram_class_t; // size = 0x48

#endif
