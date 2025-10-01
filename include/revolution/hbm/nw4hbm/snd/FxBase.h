#ifndef NW4R_SND_FX_BASE_H
#define NW4R_SND_FX_BASE_H

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/global.h"
#include "revolution/hbm/nw4hbm/snd/types.h"

#include "revolution/hbm/nw4hbm/ut.h"

namespace nw4hbm {
namespace snd {
class FxBase : ut::NonCopyable {
public:
    virtual ~FxBase() {} // 0x08

    /* 0x0C */ virtual bool StartUp() { return true; }
    virtual void Shutdown() {} // 0x10

    virtual void UpdateBuffer(int channels, void** buffer, u32 size, SampleFormat format, f32 sampleRate,
                              OutputMode mode) {} // 0x14

public:
    /* 0x04 */ ut::LinkListNode mFxLink;
};

typedef ut::LinkList<FxBase, offsetof(FxBase, mFxLink)> FxList;
} // namespace snd
} // namespace nw4hbm

#endif
