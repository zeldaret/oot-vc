#ifndef NW4R_SND_WAVE_FILE_H
#define NW4R_SND_WAVE_FILE_H
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/AxVoice.h"
#include "revolution/hbm/nw4hbm/snd/snd_Types.hpp"

#include "revolution/hbm/ut.hpp"


namespace nw4hbm {
namespace snd {
namespace detail {
namespace WaveFile {
typedef enum Format {
    FORMAT_PCM8 = 0,
    FORMAT_PCM16,
    FORMAT_ADPCM
} Format;

typedef struct WaveInfo {
    u8 format; // 0x00
    u8 loopFlag; // 0x01
    u8 numChannels; // 0x02
    u8 sampleRate24; // 0x03
    u16 sampleRate; // 0x04
    u16 padding2; // 0x06
    u32 loopStart; // 0x08
    u32 loopEnd; // 0x0C
    u32 channelInfoTableOffset; // 0x10
    u32 dataOffset; // 0x14
    u32 reserved; // 0x18
} WaveInfo;

typedef struct WaveChannelInfo {
    u32 channelDataOffset; // 0x00
    u32 adpcmOffset; // 0x04
    u32 volumeFrontLeft; // 0x08
    u32 volumeFrontRight; // 0x0C
    u32 volumeRearLeft; // 0x10
    u32 volumeRearRight; // 0x14
    u32 reserved; // 0x18
} WaveChannelInfo;
} // namespace WaveFile

typedef struct ChannelParam {
    void* dataAddr; // 0x00
    u32 volumeFrontLeft; // 0x04
    u32 volumeFrontRight; // 0x08
    u32 volumeRearLeft; // 0x0C
    u32 volumeRearRight; // 0x10
    AdpcmInfo adpcmInfo; // 0x14
} ChannelParam;

typedef struct WaveData {
    u8 format; // 0x00
    u8 loopFlag; // 0x01
    u8 numChannels; // 0x02
    int sampleRate; // 0x04
    u32 loopStart; // 0x08
    u32 loopEnd; // 0x0C
    ChannelParam channelParam[CHANNEL_MAX]; // 0x10
} WaveData;

// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x2e58c
struct WaveSoundInfo {
    f32 pitch; // size 0x04, offset 0x00
    u8 pan; // size 0x01, offset 0x04
    u8 surroundPan; // size 0x01, offset 0x05
    u8 fxSendA; // size 0x01, offset 0x06
    u8 fxSendB; // size 0x01, offset 0x07
    u8 fxSendC; // size 0x01, offset 0x08
    u8 mainSend; // size 0x01, offset 0x09
    /* 2 bytes padding */
}; // size 0x0c

// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x2e6c2
struct WaveSoundNoteInfo {
    s32 waveIndex; // size 0x04, offset 0x00
    u8 attack; // size 0x01, offset 0x04
    u8 hold; // size 0x01, offset 0x05
    u8 decay; // size 0x01, offset 0x06
    u8 sustain; // size 0x01, offset 0x07
    u8 release; // size 0x01, offset 0x08
    u8 originalKey; // size 0x01, offset 0x09
    u8 pan; // size 0x01, offset 0x0a
    u8 surroundPan; // size 0x01, offset 0x0b
    u8 volume; // size 0x01, offset 0x0c
    /* 3 bytes padding */
    f32 pitch; // size 0x04, offset 0x10
}; // size 0x14
} // namespace detail
} // namespace snd
} // namespace nw4hbm

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace snd {
namespace detail {
// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x2b2e05
class WaveFileReader {
    // methods
  public:
    explicit WaveFileReader(const WaveFile::WaveInfo* waveInfo);

    bool ReadWaveParam(WaveData* waveData, const void* waveAddr) const;
    static AxVoice::Format WaveFormatToAxFormat(u32 format);

    static SampleFormat GetSampleFormatFromWaveFileFormat(u8 format);

    // members
  private:
    const WaveFile::WaveInfo* mWaveInfo; // 0x00
};

inline AxVoice::Format WaveFormatToAxFormat(u32 format) {
    if (format == WaveFile::FORMAT_PCM16) {
        return AxVoice::FORMAT_PCM16;
    }
    if (format == WaveFile::FORMAT_PCM8) {
        return AxVoice::FORMAT_PCM8;
    }
    return AxVoice::FORMAT_ADPCM;
}
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
