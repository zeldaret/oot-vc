#ifndef _LIMITS_H_
#define _LIMITS_H_

#define SCHAR_MIN (-0x7F - 1)
#define SCHAR_MAX 0x7F
#define UCHAR_MAX 0xFFU

#if defined(__MWERKS__) && __option(unsigned_char)
#define CHAR_MIN 0U
#define CHAR_MAX UCHAR_MAX
#else
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX
#endif

#define SHRT_MIN  (-0x7FFF - 1)
#define SHRT_MAX  0x7FFF
#define USHRT_MAX 0xFFFF

#define INT_MIN  (-0x7FFFFFFF - 1)
#define INT_MAX  0x7FFFFFFF
#define UINT_MAX 0xFFFFFFFF

#define LONG_MIN  (-0x7FFFFFFFL - 1)
#define LONG_MAX  0x7FFFFFFFL
#define ULONG_MAX 0xFFFFFFFFUL

#define LLONG_MIN  (-0x7FFFFFFFFFFFFFFFLL - 1)
#define LLONG_MAX  0x7FFFFFFFFFFFFFFFLL
#define ULLONG_MAX 0xFFFFFFFFFFFFFFFFULL

#endif
