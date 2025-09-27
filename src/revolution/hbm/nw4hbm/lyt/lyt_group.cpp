#include "revolution/hbm/nw4hbm/lyt/group.h"

/*******************************************************************************
 * headers
 */

#include "cstring.hpp" // std::memcpy
#include "new.hpp"

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/common.h" // detail::EqualsPaneName
#include "revolution/hbm/nw4hbm/lyt/layout.h"
#include "revolution/hbm/nw4hbm/lyt/types.h" // detail::ConvertOffsToPtr
#include "revolution/hbm/nw4hbm/lyt/pane.h"

#include "revolution/hbm/nw4hbm/ut/LinkList.h" // IWYU pragma: keep (NW4HBM_RANGE_FOR)

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace lyt {

Group::Group() {}

Group::Group(const res::Group* pResGroup, Pane* pRootPane) {
    Init();
    std::memcpy(mName, pResGroup->name, sizeof mName);

    const char* paneName = detail::ConvertOffsToPtr<char>(pResGroup, sizeof *pResGroup);

    for (int i = 0; i < pResGroup->paneNum; i++) {
        Pane* pFindPane = pRootPane->FindPaneByName(paneName + (int)sizeof pResGroup->name * i, true);

        if (pFindPane) {
            AppendPane(pFindPane);
        }
    }
}

void Group::Init() { mbUserAllocated = false; }

Group::~Group() {
    NW4HBM_RANGE_FOR_NO_AUTO_INC(it, mPaneLinkList) {
        DECLTYPE(it) currIt = it++;

        mPaneLinkList.Erase(currIt);
        Layout::FreeMemory(&(*currIt));
    }
}

void Group::AppendPane(Pane* pPane) {
    if (void* pMem = Layout::AllocMemory(sizeof(detail::PaneLink))) {
        detail::PaneLink* pPaneLink = new (pMem) detail::PaneLink();

        pPaneLink->mTarget = pPane;
        mPaneLinkList.PushBack(pPaneLink);
    }
}

GroupContainer::~GroupContainer() {
    NW4HBM_RANGE_FOR_NO_AUTO_INC(it, mGroupList) {
        DECLTYPE(it) currIt = it++;

        mGroupList.Erase(currIt);

        if (!currIt->IsUserAllocated()) {
            currIt->~Group();
            Layout::FreeMemory(&(*currIt));
        }
    }
}

void GroupContainer::AppendGroup(Group* pGroup) { mGroupList.PushBack(pGroup); }

Group* GroupContainer::FindGroupByName(const char* findName) {
    NW4HBM_RANGE_FOR(it, mGroupList) {
        if (detail::EqualsPaneName(it->GetName(), findName)) {
            return &(*it);
        }
    }

    return nullptr;
}

} // namespace lyt
} // namespace nw4hbm
