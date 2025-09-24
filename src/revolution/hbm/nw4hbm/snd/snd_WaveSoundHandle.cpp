#include "revolution/hbm/nw4hbm/snd/snd_WaveSoundHandle.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_WaveSoundHandle.cpp
 */

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // nullptr

#include "revolution/hbm/nw4hbm/snd/snd_BasicSound.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_SoundHandle.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_WaveSound.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp" // ut::DynamicCast

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace snd {

WaveSoundHandle::WaveSoundHandle(SoundHandle *handle) :
	mSound	(nullptr)
{
	if (!handle)
		return;

	detail::BasicSound *basicSound = handle->detail_GetAttachedSound();
	if (!basicSound)
		return;

	if (detail::WaveSound *sound =
	        ut::DynamicCast<detail::WaveSound *>(basicSound))
	{
		NW4HBMAssertPointerNonnull_Line(sound, 50); // ?

		mSound = sound;

		if (mSound->IsAttachedTempSpecialHandle())
			mSound->DetachTempSpecialHandle();

		mSound->mTempSpecialHandle = this;
	}
}

void WaveSoundHandle::DetachSound()
{
	if (IsAttachedSound())
	{
		if (mSound->mTempSpecialHandle == this)
			mSound->mTempSpecialHandle = nullptr;
	}

	if (mSound)
		mSound = nullptr;
}

}} // namespace nw4hbm::snd
