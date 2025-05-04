#ifndef __STDCXX_CWCHAR__
#define __STDCXX_CWCHAR__

#include "std_size_t.hpp"

namespace std
{
	extern "C"
	{
		extern std::size_t wcslen(const wchar_t *s) __attribute__((nothrow));
	} // extern "C"
} // namespace std

#endif // __STDCXX_CWCHAR__
