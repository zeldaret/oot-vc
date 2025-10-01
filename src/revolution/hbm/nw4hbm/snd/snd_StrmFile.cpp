#include "revolution/hbm/nw4hbm/snd/StrmFile.h"
#include "revolution/hbm/nw4hbm/snd/StrmPlayer.h"
#include "revolution/hbm/nw4hbm/snd/WaveFile.h"
#include "revolution/hbm/nw4hbm/ut.h"

namespace nw4hbm {
namespace snd {
namespace detail {

bool StrmFileReader::IsValidFileHeader(const void* strmData) {
    NW4HBMAssertPointerNonnull_Line(strmData, 43);
    const ut::BinaryFileHeader* fileHeader = static_cast<const ut::BinaryFileHeader*>(strmData);

    NW4HBMAssert_Line(fileHeader->signature == StrmFile::SIGNATURE_FILE, 49);

    if (fileHeader->signature != StrmFile::SIGNATURE_FILE) {
        return false;
    }

    NW4HBMAssertMessage_Line(fileHeader->version >= NW4HBM_VERSION(1, 0), 57,
                             "strm file is not supported version.\n  please reconvert file using new version tools.\n");
    if (fileHeader->version < NW4HBM_VERSION(1, 0)) {
        return false;
    }

    NW4HBMAssertMessage_Line(fileHeader->version <= StrmFile::FILE_VERSION, 63,
                             "strm file is not supported version.\n  please reconvert file using new version tools.\n");
    if (fileHeader->version > StrmFile::FILE_VERSION) {
        return false;
    }

    return true;
}

StrmFileReader::StrmFileReader() :
    mHeader(nullptr),
    mHeadBlock(nullptr) {}

void StrmFileReader::Setup(const void* strmData) {
    NW4HBMAssertPointerNonnull_Line(strmData, 95);

    if (!IsValidFileHeader(strmData)) {
        return;
    }

    mHeader = static_cast<const StrmFile::Header*>(strmData);

    mHeadBlock = static_cast<const StrmFile::HeadBlock*>(ut::AddOffsetToPtr(mHeader, mHeader->headBlockOffset));
    NW4HBMAssert_Line(mHeadBlock->blockHeader.kind == StrmFile::SIGNATURE_HEAD_BLOCK, 104);

    StrmFile::StrmDataInfo const* info =
        Util::GetDataRefAddress0(mHeadBlock->refDataHeader, &mHeadBlock->refDataHeader);
    NW4HBMAssert_Line(info->blockSize % 32 == 0, 108);
}

bool StrmFileReader::ReadStrmInfo(StrmInfo* strmInfo) const {
    NW4HBMAssertPointerNonnull_Line(mHeader, 137);
    const StrmFile::StrmDataInfo* pStrmData =
        Util::GetDataRefAddress0(mHeadBlock->refDataHeader, &mHeadBlock->refDataHeader);

    strmInfo->format = pStrmData->format;
    strmInfo->loopFlag = pStrmData->loopFlag;
    strmInfo->numChannels = pStrmData->numChannels;
    strmInfo->sampleRate = (pStrmData->sampleRate24 << 16) + pStrmData->sampleRate;
    strmInfo->blockHeaderOffset = pStrmData->blockHeaderOffset;
    strmInfo->loopStart = pStrmData->loopStart;
    strmInfo->loopEnd = pStrmData->loopEnd;
    strmInfo->dataOffset = pStrmData->dataOffset;
    strmInfo->numBlocks = pStrmData->numBlocks;
    strmInfo->blockSize = pStrmData->blockSize;
    strmInfo->blockSamples = pStrmData->blockSamples;
    strmInfo->lastBlockSize = pStrmData->lastBlockSize;
    strmInfo->lastBlockSamples = pStrmData->lastBlockSamples;
    strmInfo->lastBlockPaddedSize = pStrmData->lastBlockPaddedSize;
    strmInfo->adpcmDataInterval = pStrmData->adpcmDataInterval;
    strmInfo->adpcmDataSize = pStrmData->adpcmDataSize;

    return true;
}

bool StrmFileReader::ReadAdpcmInfo(AdpcmInfo* adpcmInfo, int channels) const {
    NW4HBMAssertPointerNonnull_Line(mHeader, 173);
    const StrmFile::StrmDataInfo* pStrmData =
        Util::GetDataRefAddress0(mHeadBlock->refDataHeader, &mHeadBlock->refDataHeader);

    if (pStrmData->format != WaveFile::FORMAT_ADPCM) {
        return false;
    }

    const StrmFile::ChannelTable* pChannelTable =
        Util::GetDataRefAddress0(mHeadBlock->refChannelTable, &mHeadBlock->refDataHeader);

    if (channels >= pChannelTable->channelCount) {
        return false;
    }

    const StrmFile::ChannelInfo* pChannelInfo =
        Util::GetDataRefAddress0(pChannelTable->refChannelHeader[channels], &mHeadBlock->refDataHeader);
    const AdpcmInfo* pSrcInfo = Util::GetDataRefAddress0(pChannelInfo->refAdpcmInfo, &mHeadBlock->refDataHeader);

    *adpcmInfo = *pSrcInfo;
    return true;
}

bool StrmFileLoader::LoadFileHeader(void* strmData, u32 size) {
    u8 headerArea[HEADER_ALIGNED_SIZE + 32];
    u32 bytesRead;

    mStream.Seek(0, ut::FileStream::SEEK_BEG);
    bytesRead = mStream.Read(ut::RoundUp(headerArea, 32), HEADER_ALIGNED_SIZE);
    if (bytesRead != HEADER_ALIGNED_SIZE) {
        return false;
    }

    StrmFile::Header* pHeader = static_cast<StrmFile::Header*>(ut::RoundUp(headerArea, 32));

    StrmFileReader reader;
    if (!reader.IsValidFileHeader(pHeader)) {
        return false;
    }

    if (pHeader->adpcBlockOffset > size) {
        return false;
    }

    u32 loadSize = pHeader->headBlockOffset + pHeader->headBlockSize;

    mStream.Seek(0, ut::FileStream::SEEK_BEG);
    bytesRead = mStream.Read(strmData, loadSize);
    if (bytesRead != loadSize) {
        return false;
    }

    mReader.Setup(strmData);
    return true;
}

bool StrmFileLoader::ReadAdpcBlockData(u16* yn1, u16* yn2, int block, int channels) {
    if (!mReader.IsAvailable()) {
        return false;
    }

    s32 offset = mReader.GetAdpcBlockOffset() + block * channels * (2 * sizeof(u16)) + sizeof(ut::BinaryBlockHeader);

    mStream.Seek(offset, ut::FileStream::SEEK_BEG);

    u16 buffer[StrmPlayer::StrmHeader::STRM_CHANNEL_MAX * 2] ATTRIBUTE_ALIGN(32);
    // @bug Read size not validated
    mStream.Read(buffer, sizeof(buffer));

    for (int i = 0; i < channels; i++) {
        yn1[i] = buffer[i * 2];
        yn2[i] = buffer[i * 2 + 1];
    }

    return true;
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
