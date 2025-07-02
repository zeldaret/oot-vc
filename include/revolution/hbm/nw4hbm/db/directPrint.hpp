#ifndef NW4R_DB_DIRECT_PRINT_H
#define NW4R_DB_DIRECT_PRINT_H

/*******************************************************************************
 * headers
 */

#include "cstdarg.hpp" // std::va_list

#include "revolution/types.h"

#include "revolution/gx/GXFrameBuf.h" // GXRenderModeObj

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace db {
void DirectPrint_Init();
bool DirectPrint_IsActive();

void DirectPrint_EraseXfb(int posh, int posv, int sizeh, int sizev);
void DirectPrint_ChangeXfb(void* framebuf, u16 width, u16 height);
void DirectPrint_ChangeXfb(void* framebuf);

void DirectPrint_StoreCache();

void DirectPrint_DrawString(int posh, int posv, bool turnOver, char const* format, ...);

void DirectPrint_SetColor(u8 r, u8 g, u8 b);

namespace detail {
void DirectPrint_DrawStringToXfb(int posh, int posv, char const* format, std::va_list vargs, bool turnOver,
                                 bool backErase);

void* DirectPrint_SetupFB(GXRenderModeObj const* rmode);
} // namespace detail
} // namespace db
} // namespace nw4hbm

#endif // NW4R_DB_DIRECT_PRINT_H
