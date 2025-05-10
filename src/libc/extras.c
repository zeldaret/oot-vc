#define USE_CURRENT_LOCALE
#include "ctype.h"
#include "math.h"
#include "revolution/types.h"

int __msl_strnicmp(const char* pStr1, const char* pStr2, int n);
char* __msl_itoa(int value, char* str, int base);

double scalbn(double x, int n) {
    double value;
    int exp;

    value = frexp(x, &exp);
    exp += n;

    return ldexp(value, exp);
}

int stricmp(char* param_1, char* param_2) {
    s8 a_var;
    s8 b_var;

    do {
        b_var = tolower(*param_1++);
        a_var = tolower(*param_2++);

        if (b_var < a_var) {
            return -1;
        }

        if (b_var > a_var) {
            return 1;
        }
    } while (b_var != 0);

    return 0;
}

int strnicmp(const char* pStr1, const char* pStr2, int n) { return __msl_strnicmp(pStr1, pStr2, n); }

char* itoa(int value, char* str, int base) { return __msl_itoa(value, str, base); }
