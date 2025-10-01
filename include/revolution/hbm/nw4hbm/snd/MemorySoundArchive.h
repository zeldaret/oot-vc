#ifndef NW4R_SND_MEMORY_SOUND_ARCHIVE_H
#define NW4R_SND_MEMORY_SOUND_ARCHIVE_H

#include "revolution/hbm/nw4hbm/snd/SoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/SoundArchiveFile.h"
#include "revolution/hbm/nw4hbm/ut/FileStream.h"
#include "revolution/types.h"

namespace nw4hbm {
namespace snd {
class MemorySoundArchive : public SoundArchive {
  private:
    class MemoryFileStream;

  public:
    MemorySoundArchive();
    /* 0x08 */ virtual ~MemorySoundArchive();

    /* 0x0C */ virtual const void* detail_GetFileAddress(u32 id) const;
    /* 0x10 */ virtual const void* detail_GetWaveDataFileAddress(u32 id) const;

    /* 0x14 */ virtual int detail_GetRequiredStreamBufferSize() const;

    /* 0x18 */ virtual ut::FileStream* OpenStream(void* buffer, int size, u32 offset, u32 length) const;

    virtual ut::FileStream* OpenExtStream(void* buffer, int size, const char* extPath, u32 offset,
                                          /* 0x1C */ u32 length) const;

    bool Setup(const void* buffer);
    void Shutdown();

  private:
    /* 0x108 */ const void* mData;
    /* 0x10C */ detail::SoundArchiveFileReader mFileReader;
};
} // namespace snd
} // namespace nw4hbm

#endif
