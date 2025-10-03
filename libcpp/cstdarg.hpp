#ifndef __STDCXX_CSTDARG__
#define __STDCXX_CSTDARG__

#include "std_va_list.hpp"

extern "C" void *__va_arg(void *, int);

#define va_start(ap, parm)	((void)(parm), __builtin_va_info(&ap))
#define va_arg(ap, type)	(*((type *)(__va_arg(ap, _var_arg_typeof(type)))))
#define va_end(ap)			((void)(0))

#endif // __STDCXX_CSTDARG__
