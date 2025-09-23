#ifndef NW4R_SND_SOUND_ACTOR_H
#define NW4R_SND_SOUND_ACTOR_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // nullptr

#include "revolution/hbm/nw4hbm/snd/snd_BasicSound.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_ExternalSoundPlayer.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_SoundStartable.hpp"

/*******************************************************************************
 * types
 */

// forward declarations
namespace nw4hbm { namespace snd { class SoundArchivePlayer; }}

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm { namespace snd
{
	// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x278fa
	class SoundActor : public SoundStartable
	{
	// methods
	public:
		// cdtors
		SoundActor();

		// methods
		detail::ExternalSoundPlayer *detail_GetActorPlayer(int actorPlayerId)
		{
			if (actorPlayerId < 0 || ACTOR_PLAYER_COUNT <= actorPlayerId)
				return nullptr;

			return &mActorPlayer[actorPlayerId];
		}

		detail::SoundActorParam const &detail_GetActorParam() const
		{
			return mActorParam;
		}

	// static members
	public:
		static int const ACTOR_PLAYER_COUNT = 4;

	// members
	private:
		/* base SoundStartable */										// size 0x04, offset 0x00
		SoundArchivePlayer			&mSoundArchivePlayer;				// size 0x04, offset 0x04
		detail::ExternalSoundPlayer	mActorPlayer[ACTOR_PLAYER_COUNT];	// size 0x40, offset 0x08
		detail::SoundActorParam		mActorParam;						// size 0x0c, offset 0x48
	}; // size 0x54
}} // namespace nw4hbm::snd

#endif // NW4R_SND_SOUND_ACTOR_H
