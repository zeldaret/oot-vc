#ifndef NW4R_SND_SOUND_HANDLE_H
#define NW4R_SND_SOUND_HANDLE_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_BasicSound.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // ut::NonCopyable

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm { namespace snd
{
	// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x2894f
	class SoundHandle : private ut::NonCopyable
	{
	// methods
	public:
		// cdtors
		SoundHandle() : mSound() {}
		~SoundHandle() { DetachSound(); }

		// methods
		void detail_AttachSound(detail::BasicSound *sound);
		bool IsAttachedSound() const { return mSound != nullptr; }
		detail::BasicSound *detail_GetAttachedSound() { return mSound; }
		void DetachSound();

		u32 GetId() const
		{
			if (IsAttachedSound())
				return mSound->GetId();

			return detail::BasicSound::INVALID_ID;
		}

		void StartPrepared()
		{
			if (IsAttachedSound())
				mSound->StartPrepared();
		}

	// members
	private:
		/* base NonCopyable */			// size 0x00, offset 0x00
		detail::BasicSound	*mSound;	// size 0x04, offset 0x00
	}; // size 0x04
}} // namespace nw4hbm::snd

#endif // NW4R_SND_SOUND_HANDLE_H
