#ifndef NW4R_SND_FX_BASE_H
#define NW4R_SND_FX_BASE_H
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/global.h"
#include "revolution/hbm/nw4hbm/snd/snd_Types.hpp"

#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace snd {

class FxBase : private ut::NonCopyable {
	// typedefs
	public:
		typedef ut::LinkList<FxBase, 0x04> LinkList;

  public:
    virtual ~FxBase() {} // at 0x8

    virtual bool StartUp() { return true; } // at 0xC

    virtual void Shutdown() {} // at 0x10

    virtual void UpdateBuffer(int /* channels */, void** /* ppBuffer */, u32 /* size */, SampleFormat /* format */,
                              f32 /* sampleRate */, OutputMode /* mode */) {} // at 0x14
    // virtual void OnChangeOutputMode();

	public:
		ut::LinkListNode	mFxLink;	// size 0x08, offset 0x04
};

} // namespace snd
} // namespace nw4hbm

#endif
