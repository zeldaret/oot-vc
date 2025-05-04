#ifndef _XL_TEXT_H
#define _XL_TEXT_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

s32 xlTextGetLength(const char* szTextSource);
s32 xlTextCopy(char* acTextTarget, const char* szTextSource);
s32 xlTextAppend(char* acTextTarget, const char* szTextSource);

#ifdef __cplusplus
}
#endif

#endif
