#ifndef __STDCXX_CSTRING__
#define __STDCXX_CSTRING__

#include "std_size_t.hpp"

namespace std
{
	extern "C"
	{
		extern void *memcpy(void *s1, const void *s2, std::size_t n) __attribute__((nothrow));
		extern char *strcpy(char *s1, const char *s2) __attribute__((nothrow));
		extern char *strncpy(char *s1, const char *s2, std::size_t n) __attribute__((nothrow));

		extern char *strcat(char *s1, const char *s2) __attribute__((nothrow));

		extern int strcmp(const char *s1, const char *s2) __attribute__((nothrow));
		extern int strncmp(const char *s1, const char *s2, std::size_t n) __attribute__((nothrow));

		extern void *memset(void *s, int c, std::size_t n) __attribute__((nothrow));
		extern std::size_t strlen(const char *s) __attribute__((nothrow));
	} // extern "C"
} // namespace std

#endif // __STDCXX_CSTRING__
