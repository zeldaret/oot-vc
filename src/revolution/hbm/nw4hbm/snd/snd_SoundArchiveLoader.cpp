#include "revolution/hbm/nw4hbm/snd/SoundArchiveLoader.h"

#include "revolution/hbm/nw4hbm/snd/SoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/SoundMemoryAllocatable.h"
#include "revolution/hbm/nw4hbm/ut.h"

namespace nw4hbm {
namespace snd {
namespace detail {

SoundArchiveLoader::SoundArchiveLoader(const SoundArchive& rArchive) :
    mArc(rArchive),
    mStream(nullptr) {
    OSInitMutex(&mMutex);
}

SoundArchiveLoader::~SoundArchiveLoader() {}

void* SoundArchiveLoader::LoadGroup(u32 id, SoundMemoryAllocatable* allocater, void** ppWaveBuffer, u32 blockSize) {
    NW4HBMAssertPointerNonnull_Line(allocater, 83);
    ut::AutoMutexLock lock(mMutex);

    FileStreamHandle groupHandle(mArc.detail_OpenGroupStream(id, mStreamArea, sizeof(mStreamArea)));

    if (!groupHandle) {
        return NULL;
    }

    if (!groupHandle->CanSeek() || !groupHandle->CanRead()) {
        return NULL;
    }

    void* pGroupBuffer = allocater->Alloc(groupHandle->GetSize());
    if (pGroupBuffer == NULL) {
        return NULL;
    }

    mStream = groupHandle.GetFileStream();

    if (blockSize == 0) {
        s32 bytesRead = groupHandle->Read(pGroupBuffer, groupHandle->GetSize());

        if (bytesRead == 0) {
            mStream = nullptr;
            return NULL;
        }
    } else {
        u8* pReadPtr = static_cast<u8*>(pGroupBuffer);
        u32 bytesLeft = groupHandle->GetSize();

        while (bytesLeft) {
            s32 bytesRead = groupHandle->Read(pReadPtr, ut::Min(blockSize, bytesLeft));

            if (bytesRead == 0) {
                mStream = nullptr;
                return NULL;
            }

            if (bytesLeft > blockSize) {
                bytesLeft -= blockSize;
                pReadPtr += blockSize;
            } else {
                bytesLeft = 0;
            }
        }
    }

    mStream = nullptr;

    SoundArchive::GroupInfo groupInfo;
    if (!mArc.detail_ReadGroupInfo(id, &groupInfo)) {
        return NULL;
    }

    if (groupInfo.waveDataSize != 0) {
        FileStreamHandle waveHandle(mArc.detail_OpenGroupWaveDataStream(id, mStreamArea, sizeof(mStreamArea)));

        if (!waveHandle) {
            return NULL;
        }

        if (!waveHandle->CanSeek() || !waveHandle->CanRead()) {
            return NULL;
        }

        void* waveBuffer = allocater->Alloc(waveHandle->GetSize());
        if (waveBuffer == NULL) {
            return NULL;
        }

        mStream = waveHandle.GetFileStream();

        if (blockSize == 0) {
            s32 bytesRead = waveHandle->Read(waveBuffer, waveHandle->GetSize());

            if (bytesRead == 0) {
                mStream = nullptr;
                return NULL;
            }
        } else {
            u8* pReadPtr = static_cast<u8*>(waveBuffer);
            u32 bytesLeft = waveHandle->GetSize();

            while (bytesLeft) {
                s32 bytesRead = waveHandle->Read(pReadPtr, ut::Min(blockSize, bytesLeft));

                if (bytesRead == 0) {
                    mStream = nullptr;
                    return NULL;
                }

                if (bytesLeft > blockSize) {
                    bytesLeft -= blockSize;
                    pReadPtr += blockSize;
                } else {
                    bytesLeft = 0;
                }
            }
        }

        mStream = nullptr;

        if (ppWaveBuffer != NULL) {
            *ppWaveBuffer = waveBuffer;
        }
    } else if (ppWaveBuffer != NULL) {
        *ppWaveBuffer = NULL;
    }

    return pGroupBuffer;
}

s32 SoundArchiveLoader::ReadFile(u32 id, void* dst, s32 size, s32 offset) {
    ut::detail::AutoLock<OSMutex> lock(mMutex);

    FileStreamHandle fileHandle(mArc.detail_OpenFileStream(id, mStreamArea, sizeof(mStreamArea)));
    if (!fileHandle) {
        return -1;
    }

    if (!fileHandle->CanSeek() || !fileHandle->CanRead()) {
        return -1;
    }

    fileHandle->Seek(offset, ut::FileStream::SEEK_BEG);
    mStream = fileHandle.GetFileStream();
    s32 read = mStream->Read(dst, ut::RoundUp(size, 32));
    mStream = nullptr;
    if (read == 0) {
        return -1;
    }

    return size;
}

void* SoundArchiveLoader::LoadFile(u32 id, SoundMemoryAllocatable* allocater) {
    NW4HBMAssertPointerNonnull_Line(allocater, 299);

    SoundArchive::FileInfo info;
    if (!mArc.detail_ReadFileInfo(id, &info)) {
        return NULL;
    }

    u32 size = info.fileSize;
    if (size == 0) {
        return NULL;
    }

    void* buf = allocater->Alloc(size);
    if (buf == NULL) {
        return NULL;
    }

    if (ReadFile(id, buf, size, 0) != size) {
        return NULL;
    }

    DCStoreRange(buf, size);
    return buf;
}

void SoundArchiveLoader::Cancel() {
    if (mStream != NULL && mStream->CanCancel()) {
        if (mStream->CanAsync()) {
            mStream->CancelAsync(nullptr, nullptr);
        } else {
            mStream->Cancel();
        }
    }
}

} // namespace detail
} // namespace snd
} // namespace nw4hbm
