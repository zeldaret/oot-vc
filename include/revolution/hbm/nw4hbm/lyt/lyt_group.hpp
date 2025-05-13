#ifndef RVL_SDK_HBM_NW4HBM_LYT_GROUP_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_GROUP_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp" // res::DataBlockHeader
#include "revolution/hbm/nw4hbm/lyt/pane.h"
#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace lyt {
namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48b11b
struct Group {
    DataBlockHeader blockHeader; // size 0x08, offset 0x00
    char name[16]; // size 0x10, offset 0x08
    u16 paneNum; // size 0x02, offset 0x18
    byte1_t padding[2];
}; // size 0x1c
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x475eb4
class Group {
    // typedefs
  public:
    typedef ut::LinkList<Group, 4> LinkList;

    // methods
  public:
    // cdtors
    Group();
    Group(const res::Group* pResGroup, Pane* pRootPane);

    virtual ~Group();

    // methods
    const char* GetName() const { return mName; }
    bool IsUserAllocated() const { return mbUserAllocated; }
    detail::PaneLink::LinkList& GetPaneList() { return mPaneLinkList; }

    void Init();
    void AppendPane(Pane* pPane);

    // members
  private:
    /* vtable */ // size 0x04, offset 0x00
    ut::LinkListNode mLink; // size 0x08, offset 0x04
    detail::PaneLink::LinkList mPaneLinkList; // size 0x0c, offset 0x0c
    char mName[16]; // size 0x10, offset 0x18
    bool mbUserAllocated; // size 0x01, offset 0x28
    /* 3 bytes padding */
}; // size 0x2c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x477f31
class GroupContainer {
    // methods
  public:
    // cdtors
    GroupContainer() {}
    ~GroupContainer();

    // methods
    void AppendGroup(Group* pGroup);
    Group* FindGroupByName(const char* findName);

    // members
  private:
    Group::LinkList mGroupList; // size 0x0c, offset 0x00
}; // size 0x0c
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_GROUP_HPP
