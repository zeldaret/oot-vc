#include "revolution/hbm/nw4hbm/snd/StrmSound.h"

/* Original source:
 * kiwi515/ogws
 * src/nw4r/snd/snd_StrmSound.cpp
 */

/*******************************************************************************
 * headers
 */

#include "macros.h" // ARRAY_COUNT
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/BasicSound.h"
#include "revolution/hbm/nw4hbm/snd/SoundInstanceManager.h"
#include "revolution/hbm/nw4hbm/snd/StrmPlayer.h"
#include "revolution/hbm/nw4hbm/snd/snd_StrmSoundHandle.hpp"
#include "revolution/hbm/nw4hbm/snd/snd_MoveValue.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * types
 */

// forward declarations
namespace nw4hbm { namespace snd { namespace detail { class StrmBufferPool; }}}

namespace nw4hbm { namespace ut { class FileStream; }}

/*******************************************************************************
 * variables
 */

namespace nw4hbm { namespace snd { namespace detail
{
	// .sbss
	ut::detail::RuntimeTypeInfo const StrmSound::typeInfo(
		&BasicSound::typeInfo);
}}} // namespace nw4hbm::snd::detail

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace snd { namespace detail {

StrmSound::StrmSound(SoundInstanceManager<StrmSound> *manager, int priority,
                     int ambientPriority) :
	BasicSound			(priority, ambientPriority),
	mTempSpecialHandle	(nullptr),
	mManager			(manager)
{
}

void StrmSound::InitParam()
{
	BasicSound::InitParam();

	for (int i = 0; i < ARRAY_COUNT(mTrackVolume); i++)
	{
		mTrackVolume[i].InitValue(0.0f);
		mTrackVolume[i].SetTarget(1.0f, 1);
	}
}

StrmPlayer::SetupResult StrmSound::Setup(StrmBufferPool *bufferPool,
                                         int allocChannelCount,
                                         byte2_t allocTrackFlag)
{
	NW4HBMAssertPointerNonnull_Line(bufferPool, 90);

	InitParam();

	return mStrmPlayer.Setup(bufferPool, allocChannelCount, allocTrackFlag,
	                         GetVoiceOutCount());
}

bool StrmSound::Prepare(StrmPlayer::StartOffsetType startOffsetType, s32 offset,
                        ut::FileStream *fileStream)
{
	bool result = mStrmPlayer.Prepare(fileStream, startOffsetType, offset);
	if (!result)
	{
		mStrmPlayer.Shutdown();
		return false;
	}

	return true;
}

void StrmSound::UpdateMoveValue()
{
	BasicSound::UpdateMoveValue();

	for (int trackNo = 0; trackNo < (int)ARRAY_COUNT(mTrackVolume); trackNo++)
	{
		if (mStrmPlayer.GetPlayerTrack(trackNo))
			mTrackVolume[trackNo].Update();
	}
}

void StrmSound::UpdateParam()
{
	BasicSound::UpdateParam();

	for (int trackNo = 0; trackNo < (int)ARRAY_COUNT(mTrackVolume); trackNo++)
	{
		if (mStrmPlayer.GetPlayerTrack(trackNo))
		{
			mStrmPlayer.SetTrackVolume(1 << trackNo,
			                           mTrackVolume[trackNo].GetValue());
		}
	}
}

void StrmSound::Shutdown()
{
	BasicSound::Shutdown();

	mManager->Free(this);
}

void StrmSound::OnUpdatePlayerPriority()
{
	mManager->UpdatePriority(this, CalcCurrentPlayerPriority());
}

bool StrmSound::IsAttachedTempSpecialHandle()
{
	return mTempSpecialHandle != nullptr;
}

void StrmSound::DetachTempSpecialHandle()
{
	mTempSpecialHandle->DetachSound();
}

}}} // namespace nw4hbm::snd::detail
