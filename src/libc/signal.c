#include "signal.h"

sig_func signal_funcs[7];

int raise(int sig) {
    sig_func temp_r31;
    u32 temp_r0;

    if (sig < 1 || sig > 7) {
        return -1;
    }

    temp_r0 = sig - 1;
    temp_r31 = signal_funcs[temp_r0];
    if ((u32)temp_r31 != 1) {
        signal_funcs[temp_r0] = 0;
    }
    if ((u32)temp_r31 == 1 || ((int)temp_r31 == 0 && sig == 1)) {
        return 0;
    }
    if ((u32)temp_r31 == 0) {
        exit(0);
    }
    temp_r31(sig);
    return 0;
}
