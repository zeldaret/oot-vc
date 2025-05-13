#ifndef NW4R_DB_ASSERT_H
#define NW4R_DB_ASSERT_H

#include "macros.h"
#include "revolution/hbm/nw4hbm/db/db_console.hpp"
#include "revolution/types.h"

namespace nw4hbm {
namespace db {
#define NW4R_WARNING(...) nw4hbm::db::Warning(__FILE__, __LINE__, __VA_ARGS__)
#define NW4R_PANIC(...) nw4hbm::db::Panic(__FILE__, __LINE__, __VA_ARGS__)
#define NW4R_DB_ASSERT(exp, ...) \
    if (!(exp))                  \
    nw4hbm::db::Panic(__FILE__, __LINE__, __VA_ARGS__)

WEAK void VPanic(const char* file, int line, const char* fmt, std::va_list vlist);
WEAK void Panic(const char* file, int line, const char* fmt, ...);
WEAK void VWarning(const char* file, int line, const char* fmt, std::va_list vlist);
WEAK void Warning(const char* file, int line, const char* msg, ...);

namespace detail {
void Log(const char* fmt, ...);
}

detail::ConsoleHead* Assertion_SetConsole(detail::ConsoleHead* console);
detail::ConsoleHead* Assertion_GetConsole();
void Assertion_ShowConsole(u32 time);
void Assertion_HideConsole();
void Assertion_SetWarningTime(u32 time);
bool Assertion_SetAutoWarning(bool enable);
} // namespace db
} // namespace nw4hbm

#endif
