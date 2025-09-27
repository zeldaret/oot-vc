#ifndef RVL_SDK_HBM_HOMEBUTTON_BASE_HPP
#define RVL_SDK_HBM_HOMEBUTTON_BASE_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/ax.h"
#include "revolution/axfx.h"
#include "revolution/hbm/HBMConfig.h"
#include "revolution/hbm/homebutton/HBMCommon.hpp"
#include "revolution/hbm/homebutton/HBMGUIManager.hpp"
#include "revolution/hbm/nw4hbm/lyt/drawInfo.h"
#include "revolution/types.h"
#include "revolution/wpad/WPAD.h"

/*******************************************************************************
 * classes and functions
 */

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

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47f957
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

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47efb9
class HomeButton {
    // enums
  private:
    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x47f6bb
    typedef enum /* expliticly untagged */
    {
        eSeq_Normal,
        eSeq_Control,
        eSeq_Cmn,
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
        eSeq_Seq3,
        eSeq_Seq4,
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
    } eSeq;

    // nested classes
  private:
    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x47f9f8
    class BlackFader {
        // methods
      public:
        // cdtors
        BlackFader(int maxFrame) {
            init(maxFrame);
            setColor(0, 0, 0);
            flag = true;
        }

        // methods
        int getFrame() const { return frame_; }
        int getMaxFrame() const { return maxFrame_; }
        int getFadeColorEnable() const { return flag; }

        void setFadeColorEnable(bool a) { flag = a; }
        void setColor(u8 r, u8 g, u8 b) {
            red_ = r;
            green_ = g;
            blue_ = b;
        }

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
        bool flag; // size 0x01, offset 0x0c
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
    static HomeButton* getInstance();
    static void deleteInstance();

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    void createInfoEx(const HBMDataInfoEx* pHBInfoEx);
    void destroyInfoEx();

    // wiiware variant? unspecific names and shit
    const char* getBtnWarePaneName(int no) { return scBtnWareName[no]; }
    bool isThirdBarActive() const;
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

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
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    void set_text_ex();
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

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

    // members
  private: // offset goes dvd/nand
    eSeq mSequence; // size 0x004, offset 0x000/0x000
    const HBMDataInfo* mpHBInfo; // size 0x004, offset 0x004/0x004
    const HBMDataInfoEx* mpHBInfoEx; // size 0x004, offset 0x008/0x008

    int mButtonNum; // size 0x004, offset 0x00c/0x00c
    int mAnmNum; // size 0x004, offset 0x010/0x010
    int mState; // size 0x004, offset 0x014/0x014
    int mSelectAnmNum; // size 0x004, offset 0x018/0x018
    int mMsgCount; // size 0x004, offset 0x01c/0x01c
    int mPaneCounter[14]; // size 0x038, offset 0x020/0x020
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    signed at_0x058; // size 0x004, offset -----/0x058
    signed at_0x05c; // size 0x004, offset -----/0x05c
#endif
    int mPadDrawTime[WPAD_MAX_CONTROLLERS]; // size 0x010, offset 0x058/0x060
    int mForcusSEWaitTime; // size 0x004, offset 0x068/0x070
    int mBar0AnmRev; // size 0x004, offset 0x06c/0x074
    int mBar1AnmRev; // size 0x004, offset 0x070/0x078
    int mBar0AnmRevHold; // size 0x004, offset 0x074/0x07c
    int mBar1AnmRevHold; // size 0x004, offset 0x078/0x080
    int mGetPadInfoTime; // size 0x004, offset 0x07c/0x084
    bool mControllerFlag[WPAD_MAX_CONTROLLERS]; // size 0x004, offset 0x080/0x088
    int mVolumeNum; // size 0x004, offset 0x084/0x08c
    int mConnectNum; // size 0x004, offset 0x088/0x090
    bool mVibFlag; // size 0x001, offset 0x08c/0x094
    bool mControlFlag; // size 0x001, offset 0x08d/0x095
    bool mLetterFlag; // size 0x001, offset 0x08e/0x096
    bool mAdjustFlag; // size 0x001, offset 0x08f/0x097
    bool mReassignedFlag; // size 0x001, offset 0x090/0x098
    bool mSimpleSyncFlag; // size 0x001, offset 0x091/0x099
    bool mEndSimpleSyncFlag; // size 0x001, offset 0x092/0x09a
    bool mForthConnectFlag; // size 0x001, offset 0x093/0x09b
    bool mInitFlag; // size 0x001, offset 0x094/0x09c
    bool mForceSttInitProcFlag; // size 0x001, offset 0x095/0x09d
    bool mForceSttFadeInProcFlag; // size 0x001, offset 0x096/0x09e
    bool mEndInitSoundFlag; // size 0x001, offset 0x097/0x09f
    bool mForceStopSyncFlag; // size 0x001, offset 0x098/0x0a0
    bool mForceEndMsgAnmFlag; // size 0x001, offset 0x099/0x0a1
    bool mStartBlackOutFlag; // size 0x001, offset 0x09a/0x0a2
    /* 1 byte padding */
    int mSoundRetryCnt; // size 0x001, offset 0x09c/0x0a4
    int mDialogFlag[4]; // size 0x010, offset 0x0a0/0x0a8
    char* mpLayoutName; // size 0x004, offset 0x0b0/0x0b8
    char* mpAnmName; // size 0x004, offset 0x0b4/0x0bc
    HBMSelectBtnNum mSelectBtnNum; // size 0x004, offset 0x0b8/0x0c0
    wchar_t* mpText[10][6]; // size 0x0f0, offset 0x0bc/0x0c4
    WPADSyncDeviceCallback mSimpleSyncCallback; // size 0x004, offset 0x1ac/0x1b4
    f32 mOnPaneVibFrame[4]; // size 0x010, offset 0x1b0/0x1b8
    f32 mOnPaneVibWaitFrame[4]; // size 0x010, offset 0x1c0/0x1c8
    int mWaitStopMotorCount; // size 0x004, offset 0x1d0/0x1d8
    int mDisConnectCount; // size 0x004, offset 0x1d4/0x1dc
    nw4hbm::lyt::Layout* mpLayout; // size 0x004, offset 0x1d8/0x1e0
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    nw4hbm::lyt::Layout* mpLayout2; // size 0x004, offset -----/0x1e4
#endif
    nw4hbm::lyt::Layout* mpCursorLayout[WPAD_MAX_CONTROLLERS]; // size 0x010, offset 0x1dc/0x1e8
    nw4hbm::lyt::ArcResourceAccessor* mpResAccessor; // size 0x004, offset 0x1ec/0x1f8
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    nw4hbm::lyt::MultiArcResourceAccessor* mpMultiResAccessor; // size 0x004, offset -----/0x1fc
    nw4hbm::lyt::ArcResourceLink* mpHBInfoExLink; // size 0x004, offset -----/0x200
    nw4hbm::lyt::ArcResourceLink* mpHBInfoLink; // size 0x004, offset -----/0x204
#endif
    gui::PaneManager* mpPaneManager; // size 0x004, offset 0x1f0/0x208
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    nw4hbm::ut::ResFont* mpResFont; // size 0x004, offset -----/0x20c
#endif
    HomeButtonEventHandler* mpHomeButtonEventHandler; // size 0x004, offset 0x1f4/0x210
    nw4hbm::lyt::DrawInfo mDrawInfo; // size 0x054, offset 0x1f8/0x214
    Controller* mpController[WPAD_MAX_CONTROLLERS]; // size 0x010, offset 0x24c/0x268
    RemoteSpk* mpRemoteSpk; // size 0x004, offset 0x25c/0x278
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    wchar_t* at_0x27c[10][1]; // size 0x028, offset -----/0x27c // possibly fakematch?
    nw4hbm::ut::ResFont* mpResFont2; // size 0x008, offset -----/0x2a4
    GXTexObj mTexObj; // size 0x020, offset -----/0x2a8
    signed mBar2AnmRev; // size 0x004, offset -----/0x2c8
    signed mBar2AnmRevHold; // size 0x004, offset -----/0x2cc
#endif
    GroupAnmController* mpAnmController[12]; // size 0x030, offset 0x260/0x2d0
    GroupAnmController* mpGroupAnmController[74]; // size 0x128, offset 0x290/0x300
    GroupAnmController* mpPairGroupAnmController[15]; // size 0x03c, offset 0x3b8/0x428
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    GroupAnmController* mpAnmControllerSet4[7]; // size 0x01c, offset -----/0x464
#endif
    BlackFader mFader; // size 0x010, offset 0x3f4/0x480
#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
    /* 4 bytes padding */ // mFader ends at 0x404
#endif
    OSAlarm mAlarm[WPAD_MAX_CONTROLLERS]; // size 0x0c0, offset 0x408/0x490
    OSAlarm mSpeakerAlarm[WPAD_MAX_CONTROLLERS]; // size 0x0c0, offset 0x4c8/0x550
    // break
    OSAlarm mSimpleSyncAlarm; // size 0x030, offset 0x588/0x610
    int iVISetBlackFlag; // size 0x004, offset 0x5b8/0x640
    int iReConnectTime; // size 0x004, offset 0x5bc/0x644
    int iReConnectTime2; // size 0x004, offset 0x5c0/0x648
    u16 mAppVolume[3]; // size 0x006, offset 0x5c4/0x64c
    u16 mBatteryCheck; // size 0x002, offset 0x5ca/0x652
    AXFXAllocFunc* mAxFxAlloc; // size 0x004, offset 0x5cc/0x654
    AXFXFreeFunc* mAxFxFree; // size 0x004, offset 0x5d0/0x658
    AXFX_REVERBHI mAxFxReverb; // size 0x160, offset 0x5d4/0x65c
    AXAuxCallback* mAuxCallback; // size 0x004, offset 0x734/0x7bc
    void* mpAuxContext; // size 0x004, offset 0x738/0x7c0
    f32 mFadeOutSeTime; // size 0x004, offset 0x73c/0x7c4

    // static members
  private:
    static HomeButton* spHomeButtonObj;
    static OSMutex sMutex;
    static WPADInfo sWpadInfo[WPAD_MAX_CONTROLLERS];

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

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    // names here are best guesses
    static const char* scBtnWareName[2];
    static const char* scButtonBarMenuName[7];
    static const char* scButtonBarMenuGroupName[7];
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
}; // size 0x740/0x7c8
} // namespace homebutton

#endif // RVL_SDK_HBM_HOMEBUTTON_BASE_HPP
