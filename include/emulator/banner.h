#ifndef _BANNER_H
#define _BANNER_H

#include "emulator/stringtable.h"
#include "revolution/nand.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BANNER_SIZE (64 * 64)

bool fn_800641CC(NANDFileInfo* pNandFileInfo, const char* szFileName, s32 arg2, s32 arg3, s32 access);
bool bannerNANDClose(NANDFileInfo* info, u8 access);
bool bannerCreate(char* szGameName, char* szEmpty);

#ifdef __cplusplus
}
#endif

#endif
