#ifndef NW4R_SND_SEQ_FILE_H
#define NW4R_SND_SEQ_FILE_H

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/Util.h" // Util::Table

#include "revolution/hbm/nw4hbm/ut/ut_binaryFileFormat.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {
namespace SeqFile {
typedef struct Header {
    ut::BinaryFileHeader fileHeader; // 0x00
    u32 dataBlockOffset; // 0x10
    u32 dataBlockSize; // 0x14
    u32 labelBlockOffset; // 0x18
    u32 labelBlockSize; // 0x1C
} Header;

typedef struct DataBlock {
    ut::BinaryBlockHeader blockHeader; // 0x00
    u32 baseOffset; // 0x08
} DataBlock;

static const u32 SIGNATURE_DATA_BLOCK = 'DATA';
static const u32 SIGNATURE_FILE = 'RSEQ';
static const int FILE_VERSION = NW4R_VERSION(1, 0);
static const int SUPPORTED_FILE_VERSION = NW4HBM_VERSION(1, 1);
} // namespace SeqFile

class SeqFileReader {
  public:
    explicit SeqFileReader(const void* seqData);

    bool IsValidFileHeader(const void* seqData);

    const void* GetBaseAddress() const;

  private:
    const SeqFile::Header* mHeader; // 0x00
    const SeqFile::DataBlock* mDataBlock; // 0x04
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_SEQ_FILE_H
