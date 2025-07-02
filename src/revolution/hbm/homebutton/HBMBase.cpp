#include "revolution/hbm/homebutton/HBMBase.hpp"

/*******************************************************************************
 * headers
 */

#include "cstring.hpp"
#include "new.hpp" // placement new

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/homebutton/HBMAnmController.hpp"
#include "revolution/hbm/homebutton/HBMAxSound.hpp"
#include "revolution/hbm/homebutton/HBMCommon.hpp"
#include "revolution/hbm/homebutton/HBMController.hpp"
#include "revolution/hbm/homebutton/HBMFrameController.hpp"
#include "revolution/hbm/homebutton/HBMRemoteSpk.hpp"

#include "revolution/hbm/nw4hbm/lyt/lyt_animation.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_arcResourceAccessor.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp" // nw4hbm::lyt::Size
#include "revolution/hbm/nw4hbm/lyt/lyt_drawInfo.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_group.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_layout.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_textBox.hpp"
#include "revolution/hbm/nw4hbm/lyt/material.h"
#include "revolution/hbm/nw4hbm/lyt/pane.h"
#include "revolution/hbm/nw4hbm/math/math_triangular.hpp" // nw4hbm::math::Atan2Deg
#include "revolution/hbm/nw4hbm/math/math_types.hpp"
#include "revolution/hbm/nw4hbm/ut/LinkList.h" // IWYU pragma: keep (NW4HBM_RANGE_FOR)
#include "revolution/hbm/nw4hbm/ut/ut_Rect.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_ResFont.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp" // nw4hbm::ut::DynamicCast

#include "revolution/ax.h"
#include "revolution/axfx.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/mem/mem_expHeap.h"
#include "revolution/os.h"
#include "revolution/os/OSAlarm.h"
#include "revolution/os/OSMutex.h"
#include "revolution/os/OSTime.h"
#include "revolution/sc/scapi.h"
#include "revolution/vi.h"
#include "revolution/wpad/WPAD.h"

//! TODO: remove
#define OSAssert_Line(...) (void)0;

#define REVO_IPL_FONT "RevoIpl_UtrilloProGrecoStd_M_32_I4.brfnt"

struct AnmControllerTable {
    int pane; // size 0x04, offset 0x00
    int anm; // size 0x04, offset 0x04
}; // size 0x08

namespace homebutton {
static void initgx();

static void drawBlackPlate(f32 left, f32 top, f32 right, f32 bottom, GXColor clr);
static u32 get_comma_length(char* pBuf);
static void SpeakerCallback(OSAlarm* alm, OSContext* ctx);
static void MotorCallback(OSAlarm* alm, OSContext* ctx);
static void RetrySimpleSyncCallback(OSAlarm* alm, OSContext* ctx);
static void SimpleSyncCallback(s32 result, s32 num);
} // namespace homebutton

namespace homebutton {
#if 0 /* data pooling */
	// .rodata
    static const AnmControllerTable scAnmTable[12];
    static const AnmControllerTable scGroupAnmTable[74];

    const int HomeButton::scReConnectTime = 3600;
    const int HomeButton::scReConnectTime2 = 3570;
    const int HomeButton::scPadDrawWaitTime = 5;
    const int HomeButton::scGetPadInfoTime = 100;
    const int HomeButton::scForcusSEWaitTime = 2;
    const f32 HomeButton::scOnPaneVibTime = 3.0f;
    const f32 HomeButton::scOnPaneVibWaitTime = 9.0f;
    const int HomeButton::scWaitStopMotorTime = 30;
    const int HomeButton::scWaitDisConnectTime = 180;

	// .data
    const char *HomeButton::scCursorLytName[WPAD_MAX_CONTROLLERS];
    const char *HomeButton::scCursorPaneName;
    const char *HomeButton::scCursorRotPaneName;
    const char *HomeButton::scCursorSRotPaneName;
    const char *HomeButton::scBtnName[4];
    const char *HomeButton::scTxtName[4];
    const char *HomeButton::scGrName[8];
    const char *HomeButton::scAnimName[3];
    const char *HomeButton::scPairGroupAnimName[15];
    const char *HomeButton::scPairGroupName[15];
    const char *HomeButton::scGroupAnimName[22];
    const char *HomeButton::scGroupName[35];
    const char *HomeButton::scFuncPaneName[5];
    const char *HomeButton::scFuncTouchPaneName[10];
    const char *HomeButton::scFuncTextPaneName[3];
    const char *HomeButton::scBatteryPaneName[WPAD_MAX_CONTROLLERS][4];

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    const char *HomeButton::scBtnWareName[2];
    const char *HomeButton::scButtonBarMenuName[7];
    const char *HomeButton::scButtonBarMenuGroupName[7];
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
#endif // 0

// .bss
HomeButton* HomeButton::spHomeButtonObj;
OSMutex HomeButton::sMutex;
WPADInfo HomeButton::sWpadInfo[WPAD_MAX_CONTROLLERS];
} // namespace homebutton

enum HBMAllocatorType {
    HBM_ALLOCATOR_APPLI, /* application */
    HBM_ALLOCATOR_LOCAL,
    HBM_ALLOCATOR_NW4R,
};

// .bss
static MEMAllocator sAllocator;

// .data (not .sdata, HBM is -sdata 0)
MEMAllocator* spAllocator = &sAllocator;

void* HBMAllocMem(u32 size) {
    void* addr = MEMAllocFromAllocator(spAllocator, size);

    // why into a variable?

    return addr;
}

void HBMFreeMem(void* mem) { MEMFreeToAllocator(spAllocator, mem); }

static HBMAllocatorType getAllocatorType(const HBMDataInfo* pHBInfo) {
    if (pHBInfo->pAllocator) {
        return HBM_ALLOCATOR_APPLI;
    } else if (pHBInfo->mem) {
        return HBM_ALLOCATOR_LOCAL;
    } else {
        return HBM_ALLOCATOR_NW4R;
    }
}

void HBMCreate(const HBMDataInfo* pHBInfo) {
    MEMiHeapHead* hExpHeap;

    if (getAllocatorType(pHBInfo) == HBM_ALLOCATOR_LOCAL) {
        hExpHeap = MEMCreateExpHeap(pHBInfo->mem, pHBInfo->memSize);
        MEMInitAllocatorForExpHeap(&sAllocator, hExpHeap, 32);
        spAllocator = &sAllocator;
    }

    switch (getAllocatorType(pHBInfo)) {
        case HBM_ALLOCATOR_APPLI:
            nw4hbm::lyt::Layout::SetAllocator(pHBInfo->pAllocator);
            spAllocator = pHBInfo->pAllocator;
            break;

        case HBM_ALLOCATOR_LOCAL:
            nw4hbm::lyt::Layout::SetAllocator(spAllocator);
            break;

        case HBM_ALLOCATOR_NW4R:
            OSAssert_Line(87, nw4hbm::lyt::Layout::GetAllocator());

            // spAllocator = nw4hbm::lyt::Layout::GetAllocator();
            break;
    }

    // homebutton::HomeButton::createInstance(pHBInfo);
    homebutton::HomeButton::getInstance()->create();
}

namespace homebutton {
// clang-format off
static const AnmControllerTable scAnmTable[12] =
{
	{ 0, 0 }, { 0, 2 }, { 4, 1 },
	{ 1, 0 }, { 1, 2 }, { 5, 1 },
	{ 2, 0 }, { 2, 2 }, { 6, 1 },
	{ 3, 0 }, { 3, 2 }, { 7, 1 }
};

static const AnmControllerTable scGroupAnmTable[74] =
{
	{  0,  0 },
	{  1,  1 },
	{  2,  0 },
	{  3,  1 },
	{  4,  2 }, {  4, 19 },
	{  5,  3 }, {  5, 20 },
	{  6,  4 }, {  6,  7 },
	{  7,  4 }, {  7,  7 },
	{  8,  4 }, {  8,  7 },
	{  9,  4 }, {  9,  7 },
	{ 10,  4 }, { 10,  7 },
	{ 11,  5 },
	{ 12,  5 },
	{ 13,  6 }, { 13,  8 },
	{ 14, 14 }, { 14,  6 }, { 14,  8 },
	{ 15,  5 },
	{ 16,  6 }, { 16, 14 },
	{ 17, 11 }, { 17, 12 },
	{ 18, 11 }, { 18, 12 },
	{ 19, 13 },
	{ 20, 13 },
	{ 21,  9 }, { 21, 10 },
	{ 22,  9 }, { 22, 10 },
	{ 23,  9 }, { 23, 10 },
	{ 24,  9 }, { 24, 10 },
	{ 25,  9 }, { 25, 10 },
	{ 26,  9 }, { 26, 10 },
	{ 27,  9 }, { 27, 10 },
	{ 28,  9 }, { 28, 10 },
	{ 29,  9 }, { 29, 10 },
	{ 30,  9 }, { 30, 10 },
	{ 31, 15 }, { 31, 16 }, { 31, 17 }, { 31, 18 }, { 31, 21 },
	{ 32, 15 }, { 32, 16 }, { 32, 17 }, { 32, 18 }, { 32, 21 },
	{ 33, 15 }, { 33, 16 }, { 33, 17 }, { 33, 18 }, { 33, 21 },
	{ 34, 15 }, { 34, 16 }, { 34, 17 }, { 34, 18 }, { 34, 21 }
};

const int HomeButton::scReConnectTime = 3600;
const int HomeButton::scReConnectTime2 = 3570;
const int HomeButton::scPadDrawWaitTime = 5;
const int HomeButton::scGetPadInfoTime = 100;
const int HomeButton::scForcusSEWaitTime = 2;
const f32 HomeButton::scOnPaneVibTime = 3.0f;
const f32 HomeButton::scOnPaneVibWaitTime = 9.0f;
const int HomeButton::scWaitStopMotorTime = 30;
const int HomeButton::scWaitDisConnectTime = 180;

const char *HomeButton::scCursorLytName[WPAD_MAX_CONTROLLERS] =
{
	"P1_Def.brlyt",
	"P2_Def.brlyt",
	"P3_Def.brlyt",
	"P4_Def.brlyt"
};

const char *HomeButton::scCursorPaneName = "N_Trans";
const char *HomeButton::scCursorRotPaneName = "N_Rot";
const char *HomeButton::scCursorSRotPaneName = "N_SRot";

const char *HomeButton::scBtnName[4] =
{
	"B_btnL_00",
	"B_btnL_01",
	"B_btnL_10",
    nullptr
};

const char *HomeButton::scTxtName[4] =
{
	"T_btnL_00",
	"T_btnL_01",
	"T_btnL_10",
    nullptr
};

const char *HomeButton::scGrName[8] =
{
	"btnL_00_inOut",
	"btnL_01_inOut",
	"btnL_10_inOut",
	"btnL_11_inOut",
	"btnL_00_psh",
	"btnL_01_psh",
	"btnL_10_psh",
	"btnL_11_psh"
};

const char *HomeButton::scAnimName[3] =
{
	"_cntBtn_in.brlan",
	"_cntBtn_psh.brlan",
	"_cntBtn_out.brlan"
};

const char *HomeButton::scPairGroupAnimName[15] =
{
	"_ltrIcn_on.brlan",
	"_optn_bar_psh.brlan",
	"_close_bar_psh.brlan",
	"_hmMenu_bar_in.brlan",
	"_hmMenu_bar_psh.brlan",
	"_link_msg_in.brlan",
	"_link_msg_out.brlan",
	"_cmn_msg_in.brlan",
	"_cmn_msg_out.brlan",
	"_cntrl_up.brlan",
	"_cntrl_wndw_opn.brlan",
	"_cntrl_dwn.brlan",
	"_hmMenu_bar_out.brlan",
	"_cmn_msg_rtrn.brlan",
	"_12btn_on.brlan"
};

const char *HomeButton::scPairGroupName[15] =
{
	"ltrIcn_on",
	"optn_bar_psh",
	"close_bar_psh",
	"hmMenu_bar_in",
	"hmMenu_bar_psh",
	"link_msg_in",
	"link_msg_out",
	"cmn_msg_in",
	"cmn_msg_out",
	"cntrl_up",
	"cntrl_wndw_opn",
	"cntrl_dwn",
	"hmMenu_bar_out",
	"cmn_msg_rtrn",
	"12btn_on"
};

const char *HomeButton::scGroupAnimName[22] =
{
	"_hmMenu_strt.brlan",
	"_hmMenu_fnsh.brlan",
	"_optn_bar_in.brlan",
	"_optn_bar_out.brlan",
	"_optn_btn_in.brlan",
	"_optn_btn_psh.brlan",
	"_vb_btn_wht_psh.brlan",
	"_optn_btn_out.brlan",
	"_vb_btn_ylw_psh.brlan",
	"_sound_gry.brlan",
	"_sound_ylw.brlan",
	"_cmn_msg_btn_in.brlan",
	"_cmn_msg_btn_out.brlan",
	"_cmn_msg_btn_psh.brlan",
	"_vb_btn_ylw_ylw.brlan",
	"_btry_wink.brlan",
	"_btry_gry.brlan",
	"_btry_wht.brlan",
	"_btry_wink_gry.brlan",
	"_close_bar_in.brlan",
	"_close_bar_out.brlan",
	"_btry_red.brlan"
};

const char *HomeButton::scGroupName[35] =
{
	"hmMenu_strt",
	"hmMenu_fnsh",
	"hmMenuBck_strt",
	"hmMenuBck_fnsh",
	"optn_bar_in",
	"optn_bar_out",
	"optnBtn_00_inOut",
	"optnBtn_01_inOut",
	"optnBtn_10_inOut",
	"optnBtn_11_inOut",
	"optnBtn_20_inOut",
	"optnBtn_00_psh",
	"optnBtn_01_psh",
	"optnBtn_10_psh",
	"optnBtn_11_psh",
	"optnBtn_20_psh",
	"optnBtn_10_cntrl",
	"msgBtn_00_inOut",
	"msgBtn_01_inOut",
	"msgBtn_00_psh",
	"msgBtn_01_psh",
	"vol_00",
	"vol_01",
	"vol_02",
	"vol_03",
	"vol_04",
	"vol_05",
	"vol_06",
	"vol_07",
	"vol_08",
	"vol_09",
	"plyr_00",
	"plyr_01",
	"plyr_02",
	"plyr_03"
};

const char *HomeButton::scFuncPaneName[5] =
{
	"let_icn_00",
	"N_plyr_00",
	"N_plyr_01",
	"N_plyr_02",
	"N_plyr_03"
};

const char *HomeButton::scFuncTouchPaneName[10] =
{
	"B_btn_00",
	"B_bar_10",
	"B_optnBtn_00",
	"B_optnBtn_01",
	"B_optnBtn_10",
	"B_optnBtn_11",
	"B_optnBtn_20",
	"B_BtnA",
	"B_BtnB",
	"cntrl_00"
};

const char *HomeButton::scFuncTextPaneName[3] =
{
	"T_msg_00",
	"T_msg_01",
	"T_Dialog"
};

const char *HomeButton::scBatteryPaneName[WPAD_MAX_CONTROLLERS][4] =
{
	{
		"btryPwr_00_0",
		"btryPwr_00_1",
		"btryPwr_00_2",
		"btryPwr_00_3"
	},
	{
		"btryPwr_01_0",
		"btryPwr_01_1",
		"btryPwr_01_2",
		"btryPwr_01_3"
	},
	{
		"btryPwr_02_0",
		"btryPwr_02_1",
		"btryPwr_02_2",
		"btryPwr_02_3"
	},
	{
		"btryPwr_03_0",
		"btryPwr_03_1",
		"btryPwr_03_2",
		"btryPwr_03_3"
	}
};

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
const char *HomeButton::scBtnWareName[2] =
{
	"B_btn_ware_01",
	"B_btn_ware_00",
};

const char *HomeButton::scButtonBarMenuName[7] =
{
	"Button_FocusIn",
	"Button_FocusOut",
	"Button_Flash",

	"Bar_FocusIn",
	"Bar_FocusOut",

	"Menu_Strt",
	"Menu_End"
};

const char *HomeButton::scButtonBarMenuGroupName[7] =
{
	"G_ButtonExp",
	"G_ButtonExp",
	"G_ButtonExp",

	"G_Bar",
	"G_Bar",

	"G_StartEnd",
	"G_StartEnd"
};
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
// clang-format on

static void drawBlackPlate(f32 left, f32 top, f32 right, f32 bottom, GXColor clr) {
    GXSetTevColor(GX_TEVREG0, clr);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2f32(left, top);
    GXPosition2f32(left, bottom);
    GXPosition2f32(right, bottom);
    GXPosition2f32(right, top);
    GXEnd();
}

void HomeButton::createInstance(const HBMDataInfo* pHBInfo) {
    OSAssert_Line(356, !spHomeButtonObj);

    if (void* pMem = HBMAllocMem(sizeof *spHomeButtonObj)) {
        spHomeButtonObj = new (pMem) HomeButton(pHBInfo);
    }

    OSAssert_Line(362, spHomeButtonObj);
}

void HomeButton::deleteInstance() {
    OSAssert_Line(370, spHomeButtonObj);

    spHomeButtonObj->~HomeButton();
    HBMFreeMem(spHomeButtonObj);
    spHomeButtonObj = nullptr;
}

HomeButton* HomeButton::getInstance() { return spHomeButtonObj; }

void HomeButton::BlackFader::init(int maxFrame) {
    frame_ = 0;
    maxFrame_ = maxFrame;
    state_ = 0;
    flag = true;
}

void HomeButton::BlackFader::calc() {
    if (state_ == 1) {
        frame_++;
    } else if (state_ == 2) {
        frame_--;
    }

    if (frame_ < 0) {
        frame_ = 0;
    } else if (frame_ > maxFrame_) {
        frame_ = maxFrame_;
    }
}

bool HomeButton::BlackFader::isDone() {
    if (state_ == 1) {
        if (frame_ == maxFrame_) {
            return true;
        }
    }

    if (state_ == 2) {
        if (frame_ == 0) {
            return true;
        }
    }

    return false;
}

void HomeButton::BlackFader::draw() {
    u8 alpha = frame_ * 255 / maxFrame_;

    initgx();

    // clang-format off
    GXColor clr = flag ? (GXColor){red_, green_, blue_, alpha}
	                   : (GXColor){   0,      0,     0, alpha};
    // clang-format on

    drawBlackPlate(-1000.0f, -1000.0f, 1000.0f, 1000.0f, clr);
}

int HomeButton::findGroupAnimator(int pane, int anm) {
    for (int i = 0; i < (int)ARRAY_COUNT(scGroupAnmTable); i++) {
        if (scGroupAnmTable[i].pane == pane && scGroupAnmTable[i].anm == anm) {
            return i;
        }
    }

    return -1;
}

HBMSelectBtnNum HomeButton::getSelectBtnNum() {
    // Interesting
    if (mState != 18) {
        return HBM_SELECT_NULL;
    } else {
        return mSelectBtnNum;
    }
}

void HomeButton::update_sound() {
    AxSoundMain();

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mpController[i]->updateSound();
    }
}

void HomeButton::play_sound(int id) {
    int ret = 0;

    if (mpHBInfo->sound_callback) {
        ret = (*mpHBInfo->sound_callback)(5, id);
    }

    if (ret == 0) {
        PlaySeq(id);
    }
}

HomeButton::HomeButton(const HBMDataInfo* pHBInfo)
    : mpHBInfo(pHBInfo), mpHBInfoEx(nullptr), mpLayout(nullptr),
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
      mpLayout2(nullptr),
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
      mpPaneManager(nullptr), mFader(30) {
    iVISetBlackFlag = true;
    mState = 2;
    mSelectBtnNum = HBM_SELECT_NULL;
    mSelectAnmNum = -1;
    mMsgCount = 0;
    mSequence = eSeq_Normal;
    mForcusSEWaitTime = 0;
    mLetterFlag = false;
    mBar0AnmRev = 0;
    mBar1AnmRev = 0;
    mBar0AnmRevHold = 0;
    mBar1AnmRevHold = 0;
    mAdjustFlag = false;
    mReassignedFlag = false;
    mEndInitSoundFlag = false;

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        OSCreateAlarm(&mAlarm[i]);
        OSCreateAlarm(&mSpeakerAlarm[i]);
    }

    OSCreateAlarm(&mSimpleSyncAlarm);
    OSInitMutex(&sMutex);
}

HomeButton::~HomeButton() {
    int i;

    mpResAccessor->~ArcResourceAccessor();
    HBMFreeMem(mpResAccessor);

    mpLayout->~Layout();
    HBMFreeMem(mpLayout);

    if (mpHBInfo->cursor == 0) {
        for (i = 0; i < (int)ARRAY_COUNT(mpCursorLayout); i++) {
            mpCursorLayout[i]->~Layout();
            HBMFreeMem(mpCursorLayout[i]);
        }
    }

    for (i = 0; i < mAnmNum; i++) {
        mpAnmController[i]->~GroupAnmController();
        HBMFreeMem(mpAnmController[i]);
    }

    for (i = 0; i < (int)ARRAY_COUNT(mpPairGroupAnmController); i++) {
        mpPairGroupAnmController[i]->~GroupAnmController();
        HBMFreeMem(mpPairGroupAnmController[i]);
    }

    for (i = 0; i < (int)ARRAY_COUNT(mpGroupAnmController); i++) {
        mpGroupAnmController[i]->~GroupAnmController();
        HBMFreeMem(mpGroupAnmController[i]);
    }

    mpHomeButtonEventHandler->HomeButtonEventHandler::~HomeButtonEventHandler();
    HBMFreeMem(mpHomeButtonEventHandler);

    mpPaneManager->~PaneManager();
    HBMFreeMem(mpPaneManager);

    for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mpController[i]->~Controller();
        HBMFreeMem(mpController[i]);
    }

    mpRemoteSpk->~RemoteSpk();
    HBMFreeMem(mpRemoteSpk);
    mpRemoteSpk = nullptr;

    HBMFreeMem(mpLayoutName);
    HBMFreeMem(mpAnmName);

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        OSCancelAlarm(&mAlarm[i]);
        OSCancelAlarm(&mSpeakerAlarm[i]);
    }

    OSCancelAlarm(&mSimpleSyncAlarm);
}

void HomeButton::create() {
    int i;
    char anmNameBuf[64];

    mInitFlag = false;
    mForceSttInitProcFlag = false;
    mForceSttFadeInProcFlag = false;

    set_config();
    set_text();

    if (void* pMem = HBMAllocMem(sizeof *mpResAccessor)) {
        mpResAccessor = new (pMem) nw4hbm::lyt::ArcResourceAccessor();
    }

    NW4HBMAssertPointerNonnull_Line(mpResAccessor, 623);
    mpResAccessor->Attach(mpHBInfo->layoutBuf, "arc");

    if (!mpHBInfo->cursor) {
        for (i = 0; i < (int)ARRAY_COUNT(mpCursorLayout); i++) {
            if (void* pMem = HBMAllocMem(sizeof *mpCursorLayout[i])) {
                mpCursorLayout[i] = new (pMem) nw4hbm::lyt::Layout();
            }

            NW4HBMAssertPointerNonnull_Line(mpCursorLayout[i], 635);

            void* lytRes = mpResAccessor->GetResource(0, scCursorLytName[i], nullptr);

            mpCursorLayout[i]->Build(lytRes, mpResAccessor);
        }
    }

    if (void* pMem = HBMAllocMem(sizeof *mpLayout)) {
        mpLayout = new (pMem) nw4hbm::lyt::Layout();
    }

    {
        // 0x4817f1 wants lexical_block?

        void* lytRes = mpResAccessor->GetResource(0, mpLayoutName, nullptr);
        mpLayout->Build(lytRes, mpResAccessor);
    }

    for (i = 0; i < mAnmNum; i++) {
        std::strcpy(anmNameBuf, mpAnmName);
        std::strcat(anmNameBuf, scAnimName[scAnmTable[i].anm]);

        void* lpaRes = mpResAccessor->GetResource(0, anmNameBuf, nullptr);
        NW4HBMAssertPointerNonnull_Line(lpaRes, 665);

        if (void* pMem = HBMAllocMem(sizeof *mpAnmController[i])) {
            mpAnmController[i] = new (pMem) GroupAnmController();
        }

        NW4HBMAssertPointerNonnull_Line(mpAnmController[i], 671);

        mpAnmController[i]->mpAnimGroup = mpLayout->CreateAnimTransform(lpaRes, mpResAccessor);

        mpAnmController[i]->mpGroup = mpLayout->GetGroupContainer()->FindGroupByName(scGrName[scAnmTable[i].pane]);

        nw4hbm::lyt::detail::PaneLink::LinkList& list = mpAnmController[i]->mpGroup->GetPaneList();

        NW4HBM_RANGE_FOR(it, list)
        it->mTarget->BindAnimation(mpAnmController[i]->mpAnimGroup, false);

        mpAnmController[i]->init(FrameController::eAnmType_Forward, mpAnmController[i]->mpAnimGroup->GetFrameMax(), 0,
                                 mpHBInfo->frameDelta);
    }

    for (i = 0; i < (int)ARRAY_COUNT(mpGroupAnmController); i++) {
        std::strcpy(anmNameBuf, mpAnmName);
        std::strcat(anmNameBuf, scGroupAnimName[scGroupAnmTable[i].anm]);

        void* lpaRes = mpResAccessor->GetResource(0, anmNameBuf, nullptr);

        NW4HBMAssertPointerNonnull_Line(lpaRes, 697);

        if (void* pMem = HBMAllocMem(sizeof *mpGroupAnmController[i])) {
            mpGroupAnmController[i] = new (pMem) GroupAnmController();
        }

        NW4HBMAssertPointerNonnull_Line(mpGroupAnmController[i], 703);

        mpGroupAnmController[i]->mpAnimGroup = mpLayout->CreateAnimTransform(lpaRes, mpResAccessor);

        mpGroupAnmController[i]->mpGroup =
            mpLayout->GetGroupContainer()->FindGroupByName(scGroupName[scGroupAnmTable[i].pane]);

        nw4hbm::lyt::detail::PaneLink::LinkList& list = mpGroupAnmController[i]->mpGroup->GetPaneList();

        NW4HBM_RANGE_FOR(it, list) { it->mTarget->BindAnimation(mpGroupAnmController[i]->mpAnimGroup, false); }

        mpGroupAnmController[i]->init(FrameController::eAnmType_Forward,
                                      mpGroupAnmController[i]->mpAnimGroup->GetFrameMax(), 0, mpHBInfo->frameDelta);
    }

    for (i = 0; i < (int)ARRAY_COUNT(mpPairGroupAnmController); i++) {
        std::strcpy(anmNameBuf, mpAnmName);
        std::strcat(anmNameBuf, scPairGroupAnimName[i]);

        void* lpaRes = mpResAccessor->GetResource(0, anmNameBuf, nullptr);

        NW4HBMAssertPointerNonnull_Line(lpaRes, 729);

        if (void* pMem = HBMAllocMem(sizeof *mpPairGroupAnmController[i])) {
            mpPairGroupAnmController[i] = new (pMem) GroupAnmController();
        }

        NW4HBMAssertPointerNonnull_Line(mpPairGroupAnmController[i], 735);

        mpPairGroupAnmController[i]->mpAnimGroup = mpLayout->CreateAnimTransform(lpaRes, mpResAccessor);

        mpPairGroupAnmController[i]->mpGroup = mpLayout->GetGroupContainer()->FindGroupByName(scPairGroupName[i]);

        nw4hbm::lyt::detail::PaneLink::LinkList& list = mpPairGroupAnmController[i]->mpGroup->GetPaneList();

        NW4HBM_RANGE_FOR(it, list) { it->mTarget->BindAnimation(mpPairGroupAnmController[i]->mpAnimGroup, false); }

        mpPairGroupAnmController[i]->init(FrameController::eAnmType_Forward,
                                          mpPairGroupAnmController[i]->mpAnimGroup->GetFrameMax(), 0,
                                          mpHBInfo->frameDelta);
    }

    if (void* pMem = HBMAllocMem(sizeof *mpHomeButtonEventHandler)) {
        mpHomeButtonEventHandler = new (pMem) HomeButtonEventHandler(this);
    }

    NW4HBMAssertPointerNonnull_Line(mpHomeButtonEventHandler, 758);

    if (void* pMem = HBMAllocMem(sizeof *mpPaneManager)) {
        mpPaneManager = new (pMem) gui::PaneManager(mpHomeButtonEventHandler, nullptr, spAllocator);
    }

    NW4HBMAssertPointerNonnull_Line(mpPaneManager, 765);

    mpPaneManager->createLayoutScene(*mpLayout);

    if (void* pMem = HBMAllocMem(sizeof *mpRemoteSpk)) {
        mpRemoteSpk = new (pMem) RemoteSpk(mpHBInfo->spkSeBuf);
    }

    for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        if (void* pMem = HBMAllocMem(sizeof *mpController[i])) {
            mpController[i] = new (pMem) Controller(i, mpRemoteSpk);
        }
    }

    mpPaneManager->setDrawInfo(&mDrawInfo);

    nw4hbm::math::VEC2 ad_v(1.0f / mpHBInfo->adjust.x, 1.0f);
    mDrawInfo.SetLocationAdjustScale(ad_v);
    mDrawInfo.SetLocationAdjust(mAdjustFlag);

    nw4hbm::math::MTX34 viewMtx;
    nw4hbm::math::MTX34Identity(&viewMtx);
    mDrawInfo.SetViewMtx(viewMtx);

    init_msg();
}

static u32 get_comma_length(char* pBuf) {
    u32 len;

    for (len = 0; pBuf[len]; len++) {
        if (pBuf[len] == ',') {
            break;
        }
    }

    return len;
}

void HomeButton::set_config() {
    int i = 0, j = 0;

    char* pConfig = static_cast<char*>(mpHBInfo->configBuf);
    char* pEnd = static_cast<char*>(mpHBInfo->configBuf) + mpHBInfo->configBufSize;
    u32 len = get_comma_length(pConfig);

    mpLayoutName = static_cast<char*>(HBMAllocMem(len + 1));

    NW4HBMAssertPointerNonnull_Line(mpLayoutName, 827);

    std::strncpy(mpLayoutName, pConfig, len);
    mpLayoutName[len] = '\0';

    pConfig += len + 1;

    len = get_comma_length(pConfig);
    mpAnmName = static_cast<char*>(HBMAllocMem(len + 1));

    NW4HBMAssertPointerNonnull_Line(mpAnmName, 837);

    std::strncpy(mpAnmName, pConfig, len);
    mpAnmName[len] = '\0';

    pConfig += len;

    // comma operator generates a temporary
    i = 0;
    j = 0;
    for (; pConfig[i]; i++) {
        if (pConfig[i] == ',') {
            if (pConfig[i + 1] == '1') {
                mDialogFlag[j] = true;
            } else {
                mDialogFlag[j] = false;
            }

            j++;
        }
    }

    mButtonNum = j;
    mAnmNum = mButtonNum * 3;
}

void HomeButton::set_text() {
    int i = 0, j = 0, k = 0;
    bool flag = false; // more accurately insideStringFlag

    wchar_t* message = static_cast<wchar_t*>(mpHBInfo->msgBuf);
    for (; message[i]; i++) {
        if (message[i] == L'\"') {
            if (!flag) {
                flag = true;

                mpText[j][k] = &message[i + 1];
                j++;

                if (j == L'\n') {
                    j = 0;
                    k++;
                }
            } else {
                flag = false;
            }
        }
    }
}

void HomeButton::init() {
    int i;

    if (mInitFlag) {
        return;
    }

    mInitFlag = true;
    mBatteryCheck = 1;

    mForceSttInitProcFlag = false;
    mForceSttFadeInProcFlag = false;
    mStartBlackOutFlag = false;
    mForceStopSyncFlag = false;
    mSimpleSyncCallback = nullptr;
    iReConnectTime = 3600.0f / getInstance()->getHBMDataInfo()->frameDelta;
    iReConnectTime2 = 3570.0f / getInstance()->getHBMDataInfo()->frameDelta;

    if (mEndInitSoundFlag) {
        AXFXReverbHiShutdown(&mAxFxReverb);
        AXRegisterAuxACallback(*mAuxCallback, mpAuxContext);
        AXFXSetHooks(mAxFxAlloc, mAxFxFree);
        AXSetAuxAReturnVolume(mAppVolume[0]);
        AXSetAuxBReturnVolume(mAppVolume[1]);
        AXSetAuxCReturnVolume(mAppVolume[2]);

        mEndInitSoundFlag = false;
    }

    for (i = 0; i < (int)ARRAY_COUNT(mPaneCounter); i++) {
        mPaneCounter[i] = 0;
    }

    mState = 0;
    mSequence = eSeq_Normal;
    mReassignedFlag = false;

    mpPaneManager->init();
    mpPaneManager->setAllComponentTriggerTarget(false);

    // for (i = 0; i < mButtonNum; i++) {
    //     nw4hbm::lyt::Pane* pTouchPane = mpLayout->GetRootPane()->FindPaneByName(scBtnName[i], true);

    //     mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(true);
    // }

    // updateTrigPane();

    // nw4hbm::ut::Rect layoutRect = mpLayout->GetLayoutRect();
    // mDrawInfo.SetViewRect(layoutRect);
    // mpLayout->CalculateMtx(mDrawInfo);

    // nw4hbm::math::VEC2 pos(-1000.0f, -1000.0f);

    // for (i = 0; i < (int)ARRAY_COUNT(mpCursorLayout); i++) {
    //     mpCursorLayout[i]->CalculateMtx(mDrawInfo);

    //     mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorPaneName, true)->SetTranslate(pos);
    // }

    reset_guiManager(-1);

    for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        mPadDrawTime[i] = 0;

        mpController[i]->setInValidPos();
        mpController[i]->clrKpadButton();
        mpController[i]->disconnect();
        mpController[i]->clrBatteryFlag();
        mpController[i]->initCallback();
        mpController[i]->initSound();

        mOnPaneVibFrame[i] = 0.0f;
        mOnPaneVibWaitFrame[i] = 0.0f;
    }

    mpLayout->GetRootPane()->FindPaneByName(scFuncPaneName[0], true)->SetVisible(false);

    mpLayout->GetRootPane()->FindPaneByName("N_cntrl_01", true)->SetVisible(true);

    mpLayout->GetRootPane()->FindPaneByName("bar_00", true)->SetVisible(true);
    mpLayout->GetRootPane()->FindPaneByName("bar_10", true)->SetVisible(true);

    // 2-6: "B_optnBtn_XX" entries in scFuncTouchPaneName
    for (i = 2; i < 7; i++) {
        mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true)->SetVisible(false);
    }

    for (i = 0; i < (int)ARRAY_COUNT(scFuncTextPaneName); i++) {
        mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[i], true)->SetVisible(false);
    }

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    mpLayout2->CalculateMtx(mDrawInfo);

    // only matches with its own int. idk
    for (int i = 0; i < (int)ARRAY_COUNT(mpAnmControllerSet4); i++) {
        mpAnmControllerSet4[i]->stop();
        mpAnmControllerSet4[i]->initFrame();
    }

    at_0x058 = 0;
    at_0x05c = 0;

    mpLayout2->GetRootPane()->SetVisible(false);

    mBar2AnmRev = 0;
    mBar2AnmRevHold = 0;

    nw4hbm::lyt::Pane* pBtnWarePane = mpLayout2->GetRootPane()->FindPaneByName(scBtnWareName[0], true);

    pBtnWarePane->SetSize(nw4hbm::lyt::Size(324.0f, 64.0f));

    mpLayout2->GetRootPane()->FindPaneByName("bar_00", true)->DoAlphaThing();
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

    mpRemoteSpk->Start();
    calc(nullptr);

    mFader.init(30.0f / getInstance()->getHBMDataInfo()->frameDelta);
}

void HomeButton::init_msg() {
    int i, len;

    for (i = 0; i < (int)ARRAY_COUNT(scFuncTextPaneName); i++) {
        nw4hbm::lyt::Pane* p_pane = mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[i], true);
        nw4hbm::lyt::TextBox* p_text = nw4hbm::ut::DynamicCast<nw4hbm::lyt::TextBox*, nw4hbm::lyt::Pane>(p_pane);

        p_text->SetString(mpText[mpHBInfo->region][i], 0, len);
    }
}

void HomeButton::init_volume() {
    int i, anm_no;

    mVolumeNum = getVolume();
    setVolume(10);

    for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
        getController(i)->connect();
    }

    for (i = 0; i < 10; i++) {
        if (i < mVolumeNum) {
            anm_no = findGroupAnimator(i + 0x15, 10);
            mpGroupAnmController[anm_no]->start();
        } else {
            anm_no = findGroupAnimator(i + 0x15, 9);
            mpGroupAnmController[anm_no]->start();
        }
    }
}

void HomeButton::init_vib() {
    int anm_no;

    mVibFlag = getVibFlag();
    if (mVibFlag) {
        anm_no = findGroupAnimator(13, 6);
        mpGroupAnmController[anm_no]->start();

        anm_no = findGroupAnimator(14, 8);
        mpGroupAnmController[anm_no]->start();
    } else {
        anm_no = findGroupAnimator(13, 8);
        mpGroupAnmController[anm_no]->start();

        anm_no = findGroupAnimator(14, 6);
        mpGroupAnmController[anm_no]->start();
    }
}

void HomeButton::init_sound() {
    if (mpHBInfo->sound_callback) {
        (*mpHBInfo->sound_callback)(0, 0);
    }

    mAppVolume[0] = AXGetAuxAReturnVolume();
    mAppVolume[1] = AXGetAuxBReturnVolume();
    mAppVolume[2] = AXGetAuxCReturnVolume();

    AXFXGetHooks(&mAxFxAlloc, &mAxFxFree);
    AXGetAuxACallback(mAuxCallback, &mpAuxContext);
    AXFXSetHooks(&HBMAllocMem, &HBMFreeMem);

    mAxFxReverb.preDelay = 0.0f;
    mAxFxReverb.time = 2.5f;
    mAxFxReverb.coloration = 0.5f;
    mAxFxReverb.damping = 0.0f;
    mAxFxReverb.crosstalk = 0.0f;
    mAxFxReverb.mix = 1.0f;

    AXFXReverbHiInit(&mAxFxReverb);
    AXRegisterAuxACallback(&AXFXReverbHiCallback, &mAxFxReverb);
    AXSetAuxAReturnVolume(0x8000);
    AXSetAuxBReturnVolume(0);
    AXSetAuxCReturnVolume(0);

    if (mpHBInfo->sound_callback) {
        (*mpHBInfo->sound_callback)(1, 0);
    }

    mEndInitSoundFlag = true;
}

void HomeButton::init_battery(const HBMControllerData* pController) {
    int anm_no;

    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        if (pController->wiiCon[i].kpad) {
            if (!mpHBInfo->cursor) {
                mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorPaneName, true)->SetVisible(true);
            }

            anm_no = findGroupAnimator(i + 31, 17);
            mpGroupAnmController[anm_no]->start();
            mControllerFlag[i] = true;

            getController(i)->getInfoAsync(&sWpadInfo[i]);
        } else {
            if (!mpHBInfo->cursor) {
                mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorPaneName, true)->SetVisible(false);
            }

            anm_no = findGroupAnimator(i + 31, 16);
            mpGroupAnmController[anm_no]->start();
            mControllerFlag[i] = false;
        }
    }

    reset_battery();
    mGetPadInfoTime = 0;
}

void HomeButton::calc(const HBMControllerData* pController) {
    int i;

    mpPaneManager->calc();

    for (i = 0; i < mAnmNum; i++) {
        mpAnmController[i]->do_calc();
    }

    for (i = 0; i < (int)ARRAY_COUNT(mpPairGroupAnmController); i++) {
        mpPairGroupAnmController[i]->do_calc();
    }

    for (i = 0; i < (int)ARRAY_COUNT(mpGroupAnmController); i++) {
        mpGroupAnmController[i]->do_calc();
    }

    for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        if (mOnPaneVibFrame[i] > 0.0f) {
            if (!mControllerFlag[i] || !getController(i)->isRumbling()) {
                mOnPaneVibFrame[i] = 0.0f;
                mOnPaneVibWaitFrame[i] = 0.0f;

                if (getController(i)->isRumbling()) {
                    getController(i)->stopMotor();
                }

                continue;
            }

            mOnPaneVibFrame[i] -= mpHBInfo->frameDelta;
            if (mOnPaneVibFrame[i] <= 0.0f || mState == 17) {
                getController(i)->stopMotor();
                mOnPaneVibFrame[i] = 0.0f;
                mOnPaneVibWaitFrame[i] = 9.0f;
            }
            continue;
        } else if (mOnPaneVibWaitFrame[i] > 0.0f) {
            mOnPaneVibWaitFrame[i] -= mpHBInfo->frameDelta;

            if (mOnPaneVibWaitFrame[i] <= 0.0f) {
                mOnPaneVibWaitFrame[i] = 0.0f;
            }
        }
    }

    switch (mState) {
        case 0:
            if (mpHBInfo->backFlag) {
                mSelectAnmNum = findGroupAnimator(2, 0);

                mpLayout->GetRootPane()->FindPaneByName("back_00", true)->SetVisible(false);

                mpLayout->GetRootPane()->FindPaneByName("back_02", true)->SetVisible(true);
            } else {
                mSelectAnmNum = findGroupAnimator(0, 0);

                mpLayout->GetRootPane()->FindPaneByName("back_00", true)->SetVisible(true);

                mpLayout->GetRootPane()->FindPaneByName("back_02", true)->SetVisible(false);
            }

            mpGroupAnmController[mSelectAnmNum]->start();

            if (pController) {
                mState = 1;
                init_battery(pController);
            }

            break;

        case 1:
            if (!mpGroupAnmController[mSelectAnmNum]->isPlaying()) {
                init_volume();
                init_vib();
                init_sound();
                play_sound(0);

                mState = 2;
            }

            break;

        default:
        case 2:
            break;

        case 3:
            if (!mpGroupAnmController[mSelectAnmNum]->isPlaying() && mSelectAnmNum != 5) {
                reset_battery();
                mSelectAnmNum = 5;
                mpPairGroupAnmController[mSelectAnmNum]->start();
            }

            mWaitStopMotorCount--;
            if (mWaitStopMotorCount <= 0) {
                for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                    WPADDisconnect(i);
                }

                mState = 4;
            }

            break;

        case 4:
            if (mpGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                WPADDeviceType type;
                if (WPADProbe(i, &type) != WPAD_ENODEV) {
                    break;
                }
            }

            if (i < WPAD_MAX_CONTROLLERS) {
                break;
            }

            mDisConnectCount = 0;

            mState = 5;
            mMsgCount = 0;
            mSoundRetryCnt = 0;
            mSimpleSyncCallback = WPADSetSimpleSyncCallback(&SimpleSyncCallback);
            mEndSimpleSyncFlag = false;
            mForthConnectFlag = false;

            // for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
            //     getController(i)->setEnableRumble(true);
            // }

            mSimpleSyncFlag = WPADStartFastSimpleSync();

            if (!mSimpleSyncFlag) {
                setSimpleSyncAlarm(0);
            }

            break;

        case 5:
        case 6:
            if (!mSimpleSyncFlag || mpPairGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            if (mMsgCount == 0) {
                reset_control();
                reset_btn();
                mpPairGroupAnmController[14]->setAnmType(2);
                mpPairGroupAnmController[14]->start();
            }

            for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                if (!mControllerFlag[i]) {
                    break;
                }
            }

            if (i >= WPAD_MAX_CONTROLLERS) {
                mForthConnectFlag = true;
            }

            if (mForthConnectFlag) {
                if (mState != 6) {
                    if (!getController(mConnectNum)->isPlayReady() || getController(mConnectNum)->isPlayingSoundId(5)) {
                        mState = 6;
                        mMsgCount = iReConnectTime2;
                    }

                    ++mSoundRetryCnt;
                    if (mSoundRetryCnt <= iReConnectTime2) {
                        break;
                    }

                    mState = 6;
                    mMsgCount = iReConnectTime2;
                    break;
                }

                ++mMsgCount;
                if (mMsgCount <= iReConnectTime) {
                    break;
                }

                mState = 7;

                if (!WPADStopSimpleSync()) {
                    setSimpleSyncAlarm(1);
                }

                mEndSimpleSyncFlag = true;
            } else {
                ++mMsgCount;
                if (mMsgCount > iReConnectTime) {
                    mState = 7;

                    if (!WPADStopSimpleSync()) {
                        setSimpleSyncAlarm(1);
                    }

                    mEndSimpleSyncFlag = true;
                }
            }

            break;

        case 7:
            if (!mEndSimpleSyncFlag) {
                break;
            }

            WPADSetSimpleSyncCallback(mSimpleSyncCallback);
            mpRemoteSpk->ClearPcm();
            reset_guiManager(-1);

            mSelectAnmNum = 6;
            mpPairGroupAnmController[mSelectAnmNum]->start();

            mState = 8;
            mpPairGroupAnmController[14]->setAnmType(0);

            play_sound(21);

            break;

        case 8:
            if (mpPairGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            if (mSelectAnmNum == 13) {
                reset_window();

                mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[2], true)->SetVisible(false);
            } else if (mSelectAnmNum == 6) {
                mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[0], true)->SetVisible(false);

                mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[1], true)->SetVisible(false);
            }

#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
            mState = 2;
#elif HBM_APP_TYPE == HBM_APP_TYPE_NAND
            if (mSequence <= eSeq_Cmn) {
                mState = 2;
            } else {
                mSelectBtnNum = HBM_SELECT_BTN4;
                mState = 21;
            }
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

            break;

        case 9:
            if (mpGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            if (mVibFlag) {
                for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                    getController(i)->stopMotor();
                }
            }

            mState = 2;

            break;

        case 10:
            if (mpPairGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            mBar0AnmRev = 0;
            mBar1AnmRev = 0;
            mBar0AnmRevHold = 0;
            mBar1AnmRevHold = 0;

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
            mBar2AnmRev = 0;
            mBar2AnmRevHold = 0;
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

            if (mSequence != eSeq_Control) {
                // 2-6: "B_optnBtn_XX" entries in scFuncTouchPaneName
                for (i = 2; i < 7; i++) {
                    mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true)->SetVisible(false);
                }

                mState = 2;
            } else if (mSequence == eSeq_Control) // ? already true
            {
                mpLayout->GetRootPane()->FindPaneByName("bar_00", true)->SetVisible(false);

                mSelectAnmNum = 10;
                mpPairGroupAnmController[mSelectAnmNum]->start();

                mSelectAnmNum = 1;
                play_sound(7);

                mState = 8;
            }

            updateTrigPane();

            break;

        case 11:
            if (mpAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            mSelectAnmNum = 7;
            mpPairGroupAnmController[mSelectAnmNum]->start();

            mState = 12;

            break;

        case 12:
            if (mpPairGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            updateTrigPane();
            reset_btn();
            mState = 2;

            break;

        case 13: {
            if (mpGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            // only case is SELECT_NULL
            if (mSelectBtnNum >= 0) {
                mFader.start();
                mState = 19;
                mFadeOutSeTime = mFader.getMaxFrame();

                if (mSelectBtnNum != HBM_SELECT_BTN3 && mpHBInfo->sound_callback) {
                    (*mpHBInfo->sound_callback)(3, mFadeOutSeTime);
                }
            } else {
                updateTrigPane();
                mSelectAnmNum = 13;
                mpPairGroupAnmController[mSelectAnmNum]->start();
                mState = 8;
            }

            reset_guiManager(-1);
        } break;

        case 14:
            if (mpPairGroupAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            calc_fadeoutAnm();
            break;

        case 15:
            if (mpAnmController[mSelectAnmNum]->isPlaying()) {
                break;
            }

            mFader.start();
            mStartBlackOutFlag = true;
            mState = 19;
            mFadeOutSeTime = mFader.getMaxFrame();

            if (mSelectBtnNum != HBM_SELECT_BTN3 && mpHBInfo->sound_callback) {
                (*mpHBInfo->sound_callback)(3, mFadeOutSeTime);
            }

            break;

        case 16: {
            GroupAnmController* anim;

            mBatteryCheck = false;

            if (mSequence <= eSeq_Cmn) {
                anim = mpGroupAnmController[mSelectAnmNum];
            }
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
            else {
                anim = mpAnmControllerSet4[mSelectAnmNum];
            }
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

            if (!anim->isPlaying()) {
                mState = 17;

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
                mpLayout2->GetRootPane()->FindPaneByName("bar_00", true)->SetAlpha(0);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

                fadeout_sound(0.0f);

                mpRemoteSpk->ClearPcm();
                mpRemoteSpk->Stop();

                for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                    mpController[i]->stopMotor();
                    mpController[i]->clearCallback();
                }
            } else {
                f32 restFrame = anim->getMaxFrame() - anim->getCurrentFrame();
                fadeout_sound(restFrame / mFadeOutSeTime);

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
                if (restFrame < anim->getMaxFrame() / 4.0f) {
                    u8 myAlpha = 765.0f * restFrame / anim->getMaxFrame(); // 765?

                    mpLayout2->GetRootPane()->FindPaneByName("bar_00", true)->SetAlpha(myAlpha);
                }
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
            }
        } break;

        case 17:
            mState = 18;

            if (mSelectBtnNum != HBM_SELECT_BTN3) {
                StopAllSeq();

                if (mEndInitSoundFlag) {
                    AXFXReverbHiShutdown(&mAxFxReverb);
                    AXRegisterAuxACallback(*mAuxCallback, mpAuxContext);
                    AXFXSetHooks(mAxFxAlloc, mAxFxFree);
                    AXSetAuxAReturnVolume(mAppVolume[0]);
                    AXSetAuxBReturnVolume(mAppVolume[1]);
                    AXSetAuxCReturnVolume(mAppVolume[2]);
                }
                mEndInitSoundFlag = false;
            }

            setVolume(mVolumeNum);
            WPADSaveConfig(nullptr);

            if (mSelectBtnNum != HBM_SELECT_BTN3 && mpHBInfo->sound_callback) {
                (*mpHBInfo->sound_callback)(4, 0);
            }

            for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                int anm_no;

                anm_no = findGroupAnimator(i + 31, 17);
                mpGroupAnmController[anm_no]->stop();

                anm_no = findGroupAnimator(i + 31, 18);
                mpGroupAnmController[anm_no]->stop();

                anm_no = findGroupAnimator(i + 31, 15);
                mpGroupAnmController[anm_no]->stop();
            }

            mInitFlag = false;
            break;

        case 18:
            mState = 2;
            break;

        case 19:
            if (mForceSttInitProcFlag) {
                init_battery(pController);
                mForceSttInitProcFlag = false;
            }

            if (mForceSttFadeInProcFlag) {
                init_volume();
                init_vib();
                mForceSttFadeInProcFlag = false;
            }

            if (mFader.isDone()) {
                if (mForceStopSyncFlag) {
                    if (!mEndSimpleSyncFlag) {
                        break;
                    }

                    WPADSetSimpleSyncCallback(mSimpleSyncCallback);
                    mForceStopSyncFlag = false;
                }

                if (mForceEndMsgAnmFlag) {
                    int anm_no;

                    anm_no = 5;
                    mpPairGroupAnmController[anm_no]->initFrame();
                    mpPairGroupAnmController[anm_no]->stop();

                    mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[0], true)->SetVisible(false);
                    mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[1], true)->SetVisible(false);

                    anm_no = 14;
                    if (mpPairGroupAnmController[anm_no]->isPlaying()) {
                        mpPairGroupAnmController[anm_no]->initFrame();
                        mpPairGroupAnmController[anm_no]->stop();
                    }
                }

                mState = 17;

                VISetBlack(iVISetBlackFlag || !mFader.getFadeColorEnable());
                VIFlush();
                fadeout_sound(0.0f);
            } else {
                f32 restFrame = mFader.getMaxFrame() - mFader.getFrame();

                fadeout_sound(restFrame / mFadeOutSeTime);
            }

            break;

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
        case 20:
            if (mSelectAnmNum == 4) {
                if (!mpPairGroupAnmController[4]->isPlaying()) {
                    mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[2], true)->SetVisible(false);

                    if (mpHBInfo->backFlag) {
                        mSelectAnmNum = findGroupAnimator(3, 1);
                    } else {
                        mSelectAnmNum = findGroupAnimator(1, 1);
                    }

                    mpGroupAnmController[mSelectAnmNum]->start();
                }
            } else if (!mpGroupAnmController[mSelectAnmNum]->isPlaying()) {
                mSelectAnmNum = 5;
                mpAnmControllerSet4[mSelectAnmNum]->start();

                mState = 22;

                nw4hbm::lyt::Pane* pJpgPane = mpLayout2->GetRootPane()->FindPaneByName("jpg", true);

                nw4hbm::lyt::Material* pJpgMaterial = pJpgPane->GetMaterial();

                pJpgPane->SetVisible(true);
                pJpgPane->SetSize(nw4hbm::lyt::Size(mpHBInfoEx->texImageWidth, mpHBInfoEx->texImageHeight));

                if (mAdjustFlag) {
                    pJpgPane->SetScale(nw4hbm::math::VEC2(1.0f / mpHBInfo->adjust.x, 1.0f / mpHBInfo->adjust.y));
                } else {
                    pJpgPane->SetScale(nw4hbm::math::VEC2(1.0f, 1.0f));
                }

                GXInitTexObj(&mTexObj, mpHBInfoEx->texImage, mpHBInfoEx->texImageWidth, mpHBInfoEx->texImageHeight,
                             mpHBInfoEx->texImageFormat, GX_CLAMP, GX_CLAMP, false);

                pJpgMaterial->SetTexture(0, mTexObj);
            }

            break;

        case 22:
            mpLayout2->GetRootPane()->SetVisible(true);

            if (!mpAnmControllerSet4[mSelectAnmNum]->isPlaying()) {
                updateTrigPane();

                for (int i = 0; i < 8; i++) {
                    mpPaneManager->update(i, -10000.0f, -10000.0f, 0, 0, 0, nullptr);
                }

                mBar2AnmRev = 0;
                mBar2AnmRevHold = 0;
                mState = 21;
            }

        case 21:
            mpLayout->GetRootPane()->FindPaneByName("N_cntrl_01", true)->SetVisible(false);

            mpLayout->GetRootPane()->FindPaneByName("bar_00", true)->SetVisible(false);

            mpLayout->GetRootPane()->FindPaneByName("bar_10", true)->SetVisible(false);

            break;

        case 23:
            mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[2], true)->SetVisible(false);

            mState = 16;
            mFadeOutSeTime = mpAnmControllerSet4[mSelectAnmNum]->getMaxFrame();

            if (mpHBInfo->sound_callback) {
                (*mpHBInfo->sound_callback)(2, mFadeOutSeTime);
            }

            break;
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
    }

    if (mBar0AnmRev && isUpBarActive()) {
        if (mBar0AnmRev && mBar0AnmRev != mBar0AnmRevHold) {
            mpPairGroupAnmController[mBar0AnmRev]->start();
            mBar0AnmRevHold = mBar0AnmRev;
        }

        mBar0AnmRev = 0;
    }

    if (mBar1AnmRev && isDownBarActive()) {
        if (mBar1AnmRev && mBar1AnmRev != mBar1AnmRevHold) {
            mpGroupAnmController[mBar1AnmRev]->start();
            mBar1AnmRevHold = mBar1AnmRev;
        }

        mBar1AnmRev = 0;
    }

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    if (mBar2AnmRev && isThirdBarActive()) {
        if (mBar2AnmRev && mBar2AnmRev != mBar2AnmRevHold) {
            mpAnmControllerSet4[mBar2AnmRev]->start();

            mBar2AnmRevHold = mBar2AnmRev;
        }

        mBar2AnmRev = 0;
    }
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

    if (pController && mBatteryCheck) {
        update(pController);
    }

    mpLayout->Animate(0);
    mpLayout->CalculateMtx(mDrawInfo);

#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    mpLayout2->Animate(0);
    mpLayout2->CalculateMtx(mDrawInfo);

    if (mState == 20) {
        nw4hbm::lyt::Pane* pPane = mpLayout->GetRootPane()->FindPaneByName("bar_00", true);

        pPane->SetColorElement(0, 0);
        pPane->SetColorElement(1, 0);
        pPane->SetColorElement(2, 0);

        pPane->SetColorElement(4, 0);
        pPane->SetColorElement(5, 0);
        pPane->SetColorElement(6, 0);

        pPane->SetColorElement(8, 0);
        pPane->SetColorElement(9, 0);
        pPane->SetColorElement(10, 0);

        pPane->SetColorElement(12, 0);
        pPane->SetColorElement(13, 0);
        pPane->SetColorElement(14, 0);
    }
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

    if (!mpHBInfo->cursor) {
        for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
            mpCursorLayout[i]->CalculateMtx(mDrawInfo);
        }
    }

    if (mForcusSEWaitTime <= 2) {
        mForcusSEWaitTime++;
    }
}

void HomeButton::calc_battery(int chan) {
    // presumably j because it is the second index
    for (int j = 0; j < (int)ARRAY_COUNT(scBatteryPaneName[chan]); j++) {
        if (j < sWpadInfo[chan].battery) {
            mpLayout->GetRootPane()->FindPaneByName(scBatteryPaneName[chan][j], true)->SetVisible(true);
        } else {
            mpLayout->GetRootPane()->FindPaneByName(scBatteryPaneName[chan][j], true)->SetVisible(false);
        }
    }

    if (sWpadInfo[chan].battery < 2) {
        int anm_no = findGroupAnimator(chan + 31, 21);
        mpGroupAnmController[anm_no]->start();
    } else {
        int anm_no = findGroupAnimator(chan + 31, 17);
        mpGroupAnmController[anm_no]->start();
    }

    if (mGetPadInfoTime < 100) {
        mGetPadInfoTime = 0;
    }

    getController(chan)->clrBatteryFlag();
}

static void SpeakerCallback(OSAlarm* alm, OSContext*) {
    u32 data = OSGetAlarmUserDataAny(u32, alm);
    int chan = (data >> 16) & 0xffff;
    int id = data & 0xffff;

    HomeButton* pHBObj = HomeButton::getInstance();
    OSAssert_Line(1979, pHBObj);

    if (!WPADIsSpeakerEnabled(chan) || !pHBObj->getController(chan)->isPlayReady()) {
        pHBObj->setSpeakerAlarm(chan, 50);
    } else {
        pHBObj->getController(chan)->playSound(chan, id);
    }
}

static void MotorCallback(OSAlarm* alm, OSContext*) {
    Controller* pController = OSGetAlarmUserDataAny(Controller*, alm);

    pController->stopMotor();
}

void HomeButton::setSpeakerAlarm(int chan, int msec) {
    OSSetAlarmUserDataAny(&mSpeakerAlarm[chan], (chan << 16) | (chan + 2));
    OSCancelAlarm(&mSpeakerAlarm[chan]);
    OSSetAlarm(&mSpeakerAlarm[chan], OSMillisecondsToTicks(msec), &SpeakerCallback);
}

static void RetrySimpleSyncCallback(OSAlarm* alm, OSContext*) {
    HomeButton* pHBObj = HomeButton::getInstance();
    int type = OSGetAlarmUserDataAny(int, alm);
    bool retrySuccessFlag = false;

    OSAssert_Line(2014, pHBObj);

    if (type == 0) {
        if (WPADStartFastSimpleSync()) {
            pHBObj->setSimpleSyncFlag(true);
            retrySuccessFlag = true;
        }
    } else {
        if (WPADStopSimpleSync()) {
            retrySuccessFlag = true;
        }
    }

    if (!retrySuccessFlag) {
        pHBObj->setSimpleSyncAlarm(type);
    }
}

static void SimpleSyncCallback(s32 result, s32 num) {
    OSAssert_Line(2046, HomeButton::getInstance());

    if (result == 1) {
        HomeButton::getInstance()->setEndSimpleSyncFlag(true);
    }

    HomeButton::getInstance()->callSimpleSyncCallback(result, num);
}

void HomeButton::setSimpleSyncAlarm(int type) {
    OSCancelAlarm(&mSimpleSyncAlarm);
    OSSetAlarmUserDataAny(&mSimpleSyncAlarm, type);
    OSSetAlarm(&mSimpleSyncAlarm, OSMillisecondsToTicks(100), &RetrySimpleSyncCallback);
}

void HomeButton::callSimpleSyncCallback(s32 result, s32 num) {
    if (mSimpleSyncCallback) {
        (*mSimpleSyncCallback)(result, num);
    }
}

void HomeButton::update(const HBMControllerData* pController) {
#define IsValidDevType_(x) ((x)->use_devtype == WPAD_DEV_CLASSIC && (x)->kpad->dev_type == WPAD_DEV_CLASSIC)

    int i, anm_no;

    mFader.calc();

    for (i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        if (pController->wiiCon[i].kpad) {
            if (pController->wiiCon[i].kpad->wpad_err != WPAD_ENODEV) {
                if (mPadDrawTime[i] > static_cast<int>(5.0f / getInstance()->getHBMDataInfo()->frameDelta + 0.5f)) {
                    if (pController->wiiCon[i].kpad->wpad_err == WPAD_ESUCCESS ||
                        pController->wiiCon[i].kpad->wpad_err == WPAD_EBADE) {
                        bool pointerEnableFlag;

                        if (!IsValidDevType_(&pController->wiiCon[i])) {
                            if (pController->wiiCon[i].kpad->dpd_valid_fg > 0) {
                                pointerEnableFlag = true;
                            } else {
                                pointerEnableFlag = false;
                            }
                        } else {
                            pointerEnableFlag = true;
                        }

                        mpController[i]->setKpad(&pController->wiiCon[i], pointerEnableFlag);

                        if (!mpHBInfo->cursor) {
                            mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorPaneName, true)->SetVisible(true);
                        }
                    }
                } else {
                    mPadDrawTime[i]++;
                }

                if (!IsValidDevType_(&pController->wiiCon[i]) && pController->wiiCon[i].kpad->dpd_valid_fg <= 0) {
                    WPADResult result;
                    WPADDeviceType type;

                    result = WPADProbe(i, &type);

                    if (pController->wiiCon[i].kpad->wpad_err != WPAD_ECOMM && result != WPAD_ECOMM) {
                        mpController[i]->setInValidPos();
                    }
                }
            } else {
                mpController[i]->setInValidPos();

                if (!mpHBInfo->cursor) {
                    mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorPaneName, true)->SetVisible(false);
                }
            }

            if (!mControllerFlag[i]) {
                mConnectNum = i;
                mControllerFlag[i] = true;

                getController(i)->getInfoAsync(&sWpadInfo[i]);

                anm_no = findGroupAnimator(i + 31, 17);
                mpGroupAnmController[anm_no]->start();

                anm_no = findGroupAnimator(i + 31, 18);
                mpGroupAnmController[anm_no]->stop();

                anm_no = findGroupAnimator(i + 31, 15);
                mpGroupAnmController[anm_no]->start();

                play_sound(i + 17);
                getController(i)->connect();
                getController(i)->startMotor();

                OSSetAlarmUserDataAny(&mAlarm[i], getController(i));
                OSCancelAlarm(&mAlarm[i]);
                OSSetAlarm(&mAlarm[i], OSMillisecondsToTicks(300), &MotorCallback);

                setSpeakerAlarm(i, 400);
            }

            if (pController->wiiCon[i].kpad->wpad_err == 0 || pController->wiiCon[i].kpad->wpad_err == -7) {
                nw4hbm::math::VEC3 vec;

                if (IsValidDevType_(&pController->wiiCon[i])) {
                    vec = nw4hbm::math::VEC3(0.0f, 0.0f, 15.0f);
                } else {
                    Vec2 v = pController->wiiCon[i].kpad->horizon;
                    f32 mRad = nw4hbm::math::Atan2Deg(-v.y, v.x);

                    vec = nw4hbm::math::VEC3(0.0f, 0.0f, mRad);
                }

                if (!mpHBInfo->cursor) {
                    mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorRotPaneName, true)->SetRotate(vec);

                    mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorSRotPaneName, true)->SetRotate(vec);
                }

                if (mGetPadInfoTime > 100) {
                    getController(i)->getInfoAsync(&sWpadInfo[i]);
                }

                update_controller(i);

                if (!mpHBInfo->cursor) {
                    update_posController(i);
                }
            }

            if (getController(i)->getBatteryFlag()) {
                calc_battery(i);
            }
        } else {
            if (mControllerFlag[i]) {
                anm_no = findGroupAnimator(i + 31, 17);
                mpGroupAnmController[anm_no]->start();

                anm_no = findGroupAnimator(i + 31, 18);
                mpGroupAnmController[anm_no]->start();

                for (int j = 0; j < (int)ARRAY_COUNT(scBatteryPaneName[i]); j++) {
                    mpLayout->GetRootPane()->FindPaneByName(scBatteryPaneName[i][j], true)->SetVisible(false);
                }

                if (!mpHBInfo->cursor) {
                    mpCursorLayout[i]->GetRootPane()->FindPaneByName(scCursorPaneName, true)->SetVisible(false);
                }

                mPadDrawTime[i] = 0;
                mControllerFlag[i] = false;

                mpController[i]->setInValidPos();
                mpController[i]->clrKpadButton();
                mpController[i]->disconnect();
            }

            reset_guiManager(i);
        }
    }

    if (mGetPadInfoTime > 100) {
        mGetPadInfoTime = 0;
    } else {
        mGetPadInfoTime++;
    }
#undef IsValidDevType_
}

void HomeButton::update_controller(int id) {
    int anm_no;

    if (isActive()) {
        HBController* pCon = mpController[id]->getController();

        f32 x = pCon->x * 608.0f / 2.0f;
        f32 y = pCon->y * 456.0f / 2.0f;

        if (mAdjustFlag) {
            x *= mpHBInfo->adjust.x;
            y *= mpHBInfo->adjust.y;
        }

        mpPaneManager->update(id, x, -y, pCon->trig, pCon->hold, pCon->release, pCon);

        if (pCon->trig & WPAD_BUTTON_HOME && isActive()) {
            if (mSequence == eSeq_Control) {
                mpLayout->GetRootPane()->FindPaneByName("bar_00", true)->SetVisible(true);

                mpPaneManager->update(id, 0.0f, -180.0f, 0, 0, 0, 0);

                mpPairGroupAnmController[4]->start();

                mSelectAnmNum = 2;
                mpPairGroupAnmController[mSelectAnmNum]->start();

                int anm_no = 11; // ?
                mpPairGroupAnmController[anm_no]->start();

                mState = 10;
                mSequence = eSeq_Normal;
                play_sound(8);
            } else if (mSequence == eSeq_Normal) {
                if (mpPairGroupAnmController[3]->isPlaying()) {
                    mpPairGroupAnmController[3]->stop();
                }

                if (mpPairGroupAnmController[12]->isPlaying()) {
                    mpPairGroupAnmController[12]->stop();
                }

                mSelectBtnNum = HBM_SELECT_HOMEBTN;

                mSelectAnmNum = 4;
                mpPairGroupAnmController[mSelectAnmNum]->start();

                mState = 14;
                play_sound(1);
            }
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
            else if (mSequence == eSeq_Seq3) {
                if (mpAnmControllerSet4[3]->isPlaying()) {
                    mpAnmControllerSet4[3]->stop();
                }

                if (mpAnmControllerSet4[4]->isPlaying()) {
                    mpAnmControllerSet4[4]->stop();
                }

                mSelectBtnNum = HBM_SELECT_HOMEBTN;

                mSelectAnmNum = 6;
                mpAnmControllerSet4[mSelectAnmNum]->start();

                mState = 23;
                play_sound(1);
            }
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
        } else if (mSequence == eSeq_Control && isActive()) {
            if (pCon->trig & WPAD_BUTTON_MINUS) {
                if (mVolumeNum > 0) {
                    mVolumeNum--;

                    anm_no = findGroupAnimator(mVolumeNum + 21, 10);
                    mpGroupAnmController[anm_no]->stop();

                    anm_no = findGroupAnimator(mVolumeNum + 21, 9);
                    mpGroupAnmController[anm_no]->start();

                    anm_no = findGroupAnimator(11, 5);
                    mpGroupAnmController[anm_no]->start();

                    if (mVolumeNum == 0) {
                        play_sound(12);

                        for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                            getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                            getController(i)->playSound(i, 1);
                        }
                    } else {
                        play_sound(10);

                        for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                            getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                            getController(i)->playSound(i, 1);
                        }
                    }
                } else {
                    play_sound(13);
                }
            } else if (pCon->trig & WPAD_BUTTON_PLUS) {
                if (mVolumeNum < 10) {
                    anm_no = findGroupAnimator(mVolumeNum + 21, 9);
                    mpGroupAnmController[anm_no]->stop();

                    anm_no = findGroupAnimator(mVolumeNum + 21, 10);
                    mpGroupAnmController[anm_no]->start();

                    mVolumeNum++;

                    anm_no = findGroupAnimator(12, 5);
                    mpGroupAnmController[anm_no]->start();

                    if (mVolumeNum == 10) {
                        play_sound(11);

                        for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                            getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                            getController(i)->playSound(i, 1);
                        }
                    } else {
                        play_sound(9);

                        for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                            getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                            getController(i)->playSound(i, 1);
                        }
                    }
                } else {
                    play_sound(13);
                }
            }
        }
    } else if (mSequence == eSeq_Control && mState == 5 && !mpPairGroupAnmController[mSelectAnmNum]->isPlaying()) {
        HBController* pCon = mpController[id]->getController();
        if (pCon->trig) {
            mMsgCount = iReConnectTime;
        }
    }
}

void HomeButton::update_posController(int id) {
    HBController* pCon = mpController[id]->getController();
    nw4hbm::ut::Rect layoutRect = mpLayout->GetLayoutRect();

    f32 x = pCon->x * layoutRect.right;
    f32 y = pCon->y * layoutRect.bottom;
    nw4hbm::math::VEC2 pos(x, y);

    mpCursorLayout[id]->GetRootPane()->FindPaneByName(scCursorPaneName, true)->SetTranslate(pos);
}

void HomeButton::updateTrigPane() {
    int i;

    switch (mSequence) {
        case eSeq_Normal:
            for (i = 0; i < (int)ARRAY_COUNT(scFuncTouchPaneName); i++) {
                if (i < 2 || i == 9) {
                    nw4hbm::lyt::Pane* pTouchPane =
                        mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

                    mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(true);
                } else {
                    nw4hbm::lyt::Pane* pTouchPane =
                        mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

                    mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(false);
                }
            }

            break;

        case eSeq_Control:
            for (i = 0; i < (int)ARRAY_COUNT(scFuncTouchPaneName); i++) {
                if ((i > 0 && i < 7) || i == 9) {
                    nw4hbm::lyt::Pane* pTouchPane =
                        mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

                    mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(true);
                } else {
                    nw4hbm::lyt::Pane* pTouchPane =
                        mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

                    mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(false);
                }
            }

            break;

        case eSeq_Cmn:
            for (i = 0; i < (int)ARRAY_COUNT(scFuncTouchPaneName); i++) {
                if (i < 7 || i == 9) {
                    nw4hbm::lyt::Pane* pTouchPane =
                        mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

                    mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(false);
                } else {
                    nw4hbm::lyt::Pane* pTouchPane =
                        mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

                    mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(true);
                }
            }

            break;

            // #if HBM_APP_TYPE == HBM_APP_TYPE_NAND
            //         case eSeq_Seq3:
            //             for (i = 0; i < (int)ARRAY_COUNT(scFuncTouchPaneName); i++) {
            //                 nw4hbm::lyt::Pane* pTouchPane =
            //                 mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

            //                 mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(false);
            //             }

            //             for (i = 0; i < (int)ARRAY_COUNT(scBtnWareName); i++) {
            //                 nw4hbm::lyt::Pane* pBtnWarePane =
            //                 mpLayout2->GetRootPane()->FindPaneByName(scBtnWareName[i], true);

            //                 mpPaneManager->getPaneComponentByPane(pBtnWarePane)->setTriggerTarget(true);
            //             }

            //             break;

            //         case eSeq_Seq4:
            //             for (i = 0; i < (int)ARRAY_COUNT(scBtnWareName); i++) {
            //                 nw4hbm::lyt::Pane* pBtnWarePane =
            //                 mpLayout2->GetRootPane()->FindPaneByName(scBtnWareName[i], true);

            //                 mpPaneManager->getPaneComponentByPane(pBtnWarePane)->setTriggerTarget(false);
            //             }

            //             for (i = 0; i < (int)ARRAY_COUNT(scFuncTouchPaneName); i++) {
            //                 if (i >= 7 && i != 9) {
            //                     nw4hbm::lyt::Pane* pTouchPane =
            //                         mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true);

            //                     mpPaneManager->getPaneComponentByPane(pTouchPane)->setTriggerTarget(true);
            //                 }
            //             }

            //             break;
            // #endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
    }
}

void HomeButton::startPointEvent(const nw4hbm::lyt::Pane* pPane, void* pData) {
    int anm_no;
    int btn_no = getPaneNo(pPane);
    HBController* pCon = static_cast<HBController*>(pData);
    bool onFlag = false;

    if (isActive() && btn_no != -1 && !mPaneCounter[btn_no]) {
        if (mSequence != eSeq_Cmn && btn_no < mButtonNum) {
            anm_no = findAnimator(btn_no, 0);
            mpAnmController[anm_no]->start();

            if (mSequence == eSeq_Normal) {
                setForcusSE();
                onFlag = true;
            }
        } else {
            switch (btn_no - mButtonNum) {
                case 0:
                    if (mSequence == eSeq_Normal) {
                        if (isUpBarActive()) {
                            mpPairGroupAnmController[3]->start();
                            mBar0AnmRevHold = 3;
                            mBar0AnmRev = 0;
                            setForcusSE();
                            onFlag = true;
                        } else {
                            mBar0AnmRev = 3;
                        }
                    }

                    break;

                case 1:
                case 9:
                    if (mSequence == eSeq_Normal) {
                        anm_no = findGroupAnimator(4, 2);

                        if (homebutton::HomeButton::isDownBarActive()) {
                            mpGroupAnmController[anm_no]->start();
                            mBar1AnmRevHold = anm_no;
                            mBar1AnmRev = 0;
                            setForcusSE();
                            onFlag = true;
                        } else {
                            mBar1AnmRev = anm_no;
                        }
                    } else if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(4, 19);

                        if (isDownBarActive()) {
                            mpGroupAnmController[anm_no]->start();
                            mBar1AnmRevHold = anm_no;
                            mBar1AnmRev = 0;
                            setForcusSE();
                            onFlag = true;
                        } else {
                            mBar1AnmRev = anm_no;
                        }
                    }

                    break;

                case 2:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(6, 4);
                        mpGroupAnmController[anm_no]->start();

                        setForcusSE();
                        onFlag = true;
                    }

                    break;

                case 3:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(7, 4);
                        mpGroupAnmController[anm_no]->start();

                        setForcusSE();
                        onFlag = true;
                    }

                    break;

                case 4:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(8, 4);
                        mpGroupAnmController[anm_no]->start();

                        setForcusSE();
                        onFlag = true;
                    }

                    break;

                case 5:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(9, 4);
                        mpGroupAnmController[anm_no]->start();

                        setForcusSE();
                        onFlag = true;
                    }

                    break;

                case 6:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(10, 4);
                        mpGroupAnmController[anm_no]->start();

                        setForcusSE();
                        onFlag = true;
                    }

                    break;

                case 7:
#if HBM_APP_TYPE == HBM_APP_TYPE_DVD
                    if (mSequence == eSeq_Cmn)
#elif HBM_APP_TYPE == HBM_APP_TYPE_NAND
                    if (mSequence == eSeq_Cmn || mSequence == eSeq_Seq4)
#endif // HBM_APP_TYPE
                    {
                        anm_no = findGroupAnimator(17, 11);
                        mpGroupAnmController[anm_no]->start();

                        setForcusSE();
                        onFlag = true;
                    }

                    break;

                case 8:
                    if (mSequence == eSeq_Cmn) {
                        anm_no = findGroupAnimator(18, 11);
                        mpGroupAnmController[anm_no]->start();

                        setForcusSE();
                        onFlag = true;
                    }

                    break;
            }
        }
    }

    if (btn_no == mButtonNum + 1 || btn_no == mButtonNum + 9) {
        mPaneCounter[mButtonNum + 1]++;
        mPaneCounter[mButtonNum + 9]++;
    } else {
        mPaneCounter[btn_no]++;
    }

    if (onFlag && pCon) {
        if (!getController(pCon->chan)->isRumbling() && mOnPaneVibWaitFrame[pCon->chan] <= 0.0f) {
            mOnPaneVibFrame[pCon->chan] = 3.0f;
            getController(pCon->chan)->startMotor();
        }
    }
}

void HomeButton::startLeftEvent(const nw4hbm::lyt::Pane* pPane) {
    int anm_no;
    int btn_no = getPaneNo(pPane);

    if (0 < mPaneCounter[btn_no]) {
        if (btn_no == mButtonNum + 1 || btn_no == mButtonNum + 9) {
            mPaneCounter[mButtonNum + 1]--;
            mPaneCounter[mButtonNum + 9]--;
        } else {
            mPaneCounter[btn_no]--;
        }
    }

    if (isActive() && btn_no != -1 && !mPaneCounter[btn_no]) {
        if (mSequence != eSeq_Cmn && btn_no < mButtonNum) {
            anm_no = findAnimator(btn_no, 2);
            mpAnmController[anm_no]->start();
        } else {
            switch (btn_no - mButtonNum) {
                case 0:
                    if (mSequence == eSeq_Normal) {
                        if (isUpBarActive()) {
                            mpPairGroupAnmController[12]->start();
                            mBar0AnmRevHold = 12;
                            mBar0AnmRev = 0;
                        } else {
                            mBar0AnmRev = 12;
                        }
                    }

                    break;

                case 1:
                case 9:
                    if (mSequence == eSeq_Normal) {
                        anm_no = findGroupAnimator(5, 3);

                        if (isDownBarActive()) {
                            mpGroupAnmController[anm_no]->start();
                            mBar1AnmRevHold = anm_no;
                            mBar1AnmRev = 0;
                        } else {
                            mBar1AnmRev = anm_no;
                        }
                    } else if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(5, 20);

                        if (isDownBarActive()) {
                            mpGroupAnmController[anm_no]->start();
                            mBar1AnmRevHold = anm_no;
                            mBar1AnmRev = 0;
                        } else {
                            mBar1AnmRev = anm_no;
                        }
                    }

                    break;

                case 2:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(6, 7);
                        mpGroupAnmController[anm_no]->start();
                    }

                    break;

                case 3:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(7, 7);
                        mpGroupAnmController[anm_no]->start();
                    }

                    break;

                case 4:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(8, 7);
                        mpGroupAnmController[anm_no]->start();
                    }

                    break;

                case 5:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(9, 7);
                        mpGroupAnmController[anm_no]->start();
                    }

                    break;

                case 6:
                    if (mSequence == eSeq_Control) {
                        anm_no = findGroupAnimator(10, 7);
                        mpGroupAnmController[anm_no]->start();
                    }

                    break;

                case 7:
                    if (mSequence == eSeq_Cmn) {
                        anm_no = findGroupAnimator(17, 12);
                        mpGroupAnmController[anm_no]->start();
                    }

                    break;

                case 8:
                    if (mSequence == eSeq_Cmn) {
                        anm_no = findGroupAnimator(18, 12);
                        mpGroupAnmController[anm_no]->start();
                    }

                    break;
            }
        }
    }
}

void HomeButton::startTrigEvent(const nw4hbm::lyt::Pane* pPane) {
    int anm_no;
    int btn_no;

    btn_no = getPaneNo(pPane);

    if (isActive() && btn_no != -1) {
        if (mSequence == eSeq_Normal && btn_no < mButtonNum) {
            mSelectBtnNum = static_cast<HBMSelectBtnNum>(btn_no + 1);

            mSelectAnmNum = findAnimator(btn_no + 4, 1);
            mpAnmController[mSelectAnmNum]->start();
            play_sound(5);

            if (mDialogFlag[btn_no]) {
                mState = 11;
                mSequence = eSeq_Cmn;

                nw4hbm::lyt::Pane* p_pane = mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[2], true);

                OSAssert_Line(3004, p_pane);

                nw4hbm::lyt::TextBox* p_text = nw4hbm::ut::DynamicCast<nw4hbm::lyt::TextBox*>(p_pane);

                OSAssert_Line(3006, p_text);

                u16 len;
                if (mpHBInfo->messageFlag & btn_no + 1) {
                    for (len = 0; true; len++) {
                        // U+FF1F FULLWIDTH QUESTION MARK
                        if (mpText[mpHBInfo->region][btn_no + 2][len] == L'？') {
                            break;
                        }

                        // U+003F QUESTION MARK
                        if (mpText[mpHBInfo->region][btn_no + 2][len] == L'\?') {
                            break;
                        }
                    }
                } else {
                    for (len = 0; true; len++) {
                        // U+0022 QUOTATION MARK
                        if (mpText[mpHBInfo->region][btn_no + 2][len + 1] == L'\"') {
                            break;
                        }
                    }
                }

                p_text->SetString(mpText[mpHBInfo->region][btn_no + 2], 0, ++len);

                mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[2], true)->SetVisible(true);
            } else {
                mState = 15;
            }
        } else {
            switch (btn_no - mButtonNum) {
                case 0:
                    if (mpPairGroupAnmController[12]->isPlaying()) {
                        mpPairGroupAnmController[12]->stop();
                    }

                    if (mpPairGroupAnmController[3]->isPlaying()) {
                        mpPairGroupAnmController[3]->stop();
                    }

                    mSelectBtnNum = HBM_SELECT_HOMEBTN;
                    mSelectAnmNum = 4;
                    mpPairGroupAnmController[mSelectAnmNum]->start();

                    mState = 14;
                    play_sound(1);

                    break;

                case 1:
                case 9:
                    if (mSequence == eSeq_Control) {
                        mpLayout->GetRootPane()->FindPaneByName("bar_00", true)->SetVisible(true);

                        mpPairGroupAnmController[4]->start();
                        mpPairGroupAnmController[11]->start();

                        mSelectAnmNum = 2;
                        mpPairGroupAnmController[mSelectAnmNum]->start();

                        mState = 10;
                        mSequence = eSeq_Normal;
                        play_sound(8);
                    } else if (mSequence == eSeq_Normal) {
                        mpPairGroupAnmController[1]->start();
                        mSelectAnmNum = 9;
                        mpPairGroupAnmController[mSelectAnmNum]->start();
                        mState = 10;

                        for (int i = 2; i < 7; i++) {
                            mpLayout->GetRootPane()->FindPaneByName(scFuncTouchPaneName[i], true)->SetVisible(true);
                        }

                        mSequence = eSeq_Control;
                        play_sound(5);
                    }

                    break;

                case 2:
                    if (mVolumeNum > 0) {
                        mVolumeNum--;

                        anm_no = findGroupAnimator(mVolumeNum + 21, 10);
                        mpGroupAnmController[anm_no]->stop();

                        anm_no = findGroupAnimator(mVolumeNum + 21, 9);
                        mpGroupAnmController[anm_no]->start();

                        if (mVolumeNum == 0) {
                            play_sound(12);

                            for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                                getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                                getController(i)->playSound(i, 1);
                            }
                        } else {
                            play_sound(10);

                            for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                                getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                                getController(i)->playSound(i, 1);
                            }
                        }

                        anm_no = findGroupAnimator(11, 5);
                        mpGroupAnmController[anm_no]->start();
                    } else {
                        play_sound(13);
                    }

                    break;

                case 3:
                    if (mVolumeNum < 10) {
                        anm_no = findGroupAnimator(mVolumeNum + 21, 9);
                        mpGroupAnmController[anm_no]->stop();

                        anm_no = findGroupAnimator(mVolumeNum + 21, 10);
                        mpGroupAnmController[anm_no]->start();

                        mVolumeNum++;

                        if (mVolumeNum == 10) {
                            play_sound(11);

                            for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                                getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                                getController(i)->playSound(i, 1);
                            }
                        } else {
                            play_sound(9);

                            for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                                getController(i)->setSpeakerVol(mVolumeNum / 10.0f);
                                getController(i)->playSound(i, 1);
                            }
                        }

                        anm_no = findGroupAnimator(12, 5);
                        mpGroupAnmController[anm_no]->start();
                    } else {
                        play_sound(13);
                    }

                    break;

                case 4:
                    if (!mVibFlag) {
                        mVibFlag = true;

                        setVibFlag(mVibFlag);

                        anm_no = findGroupAnimator(14, 8);
                        mpGroupAnmController[anm_no]->start();

                        mSelectAnmNum = findGroupAnimator(16, 6);
                        mpGroupAnmController[mSelectAnmNum]->start();

                        play_sound(14);
                    } else {
                        mSelectAnmNum = findGroupAnimator(16, 14);
                        mpGroupAnmController[mSelectAnmNum]->start();

                        play_sound(13);
                    }

                    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                        getController(i)->startMotor();
                        mOnPaneVibFrame[i] = 0.0f;
                        mOnPaneVibWaitFrame[i] = 0.0f;
                    }

                    mState = 9;

                    break;

                case 5:
                    if (mVibFlag) {
                        mVibFlag = false;
                        setVibFlag(mVibFlag);

                        anm_no = findGroupAnimator(14, 6);
                        mpGroupAnmController[anm_no]->start();

                        mSelectAnmNum = findGroupAnimator(13, 8);
                        mpGroupAnmController[mSelectAnmNum]->start();

                        play_sound(15);
                    } else {
                        play_sound(13);
                    }

                    mState = 9;

                    break;

                case 6:
                    mSelectAnmNum = findGroupAnimator(15, 5);
                    mpGroupAnmController[mSelectAnmNum]->start();

                    mState = 3;
                    setReassignedFlag(true);

                    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                        mOnPaneVibFrame[i] = 0.0f;
                        mOnPaneVibWaitFrame[i] = 0.0f;
                        getController(i)->stopMotor();
                        // getController(i)->setEnableRumble(false);
                    }

                    mWaitStopMotorCount = 30.0f / getInstance()->getHBMDataInfo()->frameDelta;

                    mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[0], true)->SetVisible(true);

                    mpLayout->GetRootPane()->FindPaneByName(scFuncTextPaneName[1], true)->SetVisible(true);

                    play_sound(5);
                    play_sound(16);

                    break;

                case 7:
                    mSelectAnmNum = findGroupAnimator(19, 13);
                    mpGroupAnmController[mSelectAnmNum]->start();

                    mState = 13;
                    mSequence = eSeq_Normal;

                    if (mSelectBtnNum == HBM_SELECT_BTN1) {
                        play_sound(2);
                        mFader.setFadeColorEnable(false);
                    } else if (mSelectBtnNum == HBM_SELECT_BTN2) {
                        play_sound(3);
                        mFader.setFadeColorEnable(true);
                    } else if (mSelectBtnNum == HBM_SELECT_BTN4) {
                        play_sound(3);
                        mFader.setFadeColorEnable(false);
                    }

                    break;

                case 8:
                    mSelectAnmNum = findGroupAnimator(20, 13);
                    mpGroupAnmController[mSelectAnmNum]->start();

                    mState = 13;
                    mSelectBtnNum = HBM_SELECT_NULL;

                    mSequence = eSeq_Normal;

                    play_sound(6);

                    break;
            }
        }
    }
}

void HomeButton::reset_btn() {
    int anm_no;

    for (int i = 0; i < mButtonNum; i++) {
        if (!mPaneCounter[i]) {
            continue;
        }

        anm_no = findAnimator(i, 2);
        mpAnmController[anm_no]->start();
        mPaneCounter[i] = 0;
    }

    if (mPaneCounter[mButtonNum]) {
        mpPairGroupAnmController[12]->start();
        mPaneCounter[mButtonNum + 1] = 0;
    }

    if (mPaneCounter[mButtonNum + 1]) {
        if (mSequence == eSeq_Control) {
            if (!mMsgCount) {
                anm_no = findGroupAnimator(5, 20);
                mpGroupAnmController[anm_no]->start();
                mPaneCounter[mButtonNum] = 0;
            }
        } else {
            anm_no = findGroupAnimator(5, 3);
            mpGroupAnmController[anm_no]->start();
            mPaneCounter[mButtonNum] = 0;
        }
    }
    // #if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    //     if (mPaneCounter[mButtonNum + 10]) {
    //         mpAnmControllerSet4[1]->start();
    //         mPaneCounter[mButtonNum + 10] = 0;

    //         nw4hbm::lyt::Pane* pBtnWarePane = mpLayout2->GetRootPane()->FindPaneByName(scBtnWareName[0], true);

    //         pBtnWarePane->SetSize(nw4hbm::lyt::Size(324.0f, 64.0f));
    //     }

    //     if (mPaneCounter[mButtonNum + 11]) {
    //         mpAnmControllerSet4[4]->start();
    //         mPaneCounter[mButtonNum + 11] = 0;
    //     }
    // #endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND
}

void HomeButton::reset_battery() {
    for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
        for (int j = 0; j < 4; j++) {
            mpLayout->GetRootPane()->FindPaneByName(scBatteryPaneName[i][j], true)->SetVisible(false);
        }
    }
}

void HomeButton::reset_guiManager(int num) {
    if (num < 0) // presumably a -1 = all case
    {
        for (int i = 0; i < 8; i++) {
            mpPaneManager->update(i, -10000.0f, -10000.0f, 0, 0, 0, nullptr);
        }
    } else {
        mpPaneManager->update(num, -10000.0f, -10000.0f, 0, 0, 0, nullptr);
    }
}

bool HomeButton::isActive() const { return mState == 2; }

bool HomeButton::isUpBarActive() const {
    bool flag = true;

    if (!isActive() || mpPairGroupAnmController[12]->isPlaying() || mpPairGroupAnmController[3]->isPlaying()) {
        flag = false;
    }

    return flag;
}

bool HomeButton::isDownBarActive() {
    bool flag = true;

    int anm_no[4];
    anm_no[0] = findGroupAnimator(4, 2);
    anm_no[1] = findGroupAnimator(5, 3);
    anm_no[2] = findGroupAnimator(4, 19);
    anm_no[3] = findGroupAnimator(5, 20);

    if (!isActive() || mpGroupAnmController[anm_no[0]]->isPlaying() || mpGroupAnmController[anm_no[1]]->isPlaying() ||
        mpGroupAnmController[anm_no[2]]->isPlaying() || mpGroupAnmController[anm_no[3]]->isPlaying()) {
        flag = false;
    }

    return flag;
}

int HomeButton::getPaneNo(const nw4hbm::lyt::Pane* pPane) {
    int ret = -1;
    const char* panename = pPane->GetName();

    // #if HBM_APP_TYPE == HBM_APP_TYPE_NAND
    //     if (mpLayout2->GetRootPane()->FindPaneByName(panename, true) == pPane) {
    //         for (int i = 0; i < (int)ARRAY_COUNT(scBtnWareName); i++) {
    //             if (!std::strcmp(panename, getBtnWarePaneName(i))) {
    //                 ret = i + mButtonNum + 10;
    //                 break;
    //             }
    //         }
    //     }
    // #endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

    for (int i = 0; i < mButtonNum; i++) {
        if (!std::strcmp(panename, getPaneName(i))) {
            ret = i;
            break;
        }
    }

    for (int i = 0; i < 10; i++) {
        if (!std::strcmp(panename, getFuncPaneName(i))) {
            ret = i + mButtonNum;
            break;
        }
    }

    return ret;
}

int HomeButton::findAnimator(int pane, int anm) {
    for (int i = 0; i < mAnmNum; i++) {
        if (scAnmTable[i].pane == pane && scAnmTable[i].anm == anm) {
            return i;
        }
    }

    return -1;
}

void HomeButton::setForcusSE() {
    if (mForcusSEWaitTime <= 2) {
        return;
    }

    play_sound(4);
    mForcusSEWaitTime = 0;
}

void HomeButton::setAdjustFlag(int flag) {
    nw4hbm::math::VEC2 sc_v;

    mAdjustFlag = static_cast<bool>(flag);
    mDrawInfo.SetLocationAdjust(mAdjustFlag);

    if (mAdjustFlag) {
        sc_v = nw4hbm::math::VEC2(mpHBInfo->adjust.x, mpHBInfo->adjust.y);
        mpLayout->GetRootPane()->SetScale(sc_v);
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
        // mpLayout2->GetRootPane()->SetScale(sc_v);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

        if (!mpHBInfo->cursor) {
            for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                mpCursorLayout[i]->GetRootPane()->SetScale(sc_v);
            }
        }
    } else {
        sc_v = nw4hbm::math::VEC2(1.0f, 1.0f);
        mpLayout->GetRootPane()->SetScale(sc_v);
#if HBM_APP_TYPE == HBM_APP_TYPE_NAND
        // mpLayout2->GetRootPane()->SetScale(sc_v);
#endif // HBM_APP_TYPE == HBM_APP_TYPE_NAND

        if (!mpHBInfo->cursor) {
            for (int i = 0; i < WPAD_MAX_CONTROLLERS; i++) {
                mpCursorLayout[i]->GetRootPane()->SetScale(sc_v);
            }
        }
    }
}

void HomeButton::setVibFlag(bool flag) { WPADEnableMotor(flag); }

bool HomeButton::getVibFlag() { return WPADIsMotorEnabled() ? true : false; }

void HomeButtonEventHandler::onEvent(u32 uID, u32 uEvent, void* pData) {
    gui::PaneComponent* p_panecpt = // force downcast
        static_cast<gui::PaneComponent*>(mpManager->getComponent(uID));
    const nw4hbm::lyt::Pane* pPane = p_panecpt->getPane();

    HomeButton* p_hbtn = getHomeButton();
    OSAssert_Line(3799, p_hbtn);

    HBController* pCon = static_cast<HBController*>(pData);

    switch (uEvent) {
        case 1:
            p_hbtn->startPointEvent(pPane, pData);
            break;

        case 2:
            p_hbtn->startLeftEvent(pPane);
            break;

        case 0:
            if (pCon->trig & WPAD_BUTTON_A) {
                p_hbtn->startTrigEvent(pPane);
            }

            break;
    }
}

void HomeButton::startBlackOut() {
    if (!mStartBlackOutFlag) {
        mStartBlackOutFlag = true;

        mForceSttInitProcFlag = false;
        mForceSttFadeInProcFlag = false;
        mForceStopSyncFlag = false;
        mForceEndMsgAnmFlag = false;

        switch (mState) {
            case 0:
                mForceSttInitProcFlag = true;

            case 1:
                mForceSttFadeInProcFlag = true;
                break;

            case 3:
            case 5:
            case 6:
            case 7:
                if ((mState == 3 && mSelectAnmNum == 5) || (mState == 5 && !mSimpleSyncFlag)) {
                    OSCancelAlarm(&mSimpleSyncAlarm);
                    WPADSetSimpleSyncCallback(mSimpleSyncCallback);
                } else if (!mEndSimpleSyncFlag && mState > 3) {
                    mForceStopSyncFlag = true;

                    if (!WPADStopSimpleSync()) {
                        OSCancelAlarm(&mSimpleSyncAlarm);

                        OSSetAlarmUserDataAny(&mSimpleSyncAlarm, 1);
                        OSSetAlarm(&mSimpleSyncAlarm, OSMillisecondsToTicks(100), &RetrySimpleSyncCallback);
                    }
                } else {
                    WPADSetSimpleSyncCallback(mSimpleSyncCallback);
                }

                mForceEndMsgAnmFlag = true;
                break;
        }

        mState = 19;
        mFader.start();

        mSelectBtnNum = HBM_SELECT_BTN2;

        f32 maxFrame = mFader.getMaxFrame();
        mFadeOutSeTime = maxFrame;

        if (mpHBInfo->sound_callback) {
            (*mpHBInfo->sound_callback)(3, maxFrame);
        }
    }
}

static void initgx() {
    Mtx view_mtx;

    GXSetCullMode(GX_CULL_NONE);
    PSMTXIdentity(view_mtx);
    GXLoadPosMtxImm(view_mtx, 0);
    GXSetCurrentMtx(0);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_CLR_RGB, GX_F32, 0);

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);

    GXSetNumTexGens(0);

    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);

    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaUpdate(false);
    GXSetZMode(false, GX_ALWAYS, false);
    GXSetDispCopyGamma(GX_GM_1_0);

    GXSetNumIndStages(0);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
}

} // namespace homebutton