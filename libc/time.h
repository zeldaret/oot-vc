#ifndef _TIME_H
#define _TIME_H

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__cplusplus) && __STDC_VERSION__ >= 199901L
#define RESTRICT restrict
#else
#define RESTRICT
#endif

typedef long time_t;
typedef long clock_t;

struct tm {
    /* 0x00 */ int tm_sec;
    /* 0x04 */ int tm_min;
    /* 0x08 */ int tm_hour;
    /* 0x0C */ int tm_mday;
    /* 0x10 */ int tm_mon;
    /* 0x14 */ int tm_year;
    /* 0x18 */ int tm_wday;
    /* 0x1C */ int tm_yday;
    /* 0x20 */ int tm_isdst;
}; // size = 0x24

time_t time(time_t* timer);
double difftime(time_t time_end, time_t time_beg);
clock_t clock(void);
char* ctime(const time_t* timer);
char* asctime(const struct tm* timeptr);
size_t strftime(char* RESTRICT str, size_t count, const char* RESTRICT format, const struct tm* RESTRICT tp);
time_t mktime(struct tm* timeptr);
struct tm* gmtime(const time_t* timer);
struct tm* localtime(const time_t* timer);

#ifdef __cplusplus
}
#endif

#endif
