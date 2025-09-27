#ifndef NW4HBM_LYT_GROUP_H
#define NW4HBM_LYT_GROUP_H

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/pane.h"

#include "revolution/hbm/nw4hbm/ut/LinkList.h"

namespace nw4hbm {
namespace lyt {
namespace detail {
typedef struct PaneLink {
    ut::LinkListNode mLink;

    Pane* mTarget; // 0x08
} PaneLink;
} // namespace detail
typedef ut::LinkList<detail::PaneLink, offsetof(detail::PaneLink, mLink)> PaneLinkList;

class Group {
  public:
    Group();
    Group(const res::Group* pResGroup, Pane* pRootPane);
    virtual ~Group();

    const char* GetName() const { return mName; }
    bool IsUserAllocated() const { return mbUserAllocated; }

    PaneLinkList& GetPaneList() { return mPaneLinkList; };

    void Init();
    void AppendPane(Pane* pane);

    ut::LinkListNode mLink; // 0x04

  protected:
    PaneLinkList mPaneLinkList; // 0x0C
    char mName[16]; // 0x18

    bool mbUserAllocated; // 0x29

    u8 mPadding[2]; // 0x2A
};
typedef ut::LinkList<Group, offsetof(Group, mLink)> GroupList;

class GroupContainer {
  public:
    GroupContainer() {}
    ~GroupContainer();

    GroupList& GetGroupList() { return mGroupList; }

    void AppendGroup(Group* pGroup);
    Group* FindGroupByName(const char* findName);

  protected:
    GroupList mGroupList; // 0x00
};
} // namespace lyt
} // namespace nw4hbm

#endif // NW4HBM_LYT_GROUP_H
