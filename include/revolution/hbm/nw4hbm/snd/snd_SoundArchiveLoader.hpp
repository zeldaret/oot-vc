#ifndef NW4R_SND_SOUND_ARCHIVE_LOADER_H
#define NW4R_SND_SOUND_ARCHIVE_LOADER_H

#include "revolution/hbm/ut.hpp" // IWYU pragma: export
#include "revolution/os.h"  // IWYU pragma: export

namespace nw4hbm {
namespace snd {

// Forward declarations
class SoundMemoryAllocatable;

namespace detail {

class FileStreamHandle {
public:
    FileStreamHandle(ut::FileStream *pFileStream) : mStream(pFileStream) {}

    ~FileStreamHandle() {
        if (mStream != NULL) {
            mStream->Close();
        }
    }

    ut::FileStream *GetFileStream() {
        return mStream;
    }

    ut::FileStream *operator->() {
        return mStream;
    }

    operator bool() const {
        return mStream;
    }

private:
    ut::FileStream *mStream; // at 0x0
};

class SoundArchiveLoader {
public:
    explicit SoundArchiveLoader(const SoundArchive &rArchive);
    ~SoundArchiveLoader();

    void *LoadGroup(u32 id, SoundMemoryAllocatable *pAllocatable, void **ppWaveBuffer, u32 blockSize);
    s32 ReadFile(u32, void *, s32, s32);
    void *LoadFile(u32 id, SoundMemoryAllocatable* pAllocatable);
    void Cancel();

private:
    mutable OSMutex mMutex;   // at 0x0
    const SoundArchive &mArc; // at 0x18
    u8 mStreamArea[512];      // at 0x1C
    ut::FileStream *mStream;  // at 0x21C
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
