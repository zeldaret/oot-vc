/*******************************************************************************
 * headers
 */
#include "revolution.h"

/*******************************************************************************
 * functions
 */

namespace homebutton {

FrameController::FrameController() {
    // this->mUnk_20 = nullptr;
    // this->mUnk_24 = 0;
}

void FrameController::fn_80109BF4() {
    int var_r27;

    if (this->isPlaying()) {
        this->calc();
        var_r27 = 1;
    } else {
        var_r27 = 0;
    }

    // do {
    //     NW4HBM_ASSERT_PTR_NULL(this->mUnk_20, 0x23D);

    // } while (this->mFrameDelta != this->mUnk_20->mFrameDelta);
}

void FrameController::init(int anm_type, f32 max_frame, f32 min_frame, f32 delta) {
    mAnmType = anm_type;
    mMaxFrame = max_frame;
    mMinFrame = min_frame;

    mFrameDelta = delta;
    mState = eState_Stopped;
    mAltFlag = false;

    initFrame();
}

void FrameController::initFrame() { mCurFrame = mAnmType == eAnmType_Backward ? mMaxFrame : mMinFrame; }

void FrameController::calc() {
    if (mState != eState_Playing) {
        return;
    }

    switch (mAnmType) {
        case eAnmType_Forward:
            if ((mCurFrame += mFrameDelta) >= getLastFrame()) {
                mCurFrame = getLastFrame();
                stop();
            }

            break;

        case eAnmType_Backward:
            if ((mCurFrame -= mFrameDelta) <= mMinFrame) {
                mCurFrame = mMinFrame;
                stop();
            }

            break;

        case eAnmType_Wrap:
            if ((mCurFrame += mFrameDelta) >= mMaxFrame) {
                mCurFrame -= mMaxFrame - mMinFrame;
            }

            break;

        case eAnmType_Oscillate:
            if (!mAltFlag) {
                if ((mCurFrame += mFrameDelta) >= getLastFrame()) {
                    mCurFrame = getLastFrame();
                    mAltFlag = true;
                }
            } else {
                if ((mCurFrame -= mFrameDelta) <= mMinFrame) {
                    mCurFrame = mMinFrame;
                    mAltFlag = false;
                }
            }

            break;
    }
}

} // namespace homebutton
