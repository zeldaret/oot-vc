#include "revolution/hbm/nw4hbm/ut/ut_binaryFileFormat.hpp"

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/types.h"

/*******************************************************************************
 * functions
 */

//! TODO: remove once matched
extern "C" void fn_8010CB20(char*, int, ...);
extern "C" void fn_8010CBAC(char*, int, ...);

namespace nw4hbm {
namespace ut {

bool IsValidBinaryFile(const BinaryFileHeader* header, byte4_t signature, u16 version, u16 minBlocks) {
    NW4HBM_ASSERT_PTR(header, 48);

    if (header->signature != signature) {
        fn_8010CBAC(__FILE__, 60, "Signature check failed ('%c%c%c%c') must be'%c%c%c%c').",
                    (header->signature >> 24) & 0xFF, (header->signature >> 16) & 0xFF, (header->signature >> 8) & 0xFF,
                    (header->signature & 0xFF), (signature >> 24) & 0xFF, (signature >> 16) & 0xFF,
                    (signature >> 8) & 0xFF, (signature & 0xFF));
        return false;
    }

    // U+FEFF * BYTE ORDER MARK
    if (header->byteOrder != 0xFEFF) {
        fn_8010CBAC(__FILE__, 65, "Unsupported byte order.");
        return false;
    }

    if (header->version != version) {
        fn_8010CBAC(__FILE__, 75, "Version check faild ('%d.%d' must be '%d.%d').", (header->version >> 8) & 0xFF,
                    header->version & 0xFF, (version >> 8) & 0xFF, version & 0xFF);
        return false;
    }

    if (header->fileSize < sizeof *header + sizeof(BinaryBlockHeader) * minBlocks) {
        fn_8010CBAC(__FILE__, 80, "Too small file size(=%d).", header->fileSize);
        return false;
    }

    if (header->dataBlocks < minBlocks) {
        fn_8010CBAC(__FILE__, 85, "Too small number of data blocks(=%d).", header->dataBlocks);
        return false;
    }

    return true;
}

} // namespace ut
} // namespace nw4hbm
