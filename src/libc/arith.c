#include "arith.h"
#include "limits.h"

long labs(long __x) { return __x > 0 ? __x : -__x; }

div_t div(int __numer, int __denom) {
    int sign_numer;
    int sign_denom;
    div_t ret;

    sign_numer = 1;
    sign_denom = 1;

    if (__numer < 0) {
        __numer = -__numer;
        sign_numer = -1;
    }

    if (__denom < 0) {
        __denom = -__denom;
        sign_denom = -1;
    }

    ret.quot = (__numer / __denom) * (sign_numer * sign_denom);
    ret.rem = (__numer * sign_numer) - (ret.quot * __denom * sign_denom);
    return ret;
}

int __msl_add(int* __x, int __y) {
    int x = *__x;

    if (__y < 0) {
        if (x < 0 && __y < INT_MIN - x) {
            return 0;
        }
    } else {
        if (x > 0 && __y > INT_MAX - x) {
            return 0;
        }
    }

    *__x = x + __y;
    return 1;
}

int __msl_ladd(long* __x, long __y) {
    long x = *__x;

    if (__y < 0) {
        if (x < 0 && __y < INT_MIN - x) {
            return 0;
        }
    } else {
        if (x > 0 && __y > INT_MAX - x) {
            return 0;
        }
    }

    *__x = x + __y;
    return 1;
}

int __msl_mul(int* __x, int __y) {
    int x = *__x;
    int sign;

    sign = ((x < 0) ^ (__y < 0)) ? -1 : 1;

    if (x < 0) {
        x = -x;
    }

    if (__y < 0) {
        __y = -__y;
    }

    if (x > INT_MAX / __y) {
        return 0;
    }

    *__x = x * __y * sign;
    return 1;
}

div_t __msl_div(int __x, int __y) {
    int quot;
    int rem;
    int sign_x;
    int sign_y;
    int sign_quot;
    div_t ret;

    sign_x = 1;
    sign_y = 1;

    if (__x < 0) {
        __x = -__x;
        sign_x = -1;
    }

    if (__y < 0) {
        __y = -__y;
        sign_y = -1;
    }

    sign_quot = sign_x * sign_y;
    quot = (__x / __y) * sign_quot;
    rem = (__x * sign_x) - (quot * __y * sign_y);

    if ((rem != 0) && (sign_quot < 0)) {
        quot -= 1;
        rem += __y * sign_y;
    }

    ret.quot = quot;
    ret.rem = rem;
    return ret;
}

int __msl_mod(int __x, int __y) {
    int quot;
    int sign_x;
    int sign_y;
    int rem;
    int sign_quot;

    sign_x = 1;
    sign_y = 1;

    if (__x < 0) {
        __x = -__x;
        sign_x = -1;
    }

    if (__y < 0) {
        __y = -__y;
        sign_y = -1;
    }

    sign_quot = sign_x * sign_y;
    quot = (__x / __y) * sign_quot;
    rem = (__x * sign_x) - (quot * __y * sign_y);

    if (rem != 0 && sign_quot < 0) {
        return rem + (__y * sign_y);
    }

    return rem;
}
