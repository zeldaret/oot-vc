#include "locale.h"
#include "ctype.h"
#include "limits.h"
#include "mbstring.h"

struct lconv __lconv = {
    ".", // decimal_point
    "", // thousands_sep
    "", // grouping
    "", // mon_decimal_point
    "", // mon_thousands_sep
    "", // mon_grouping
    "", // positive_sign
    "", // negative_sign
    "", // currency_symbol
    CHAR_MAX, // frac_digits
    CHAR_MAX, // p_cs_precedes
    CHAR_MAX, // n_cs_precedes
    CHAR_MAX, // p_sep_by_space
    CHAR_MAX, // n_sep_by_space
    CHAR_MAX, // p_sign_posn
    CHAR_MAX, // n_sign_posn
    "", // int_curr_symbol
    CHAR_MAX, // int_frac_digits
    CHAR_MAX, // int_p_cs_precedes
    CHAR_MAX, // int_n_cs_precedes
    CHAR_MAX, // int_p_sep_by_space
    CHAR_MAX, // int_n_sep_by_space
    CHAR_MAX, // int_p_sign_posn
    CHAR_MAX, // int_n_sign_posn
};

extern unsigned short __ctype_map[256];
extern unsigned char __upper_map[256];
extern unsigned char __lower_map[256];
extern unsigned short __wctype_map[256];
extern wchar_t __wupper_map[256];
extern wchar_t __wlower_map[256];

struct _loc_ctype_cmpt __ctype_cmpt = {
    "C", // CmptName
    __ctype_map, // ctype_map_ptr
    __upper_map, // upper_map_ptr
    __lower_map, // lower_map_ptr
    __wctype_map, // wctype_map_ptr
    __wupper_map, // wupper_map_ptr
    __wlower_map, // wlower_map_ptr
    __mbtowc_noconv, // decode_mb
    __wctomb_noconv // encode_wc
};

unsigned short char_coll_table[96] = {
    1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 17, 18, 19, 20, 21, 22, 23, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71,
    73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 24, 25, 26, 27, 28, 0,  44, 46, 48, 50, 52, 54, 56,
    58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 29, 30, 31, 32, 0,
};

struct _loc_coll_cmpt __coll_cmpt = {
    "C", // CmptName
    ' ', // char_start_value
    110, // char_coll_tab_size
    0, // char_spec_accents
    char_coll_table, // char_coll_table_ptr
    NULL // wchar_coll_seq_ptr
};

struct _loc_mon_cmpt __mon_cmpt = {
    "C", // CmptName
    "", // mon_decimal_point
    "", // mon_thousands_sep
    "", // mon_grouping
    "", // positive_sign
    "", // negative_sign
    "", // currency_symbol
    CHAR_MAX, // frac_digits
    CHAR_MAX, // p_cs_precedes
    CHAR_MAX, // n_cs_precedes
    CHAR_MAX, // p_sep_by_space
    CHAR_MAX, // n_sep_by_space
    CHAR_MAX, // p_sign_posn
    CHAR_MAX, // n_sign_posn
    "", // int_curr_symbol
    CHAR_MAX, // int_frac_digits
    CHAR_MAX, // int_p_cs_precedes
    CHAR_MAX, // int_n_cs_precedes
    CHAR_MAX, // int_p_sep_by_space
    CHAR_MAX, // int_n_sep_by_space
    CHAR_MAX, // int_p_sign_posn
    CHAR_MAX // int_n_sign_posn
};

struct _loc_num_cmpt __num_cmpt = {
    "C", // CmptName
    ".", // decimal_point
    "", // thousands_sep
    "" // grouping
};

struct _loc_time_cmpt __time_cmpt = {
    "C", // CmptName
    "AM|PM", // am_pm
    "%a %b %e %T %Y", // DateTime_Format
    "%I:%M:%S %p", // Twelve_hr_format
    "%m/%d/%y", // Date_Format
    "%T", // Time_Format
    "Sun|Sunday|Mon|Monday|Tue|Tuesday|Wed|Wednesday|Thu|Thursday|Fri|Friday|Sat|Saturday", // Day_Names
    "Jan|January|Feb|February|Mar|March|Apr|April|May|May|Jun|June|Jul|July|Aug|August|Sep|September|Oct|October|Nov|"
    "November|Dec|December", // MonthNames
    "" // TimeZone
};

struct __locale _current_locale = {
    NULL, // next_locale
    "C", // locale_name
    &__coll_cmpt, // coll_cmpt_ptr
    &__ctype_cmpt, // ctype_cmpt_ptr
    &__mon_cmpt, // mon_cmpt_ptr
    &__num_cmpt, // num_cmpt_ptr
    &__time_cmpt // time_cmpt_ptr
};
