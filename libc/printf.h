#ifndef _PRINTF_H
#define _PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdarg.h"
#include "stdio.h"

int printf(const char* format, ...);
int fprintf(FILE*, const char* format, ...);
int vprintf(const char*, va_list);
int vsnprintf(char*, size_t, const char*, va_list);
int vsprintf(char*, const char*, va_list);
int snprintf(char*, size_t, const char*, ...);
int sprintf(char*, const char*, ...);

#ifdef __cplusplus
}
#endif

#endif
