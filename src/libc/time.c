#include "time.h"

extern clock_t __get_clock(void);

tm lbl_8025A1D0;

clock_t clock(void) { return __get_clock(); }

time_t mktime(tm* arg) {
    tm spC = *arg;
    time_t sp8;

    if (!fn_8015F2F8(&spC, &sp8)) {
        return -1;
    }

    *arg = spC;
    return sp8;
}

tm* localtime(const time_t* timer) {
    if (timer == NULL) {
        lbl_8025A1D0.tm_sec = 0;
        lbl_8025A1D0.tm_min = 0;
        lbl_8025A1D0.tm_hour = 0;
        lbl_8025A1D0.tm_mday = 1;
        lbl_8025A1D0.tm_mon = 0;
        lbl_8025A1D0.tm_year = 0;
        lbl_8025A1D0.tm_wday = 1;
        lbl_8025A1D0.tm_yday = 0;
        lbl_8025A1D0.tm_isdst = -1;
        lbl_8025A1D0.tm_isdst = fn_80160A04();
        return &lbl_8025A1D0;
    }

    fn_8015F11C(*timer, &lbl_8025A1D0);
    lbl_8025A1D0.tm_isdst = fn_80160A04();
    return &lbl_8025A1D0;
}
