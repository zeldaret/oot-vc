#ifndef _SCANF_H
#define _SCANF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdarg.h"

int vsscanf(const char*, const char*, va_list);
int sscanf(const char*, const char*, ...);

#ifdef __cplusplus
}
#endif

#endif
