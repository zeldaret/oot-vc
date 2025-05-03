#define USE_CURRENT_LOCALE
#include "ctype.h"
#include "math.h"
#include "revolution/types.h"

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

void strnicmp(void) { fn_80158300(); }

void itoa(void) { fn_801583C0(); }
