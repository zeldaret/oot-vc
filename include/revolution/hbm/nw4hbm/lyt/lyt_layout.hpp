#ifndef RVL_SDK_HBM_NW4HBM_LYT_LAYOUT_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_LAYOUT_HPP

/*******************************************************************************
 * headers
 */

#include "macros.h" // ATTR_NOINLINE
#include "revolution/hbm/nw4hbm/lyt/lyt_animation.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/mem/mem_allocator.h"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace ut {
struct Rect;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace ut {
template <typename> class TagProcessorBase;
}
} // namespace nw4hbm

namespace nw4hbm {
namespace lyt {
// forward declarations
class DrawInfo;
class GroupContainer;
class Pane;
class ResourceAccessor;

namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x
struct Layout {
    DataBlockHeader blockHeader; // size 0x08, offset 0x00
    u8 originType; // size 0x01, offset 0x08
    byte1_t padding[3];
    Size layoutSize; // size 0x08, offset 0x0c
}; // size 0x14
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x
class Layout {
    // methods
  public:
    // cdtors
    Layout();
    virtual ~Layout();

    // virtual function ordering
    // vtable Layout
    virtual bool Build(const void* lytResBuf, ResourceAccessor* pResAcsr);
    virtual AnimTransform* CreateAnimTransform(const void* anmResBuf, ResourceAccessor* pResAcsr);
    virtual void BindAnimation(AnimTransform* pAnimTrans);
    virtual void UnbindAnimation(AnimTransform* pAnimTrans);
    virtual void UnbindAllAnimation();
    virtual void SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable);
    virtual void CalculateMtx(const DrawInfo& drawInfo);
    virtual void Draw(const DrawInfo& drawInfo);
    virtual void Animate(u32 option);
    virtual void SetTagProcessor(ut::TagProcessorBase<wchar_t>* pTagProcessor);

    // methods
    Pane* GetRootPane() const { return mpRootPane; }
    GroupContainer* GetGroupContainer() const { return mpGroupContainer; }

    ut::Rect GetLayoutRect() const;

    // static methods
    static void* AllocMemory(u32 size) { return MEMAllocFromAllocator(mspAllocator, size); }
    static void FreeMemory(void* ptr) { MEMFreeToAllocator(mspAllocator, ptr); }

    static MEMAllocator* GetAllocator() { return mspAllocator; }
    static void SetAllocator(MEMAllocator* pAllocator) { mspAllocator = pAllocator; }

    // Layout::Build
    static Pane* BuildPaneObj(s32 kind, const void* dataPtr, const ResBlockSet& resBlockSet) NO_INLINE;

    // members
  private:
    /* vtable */ // size 0x04, offset 0x00
    AnimTransform::LinkList mAnimTransList; // size 0x0c, offset 0x04
    Pane* mpRootPane; // size 0x04, offset 0x10
    GroupContainer* mpGroupContainer; // size 0x04, offset 0x14
    Size mLayoutSize; // size 0x08, offset 0x18
    u8 mOriginType; // size 0x01, offset 0x20
    /* 3 bytes padding */

    // static members
  private:
    static MEMAllocator* mspAllocator;
}; // size 0x24
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_LAYOUT_HPP
