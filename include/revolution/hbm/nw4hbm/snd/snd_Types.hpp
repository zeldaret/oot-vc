#ifndef NW4R_SND_TYPES_H
#define NW4R_SND_TYPES_H

#include "revolution/types.h"
#include "revolution/hbm/nw4hbm/ut/LinkList.h"

namespace nw4hbm {
namespace snd {

static const int CHANNEL_MIN = 1;
static const int CHANNEL_MAX = 2;

static const int THREAD_STACK_SIZE = 1024;
static const int STREAM_BUFFER_SIZE = 512;

static const int REMOTE_FILTER_MAX = 127;

// Volume in range [-90.4db, 6.0db]
static const f32 VOLUME_MIN_DB = -90.4f;
static const f32 VOLUME_MAX_DB = 6.0f;
static const f32 VOLUME_RANGE_DB = -(VOLUME_MIN_DB - VOLUME_MAX_DB);
static const int VOLUME_RANGE_MB = static_cast<int>(10 * VOLUME_RANGE_DB);

enum OutputLineFlag {
    OUTPUT_LINE_MAIN = (1 << 0),
    OUTPUT_LINE_REMOTE_N = (1 << 1),
};

class LinkedInstance {
  public:
    ut::LinkListNode mInstanceLink; // 0x00
};

namespace detail {

typedef struct AdpcmParam {
    u16 coef[16]; // 0x00
    u16 gain; // 0x20
    u16 pred_scale; // 0x22
    u16 yn1; // 0x24
    u16 yn2; // 0x26
} AdpcmParam;

typedef struct AdpcmLoopParam {
    u16 loop_pred_scale; // 0x00
    u16 loop_yn1; // 0x02
    u16 loop_yn2; // 0x04
} AdpcmLoopParam;

typedef struct AdpcmInfo {
    AdpcmParam adpcm; // 0x08
    AdpcmLoopParam adpcmloop; // 0x28
    u16 padding; // 0x2E
} AdpcmInfo;

typedef struct VoiceChannelParam {
    void* waveData; // 0x00
    AdpcmInfo adpcmInfo; // 0x04
} VoiceChannelParam;

typedef struct SoundParam {
    f32 volume; // 0x00
    f32 pitch; // 0x04
    f32 pan; // 0x08
    f32 surroundPan; // 0x0C
    f32 fxSend; // 0x10
    f32 lpf; // 0x14
    s32 priority; // 0x18
} SoundParam;

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
