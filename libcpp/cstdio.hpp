#ifndef __STDCXX_CSTDIO__
#define __STDCXX_CSTDIO__

#include "std_size_t.hpp"

// Not provided by <cstdio>, but necessary for some declarations
#include "std_va_list.hpp"

namespace std
{
    extern "C"
	{
	    extern int vsnprintf(char *s, std::size_t n, const char *format, std::va_list arg);

	    extern int vswprintf(wchar_t *s, std::size_t n, const wchar_t *format, std::va_list arg);
	} // extern "C"
} // namespace std

#endif // __STDCXX_CSTDIO__
