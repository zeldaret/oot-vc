#include "time.h"
#include "arith.h"
#include "limits.h"
#include "locale.h"
#include "printf.h"
#include "stdarg.h"
#include "string.h"

#define seconds_per_minute (60L)
#define seconds_per_hour (60L * seconds_per_minute)
#define seconds_per_day (24L * seconds_per_hour)
#define seconds_1900_to_1970 (((365 * 70UL) + 17) * 24 * 60 * 60)

const short __month_to_days[2][13] = {{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
                                      {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}};

extern clock_t __get_clock(void);

int __leap_year(int year) {
    return __msl_mod(year, 4) == 0 && (__msl_mod(year, 100) != 0 || __msl_mod(year, 400) == 100);
}

int leap_days(int year, int month) {
    int n;
    div_t q;

    q = div(year, 4);
    n = q.quot;

    q = div(year, 100);
    n -= q.quot;

    if (year < 100) {
        q = __msl_div(year + 899, 1000);
        n += q.quot;
    } else {
        q = __msl_div(year - 100, 1000);
        n += q.quot + 1;
    }

    if (__leap_year(year)) {
        if (year < 0) {
            if (month > 1) {
                n++;
            }
        } else {
            if (month <= 1) {
                n--;
            }
        }
    }

    return n;
}

static int adjust(int* x, int y, int* z) {
    div_t q;

    q = __msl_div(*x, y);
    *x = q.rem;
    return __msl_add(z, q.quot);
}

void __time2tm(time_t in_time, struct tm* tm) {
    unsigned long years, months, days, seconds;
    int is_leap_year;

    unsigned long time = in_time + seconds_1900_to_1970;

    if (!tm) {
        return;
    }

    tm->tm_isdst = __isdst();

    days = time / seconds_per_day;
    seconds = time % seconds_per_day;

    tm->tm_wday = (days + 1) % 7;
    years = 0;

    for (;;) {
        unsigned long days_this_year = __leap_year(years) ? 366 : 365;

        if (days < days_this_year) {
            break;
        }

        days -= days_this_year;
        years += 1;
    }

    tm->tm_year = years;
    tm->tm_yday = days;

    months = 0;

    is_leap_year = __leap_year(years);

    for (;;) {
        unsigned long days_thru_this_month = __month_to_days[is_leap_year][months + 1];

        if (days < days_thru_this_month) {
            days -= __month_to_days[is_leap_year][months];
            break;
        }

        ++months;
    }

    tm->tm_mon = months;
    tm->tm_mday = days + 1;

    tm->tm_hour = seconds / seconds_per_hour;

    seconds %= seconds_per_hour;

    tm->tm_min = seconds / seconds_per_minute;
    tm->tm_sec = seconds % seconds_per_minute;
}

static int __tm2time(struct tm* tm, time_t* time) {
    long days;
    time_t seconds, day_secs;

    if (!tm || !time) {
        return 0;
    }

    --tm->tm_mday;

    if (!adjust(&tm->tm_sec, 60, &tm->tm_min)) {
        goto no_exit;
    }
    if (!adjust(&tm->tm_min, 60, &tm->tm_hour)) {
        goto no_exit;
    }
    if (!adjust(&tm->tm_hour, 24, &tm->tm_mday)) {
        goto no_exit;
    }
    if (!adjust(&tm->tm_mon, 12, &tm->tm_year)) {
        goto no_exit;
    }

    days = tm->tm_year;

    if (!__msl_lmul(&days, 365)) {
        goto no_exit;
    }
    if (!__msl_ladd(&days, leap_days(tm->tm_year, tm->tm_mon))) {
        goto no_exit;
    }
    if (!__msl_ladd(&days, __month_to_days[0][tm->tm_mon])) {
        goto no_exit;
    }
    if (!__msl_ladd(&days, tm->tm_mday)) {
        goto no_exit;
    }
    if (days < 0 || days > (ULONG_MAX / seconds_per_day)) {
        goto no_exit;
    }

    day_secs = days * seconds_per_day;

    seconds = (tm->tm_hour * seconds_per_hour) + (tm->tm_min * seconds_per_minute) + tm->tm_sec;

    if (seconds > ULONG_MAX - day_secs) {
        goto no_exit;
    }

    seconds += day_secs;
    seconds -= seconds_1900_to_1970;
    *time = seconds;

    __time2tm(seconds, tm);
    return 1;

no_exit:
    return 0;
}

char* find_name(char* names, int index) {
    static char buffer[60];
    int i;
    char* start = names;
    char* end;

    for (i = 0; i <= index; i++) {
        end = strchr(start, '|');
        if (i == index) {
            if (end == NULL) {
                strcpy(buffer, start);
            } else {
                strncpy(buffer, start, end - start);
                buffer[end - start] = '\0';
            }
            break;
        }
        start = end + 1;
    }

    return buffer;
}

// unused but has string literals
void asciitime(struct tm tm, char* str) {
    sprintf(str, "xxx xxx xx xx:xx:xx xxxx\n");
    sprintf(str, "%s %s%3d %.2d:%.2d:%.2d %d\n");
    sprintf(str, "%.3s %.3s %2d %.2d:%.2d:%.2d %4d\n");
}

static void clear_tm(struct tm* tm) {
    tm->tm_sec = 0;
    tm->tm_min = 0;
    tm->tm_hour = 0;
    tm->tm_mday = 1;
    tm->tm_mon = 0;
    tm->tm_year = 0;
    tm->tm_wday = 1;
    tm->tm_yday = 0;
    tm->tm_isdst = -1;
}

clock_t clock(void) { return __get_clock(); }

double difftime(time_t time1, time_t time0) {
    time_t diff;

    if (time1 >= time0) {
        return time1 - time0;
    }

    diff = time0 - time1;
    return -(double)diff;
}

time_t mktime(struct tm* timeptr) {
    struct tm tm = *timeptr;
    time_t time;

    if (!__tm2time(&tm, &time)) {
        return (time_t)-1;
    }

    *timeptr = tm;
    return time;
}

time_t time(time_t* timer) {
    time_t time = __get_time();

    if (timer) {
        *timer = time;
    }

    return time;
}

struct tm* gmtime(const time_t* timer) {
    time_t time;
    static struct tm tm;

    if (!timer) {
        clear_tm(&tm);
        return &tm;
    }

    time = *timer;

    if (!__to_gm_time(&time)) {
        return NULL;
    }

    __time2tm(time, &tm);

    tm.tm_isdst = 0;
    return &tm;
}

struct tm* localtime(const time_t* timer) {
    static struct tm tm;

    if (!timer) {
        clear_tm(&tm);
        tm.tm_isdst = __isdst();
        return &tm;
    } else {
        __time2tm(*timer, &tm);
        tm.tm_isdst = __isdst();
        return &tm;
    }
}

static int emit(char* str, size_t size, size_t* max_size, const char* format_str, ...) {
    va_list args;
    va_start(args, format_str);

    if (size > *max_size) {
        return 0;
    }

    *max_size -= size;
    return vsprintf(str, format_str, args);
}

static int week_num(const struct tm* tm, int starting_day) {
    int days = tm->tm_yday;

    days -= __msl_mod(tm->tm_wday - starting_day, 7);

    if (days < 0) {
        return 0;
    }

    return (days / 7) + 1;
}

time_t ISO8601NewYear(int year) {
    struct tm ts0;
    time_t timeval;
    short StartMday[7] = {2, 3, 4, 29, 30, 31, 1};

    ts0.tm_sec = 0;
    ts0.tm_min = 0;
    ts0.tm_hour = 0;
    ts0.tm_mon = 0;
    ts0.tm_isdst = -1;
    ts0.tm_mday = 0;
    ts0.tm_wday = 7;
    ts0.tm_year = year;
    while ((ts0.tm_wday != 0) && (ts0.tm_mday <= 7)) {
        ts0.tm_mday++;
        timeval = mktime(&ts0);
    }
    ts0.tm_mday = StartMday[ts0.tm_mday - 1];
    if (ts0.tm_mday >= 29) {
        ts0.tm_mon = 11;
        ts0.tm_year--;
    }
    return mktime(&ts0);
}

int ISO8601Week(const struct tm* tmptr, int* WYear) {
    struct WeekYear {
        int Year;
        time_t Start;
    };
    struct WeekYear LastYear, ThisYear, NextYear;
    struct WeekYear* WyPtr;
    struct tm Localtm = *tmptr;
    double Days;
    int WeekNo;
    time_t GivenTimeVal;

    LastYear.Year = tmptr->tm_year - 1;
    LastYear.Start = ISO8601NewYear(LastYear.Year);
    ThisYear.Year = tmptr->tm_year;
    ThisYear.Start = ISO8601NewYear(ThisYear.Year);
    NextYear.Year = tmptr->tm_year + 1;
    NextYear.Start = ISO8601NewYear(NextYear.Year);

    GivenTimeVal = mktime(&Localtm);
    if ((LastYear.Start <= GivenTimeVal) && (GivenTimeVal < ThisYear.Start)) {
        WyPtr = &LastYear;
    } else if ((ThisYear.Start <= GivenTimeVal) && (GivenTimeVal < NextYear.Start)) {
        WyPtr = &ThisYear;
    } else {
        WyPtr = &NextYear;
    }
    *WYear = WyPtr->Year;
    Days = difftime(GivenTimeVal, WyPtr->Start) / 86400;
    WeekNo = (int)(Days / 7) + 1;
    return WeekNo;
}

size_t strftime(char* str, size_t max_size, const char* format_str, const struct tm* timeptr) {
    struct tm tm;
    struct tm default_tm = {0, 0, 0, 1, 0, 0, 1, 0, -1};
    char name[32];
    size_t num_chars, chars_written, space_remaining;
    struct __locale* locale = &_current_locale;
    const char* format_ptr;
    const char* curr_format;
    int n, ISO8601Year, ISO8601WeekNo;

    if ((space_remaining = --max_size) <= 0) {
        return 0;
    }

    tm = default_tm;

    if (timeptr) {
        tm = *timeptr;

        if (mktime(&tm) == (time_t)-1) {
            tm = default_tm;
        }
    }

    format_ptr = format_str;
    chars_written = 0;

    while (*format_ptr) {
        if (!(curr_format = strchr(format_ptr, '%'))) {
            if ((num_chars = strlen(format_ptr)) != 0) {
                if (num_chars <= space_remaining) {
                    memcpy(str, format_ptr, num_chars);

                    chars_written += num_chars;
                    str += num_chars;
                    space_remaining -= num_chars;
                } else {
                    return 0;
                }
            }
            break;
        }

        if ((num_chars = curr_format - format_ptr) != 0) {
            if (num_chars <= space_remaining) {
                memcpy(str, format_ptr, num_chars);

                chars_written += num_chars;
                str += num_chars;
                space_remaining -= num_chars;
            } else {
                return 0;
            }
        }

        format_ptr = curr_format;
        if (format_ptr[1] == 'E' || format_ptr[1] == 'O') {
            ++format_ptr;
        }

        switch (*++format_ptr) {
            case 'a':
                strcpy(name, find_name(locale->time_cmpt_ptr->Day_Names, tm.tm_wday * 2));
                num_chars = emit(str, strlen(name), &space_remaining, "%s", name);
                break;

            case 'A':
                strcpy(name, find_name(locale->time_cmpt_ptr->Day_Names, tm.tm_wday * 2 + 1));
                num_chars = emit(str, strlen(name), &space_remaining, "%s", name);
                break;

            case 'b':
            case 'h': {
                num_chars =
                    emit(str, 3, &space_remaining, "%.3s", find_name(locale->time_cmpt_ptr->MonthNames, tm.tm_mon * 2));
                break;
            }

            case 'B':
                strcpy(name, find_name(locale->time_cmpt_ptr->MonthNames, tm.tm_mon * 2 + 1));
                num_chars = emit(str, strlen(name), &space_remaining, "%s", name);
                break;

            case 'c':
                num_chars = strftime(str, space_remaining + 1, locale->time_cmpt_ptr->DateTime_Format, &tm);
                space_remaining -= num_chars;
                break;

            case 'd':
                num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_mday);
                break;

            case 'D':
                num_chars = strftime(str, space_remaining + 1, "%m/%d/%y", &tm);
                break;

            case 'e':
                num_chars = emit(str, 2, &space_remaining, "%2d", tm.tm_mday);
                break;

            case 'F':
                num_chars =
                    emit(str, 10, &space_remaining, "%.4d-%.2d-%.2d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                break;

            case 'g':
                ISO8601WeekNo = ISO8601Week(timeptr, &ISO8601Year);
                num_chars = emit(str, 2, &space_remaining, "%.2d", ISO8601Year % 100);
                break;

            case 'G':
                ISO8601WeekNo = ISO8601Week(timeptr, &ISO8601Year);
                num_chars = emit(str, 4, &space_remaining, "%.4d", ISO8601Year + 1900);
                break;

            case 'H':
                num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_hour);
                break;

            case 'I':
                num_chars = emit(str, 2, &space_remaining, "%.2d", (n = tm.tm_hour % 12) ? n : 12);
                break;

            case 'j':
                num_chars = emit(str, 3, &space_remaining, "%.3d", tm.tm_yday + 1);
                break;

            case 'm':
                num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_mon + 1);
                break;

            case 'M':
                num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_min);
                break;

            case 'n':
                num_chars = emit(str, 2, &space_remaining, "\n");
                break;

            case 'p': {
                char* name;

                name = find_name(locale->time_cmpt_ptr->am_pm, tm.tm_hour < 12 ? 0 : 1);
                num_chars = emit(str, strlen(name), &space_remaining, "%s", name);
                break;
            }

            case 'r':
                num_chars = strftime(str, space_remaining + 1, locale->time_cmpt_ptr->Twelve_hr_format, &tm);
                space_remaining -= num_chars;
                break;

            case 'R':
                num_chars = strftime(str, space_remaining + 1, "%H:%M", &tm);
                space_remaining -= num_chars;
                break;

            case 'S':
                num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_sec);
                break;

            case 't':
                num_chars = emit(str, 2, &space_remaining, "\t");
                break;

            case 'T':
                num_chars = strftime(str, space_remaining + 1, "%H:%M:%S", &tm);
                space_remaining -= num_chars;
                break;

            case 'u':
                if (tm.tm_wday == 0) {
                    num_chars = emit(str, 1, &space_remaining, "7");
                } else {
                    num_chars = emit(str, 1, &space_remaining, "%.1d", tm.tm_wday);
                }
                break;

            case 'U':
                num_chars = emit(str, 2, &space_remaining, "%.2d", week_num(&tm, 0));
                break;

            case 'V':
                ISO8601WeekNo = ISO8601Week(timeptr, &ISO8601Year);
                num_chars = emit(str, 2, &space_remaining, "%.2d", ISO8601WeekNo);
                break;

            case 'w':
                num_chars = emit(str, 1, &space_remaining, "%.1d", tm.tm_wday);
                break;

            case 'W':
                num_chars = emit(str, 2, &space_remaining, "%.2d", week_num(&tm, 1));
                break;

            case 'x':
                num_chars = strftime(str, space_remaining + 1, locale->time_cmpt_ptr->Date_Format, &tm);
                space_remaining -= num_chars;
                break;

            case 'X':
                num_chars = strftime(str, space_remaining + 1, locale->time_cmpt_ptr->Time_Format, &tm);
                space_remaining -= num_chars;
                break;

            case 'y':
            case 'C':
                num_chars = emit(str, 2, &space_remaining, "%.2d", tm.tm_year % 100);
                break;

            case 'Y':
                num_chars = emit(str, 4, &space_remaining, "%.4d", tm.tm_year + 1900);
                break;

            case 'z': {
                time_t local, utc, now;
                struct tm localtm, *utctmptr;
                double diff, diffmins, diffhours;

                now = time(NULL);
                utctmptr = gmtime(&now);
                if (utctmptr == NULL) {
                    num_chars = emit(str, 4, &space_remaining, "0000");
                } else {
                    localtm = *localtime(&now);
                    local = mktime(&localtm);
                    utc = mktime(utctmptr);
                    diff = difftime(local, utc);
                    diffhours = (int)diff / 3600;
                    diffmins = __abs(diff / 60 - diffhours * 60);
                    num_chars = emit(str, 5, &space_remaining, "%+03.0f%02.0f", diffhours, diffmins);
                }
                break;
            }

            case 'Z': {
                char* TimeZone = locale->time_cmpt_ptr->TimeZone;

                if (*TimeZone == '\0') {
                    num_chars = 0;
                    *str = '\0';
                } else {
                    num_chars = emit(str, strlen(TimeZone), &space_remaining, "%s", TimeZone);
                }
                break;
            }

            case '%':
                num_chars = emit(str, 2, &space_remaining, "%%", *format_ptr);
                break;

            default:
                num_chars = emit(str, 2, &space_remaining, "%%%c", *format_ptr);
                break;
        }

        if (!num_chars) {
            return 0;
        }

        chars_written += num_chars;
        str += num_chars;
        ++format_ptr;
    }

    *str = 0;
    if (max_size < chars_written) {
        return 0;
    } else {
        return chars_written;
    }
}
