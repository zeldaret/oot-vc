#include "revolution/hbm/nw4hbm/snd/StrmFile.h"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_StrmFile.cpp
 */

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_adpcm.hpp"
#include "revolution/hbm/nw4hbm/snd/global.h"
#include "revolution/hbm/nw4hbm/snd/Util.h"
#include "revolution/hbm/nw4hbm/snd/WaveFile.h"

#include "revolution/hbm/nw4hbm/ut/ut_binaryFileFormat.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_FileStream.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace snd { namespace detail {

bool StrmFileReader::IsValidFileHeader(void const *strmData)
{
	NW4HBMAssertPointerNonnull_Line(strmData, 42);

	ut::BinaryFileHeader const *fileHeader =
		static_cast<ut::BinaryFileHeader const *>(strmData);

	NW4HBMAssertMessage_Line(
		fileHeader->signature == StrmFile::SIGNATURE_FILE, 51,
		"invalid file signature. strm data is not available.");

	if (fileHeader->signature != StrmFile::SIGNATURE_FILE)
		return false;

	NW4HBMAssertMessage_Line(fileHeader->version >= NW4HBM_VERSION(1, 0), 59,
	            "strm file is not supported version.\n"
	            "  please reconvert file using new version tools.\n");

	if (fileHeader->version < NW4HBM_VERSION(1, 0))
		return false;

	NW4HBMAssertMessage_Line(fileHeader->version <= SUPPORTED_FILE_VERSION, 65,
	            "strm file is not supported version.\n"
	            "  please reconvert file using new version tools.\n");

	if (fileHeader->version > SUPPORTED_FILE_VERSION)
		return false;

	return true;
}

StrmFileReader::StrmFileReader() :
	mHeader		(nullptr),
	mHeadBlock	(nullptr)
{
}

void StrmFileReader::Setup(void const *strmData)
{
	NW4HBMAssertPointerNonnull_Line(strmData, 97);

	if (!IsValidFileHeader(strmData))
		return;

	mHeader		= static_cast<StrmFile::Header const *>(strmData);
	mHeadBlock	= static_cast<StrmFile::HeadBlock const *>(
		ut::AddOffsetToPtr(mHeader, mHeader->headBlockOffset));

	NW4HBMAssert_Line(mHeadBlock->blockHeader.kind
	                         == StrmFile::SIGNATURE_HEAD_BLOCK, 106);

	StrmFile::StrmDataInfo const *info = Util::GetDataRefAddress0(
		mHeadBlock->refDataHeader, &mHeadBlock->refDataHeader);

	// definitely could have just used align assert here
	NW4HBMAssert_Line(info->blockSize % 32 == 0, 113);
}

int StrmFileReader::GetTrackCount() const
{
	NW4HBMAssertPointerNonnull_Line(mHeader, 142);

	StrmFile::TrackTable const *trackTable = Util::GetDataRefAddress0(
		mHeadBlock->refTrackTable, &mHeadBlock->refDataHeader);

	return trackTable->trackCount;
}

int StrmFileReader::GetChannelCount() const
{
	NW4HBMAssertPointerNonnull_Line(mHeader, 163);

	StrmFile::ChannelTable const *channelTable = Util::GetDataRefAddress0(
		mHeadBlock->refChannelTable, &mHeadBlock->refDataHeader);

	return channelTable->channelCount;
}

bool StrmFileReader::ReadStrmInfo(StrmInfo *strmInfo) const
{
	NW4HBMAssertPointerNonnull_Line(mHeader, 184);

	StrmFile::StrmDataInfo const *info = Util::GetDataRefAddress0(
		mHeadBlock->refDataHeader, &mHeadBlock->refDataHeader);

	NW4HBMAssertAligned_Line(info->blockHeaderOffset, 32, 192);
	NW4HBMAssertAligned_Line(info->blockSize, 32, 193);
	NW4HBMAssertAligned_Line(info->lastBlockPaddedSize, 32, 194);

	// clang-format off
	strmInfo->sampleFormat			= GetSampleFormatFromStrmFileFormat(info->format);
	strmInfo->loopFlag				= info->loopFlag;
	strmInfo->numChannels			= info->numChannels;
	strmInfo->sampleRate			= (info->sampleRate24 << 16) + info->sampleRate;
	strmInfo->blockHeaderOffset		= info->blockHeaderOffset;
	strmInfo->loopStart				= info->loopStart;
	strmInfo->loopEnd				= info->loopEnd;
	strmInfo->dataOffset			= info->dataOffset;
	strmInfo->numBlocks				= info->numBlocks;
	strmInfo->blockSize				= info->blockSize;
	strmInfo->blockSamples			= info->blockSamples;
	strmInfo->lastBlockSize			= info->lastBlockSize;
	strmInfo->lastBlockSamples		= info->lastBlockSamples;
	strmInfo->lastBlockPaddedSize	= info->lastBlockPaddedSize;
	strmInfo->adpcmDataInterval		= info->adpcmDataInterval;
	strmInfo->adpcmDataSize			= info->adpcmDataSize;
	// clang-format on

	return true;
}

bool StrmFileReader::ReadStrmTrackInfo(StrmTrackInfo *trackInfo,
                                       int trackIndex) const
{
	NW4HBMAssertPointerNonnull_Line(mHeader, 218);

	StrmFile::TrackTable const *trackTable = Util::GetDataRefAddress0(
		mHeadBlock->refTrackTable, &mHeadBlock->refDataHeader);
	if (trackIndex >= trackTable->trackCount)
		return false;

	switch (trackTable->trackDataType)
	{
	case 0:
	{
		StrmFile::TrackInfo const *src = Util::GetDataRefAddress0(
			trackTable->refTrackHeader[trackIndex], &mHeadBlock->refDataHeader);
		if (!src)
			return false;

		trackInfo->volume		= 127;
		trackInfo->pan			= 64;
		trackInfo->channelCount	= src->channelCount;

		int count = ut::Min(trackInfo->channelCount, 32);

		for (int i = 0; i < count; i++)
			trackInfo->channelIndexTable[i] = src->channelIndexTable[i];
	}
		break;

	case 1:
	{
		StrmFile::TrackInfoEx const *src = Util::GetDataRefAddress1(
			trackTable->refTrackHeader[trackIndex], &mHeadBlock->refDataHeader);
		if (!src)
			return false;

		trackInfo->volume		= src->volume;
		trackInfo->pan			= src->pan;
		trackInfo->channelCount	= src->channelCount;

		int count = ut::Min(trackInfo->channelCount, 32);

		for (int i = 0; i < count; i++)
			trackInfo->channelIndexTable[i] = src->channelIndexTable[i];
	}
		break;

	default:
		// NW4HBMPanic_Line(268);
		// return false here for NDEBUG?
		break;
	}

	return true;
}

bool StrmFileReader::ReadAdpcmInfo(AdpcmParam *adpcmParam,
                                   AdpcmLoopParam *adpcmLoopParam,
                                   int channelIndex) const
{
	NW4HBMAssertPointerNonnull_Line(mHeader, 289);
	NW4HBMAssertPointerNonnull_Line(adpcmParam, 290);
	NW4HBMAssertPointerNonnull_Line(adpcmLoopParam, 291);

	StrmFile::StrmDataInfo const *info = Util::GetDataRefAddress0(
		mHeadBlock->refDataHeader, &mHeadBlock->refDataHeader);
	if (info->format != 2)
		return false;

	StrmFile::ChannelTable const *channelTable = Util::GetDataRefAddress0(
		mHeadBlock->refChannelTable, &mHeadBlock->refDataHeader);
	if (channelIndex >= channelTable->channelCount)
		return false;

	StrmFile::ChannelInfo const *channelInfo =
		Util::GetDataRefAddress0(channelTable->refChannelHeader[channelIndex],
	                               &mHeadBlock->refDataHeader);

	StrmFile::AdpcmParamSet const *src = Util::GetDataRefAddress0(
		channelInfo->refAdpcmInfo, &mHeadBlock->refDataHeader);

	*adpcmParam		= src->adpcmParam;
	*adpcmLoopParam	= src->adpcmLoopParam;

	return true;
}

SampleFormat StrmFileReader::GetSampleFormatFromStrmFileFormat(u8 format)
{
	switch (format)
	{
	case 2:
		return SAMPLE_FORMAT_DSP_ADPCM;

	case 1:
		return SAMPLE_FORMAT_PCM_S16;

	case 0:
		return SAMPLE_FORMAT_PCM_S8;

	default:
		NW4HBMPanicMessage_Line(333, "Unknown strm data format %d", format);
		return SAMPLE_FORMAT_DSP_ADPCM;
	}
}

bool StrmFileLoader::LoadFileHeader(void *buffer, u32 size)
{
	byte_t buffer2[32 + ROUND_UP(sizeof(StrmFile::Header), 0x20)];

	mStream.Seek(0, ut::FileStream::SEEK_BEG);

	s32 readSize = mStream.Read(ut::RoundUp(buffer2, 32),
	                            ROUND_UP(sizeof(StrmFile::Header), 0x20));
	if (readSize != ROUND_UP(sizeof(StrmFile::Header), 0x20))
		return false;

	StrmFile::Header *header =
		static_cast<StrmFile::Header *>(ut::RoundUp(buffer2, 32));

	StrmFileReader reader;
	if (!reader.IsValidFileHeader(header))
		return false;

	if (header->adpcBlockOffset > size)
		return false;

	u32 loadSize = header->headBlockOffset + header->headBlockSize;

	mStream.Seek(0, ut::FileStream::SEEK_BEG);

	readSize = mStream.Read(buffer, loadSize);
	if (readSize != loadSize)
		return false;

	mReader.Setup(buffer);

	return true;
}

int StrmFileLoader::GetTrackCount() const
{
	if (!mReader.IsAvailable())
		return 0;

	return mReader.GetTrackCount();
}

int StrmFileLoader::GetChannelCount() const
{
	if (!mReader.IsAvailable())
		return 0;

	return mReader.GetChannelCount();
}

bool StrmFileLoader::ReadStrmInfo(StrmFileReader::StrmInfo *strmInfo) const
{
	if (!mReader.IsAvailable())
		return false;

	mReader.ReadStrmInfo(strmInfo);
	return true;
}

bool StrmFileLoader::ReadStrmTrackInfo(StrmFileReader::StrmTrackInfo *trackInfo,
                                       int trackIndex) const
{
	if (!mReader.IsAvailable())
		return false;

	mReader.ReadStrmTrackInfo(trackInfo, trackIndex);
	return true;
}

bool StrmFileLoader::ReadAdpcmInfo(AdpcmParam *adpcmParam,
                                   AdpcmLoopParam *adpcmLoopParam,
                                   int channelIndex) const
{
	if (!mReader.IsAvailable())
		return false;

	mReader.ReadAdpcmInfo(adpcmParam, adpcmLoopParam, channelIndex);
	return true;
}

bool StrmFileLoader::ReadAdpcBlockData(u16 *yn1, u16 *yn2, int blockIndex,
                                       int channelCount)
{
	if (!mReader.IsAvailable())
		return false;

	s32 readOffset = mReader.GetAdpcBlockOffset()
	               + blockIndex * channelCount * (sizeof(u16) * 2)
	               + sizeof(ut::BinaryBlockHeader);

	mStream.Seek(readOffset, ut::FileStream::SEEK_BEG);

	u32 readDataSize = channelCount * (sizeof(u16) * 2);
	NW4HBMAssert_Line(readDataSize <= 32, 499);

	alignas(32) u16 buffer[2 * 8];

	int readSize = mStream.Read(buffer, sizeof buffer);
	if (readSize != 32u)
		return false;

	for (int i = 0; i < channelCount; i++)
	{
		yn1[i] = buffer[i * 2];
		yn2[i] = buffer[i * 2 + 1];
	}

	return true;
}

}}} // namespace nw4hbm::snd::detail
