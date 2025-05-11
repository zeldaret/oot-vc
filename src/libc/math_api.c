#include "math.h"

int __signbitd(double x) { return __HI(x) & 0x80000000; }

int __fpclassifyd(double x) {
    switch (__HI(x) & 0x7FF00000) {
        case 0x7FF00000:
            if ((__HI(x) & 0x000FFFFF) || (__LO(x) & 0xFFFFFFFF)) {
                return 1;
            } else {
                return 2;
            }
            break;
        case 0:
            if ((__HI(x) & 0x000FFFFf) || (__LO(x) & 0xFFFFFFFF)) {
                return 5;
            } else {
                return 3;
            }
            break;
    }
    return 4;
}
