#ifndef NW4HBM_SND_SOUND_ARCHIVE_LOADER_H
#define NW4HBM_SND_SOUND_ARCHIVE_LOADER_H

#include "revolution/hbm/nw4hbm/snd/types.h"
#include "revolution/hbm/nw4hbm/ut.h"
#include "revolution/os.h"

namespace nw4hbm {
namespace snd {
class SoundArchive;
class SoundMemoryAllocatable;

namespace detail {

class FileStreamHandle {
public:
    FileStreamHandle(ut::FileStream* pStream) :
        mStream(pStream) {}

    ~FileStreamHandle() {
        if (mStream != NULL) {
            mStream->Close();
        }
    }

    ut::FileStream* GetFileStream() { return mStream; }

    ut::FileStream* operator->() { return mStream; }

    operator bool() const { return mStream; }

private:
    /* 0x00 */ ut::FileStream* mStream;
};

class SoundArchiveLoader {
public:
    explicit SoundArchiveLoader(const SoundArchive& soundArchive);
    ~SoundArchiveLoader();

    void* LoadGroup(u32 id, SoundMemoryAllocatable* allocater, void** waveBuffer, u32 blockSize);
    s32 ReadFile(u32 id, void* dst, s32 size, s32 offset);
    void* LoadFile(u32 id, SoundMemoryAllocatable* allocater);

    void Cancel();

private:
    /* 0x00 */ mutable OSMutex mMutex;
    /* 0x18 */ const SoundArchive& mArc;
    /* 0x1C */ u8 mStreamArea[STREAM_BUFFER_SIZE];
    /* 0x21C */ ut::FileStream* mStream;
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
