#ifndef NW4R_DB_ASSERT_H
#define NW4R_DB_ASSERT_H

/*******************************************************************************
 * headers
 */

#include "cstdarg.hpp" // std::va_list

#include "macros.h" // WEAK
#include "revolution/types.h" // u32

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace db {
WEAK void VPanic(char const* file, int line, char const* fmt, std::va_list vlist);
WEAK void Panic(char const* file, int line, char const* fmt, ...);

WEAK void VWarning(char const* file, int line, char const* fmt, std::va_list vlist);
WEAK void Warning(char const* file, int line, char const* fmt, ...);

void Assertion_ShowConsole(u32 time);
} // namespace db
} // namespace nw4hbm

#endif // NW4R_DB_ASSERT_H
