#ifndef NW4R_SND_SEQ_SOUND_H
#define NW4R_SND_SEQ_SOUND_H

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/snd/debug.h"
// #include "revolution/hbm/nw4hbm/snd/Task.h"

#include "revolution/hbm/nw4hbm/ut/LinkList.h"

namespace nw4hbm {
namespace snd {

class SeqSoundHandle;

namespace detail {
class NoteOnCallback;
class SeqTrackAllocator;
class SeqPlayer;
class BasicSound;
template <typename T> class SoundInstanceManager;
} // namespace detail

namespace detail {
class SeqSound : public BasicSound {
    friend class SeqSoundHandle;

  public:
    NW4HBM_UT_RUNTIME_TYPEINFO;
    typedef void (*NotifyAsyncEndCallback)(bool result, const void* seqBase, s32 seqOffset, void* userData);

    class SeqLoadCallback {
      public:
        typedef enum Result {
            RESULT_SUCCESS = 0,
            RESULT_FAILED,
            RESULT_CANCELED,
            RESULT_ASYNC,
            RESULT_RETRY
        } Result;

        virtual ~SeqLoadCallback() {}

        virtual Result LoadData(SeqSound::NotifyAsyncEndCallback callback, void* callbackArg, u32 userData) const = 0;
        virtual void CancelLoading(u32 userData) const = 0;
    };
    explicit SeqSound(SoundInstanceManager<SeqSound>* nanager);

    /* 0x28 */ virtual void Shutdown();

    /* 0x4C */ virtual void SetPlayerPriority(int priority);

    /* 0x5C */ virtual bool IsAttachedTempSpecialHandle();
    /* 0x60 */ virtual void DetachTempSpecialHandle();

    /* 0x64 */ virtual void InitParam();

    virtual BasicPlayer& GetBasicPlayer() { return mSeqPlayer; } // 0x68
    virtual const BasicPlayer& GetBasicPlayer() const { return mSeqPlayer; } // 0x6C

    SeqPlayer::SetupResult Setup(SeqTrackAllocator* trackAllocator, u32 allocTrackFlags, int voices,
                                 NoteOnCallback* callback);

    void Prepare(const void* seqBase, s32 seqOffset);
    void Prepare(const SeqLoadCallback* callback, u32 callbackData);

    void SetChannelPriority(int priority);

  private:
    static void NotifyLoadAsyncEndSeqData(bool result, const void* seqBase, s32 seqOffset, void* userData);

    /* 0xD8 */ SeqPlayer mSeqPlayer;
    /* 0x1F4 */ SeqSoundHandle* mTempSpecialHandle;
    /* 0x1F8 */ SoundInstanceManager<SeqSound>* mManager;

    /* 0x1E8 */ bool mLoadingFlag;
    /* 0x1EC */ const SeqLoadCallback* mCallback;
    /* 0x1F0 */ u32 mCallbackData;
};
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
