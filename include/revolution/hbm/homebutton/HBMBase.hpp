#ifndef RVL_SDK_HBM_HOMEBUTTON_BASE_HPP
#define RVL_SDK_HBM_HOMEBUTTON_BASE_HPP

#include "revolution/ax.h"
#include "revolution/axfx.h"
#include "revolution/hbm/HBMConfig.h"
#include "revolution/hbm/homebutton/HBMCommon.hpp"
#include "revolution/hbm/homebutton/HBMController.hpp"
#include "revolution/hbm/homebutton/HBMGUIManager.hpp"
#include "revolution/hbm/nw4hbm/lyt/drawInfo.h"
#include "revolution/hbm/nw4hbm/lyt/layout.h"
#include "revolution/hbm/nw4hbm/snd/DvdSoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/MemorySoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/NandSoundArchive.h"
#include "revolution/hbm/nw4hbm/snd/SoundArchivePlayer.h"
#include "revolution/hbm/nw4hbm/snd/SoundPlayer.h"
#include "revolution/hbm/nw4hbm/snd/SoundSystem.h"
#include "revolution/types.h"
#include "revolution/wpad/WPAD.h"

#include "new.hpp"

namespace homebutton {
static void initgx();
static void drawBlackPlate(f32 left, f32 top, f32 right, f32 bottom, GXColor clr);
static u32 get_comma_length(char* pBuf);
static void SpeakerCallback(OSAlarm* alm, OSContext* ctx);
static void MotorCallback(OSAlarm* alm, OSContext* ctx);
static void RetrySimpleSyncCallback(OSAlarm* alm, OSContext* ctx);
static void SimpleSyncCallback(s32 result, s32 num);
} // namespace homebutton

// forward declarations
namespace nw4hbm {
namespace lyt {
class ArcResourceAccessor;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace lyt {
class ArcResourceLink;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace lyt {
class Layout;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace lyt {
class MultiArcResourceAccessor;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace lyt {
class Pane;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace ut {
class ResFont;
}
} // namespace nw4hbm

namespace homebutton {
// forward declarations
class HomeButton;
class Controller;
class GroupAnmController;
class RemoteSpk;

class HomeButtonEventHandler : public gui::EventHandler {
    // methods
  public:
    // cdtors
    HomeButtonEventHandler(homebutton::HomeButton* pHomeButton) : mpHomeButton(pHomeButton) {}

    // virtual function ordering
    // vtable EventHandler
    virtual void onEvent(u32 uID, u32 uEvent, void* pData);

    // methods
    homebutton::HomeButton* getHomeButton() { return mpHomeButton; }

    // members
  private:
    /* base EventHandler */ // size 0x08, offset 0x00
    HomeButton* mpHomeButton; // size 0x04, offset 0x08
}; // size 0x0c

class HomeButton {
    // enums
  private:
    typedef enum /* expliticly untagged */
    {
        eSeq_Normal,
        eSeq_Control,
        eSeq_Cmn,
    } eSeq;

    // nested classes
  private:
    class BlackFader {
        // methods
      public:
        // cdtors
        BlackFader(int maxFrame) {
            init(maxFrame);
            setColor(0, 0, 0);
        }

        // methods
        int getFrame() const { return frame_; }
        int getMaxFrame() const { return maxFrame_; }

        void setColor(u8 r, u8 g, u8 b) {
            red_ = r;
            green_ = g;
            blue_ = b;
        }
        GXColor GetColor(u8 alpha) { return (GXColor){red_, green_, blue_, alpha}; }

        bool isDone();

        void init(int maxFrame);
        void calc();
        void draw();

        void start() { state_ = 1; }

        // members
      private:
        int frame_; // size 0x04, offset 0x00
        int maxFrame_; // size 0x04, offset 0x04
        int state_; // size 0x04, offset 0x08
        u8 red_; // size 0x01, offset 0x0d
        u8 green_; // size 0x01, offset 0x0e
        u8 blue_; // size 0x01, offset 0x0f
    }; // size 0x10

    // methods
  public:
    // cdtors
    HomeButton(const HBMDataInfo* dataInfo);
    ~HomeButton();

    // gethods
    const HBMDataInfo* getHBMDataInfo() { return mpHBInfo; }
    Controller* getController(int chan) { return mpController[chan]; }
    int getVolume();
    HBMSelectBtnNum getSelectBtnNum();
    const char* getFuncPaneName(int no) { return scFuncTouchPaneName[no]; }
    const char* getPaneName(int no) { return scBtnName[no]; }
    bool getReassignedFlag() const { return mReassignedFlag; }
    HomeButtonEventHandler* getEventHandler() const { return mpHomeButtonEventHandler; }
    bool isActive() const;
    bool isUpBarActive() const;
    bool isDownBarActive();

    // sethods
    void setAdjustFlag(int flag);
    void setEndSimpleSyncFlag(bool flag) { mEndSimpleSyncFlag = flag; }
    void setForcusSE();
    void setReassignedFlag(bool flag) { mReassignedFlag = flag; }
    void setSimpleSyncAlarm(int type);
    void setSimpleSyncFlag(bool flag) { mSimpleSyncFlag = flag; }
    void setSpeakerAlarm(int chan, int msec);
    void setVolume(int vol);

    // get methods
    bool getVibFlag();
    int getPaneNo(const nw4hbm::lyt::Pane*);

    // set methods
    void setVibFlag(bool flag);

    // methods
    void create();
    void init();
    void calc(const HBMControllerData* pController);
    void draw();
    void update(const HBMControllerData* pController);
    void updateTrigPane();

    void startPointEvent(const nw4hbm::lyt::Pane* pPane, void* pData);
    void startLeftEvent(const nw4hbm::lyt::Pane* pPane);
    void startTrigEvent(const nw4hbm::lyt::Pane* pPane);

    int findAnimator(int pane, int anm);
    int findGroupAnimator(int pane, int anm);

    void callSimpleSyncCallback(s32 result, s32 num);

    void startBlackOut();

    // static methods
    static void createInstance(const HBMDataInfo* dataInfo);
    static HomeButton* getInstance() { return spHomeButtonObj; }
    static void deleteInstance();

    // implementation details? at least i think that's what the snake case means
  private:
    void init_battery(const HBMControllerData* pController);
    void calc_battery(int chan);
    void reset_battery();

    void init_sound();
    void play_sound(int id);
    void fadeout_sound(f32 gain);

    void init_msg();
    void init_vib();
    void init_volume();

    void set_config();
    void set_text();

    void calc_fadeoutAnm();

    void update_controller(int id);
    void update_posController(int id);

    void reset_btn();
    void reset_control();
    void reset_guiManager(int num);
    void reset_window();

    // exception
  public: // HBMUpdateSound
    void update_sound();

    //! TODO: cleanup
    int GetState() { return mState; }
    HBMSelectBtnNum GetSelectBtnNum() { return mSelectBtnNum; }
    BlackFader* GetFader() { return &mFader; }
    nw4hbm::lyt::Layout* GetLayout() { return mpLayout; }
    nw4hbm::lyt::DrawInfo* GetDrawInfo() { return &mDrawInfo; }
    void fn_80109A74();
    void fn_8010984C(nw4hbm::snd::NandSoundArchive* pNandSoundArchive, int param2);

    void draw_impl() {
        u8 alpha;
        int i;
        BlackFader* pFader;

        mpLayout->Draw(mDrawInfo);

        if (mpHBInfo->cursor == 0) {
            for (i = WPAD_MAX_CONTROLLERS - 1; i >= WPAD_CHAN0; i--) {
                mpCursorLayout[i]->Draw(mDrawInfo);
            }
        }

        pFader = &mFader;
        alpha = pFader->getFrame() * 255 / pFader->getMaxFrame();
        initgx();
        drawBlackPlate(-1000.0f, -1000.0f, 1000.0f, 1000.0f, pFader->GetColor(alpha));
    }

    static void createInstance_impl(const HBMDataInfo* pHBInfo) {
        if (void* pMem = HBMAllocMem(sizeof(*spHomeButtonObj))) {
            spHomeButtonObj = new (pMem) HomeButton(pHBInfo);
        }
    }

    static void deleteInstance_impl() {
        spHomeButtonObj->~HomeButton();
        HBMFreeMem(spHomeButtonObj);
        spHomeButtonObj = nullptr;
    }

    void fn_80100B88_impl() {
        if (mpSoundArchivePlayer != NULL) {
            mpSoundArchivePlayer->Update();
        }
    }

    void fn_80100BA0_impl(f32 volume) {
        AXSetMasterVolume(volume * 32768.0f);

        if (mpSoundArchivePlayer != NULL) {
            for (int i = 0; i < mpSoundArchivePlayer->GetSoundPlayerCount(); i++) {
                mpSoundArchivePlayer->GetSoundPlayer(i).SetVolume(volume);
            }
        }
    }

    void fn_80100C38_impl(bool checkFlag) {
        if (mpSoundArchivePlayer != NULL) {
            for (int i = 0; i < mpSoundArchivePlayer->GetSoundPlayerCount(); i++) {
                mpSoundArchivePlayer->GetSoundPlayer(i).StopAllSound(0);
            }
        }

        if (checkFlag && !mEndInitSoundFlag) {
            return;
        }

        AXFXReverbHiShutdown(&mAxFxReverb);
        AXRegisterAuxACallback(mAuxCallback, mpAuxContext);
        AXFXSetHooks(mAxFxAlloc, mAxFxFree);
        AXSetMasterVolume(mAppVolume[0]);
        AXSetAuxAReturnVolume(mAppVolume[1]);
        AXSetAuxBReturnVolume(mAppVolume[2]);
    }

    void PlaySeq(int num) {
        if (mpSoundArchivePlayer != NULL && mpSoundHandle != NULL) {
            mpSoundHandle->DetachSound();
            mpSoundArchivePlayer->StartSound(mpSoundHandle, num);
        }
    }

    void fn_80100CD8_impl(const char* path);
    void fn_80100E40_impl();
    static void fn_80100CD8(const char* path, void* param1, int param2);
    static void fn_80100E0C(void);
    static void fn_80100E40(void);

    // members
    // private: // offset goes dvd/nand
  public: // TEMP
    /* 0x000 */ eSeq mSequence;
    /* 0x004 */ const HBMDataInfo* mpHBInfo;
    /* 0x008 */ int mButtonNum;
    /* 0x00C */ int mAnmNum;
    /* 0x010 */ int mState;
    /* 0x014 */ int mSelectAnmNum;
    /* 0x018 */ int mMsgCount;
    /* 0x01C */ int mPaneCounter[14];
    /* 0x054 */ int mPadDrawTime[WPAD_MAX_CONTROLLERS];
    /* 0x064 */ int mForcusSEWaitTime;
    /* 0x068 */ int mBar0AnmRev;
    /* 0x06C */ int mBar1AnmRev;
    /* 0x070 */ int mBar0AnmRevHold;
    /* 0x074 */ int mBar1AnmRevHold;
    /* 0x078 */ int mGetPadInfoTime;
    /* 0x07C */ bool mControllerFlag[WPAD_MAX_CONTROLLERS];
    /* 0x080 */ int mVolumeNum;
    /* 0x084 */ bool mVibFlag;
    /* 0x085 */ bool mControlFlag;
    /* 0x086 */ bool mLetterFlag;
    /* 0x087 */ bool mAdjustFlag;
    /* 0x088 */ bool mReassignedFlag;
    /* 0x089 */ bool mSimpleSyncFlag;
    /* 0x08A */ bool mEndSimpleSyncFlag;
    /* 0x08B */ bool mInitFlag;
    /* 0x08C */ bool mForceSttInitProcFlag;
    /* 0x08D */ bool mForceSttFadeInProcFlag;
    /* 0x08E */ bool mEndInitSoundFlag;
    /* 0x08F */ bool mForceStopSyncFlag;
    /* 0x090 */ bool mForceEndMsgAnmFlag;
    /* 0x094 */ int mSoundRetryCnt;
    /* 0x098 */ int mDialogFlag[4];
    /* 0x0A8 */ char* mpLayoutName;
    /* 0x0AC */ char* mpAnmName;
    /* 0x0B0 */ HBMSelectBtnNum mSelectBtnNum;
    /* 0x0B4 */ wchar_t* mpText[7][6];
    /* 0x15C */ WPADInfo mWpadInfo[WPAD_MAX_CONTROLLERS];
    WPADSyncDeviceCallback mSimpleSyncCallback;
    /* 0x1BC */ f32 mOnPaneVibFrame[4];
    /* 0x1CC */ f32 mOnPaneVibWaitFrame[4];
    /* 0x1DC */ int mWaitStopMotorCount;
    // /* 0x1E0 */ int mDisConnectCount;
    /* 0x1E4 */ nw4hbm::lyt::Layout* mpLayout;
    /* 0x1E8 */ nw4hbm::lyt::Layout* mpCursorLayout[WPAD_MAX_CONTROLLERS];
    /* 0x1F8 */ nw4hbm::lyt::ArcResourceAccessor* mpResAccessor;
    /* 0x1FC */ gui::PaneManager* mpPaneManager;
    /* 0x200 */ HomeButtonEventHandler* mpHomeButtonEventHandler;
    /* 0x204 */ nw4hbm::lyt::DrawInfo mDrawInfo;
    /* 0x258 */ Controller* mpController[WPAD_MAX_CONTROLLERS];
    /* 0x268 */ RemoteSpk* mpRemoteSpk;
    /* 0x26C */ GroupAnmController* mpAnmController[12];
    /* 0x29C */ GroupAnmController* mpGroupAnmController[74];
    /* 0x3C4 */ GroupAnmController* mpPairGroupAnmController[15];
    /* 0x400 */ BlackFader mFader;
    /* 0x410 */ OSAlarm mAlarm[WPAD_MAX_CONTROLLERS];
    /* 0x4D0 */ OSAlarm mSpeakerAlarm[WPAD_MAX_CONTROLLERS];
    /* 0x590 */ OSAlarm mSimpleSyncAlarm;
    /* 0x5C0 */ nw4hbm::snd::SoundArchivePlayer* mpSoundArchivePlayer;
    /* 0x5C4 */ nw4hbm::snd::DvdSoundArchive* mpDvdSoundArchive;
    /* 0x5C8 */ nw4hbm::snd::MemorySoundArchive* mpMemorySoundArchive;
    /* 0x5CC */ nw4hbm::snd::NandSoundArchive* mpNandSoundArchive;
    /* 0x5D0 */ nw4hbm::snd::SoundHeap* mpSoundHeap;
    /* 0x5D4 */ nw4hbm::snd::SoundHandle* mpSoundHandle;
    /* 0x5D8 */ u16 mAppVolume[3];
    /* 0x5E0 */ AXFXAllocFunc* mAxFxAlloc;
    /* 0x5E4 */ AXFXFreeHook mAxFxFree;
    /* 0x5E8 */ AXFX_REVERBHI mAxFxReverb;
    /* 0x748 */ AXAuxCallback mAuxCallback;
    /* 0x74C */ void* mpAuxContext;
    /* 0x750 */ f32 mFadeOutSeTime;

    // static members
  private:
    static HomeButton* spHomeButtonObj;
    static OSMutex sMutex;

    // tables and stuff
    static const int scSoundHeapSize_but2;
    static const int scSoundHeapSize_but3;
    static const int scSoundThreadPrio;
    static const int scDvdThreadPrio;

    static const int scReConnectTime;
    static const int scReConnectTime2;
    static const int scPadDrawWaitTime;
    static const int scGetPadInfoTime;
    static const int scForcusSEWaitTime;
    static const f32 scOnPaneVibTime;
    static const f32 scOnPaneVibWaitTime;
    static const int scWaitStopMotorTime;
    static const int scWaitDisConnectTime;

    static const char* scCursorLytName[WPAD_MAX_CONTROLLERS];
    static const char* scCursorPaneName;
    static const char* scCursorRotPaneName;
    static const char* scCursorSRotPaneName;

    static const char* scBtnName[4];
    static const char* scTxtName[4];
    static const char* scGrName[8];
    static const char* scAnimName[3];
    static const char* scPairGroupAnimName[15];
    static const char* scPairGroupName[15];
    static const char* scGroupAnimName[22];
    static const char* scGroupName[35];
    static const char* scFuncPaneName[5];
    static const char* scFuncTouchPaneName[10];
    static const char* scFuncTextPaneName[3];
    static const char* scBatteryPaneName[WPAD_MAX_CONTROLLERS][4];
}; // size 0x740/0x7c8
} // namespace homebutton

#endif // RVL_SDK_HBM_HOMEBUTTON_BASE_HPP
