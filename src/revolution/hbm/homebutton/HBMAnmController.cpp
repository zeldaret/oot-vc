#include "revolution/hbm/homebutton/HBMAnmController.hpp"

#include "revolution/hbm/nw4hbm/lyt/animation.h"
#include "revolution/hbm/nw4hbm/lyt/group.h"

namespace homebutton {

GroupAnmController::GroupAnmController() : mpGroup(), mpAnimGroup() {}

GroupAnmController::~GroupAnmController() {}

void GroupAnmController::do_calc() {
    bool flag;

    if (mState == eState_Playing) {
        calc();
        flag = true;

        mpAnimGroup->SetFrame(mCurFrame);
    } else {
        flag = false;
    }

    nw4hbm::lyt::PaneLinkList& list = mpGroup->GetPaneList();

    NW4HBM_RANGE_FOR(it, list) { it->mTarget->SetAnimationEnable(mpAnimGroup, flag, false); }
}

} // namespace homebutton
