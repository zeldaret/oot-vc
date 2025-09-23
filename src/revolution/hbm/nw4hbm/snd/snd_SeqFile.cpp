#include "revolution/hbm/nw4hbm/snd/snd_SeqFile.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_SeqFile.cpp
 */

/*******************************************************************************
 * headers
 */

#include <cstring.hpp>

#include "macros.h" // NW4HBM_VERSION
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_util.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_binaryFileFormat.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // ut::AddOffsetToPtr

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {

bool SeqFileReader::IsValidFileHeader(void const* seqData) {
    ut::BinaryFileHeader const* fileHeader = static_cast<ut::BinaryFileHeader const*>(seqData);

    NW4HBMAssertMessage_Line(fileHeader->signature == SeqFile::SIGNATURE_FILE, 43,
                             "invalid file signature. seq data is not available.");

    if (fileHeader->signature != SeqFile::SIGNATURE_FILE) {
        return false;
    }

    u16 version = Util::ReadBigEndian(fileHeader->version);

    NW4HBMAssertMessage_Line(version >= NW4HBM_VERSION(1, 0), 51,
                             "seq file is not supported version.\n"
                             "  please reconvert file using new version tools.\n");

    version = Util::ReadBigEndian(fileHeader->version); // ? again?
    if (version < NW4HBM_VERSION(1, 0)) {
        return false;
    }

    NW4HBMAssertMessage_Line(version <= SUPPORTED_FILE_VERSION, 59,
                             "seq file is not supported version.\n"
                             "  please reconvert file using new version tools.\n");

    if (version > SUPPORTED_FILE_VERSION) {
        return false;
    }

    return true;
}

SeqFileReader::SeqFileReader(void const* seqData) : mHeader(nullptr), mDataBlock(nullptr) {
    NW4HBMAssertPointerNonnull_Line(seqData, 78);

    if (!IsValidFileHeader(seqData)) {
        return;
    }

    mHeader = static_cast<SeqFile::Header const*>(seqData);
    mDataBlock = static_cast<SeqFile::DataBlock const*>(
        ut::AddOffsetToPtr(mHeader, Util::ReadBigEndian(mHeader->dataBlockOffset)));

    NW4HBMAssert_Line(mDataBlock->blockHeader.kind == SeqFile::SIGNATURE_DATA_BLOCK, 87);
}

void const* SeqFileReader::GetBaseAddress() const {
    NW4HBMAssertPointerNonnull_Line(mHeader, 101);

    return ut::AddOffsetToPtr(mDataBlock, Util::ReadBigEndian(mDataBlock->baseOffset));
}

bool SeqFileReader::ReadOffsetByLabel(char const* labelName, u32* offsetPtr) const {
    NW4HBMAssertPointerNonnull_Line(offsetPtr, 117);

    // NOTE: reinterpret_cast necessary instead of static_cast for regalloc(???)
    SeqFile::LabelBlock const* labelBlock = reinterpret_cast<SeqFile::LabelBlock const*>(
        ut::AddOffsetToPtr(mHeader, Util::ReadBigEndian(mHeader->labelBlockOffset)));

    if (!labelBlock) {
        return false;
    }

    u32 labelNameLen = std::strlen(labelName);

    for (int index = 0; index < Util::ReadBigEndian(labelBlock->labelInfoTable.count); ++index) {
        u32 ofs = labelBlock->labelInfoTable.items[index];

        // NOTE: reinterpret_cast necessary here too
        SeqFile::LabelInfo const* labelInfo =
            reinterpret_cast<SeqFile::LabelInfo const*>(ut::AddOffsetToPtr(labelBlock, Util::ReadBigEndian(ofs) + 8));

        NW4HBMAssertPointerNonnull_Line(labelInfo, 133);

        if (labelNameLen == Util::ReadBigEndian(labelInfo->nameLen) &&
            std::strncmp(labelName, labelInfo->name, labelNameLen) == 0) {
            *offsetPtr = Util::ReadBigEndian(labelInfo->offset);
            return true;
        }
    }

    return false;
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
