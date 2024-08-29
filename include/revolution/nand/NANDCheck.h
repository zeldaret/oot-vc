#ifndef _RVL_SDK_NAND_CHECK_H
#define _RVL_SDK_NAND_CHECK_H

#include "revolution/nand/nand.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NAND_CHECK_TOO_MANY_APP_BLOCKS = (1 << 0),
    NAND_CHECK_TOO_MANY_APP_FILES = (1 << 1),
    NAND_CHECK_TOO_MANY_USER_BLOCKS = (1 << 2),
    NAND_CHECK_TOO_MANY_USER_FILES = (1 << 3),
} NANDCheckFlags;

s32 NANDCheck(u32 neededBlocks, u32 neededFiles, u32* answer);

#ifdef __cplusplus
}
#endif

#endif
