#ifndef _MBSTRING_H
#define _MBSTRING_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int __mbtowc_noconv(wchar_t*, const char*, size_t);
int __wctomb_noconv(char*, wchar_t);
int mbtowc(wchar_t*, const char*, size_t);
size_t mbstowcs(wchar_t*, const char*, size_t);
size_t wcstombs(char*, const wchar_t*, size_t);

#ifdef __cplusplus
}
#endif

#endif
