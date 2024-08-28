#ifndef _RVL_SDK_NWC24_UTILS_H
#define _RVL_SDK_NWC24_UTILS_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NWC24Data {
    /* 0x0 */ const void* ptr;
    /* 0x4 */ u32 size;
} NWC24Data;

typedef struct NWC24Date {
    /* 0x0 */ u16 year;
    /* 0x2 */ u8 month;
    /* 0x3 */ u8 day;
    /* 0x4 */ u8 hour;
    /* 0x5 */ u8 sec;
    /* 0x6 */ u8 min;
    u8 BYTE_0x7;
} NWC24Date;

void NWC24Data_Init(NWC24Data* data);
void NWC24Data_SetDataP(NWC24Data* data, const void* ptr, u32 size);
void NWC24Date_Init(NWC24Date* date);
void NWC24iConvIdToStr(u64 addr, char* out);

#ifdef __cplusplus
}
#endif

#endif
