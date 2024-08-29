#ifndef _XL_TEXT_H
#define _XL_TEXT_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 xlTextGetLength(char* szTextSource);
s32 xlTextCopy(char* acTextTarget, char* szTextSource);
s32 xlTextAppend(char* acTextTarget, char* szTextSource);

#ifdef __cplusplus
}
#endif

#endif
