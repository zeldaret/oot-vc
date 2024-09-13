#ifndef _BANNER_H
#define _BANNER_H

#include "emulator/stringtable.h"
#include "revolution/nand.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BANNER_TITLE_MAX 32
#define BANNER_BUFFER_SIZE 0xF0A0

typedef struct struct_80174988 {
    NANDResult result;
    StringIndex eStringIndex;
} struct_80174988;

s32 fn_800641CC(NANDFileInfo* nandFileInfo, char* szFileName, u32 arg2, s32 arg3, u8 access);
bool bannerNANDClose(NANDFileInfo* info, u8 access);
bool bannerCreate(char* szGameName, char* szEmpty);

#ifdef __cplusplus
}
#endif

#endif
