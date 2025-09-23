#ifndef NW4R_SND_SOUND_INSTANCE_MANAGER_H
#define NW4R_SND_SOUND_INSTANCE_MANAGER_H

/*******************************************************************************
 * headers
 */

#include <new.hpp>

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/snd_InstancePool.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_lock.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"

#include "revolution/os/OSMutex.h"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm { namespace snd { namespace detail
{
	// [R89JEL]:/bin/RVL/Debug/mainD.elf:.debug::0x2f03c, 0x30a63...
	template <class Sound>
	class SoundInstanceManager
	{
	// methods
	public:
		// cdtors
		SoundInstanceManager() { OSInitMutex(&mMutex); }

		// methods
		u32 Create(void *buffer, u32 size)
		{
			NW4HBMAssertPointerNonnull_Line(buffer, 67);

			ut::detail::AutoLock<OSMutex> lock(mMutex);

			return mPool.Create(buffer, size);
		}

		void Destroy(void *buffer, u32 size)
		{
			NW4HBMAssertPointerNonnull_Line(buffer, 86);

			ut::detail::AutoLock<OSMutex> lock(mMutex);

			mPool.Destroy(buffer, size);
		}

		Sound *Alloc(int priority, int ambientPriority)
		{
			int allocPriority =
				ut::Clamp(priority + ambientPriority, Sound::PRIORITY_MIN,
				          Sound::PRIORITY_MAX);

			ut::detail::AutoLock<OSMutex> lock(mMutex);
			Sound *sound = nullptr;

			while (!sound)
			{
				if (void *ptr = mPool.Alloc())
				{
					sound = new (ptr) Sound(this, priority, ambientPriority);
					continue;
				}

				Sound *sound = GetLowestPrioritySound();
				if (!sound)
					return nullptr;

				if (allocPriority < sound->CalcCurrentPlayerPriority())
					return nullptr;

				OSUnlockMutex(&mMutex);

				NW4HBMCheckMessage_Line(133,
					!Debug_GetWarningFlag(
						Debug_GetDebugWarningFlagFromSoundType(
							sound->GetSoundType())),
					"Sound (id:%d) is stopped for not enough %s sound "
					"instance.",
					sound->GetId(),
					Debug_GetSoundTypeString(sound->GetSoundType()));

				sound->Stop(0);

				OSLockMutex(&mMutex);
			}

			InsertPriorityList(sound, allocPriority);
			return sound;
		}

		void Free(Sound *sound)
		{
			NW4HBMAssertPointerNonnull_Line(sound, 158);

			ut::detail::AutoLock<OSMutex> lock(mMutex);

			if (mPriorityList.IsEmpty())
				return;

			RemovePriorityList(sound);
			sound->~Sound();
			mPool.Free(sound);
		}

		u32 GetActiveCount() const { return mPriorityList.GetSize(); }

		Sound *GetLowestPrioritySound()
		{
			if (mPriorityList.IsEmpty())
				return nullptr;

			return &mPriorityList.GetFront();
		}

		void InsertPriorityList(Sound *sound, int priority)
		{
			DECLTYPE(mPriorityList.GetBeginIter()) itr =
				mPriorityList.GetBeginIter();

			for (; itr != mPriorityList.GetEndIter(); ++itr)
			{
				if (priority < itr->CalcCurrentPlayerPriority())
					break;
			}

			mPriorityList.Insert(itr, sound);
		}

		void SortPriorityList()
		{
			int const TMP_NUM = Sound::PRIORITY_MAX + 1;

			if (mPriorityList.GetSize() < 2)
				return;

			ut::detail::AutoLock<OSMutex> lock(mMutex);

			typename Sound::PriorityLinkList tmpList[TMP_NUM];

			while (!mPriorityList.IsEmpty())
			{
				Sound &front = mPriorityList.GetFront();
				mPriorityList.PopFront();

				tmpList[front.CalcCurrentPlayerPriority()].PushBack(&front);
			}

			for (int i = 0; i < TMP_NUM; i++)
			{
				while (!tmpList[i].IsEmpty())
				{
					Sound &front = tmpList[i].GetFront();
					tmpList[i].PopFront();

					mPriorityList.PushBack(&front);
				}
			}
		}

		void RemovePriorityList(Sound *sound) { mPriorityList.Erase(sound); }

		void UpdatePriority(Sound *sound, int priority)
		{
			ut::detail::AutoLock<OSMutex> lock(mMutex);

			RemovePriorityList(sound);
			InsertPriorityList(sound, priority);
		}

	// members
	private:
		MemoryPool<Sound>					mPool;			// size 0x04, offset 0x00
		typename Sound::PriorityLinkList	mPriorityList;	// size 0x0c, offset 0x04
		mutable OSMutex						mMutex;			// size 0x18, offset 0x10
	}; // size 0x28
}}} // namespace nw4hbm::snd::detail

#endif // NW4R_SND_SOUND_INSTANCE_MANAGER_H
