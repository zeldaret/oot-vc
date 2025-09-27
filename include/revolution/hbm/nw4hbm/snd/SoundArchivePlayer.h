#ifndef NW4R_SND_SOUND_ARCHIVE_PLAYER_H
#define NW4R_SND_SOUND_ARCHIVE_PLAYER_H

#include "revolution/types.h"

// WARNING: DO NOT REORDER these #include directives, data pooling depends on it

// clang-format off
#include "revolution/hbm/nw4hbm/snd/BasicSound.h" // needed for SoundStartable
#include "revolution/hbm/nw4hbm/snd/PlayerHeap.h" // PlayerHeap needs to be before DisposeCallbackManager

#define MAKE_DTOR_ZERO
#include "revolution/hbm/nw4hbm/snd/DisposeCallbackManager.h" // DisposeCallbackManager needs to be before MmlSeqTrackAllocator
#undef MAKE_DTOR_ZERO

#include "revolution/hbm/nw4hbm/snd/MmlSeqTrackAllocator.h" // MmlSeqTrackAllocator needs to be before NoteOnCallback
#include "revolution/hbm/nw4hbm/snd/NoteOnCallback.h"
#include "revolution/hbm/nw4hbm/snd/MmlParser.h"

#include "revolution/hbm/nw4hbm/snd/SeqPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SeqSound.h"
#include "revolution/hbm/nw4hbm/snd/SoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/SoundInstanceManager.h"
#include "revolution/hbm/nw4hbm/snd/SoundStartable.h"
#include "revolution/hbm/nw4hbm/snd/WsdTrack.h"
#include "revolution/hbm/nw4hbm/snd/WaveSound.h"

#include "revolution/hbm/nw4hbm/snd/StrmSound.h" // StrmSound needs to be before Task
#include "revolution/hbm/nw4hbm/snd/Task.h"
// clang-format on

#include "revolution/hbm/HBMAssert.hpp"

// forward declarations
namespace nw4hbm {
namespace snd {
class SoundMemoryAllocatable;
class SoundPlayer;

namespace detail {
class SeqTrackAllocator;
class SoundArchiveLoader;
} // namespace detail

class SoundArchivePlayer_FileManager {
  public:
    virtual const void* GetFileAddress(u32 id) = 0; // 0x8
    virtual const void* GetFileWaveDataAddress(u32 id) = 0; // 0x8
};

class SoundArchivePlayer : public detail::DisposeCallback, public SoundStartable {
  public:
    SoundArchivePlayer();
    virtual ~SoundArchivePlayer(); // 0x08

    virtual void InvalidateData(const void* start, const void* end); // 0x0C
    virtual void InvalidateWaveData(const void* start, const void* end); // 0x10

    virtual StartResult detail_SetupSound(SoundHandle* handle, u32 id,
                                          detail::BasicSound::AmbientArgInfo* ambientArgInfoInfo,
                                          detail::ExternalSoundPlayer* extPlayer, bool hold,
                                          const StartInfo* startInfo); // 0x28

    virtual u32 detail_ConvertLabelStringToSoundId(const char* label) {
        NW4HBMAssertPointerNonnull_Line(mSoundArchive, 355);
        return mSoundArchive->ConvertLabelStringToSoundId(label);
    } // 0x2C

    bool IsAvailable() const;

    bool Setup(const SoundArchive* arc, void* buffer, u32 mramBufferSize, void* strmBuffer,
               u32 strmBufferSize);

    void Shutdown();

    u32 GetRequiredMemSize(const SoundArchive* arc);
    u32 GetRequiredStrmBufferSize(const SoundArchive* arc);

    void Update();

    const SoundArchive& GetSoundArchive() const;

    SoundPlayer& GetSoundPlayer(u32 playerId);
    SoundPlayer& GetSoundPlayer(int playerId) { return GetSoundPlayer(static_cast<u32>(playerId)); }

    const void* detail_GetFileAddress(u32 id) const;
    const void* detail_GetFileWaveDataAddress(u32 id) const;

    const void* GetGroupAddress(u32 groupId) const;
    void SetGroupAddress(u32 groupId, const void* addr);

    const void* GetGroupWaveDataAddress(u32 groupId) const;
    void SetGroupWaveDataAddress(u32 groupId, const void* addr);

    bool LoadGroup(u32 id, SoundMemoryAllocatable* allocatable, u32 loadBlockSize);
    bool LoadGroup(const char* label, SoundMemoryAllocatable* allocatable, u32 loadBlockSize);

    bool LoadGroup(int id, SoundMemoryAllocatable* allocatable, u32 loadBlockSize) {
        return LoadGroup(static_cast<u32>(id), allocatable, loadBlockSize);
    }
    bool LoadGroup(u32 id, SoundMemoryAllocatable* allocatable, unsigned int loadBlockSize) {
        return LoadGroup(static_cast<u32>(id), allocatable, loadBlockSize);
    }

    u32 GetSoundPlayerCount() const { return mSoundPlayerCount; }

    u32 GetFreeSeqSoundCount() const { return mSeqSoundInstanceManager.GetFreeCount(); }
    u32 GetFreeStrmSoundCount() const { return mStrmSoundInstanceManager.GetFreeCount(); }
    u32 GetFreeWaveSoundCount() const { return mWaveSoundInstanceManager.GetFreeCount(); }

  private:
    class SeqLoadCallback : public detail::SeqSound::SeqLoadCallback {
      public:
        SeqLoadCallback(const SoundArchivePlayer& player);

        virtual Result LoadData(detail::SeqSound::NotifyAsyncEndCallback callback, void* callbackArg,
                                u32 userData) const;
        virtual void CancelLoading(u32 userData) const;

      private:
        const SoundArchivePlayer& mSoundArchivePlayer; // 0x04
        mutable OSMutex mMutex; // 0x08
    };

    class SeqNoteOnCallback : public detail::NoteOnCallback {
      public:
        SeqNoteOnCallback(const SoundArchivePlayer& player) : mSoundArchivePlayer(player) {}

        virtual detail::Channel* NoteOn(detail::SeqPlayer* seqPlayer, int bankNo, const detail::NoteOnInfo& noteOnInfo);

      private:
        const SoundArchivePlayer& mSoundArchivePlayer; // 0x04
    };
    friend class SoundArchivePlayer::SeqNoteOnCallback;

    class StrmCallback : public detail::StrmPlayer::StrmCallback {
      public:
        StrmCallback(const SoundArchivePlayer& player);

        virtual Result LoadHeader(detail::StrmPlayer::NotifyLoadHeaderAsyncEndCallback callback, void* callbackData,
                                  u32 userId, u32 userData) const;
        virtual Result LoadStream(void* mramAddr, u32 size, s32 offset, int numChannels, u32 blockSize,
                                  s32 blockHeaderOffset, bool needUpdateAdpcmLoop,
                                  detail::StrmPlayer::LoadCommand& callback, u32 userId, u32 userData) const;
        virtual void CancelLoading(u32 userId, u32 userData) const;

      private:
        const SoundArchivePlayer& mSoundArchivePlayer; // 0x04
        mutable OSMutex mMutex; // 0x08
    };

    class WsdCallback : public detail::WsdTrack::WsdCallback {
      public:
        WsdCallback(const SoundArchivePlayer& player) : mSoundArchivePlayer(player) {}

        virtual bool GetWaveSoundData(detail::WaveSoundInfo* soundInfo, detail::WaveSoundNoteInfo* noteInfo,
                                      detail::WaveData* waveData, const void* waveSoundData, int index, int noteIndex,
                                      u32 userData) const;

      private:
        const SoundArchivePlayer& mSoundArchivePlayer; // 0x04
    };

    class SeqLoadTask : public detail::Task {
      public:
        SeqLoadTask(detail::SeqSound::NotifyAsyncEndCallback callback, void* callbackArg, const SoundArchive& arc,
                    u32 fileId, u32 dataOffset, SoundHeap& heap, u32 taskId, OSMutex& mutex);

        virtual void Execute();
        virtual void Cancel();

      private:
        detail::SoundArchiveLoader* mLoader; // 0x10
        const SoundArchive& mSoundArchive; // 0x14
        u32 mFileId; // 0x18
        u32 mDataOffset; // 0x1C
        SoundHeap& mHeap; // 0x20

        detail::SeqSound::NotifyAsyncEndCallback mCallback; // 0x24
        void* mCallbackData; // 0x28

        OSMutex& mMutex; // 0x2C
    };

    class StrmHeaderLoadTask : public detail::Task {
      public:
        StrmHeaderLoadTask(detail::StrmPlayer::NotifyLoadHeaderAsyncEndCallback callback, void* callbackData,
                           const SoundArchive& arc, u32 fileId, u32 taskId, OSMutex& mutex);

        virtual void Execute();
        virtual void Cancel();

      private:
        ut::FileStream* mStream; // 0x10
        const SoundArchive& mSoundArchive; // 0x14
        u32 mFileId; // 0x18

        detail::StrmPlayer::NotifyLoadHeaderAsyncEndCallback mCallback; // 0x1C
        void* mCallbackData; // 0x24

        OSMutex& mMutex; // 0x28
    };

    class StrmDataLoadTask : public detail::Task {
      public:
        StrmDataLoadTask(void* addr, u32 size, s32 offset, int numChannels, u32 blockSize, s32 blockHeaderOffset,
                         bool needUpdateAdpcmLoop, detail::StrmPlayer::LoadCommand& callback, const SoundArchive& arc,
                         u32 fileId, u32 taskId, OSMutex& mutex) NO_INLINE;
        virtual void Execute();
        virtual void Cancel();

      private:
        detail::StrmPlayer::LoadCommand* mCallback; // 0x10

        ut::FileStream* mStream; // 0x14
        const SoundArchive& mSoundArchive; // 0x18
        u32 mFileId; // 0x1C
        void* mAddr; // 0x20
        u32 mSize; // 0x24
        s32 mOffset; // 0x28

        s32 mNumChannels; // 0x2C

        u32 mBlockSize; // 0x30
        s32 mBlockHeaderOffset; // 0x34

        bool mNeedUpdateAdpcmLoop; // 0x38
        OSMutex& mMutex; // 0x3C
    };

    bool SetupMram(const SoundArchive* arc, void* buffer, u32 size);

    detail::PlayerHeap* CreatePlayerHeap(void* buffer, u32 bufferSize);

    bool SetupSoundPlayer(const SoundArchive* arc, void** buffer, void* end);

    bool CreateGroupAddressTable(const SoundArchive* arc, void** buffer, void* end);

    bool SetupSeqSound(const SoundArchive* arc, int sounds, void** buffer, void* end);
    bool SetupWaveSound(const SoundArchive* arc, int sounds, void** buffer, void* end);
    bool SetupStrmSound(const SoundArchive* arc, int sounds, void** buffer, void* end);
    bool SetupSeqTrack(const SoundArchive* arc, int tracks, void** buffer, void* end);
    bool SetupStrmBuffer(const SoundArchive* arc, void* buffer, u32 bufferSize);

    StartResult PrepareSeqImpl(detail::SeqSound* sound, const SoundArchive::SoundInfo* soundInfo,
                               const SoundArchive::SeqSoundInfo* seqSoundInfo, int voices);
    StartResult PrepareStrmImpl(detail::StrmSound* sound, const SoundArchive::SoundInfo* soundInfo,
                                const SoundArchive::StrmSoundInfo* strmSoundInfo,
                                SoundStartable::StartInfo::StartOffsetType startOffsetType, int startOffset,
                                int voices);

    StartResult PrepareWaveSoundImpl(detail::WaveSound* sound, const SoundArchive::SoundInfo* soundInfo,
                                     const SoundArchive::WaveSoundInfo* waveSoundInfo, int voices);

    typedef struct Group {
        const void* address; // 0x0
        const void* waveDataAddress; // 0x4
    } Group;
    typedef detail::Util::Table<Group> GroupTable;

    const SoundArchive* mSoundArchive; // 0x10
    GroupTable* mGroupTable; // 0x14

    SeqLoadCallback mSeqLoadCallback; // 0x18
    SeqNoteOnCallback mSeqCallback; // 0x38
    WsdCallback mWsdCallback; // 0x40
    StrmCallback mStrmCallback; // 0x48

    detail::SeqTrackAllocator* mSeqTrackAllocator; // 0x68

    SoundArchivePlayer_FileManager* mFileManager; // 0x6C

    u32 mSoundPlayerCount; // 0x70
    SoundPlayer* mSoundPlayers; // 0x74

    detail::SoundInstanceManager<detail::SeqSound> mSeqSoundInstanceManager; // 0x78
    detail::SoundInstanceManager<detail::StrmSound> mStrmSoundInstanceManager; // 0x88
    detail::SoundInstanceManager<detail::WaveSound> mWaveSoundInstanceManager; // 0x98

    detail::StrmBufferPool mStrmBufferPool; // 0xA8

    detail::MmlParser mMmlParser; // 0xC0
    detail::MmlSeqTrackAllocator mMmlSeqTrackAllocator; // 0xC4

    void* mSetupBufferAddress; // 0xD0
    u32 mSetupBufferSize; // 0xD4
};
} // namespace snd
} // namespace nw4hbm

#endif // NW4R_SND_SOUND_ARCHIVE_PLAYER_H
