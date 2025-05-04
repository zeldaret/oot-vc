#ifndef RVL_SDK_HBM_NW4HBM_UT_BINARY_FILE_HEADER_HPP
#define RVL_SDK_HBM_NW4HBM_UT_BINARY_FILE_HEADER_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace ut {
struct BinaryFileHeader {
    byte4_t signature; // size 0x04, offset 0x00
    byte2_t byteOrder; // size 0x02, offset 0x04
    u16 version; // size 0x02, offset 0x06
    u32 fileSize; // size 0x04, offset 0x08
    u16 headerSize; // size 0x02, offset 0x0c
    u16 dataBlocks; // size 0x02, offset 0x0e
}; // size 0x10

struct BinaryBlockHeader {
    byte4_t kind; // size 0x04, offset 0x00
    u32 size; // size 0x04, offset 0x04
}; // size 0x08

bool IsValidBinaryFile(const BinaryFileHeader* header, byte4_t signature, u16 version, u16 minBlocks);
} // namespace ut
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_UT_BINARY_FILE_HEADER_HPP
