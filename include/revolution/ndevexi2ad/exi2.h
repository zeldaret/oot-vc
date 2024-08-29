#ifndef _RVL_SDK_NDEV_EXI2_AD_EXI2_H
#define _RVL_SDK_NDEV_EXI2_AD_EXI2_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

bool __EXI2Imm(void* mem, s32 size, u32 type);
void __DBEXIInit(void);
bool __DBEXIReadReg(u32 cmd, void* mem, s32 size);
bool __DBEXIWriteReg(u32 cmd, const void* mem, s32 size);
bool __DBEXIReadRam(u32 cmd, void* mem, s32 size);
bool __DBEXIWriteRam(u32 cmd, const void* mem, s32 size);

#ifdef __cplusplus
}
#endif

#endif
