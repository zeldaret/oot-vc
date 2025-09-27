#ifndef NW4HBM_LYT_LAYOUT_H
#define NW4HBM_LYT_LAYOUT_H

#include "decomp.h"

#include "revolution/mem.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/animation.h"
#include "revolution/hbm/nw4hbm/lyt/drawInfo.h"
#include "revolution/hbm/nw4hbm/lyt/group.h"
#include "revolution/hbm/nw4hbm/lyt/resourceAccessor.h"
#include "revolution/hbm/nw4hbm/lyt/types.h"

#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include "revolution/hbm/nw4hbm/ut/TagProcessorBase.h"

#include "macros.h"

namespace nw4hbm {
namespace lyt {
class Layout {
  public:
    Layout();
    virtual ~Layout();

    const ut::Rect GetLayoutRect() const;

    virtual bool Build(const void* lytResBuf, ResourceAccessor* pResAcsr);

    virtual AnimTransform* CreateAnimTransform(const void* anmResBuf, ResourceAccessor* pResAcsr);

    virtual void BindAnimation(AnimTransform* animTrans);
    virtual void UnbindAnimation(AnimTransform* animTrans);
    virtual void UnbindAllAnimation();

    virtual void SetAnimationEnable(AnimTransform* animTrans, bool bEnable = true);

    virtual void CalculateMtx(const DrawInfo& drawInfo);
    virtual void Draw(const DrawInfo& drawInfo);

    virtual void Animate(u32 option = 0);

    virtual void SetTagProcessor(ut::WideTagProcessor* pTagProcessor) NO_INLINE;

    Pane* GetRootPane() const { return mpRootPane; }
    GroupContainer* GetGroupContainer() const { return mpGroupContainer; }

    static MEMAllocator* GetAllocator() { return mspAllocator; }
    static void SetAllocator(MEMAllocator* allocator) { mspAllocator = allocator; }

    static void* AllocMemory(u32 size) { return MEMAllocFromAllocator(mspAllocator, size); }
    static void FreeMemory(void* ptr) { MEMFreeToAllocator(mspAllocator, ptr); }

    static Pane* BuildPaneObj(s32 kind, const void* dataPtr, const ResBlockSet& resBlockSet) NO_INLINE;

  private:
    AnimTransformList mAnimTransList; // 0x04

    Pane* mpRootPane; // 0x10
    GroupContainer* mpGroupContainer; // 0x14

    Size mLayoutSize; // 0x18

    u8 mOriginType; // 0x20

    static MEMAllocator* mspAllocator;
};
} // namespace lyt
} // namespace nw4hbm

#endif // NW4HBM_LYT_LAYOUT_H
