#include "wstring.h"

size_t wcslen(const wchar_t* str) {
    size_t len = -1;
    wchar_t* p = (wchar_t*)str - 1;

    do {
        len++;
    } while (*++p);

    return len;
}

wchar_t* wcsncpy(wchar_t* dest, const wchar_t* src, size_t num) {
    const wchar_t* p = (const wchar_t*)src - 1;
    wchar_t* q = (wchar_t*)dest - 1;
    wchar_t zero = 0;

    num++;

    while (--num) {
        if (!(*++q = *++p)) {
            while (--num) {
                *++q = 0;
            }

            break;
        }
    }

    return dest;
}
