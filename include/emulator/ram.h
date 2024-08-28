#ifndef _RAM_H
#define _RAM_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// RDRAM Interface Registers
#define RI_MODE 0x00
#define RI_CONFIG 0x04
#define RI_CURRENT_LOAD 0x08
#define RI_SELECT 0x0C
#define RI_REFRESH 0x10
#define RI_LATENCY 0x14
#define RI_RERROR 0x18
#define RI_WERROR 0x1C

// RDRAM Control Registers
#define RDRAM_CONFIG 0x00
#define RDRAM_DEVICE_ID 0x04
#define RDRAM_DELAY 0x08
#define RDRAM_MODE 0x0C
#define RDRAM_REF_INTERVAL 0x10
#define RDRAM_REF_NOW 0x14
#define RDRAM_RAS_INTERVAL 0x18
#define RDRAM_MIN_INTERVAL 0x1C
#define RDRAM_ADDR_SELECT 0x20
#define RDRAM_DEVICE_MANUF 0x24

// __anon_0x4BFE7
typedef struct Ram {
    /* 0x00 */ void* pHost;
    /* 0x04 */ u8* pBuffer; // void* makes ramPut8 not matching...
    /* 0x08 */ u32 nSize;
    /* 0x0C */ u32 RDRAM_CONFIG_REG;
    /* 0x10 */ u32 RDRAM_DEVICE_ID_REG;
    /* 0x14 */ u32 RDRAM_DELAY_REG;
    /* 0x18 */ u32 RDRAM_MODE_REG;
    /* 0x1C */ u32 RDRAM_REF_INTERVAL_REG;
    /* 0x20 */ u32 RDRAM_REF_ROW_REG;
    /* 0x24 */ u32 RDRAM_RAS_INTERVAL_REG;
    /* 0x28 */ u32 RDRAM_MIN_INTERVAL_REG;
    /* 0x2C */ u32 RDRAM_ADDR_SELECT_REG;
    /* 0x30 */ u32 RDRAM_DEVICE_MANUF_REG;
    /* 0x34 */ u32 RI_MODE_REG;
    /* 0x38 */ u32 RI_CONFIG_REG;
    /* 0x3C */ u32 RI_SELECT_REG;
    /* 0x40 */ u32 RI_REFRESH_REG;
    /* 0x44 */ u32 RI_LATENCY_REG;
} Ram; // size = 0x48

bool ramGetBuffer(Ram* pRAM, void** ppRAM, u32 nOffset, u32* pnSize);
bool ramWipe(Ram* pRAM);
bool ramSetSize(Ram* pRAM, s32 nSize);
bool ramGetSize(Ram* pRAM, s32* nSize);
bool ramEvent(Ram* pRAM, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassRAM;

#ifdef __cplusplus
}
#endif

#endif
