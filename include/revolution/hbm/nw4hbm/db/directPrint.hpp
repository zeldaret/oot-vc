#ifndef NW4R_DB_DIRECT_PRINT_H
#define NW4R_DB_DIRECT_PRINT_H

/*******************************************************************************
 * headers
 */

#include "cstdarg.hpp" // std::va_list

#include "revolution/types.h"

#include "revolution/gx/GXFrameBuf.h" // GXRenderModeObj

#include "revolution/hbm/nw4hbm/ut/Color.hpp"

/*******************************************************************************
 * functions
 */

// [SPQE7T]/ISpyD.elf:.debug_info::0x39919b
struct FrameBufferInfo {
    u8* frameMemory; // size 0x04, offset 0x00
    u32 frameSize; // size 0x04, offset 0x04
    u16 frameWidth; // size 0x02, offset 0x08
    u16 frameHeight; // size 0x02, offset 0x0a
    u16 frameRow; // size 0x02, offset 0x0c
    u16 reserved; // size 0x02, offset 0x0e
}; // size 0x10

// [SPQE7T]/ISpyD.elf:.debug_info::0x399233
struct YUVColorInfo {
    GXColor colorRGBA; // size 0x04, offset 0x00
    u16 colorY256; // size 0x02, offset 0x04
    u16 colorU; // size 0x02, offset 0x06
    u16 colorU2; // size 0x02, offset 0x08
    u16 colorU4; // size 0x02, offset 0x0a
    u16 colorV; // size 0x02, offset 0x0c
    u16 colorV2; // size 0x02, offset 0x0e
    u16 colorV4; // size 0x02, offset 0x10
    u16 reserved; // size 0x02, offset 0x12
}; // size 0x14

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
