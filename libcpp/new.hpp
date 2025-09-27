#ifndef __STDCXX_NEW__
#define __STDCXX_NEW__

#include "std_size_t.hpp"

inline void* operator new(std::size_t, void* ptr) __attribute__((nothrow)) { return ptr; }

#endif
