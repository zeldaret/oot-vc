#ifndef NW4R_SND_MEMORY_SOUND_ARCHIVE_H
#define NW4R_SND_MEMORY_SOUND_ARCHIVE_H

#include "revolution/types.h"

// WARNING: DO NOT REORDER these #include directives, data pooling depends on it

// clang-format off
#include "revolution/hbm/nw4hbm/ut/ut_FileStream.hpp" // This needs to be
#include "revolution/hbm/nw4hbm/snd/SoundArchive.h" // before this
// clang-format on

#include "revolution/hbm/nw4hbm/snd/SoundArchiveFile.h"

namespace nw4hbm {
namespace snd {
class MemorySoundArchive : public SoundArchive {
  private:
    class MemoryFileStream;

  public:
    MemorySoundArchive();
    virtual ~MemorySoundArchive(); // 0x08

    virtual const void* detail_GetFileAddress(u32 id) const; // 0x0C
    virtual const void* detail_GetWaveDataFileAddress(u32 id) const; // 0x10

    virtual int detail_GetRequiredStreamBufferSize() const; // 0x14

    virtual ut::FileStream* OpenStream(void* buffer, int size, u32 offset, u32 length) const; // 0x18

    virtual ut::FileStream* OpenExtStream(void* buffer, int size, const char* extPath, u32 offset,
                                          u32 length) const; // 0x1C

    bool Setup(const void* buffer);
    void Shutdown();

  private:
    const void* mData; // 0x108
    detail::SoundArchiveFileReader mFileReader; // 0x10C
};
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_MEMORY_SOUND_ARCHIVE_H
