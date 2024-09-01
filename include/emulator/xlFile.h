#ifndef _XL_FILE_H
#define _XL_FILE_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

bool xlFileGetSize(s32* pnSize, char* szFileName);
bool xlFileLoad(char* szFileName, void** pTarget);

#ifdef __cplusplus
}
#endif

#endif
