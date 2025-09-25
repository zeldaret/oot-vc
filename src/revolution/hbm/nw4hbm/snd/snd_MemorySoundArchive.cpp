#include "revolution/hbm/nw4hbm/snd/MemorySoundArchive.h"

#include <cstring.hpp> // std::memcpy
#include <new.hpp>

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/SoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/SoundArchiveFile.h" // SoundArchiveFileReader

#include "revolution/hbm/nw4hbm/ut/ut_FileStream.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp" // IWYU pragma: keep (need the complete type)
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"

#include "revolution/hbm/HBMAssert.hpp"

namespace nw4hbm {
namespace snd {

class MemorySoundArchive::MemoryFileStream : public ut::FileStream {
  public:
    MemoryFileStream(const void* buffer, u32 size);

    virtual void Close(); // 0x10
    virtual s32 Read(void* dst, u32 size); // 0x14
    virtual void Seek(s32 offset, u32 origin); // 0x44

    virtual bool CanSeek() const { return true; } // 0x50
    virtual bool CanCancel() const { return true; } // 0x54
    virtual bool CanAsync() const { return false; } // 0x28
    virtual bool CanRead() const { return true; } // 0x2C
    virtual bool CanWrite() const { return false; } // 0x30

    virtual u32 Tell() const { return mOffset; } // 0x58

    virtual u32 GetSize() const { return mSize; } // 0x40

  private:
    const void* mData; // 0x14
    s32 mSize; // 0x18
    s32 mOffset; // 0x1C
};

MemorySoundArchive::MemorySoundArchive() : mData(NULL) {}

MemorySoundArchive::~MemorySoundArchive() {}

bool MemorySoundArchive::Setup(const void* buffer) {
    mFileReader.Init(buffer);
    SoundArchive::Setup(&mFileReader);

    const void* pInfoChunk = ut::AddOffsetToPtr(buffer, mFileReader.GetInfoChunkOffset());

    mFileReader.SetInfoChunk(pInfoChunk, mFileReader.GetInfoChunkSize());

    const void* pStringChunk = ut::AddOffsetToPtr(buffer, mFileReader.GetLabelStringChunkOffset());

    mFileReader.SetStringChunk(pStringChunk, mFileReader.GetLabelStringChunkSize());

    mData = buffer;
    return true;
}

void MemorySoundArchive::Shutdown() {
    mData = NULL;
    SoundArchive::Shutdown();
}

const void* MemorySoundArchive::detail_GetFileAddress(u32 id) const {
    SoundArchive::FilePos pos;
    if (!detail_ReadFilePos(id, 0, &pos)) {
        return NULL;
    }

    SoundArchive::GroupInfo groupInfo;
    if (!detail_ReadGroupInfo(pos.groupId, &groupInfo)) {
        return NULL;
    }

    SoundArchive::GroupItemInfo itemInfo;
    if (!detail_ReadGroupItemInfo(pos.groupId, pos.index, &itemInfo)) {
        return NULL;
    }

    if (groupInfo.extFilePath != NULL) {
        return NULL;
    }

    return ut::AddOffsetToPtr(mData, groupInfo.offset + itemInfo.offset);
}

const void* MemorySoundArchive::detail_GetWaveDataFileAddress(u32 id) const {
    SoundArchive::FilePos pos;
    if (!detail_ReadFilePos(id, 0, &pos)) {
        return NULL;
    }

    SoundArchive::GroupInfo groupInfo;
    if (!detail_ReadGroupInfo(pos.groupId, &groupInfo)) {
        return NULL;
    }

    SoundArchive::GroupItemInfo itemInfo;
    if (!detail_ReadGroupItemInfo(pos.groupId, pos.index, &itemInfo)) {
        return NULL;
    }

    if (groupInfo.extFilePath != NULL) {
        return NULL;
    }

    return ut::AddOffsetToPtr(mData, groupInfo.waveDataOffset + itemInfo.waveDataOffset);
}

MemorySoundArchive::MemoryFileStream::MemoryFileStream(const void* buffer, u32 size)
    : mData(buffer), mSize(size), mOffset(0) {}

ut::FileStream* MemorySoundArchive::OpenStream(void* buffer, int size, u32 offset, u32 length) const {
    if (mData == NULL) {
        return nullptr;
    }

    if (size < sizeof(MemoryFileStream)) {
        return nullptr;
    }

    return new (buffer) MemoryFileStream(ut::AddOffsetToPtr(mData, offset), length);
}

ut::FileStream* MemorySoundArchive::OpenExtStream(void* buffer, int size, const char* extPath, u32 offset,
                                                  u32 length) const {
    return nullptr;
}

int MemorySoundArchive::detail_GetRequiredStreamBufferSize() const { return sizeof(MemoryFileStream); }

void MemorySoundArchive::MemoryFileStream::Close() {
    mData = NULL;
    mSize = 0;
    mOffset = 0;
}

s32 MemorySoundArchive::MemoryFileStream::Read(void* dst, u32 size) {
    u32 bytesRead = ut::Min<u32>(size, mSize - mOffset);
    std::memcpy(dst, ut::AddOffsetToPtr(mData, mOffset), bytesRead);

    return bytesRead;
}

void MemorySoundArchive::MemoryFileStream::Seek(s32 offset, u32 origin) {
    switch (origin) {
        case SEEK_BEG: {
            mOffset = offset;
            break;
        }
        case SEEK_CUR: {
            mOffset += offset;
            break;
        }
        case SEEK_END: {
            mOffset = mSize - offset;
            break;
        }
        default: {
            return;
        }
    }
}

} // namespace snd
} // namespace nw4hbm
