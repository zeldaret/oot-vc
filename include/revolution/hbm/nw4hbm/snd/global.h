#ifndef NW4R_SND_GLOBAL_H
#define NW4R_SND_GLOBAL_H

#include "revolution/types.h" // f32

namespace nw4hbm {
namespace snd {

enum PanMode {
    PAN_MODE_DUAL,
    PAN_MODE_BALANCE,
};

enum PanCurve {
    PAN_CURVE_SQRT,
    PAN_CURVE_SQRT_0DB,
    PAN_CURVE_SQRT_0DB_CLAMP,

    PAN_CURVE_SINCOS,
    PAN_CURVE_SINCOS_0DB,
    PAN_CURVE_SINCOS_0DB_CLAMP,

    PAN_CURVE_LINEAR,
    PAN_CURVE_LINEAR_0DB,
    PAN_CURVE_LINEAR_0DB_CLAMP,
};

struct VoiceOutParam {
    // methods
  public:
    // cdtors
    VoiceOutParam() : volume(1.0f), pitch(1.0f), pan(0.0f), surroundPan(0.0f), fxSend(0.0f), lpf(0.0f) {}

    // members
  public:
    f32 volume; // size 0x04, offset 0x00
    f32 pitch; // size 0x04, offset 0x04
    f32 pan; // size 0x04, offset 0x08
    f32 surroundPan; // size 0x04, offset 0x0c
    f32 fxSend; // size 0x04, offset 0x10
    f32 lpf; // size 0x04, offset 0x14
}; // size 0x18
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_GLOBAL_H
