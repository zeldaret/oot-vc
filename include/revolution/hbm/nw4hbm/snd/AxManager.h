#ifndef NW4R_SND_AX_MANAGER_H
#define NW4R_SND_AX_MANAGER_H

#include "revolution/hbm/nw4hbm/snd/FxBase.h"

#include "revolution/hbm/nw4hbm/snd/AxVoice.h"
#include "revolution/hbm/nw4hbm/snd/MoveValue.h"

#include "revolution/ai.h"
#include "revolution/ax.h"

namespace nw4hbm {
namespace snd {
namespace detail {
class AxManager {
public:
    typedef struct CallbackListNode {
        /* 0x00 */ ut::LinkListNode link;
        /* 0x08 */ AXOutCallback callback;
    } CallbackListNode;

    typedef ut::LinkList<CallbackListNode, offsetof(CallbackListNode, link)> CallbackList;

    static AxManager& GetInstance();

    void Init();
    void Shutdown();
    void Update();

    bool CheckInit() { return mInitialized; }

    bool IsDiskError() const { return mDiskErrorFlag; }
    bool IsHomeButtonMenu() const { return mHomeButtonMuteFlag; }

    bool IsResetReady() const { return mResetReadyCounter == 0; }

    f32 GetOutputVolume() const;
    void* GetZeroBufferAddress();

    void RegisterCallback(CallbackListNode* node, AXOutCallback callback);
    void UnregisterCallback(CallbackListNode* node);

    void SetOutputMode(OutputMode mode);
    OutputMode GetOutputMode();

    void UpdateAllVoicesPriority();
    void UpdateAllVoices();
    void UpdateAllVoicesSync(u32 syncFlag);

    f32 GetMasterVolume() const { return mMasterVolume.GetValue(); }
    void SetMasterVolume(f32 volume, int frame);

    bool AppendEffect(AuxBus bus, FxBase* fx);
    void ClearEffect(AuxBus bus, int frame);
    void ShutdownEffect(AuxBus bus);

    FxList& GetEffectList(AuxBus bus) {
        NW4HBMAssertHeaderClampedLValue_Line(bus, AUX_A, AUX_BUS_NUM, 189);
        return mFxList[bus];
    }

    void AppendVoiceList(AxVoice* voice);
    void RemoveVoiceList(AxVoice* voice);

    AxVoice* AllocVoice(int channels, int voices, int priority, AxVoice::AxVoiceCallback callback, void* callbackData);
    void FreeVoice(AxVoice* voice);

    void ChangeVoicePriority(AxVoice* voice);
    void LockUpdateVoicePriority();
    void UnlockUpdateVoicePriority();

    int DropLowestPriorityVoice(int priority);

    AxVoiceList& GetVoiceList() { return mPrioVoiceList; }

private:
    static const u8 AUX_CALLBACK_WAIT_FRAME = 6;

    static const int FX_SAMPLE_RATE = AX_SAMPLE_RATE;
    static const SampleFormat FX_SAMPLE_FORMAT = SAMPLE_FORMAT_PCM_S32;
    static const int FX_BUFFER_SIZE = AX_FRAME_SIZE;

    static const int ZERO_BUFFER_SIZE = 256;

    AxManager();

    static void AxCallbackFunc();
    static void AuxCallbackFunc(void* chans, void* context);

    /* 0x00 */ OutputMode mOutputMode;

    /* 0x04 */ void* mZeroBufferAddress;

    /* 0x08 */ CallbackList mCallbackList;
    /* 0x14 */ AxVoiceList mPrioVoiceList;
    /* 0x20 */ AxVoiceList mFreeVoiceList;

    /* 0x2C */ AxVoice mVoices[AX_VOICE_MAX];

    /* 0xA22C */ AXOutCallback mNextAxRegisterCallback;

    /* 0xA230 */ bool mInitialized;

    /* 0xA231 */ bool mUpdateVoicePrioFlag;
    /* 0xA232 */ bool mHomeButtonMuteFlag;
    /* 0xA233 */ bool mDiskErrorFlag;

    /* 0xA234 */ MoveValue<f32, int> mHomeButtonMenuVolume;
    /* 0xA244 */ MoveValue<f32, int> mMasterVolume;
    /* 0xA254 */ MoveValue<f32, int> mVolumeForReset;

    /* 0xA264 */ AIDMACallback mOldAidCallback;

    /* 0xA268 */ vs32 mResetReadyCounter;

    /* 0xA26C */ MoveValue<f32, int> mAuxFadeVolume[AUX_BUS_NUM];
    /* 0xA29C */ MoveValue<f32, int> mAuxUserVolume[AUX_BUS_NUM];

    /* 0xA2CC */ FxList mFxList[AUX_BUS_NUM];

    /* 0xA2F0 */ AXAuxCallback mAuxCallback[AUX_BUS_NUM];
    /* 0xA2FC */ void* mAuxCallbackContext[AUX_BUS_NUM];
    /* 0xA308 */ u8 mAuxCallbackWaitCounter[AUX_BUS_NUM];

    static u8 sZeroBuffer[ZERO_BUFFER_SIZE];
};
} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
