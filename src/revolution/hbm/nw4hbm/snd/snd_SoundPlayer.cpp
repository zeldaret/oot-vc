#include "revolution/hbm/nw4hbm/snd/snd_SoundPlayer.hpp"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_SoundPlayer.cpp
 */

/*******************************************************************************
 * headers
 */

#include <limits.h> // INT_MAX

#include <decomp.h>
#include "macros.h"
#include "revolution/types.h" // nullptr

#include "revolution/hbm/nw4hbm/snd/snd_BasicSound.hpp"
#include "revolution/hbm/nw4hbm/snd/global.h" // AUX_BUS_NUM
#include "revolution/hbm/nw4hbm/snd/snd_PlayerHeap.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_SoundThread.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // ut::Clamp

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace snd {

SoundPlayer::SoundPlayer() :
	mPlayableCount		(1),
	mPlayableLimit		(INT_MAX),
	mVolume				(1.0f),
	mLpfFreq			(0.0f),
	mOutputLineFlag		(1),
	mMainOutVolume		(1.0f),
	mBiquadType			(0),
	mBiquadValue		(0.0f),
	mMainSend			(0.0f)
{
	for (int i = 0; i < AUX_BUS_NUM; i++)
		mFxSend[i] = 0.0f;
}

SoundPlayer::~SoundPlayer()
{
	StopAllSound(0);
}

void SoundPlayer::Update()
{
	detail::SoundThread::AutoLock lock;

	NW4R_RANGE_FOR_NO_AUTO_INC(itr, mSoundList)
	{
		DECLTYPE(itr) curItr = itr++;

		curItr->Update();
	}

	detail_SortPriorityList();
}

void SoundPlayer::StopAllSound(int fadeFrames)
{
	detail::SoundThread::AutoLock lock;

	NW4R_RANGE_FOR_NO_AUTO_INC(itr, mSoundList)
	{
		DECLTYPE(itr) curItr = itr++;

		curItr->Stop(fadeFrames);
	}
}

void SoundPlayer::PauseAllSound(bool flag, int fadeFrames)
{
	detail::SoundThread::AutoLock lock;

	NW4R_RANGE_FOR_NO_AUTO_INC(itr, mSoundList)
	{
		DECLTYPE(itr) curItr = itr++;

		curItr->Pause(flag, fadeFrames);
	}
}

// SoundPlayer::SetVolume ([R89JEL]:/bin/RVL/Debug/mainD.MAP:14397)
/* also __FILE__ because this is the first assert in the file so it needs to
 * reference that first as well
 */
DECOMP_FORCE(__FILE__);
DECOMP_FORCE(NW4RAssert_String(volume >= 0.0f));

// SoundPlayer::SetFxSend ([R89JEL]:/bin/RVL/Debug/mainD.MAP:14405)
DECOMP_FORCE(NW4RAssertHeaderClampedLValue_String(bus));

void SoundPlayer::RemoveSoundList(detail::BasicSound *sound)
{
	detail::SoundThread::AutoLock lock;

	mSoundList.Erase(sound);
	sound->DetachSoundPlayer(this);
}

void SoundPlayer::InsertPriorityList(detail::BasicSound *sound)
{
	detail::SoundThread::AutoLock lock;

	DECLTYPE(mPriorityList.GetBeginIter()) itr = mPriorityList.GetBeginIter();
	for (; itr != mPriorityList.GetEndIter(); ++itr)
	{
		if (sound->CalcCurrentPlayerPriority()
		    < itr->CalcCurrentPlayerPriority())
		{
			break;
		}
	}

	mPriorityList.Insert(itr, sound);
}

void SoundPlayer::RemovePriorityList(detail::BasicSound *sound)
{
	detail::SoundThread::AutoLock lock;

	mPriorityList.Erase(sound);
}

void SoundPlayer::detail_SortPriorityList(detail::BasicSound *sound)
{
	RemovePriorityList(sound);
	InsertPriorityList(sound);
}

void SoundPlayer::detail_SortPriorityList()
{
	detail::SoundThread::AutoLock lock;

	if (mPriorityList.GetSize() < 2)
		return;

	static detail::BasicSound::SoundPlayerPriorityLinkList
		tmplist[detail::BasicSound::PRIORITY_MAX + 1];

	while (!mPriorityList.IsEmpty())
	{
		detail::BasicSound &front = mPriorityList.GetFront();
		mPriorityList.PopFront();
		tmplist[front.CalcCurrentPlayerPriority()].PushBack(&front);
	}

	for (int i = 0; i < (int)ARRAY_COUNT(tmplist); i++)
	{
		while (!tmplist[i].IsEmpty())
		{
			detail::BasicSound &front = tmplist[i].GetFront();

			tmplist[i].PopFront();
			mPriorityList.PushBack(&front);
		}
	}
}

bool SoundPlayer::detail_AppendSound(detail::BasicSound *sound)
{
	NW4HBMAssertPointerNonnull_Line(sound, 402);

	detail::SoundThread::AutoLock lock;

	int allocPriority = sound->CalcCurrentPlayerPriority();

	if (GetPlayableSoundCount() == 0)
		return false;

	while (GetPlayingSoundCount() >= GetPlayableSoundCount())
	{
		detail::BasicSound *dropSound = GetLowestPrioritySound();
		if (!dropSound)
			return false;

		if (allocPriority < dropSound->CalcCurrentPlayerPriority())
			return false;

		dropSound->Shutdown();
	}

	mSoundList.PushBack(sound);
	InsertPriorityList(sound);
	sound->AttachSoundPlayer(this);

	return true;
}

void SoundPlayer::detail_RemoveSound(detail::BasicSound *sound)
{
	RemovePriorityList(sound);
	RemoveSoundList(sound);
}

void SoundPlayer::SetPlayableSoundCount(int count)
{
	NW4HBMAssert_Line(count >= 0, 453);

	detail::SoundThread::AutoLock lock;

	NW4HBMCheckMessage_Line(458, count <= mPlayableLimit,
	                      "playable sound count is over limit.");

	mPlayableCount = ut::Clamp(count, 0, mPlayableLimit);

	while (GetPlayingSoundCount() > GetPlayableSoundCount())
	{
		detail::BasicSound *dropSound = GetLowestPrioritySound();
		NW4HBMAssertPointerNonnull_Line(dropSound, 467);

		dropSound->Shutdown();
	}
}

void SoundPlayer::detail_SetPlayableSoundLimit(int limit)
{
	NW4HBMAssert_Line(limit >= 0, 483);

	mPlayableLimit = limit;
}

bool SoundPlayer::detail_CanPlaySound(int startPriority)
{
	detail::SoundThread::AutoLock lock;

	if (GetPlayableSoundCount() == 0)
		return false;

	if (GetPlayingSoundCount() >= GetPlayableSoundCount())
	{
		detail::BasicSound *dropSound = GetLowestPrioritySound();
		if (!dropSound)
			return false;

		if (startPriority < dropSound->CalcCurrentPlayerPriority())
			return false;
	}

	return true;
}

void SoundPlayer::detail_AppendPlayerHeap(detail::PlayerHeap *heap)
{
	NW4HBMAssertPointerNonnull_Line(heap, 524);

	detail::SoundThread::AutoLock lock;

	heap->AttachSoundPlayer(this);
	mHeapList.PushBack(heap);
}

detail::PlayerHeap *SoundPlayer::detail_AllocPlayerHeap(detail::BasicSound *sound)
{
	NW4HBMAssertPointerNonnull_Line(sound, 557);

	detail::SoundThread::AutoLock lock;

	if (mHeapList.IsEmpty())
		return nullptr;

	detail::PlayerHeap &playerHeap = mHeapList.GetFront();
	mHeapList.PopFront();

	playerHeap.AttachSound(sound);
	sound->AttachPlayerHeap(&playerHeap);
	playerHeap.Clear();

	return &playerHeap;
}

void SoundPlayer::detail_FreePlayerHeap(detail::BasicSound *sound)
{
	NW4HBMAssertPointerNonnull_Line(sound, 587);

	detail::SoundThread::AutoLock lock;

	detail::PlayerHeap *heap = sound->GetPlayerHeap();
	if (!heap)
		return;

	heap->DetachSound(sound);
	sound->DetachPlayerHeap(heap);
	mHeapList.PushBack(heap);
}

}} // namespace nw4hbm::snd
