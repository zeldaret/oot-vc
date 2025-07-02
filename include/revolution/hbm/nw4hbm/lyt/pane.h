#ifndef RVL_SDK_HBM_NW4HBM_LYT_PANE_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_PANE_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/lyt/lyt_animation.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_types.hpp"
#include "revolution/hbm/nw4hbm/math/math_types.hpp"
#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include "revolution/hbm/nw4hbm/ut/Color.hpp"
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
namespace detail {
class RuntimeTypeInfo;
}
} // namespace ut
} // namespace nw4hbm

namespace nw4hbm {
namespace lyt {
// forward declarations
class DrawInfo;
class Material;
class Pane;

namespace detail {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x476226
struct PaneLink {
    // typedefs
  public:
    typedef ut::LinkList<PaneLink, 0> LinkList;

    // members
  public:
    ut::LinkListNode mLink; // size 0x08, offset 0x00
    Pane* mTarget; // size 0x04, offset 0x08
}; // size 0x0c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4763c3
class PaneBase {
    // methods
  public:
    // cdtors
    PaneBase();
    virtual ~PaneBase();

    // members
  private:
    /* vtable */ // size 0x04, offset 0x00
    ut::LinkListNode mLink; // size 0x08, offset 0x04
}; // size 0x0c
} // namespace detail

namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48966f
struct Pane {
    DataBlockHeader blockHeader; // size 0x08, offset 0x00
    u8 flag; // size 0x01, offset 0x08
    u8 basePosition; // size 0x01, offset 0x09
    u8 alpha; // size 0x01, offset 0x0a
    byte1_t padding;
    char name[16]; // size 0x10, offset 0x0c
    char userData[8]; // size 0x08, offset 0x1c
    math::VEC3 translate; // size 0x0c, offset 0x24
    math::VEC3 rotate; // size 0x0c, offset 0x30
    math::VEC2 scale; // size 0x08, offset 0x3c
    Size size; // size 0x08, offset 0x44
}; // size 0x4c
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47625e
class Pane : public detail::PaneBase {
    // typedefs
  public:
    typedef ut::LinkList<Pane, 4> LinkList;

    // methods
  public:
    // cdtors
    Pane(const res::Pane* pBlock);
    virtual ~Pane();

    // virtual function ordering
    // vtable Pane
    virtual const ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; }
    virtual void CalculateMtx(const DrawInfo& drawInfo);
    virtual void Draw(const DrawInfo& drawInfo);
    virtual void DrawSelf(const DrawInfo& drawInfo);
    virtual void Animate(u32 option);
    virtual void AnimateSelf(u32 option);
    virtual const ut::Color GetVtxColor(u32 idx) const;
    virtual void SetVtxColor(u32 idx, ut::Color value);
    virtual u8 GetColorElement(u32 idx) const;
    virtual void SetColorElement(u32 idx, u8 value);
    virtual u8 GetVtxColorElement(u32 idx) const;
    virtual void SetVtxColorElement(u32 idx, u8 value);
    virtual Pane* FindPaneByName(const char* findName, bool bRecursive);
    virtual Material* FindMaterialByName(const char* findName, bool bRecursive);
    virtual void BindAnimation(AnimTransform* pAnimTrans, bool bRecursive);
    virtual void UnbindAnimation(AnimTransform* pAnimTrans, bool bRecursive);
    virtual void UnbindAllAnimation(bool bRecursive);
    virtual void UnbindAnimationSelf(AnimTransform* pAnimTrans);
    virtual AnimationLink* FindAnimationLink(AnimTransform* pAnimTrans);
    virtual void SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable, bool bRecursive);
    virtual Material* GetMaterial() const;
    virtual void LoadMtx(const DrawInfo& drawInfo);

    // methods
    Pane* GetParent() const { return mpParent; }
    LinkList& GetChildList() { return mChildList; }
    const math::VEC3& GetTranslate() const { return mTranslate; }
    const Size& GetSize() const { return mSize; }
    const math::MTX34& GetGlobalMtx() const { return mGlbMtx; }
    bool IsVisible() const { return detail::TestBit(mFlag, 0); }
    const char* GetName() const { return mName; }
    bool IsUserAllocated() const { return mbUserAllocated; }

    const ut::Rect GetPaneRect(const DrawInfo& drawInfo) const;
    math::VEC2 GetVtxPos() const;

    void SetRotate(const math::VEC3& value) { mRotate = value; }
    void SetScale(const math::VEC2& value) { mScale = value; }
    void SetSRTElement(u32 idx, f32 value) {
        NW4HBM_ASSERT2(idx < ANIMTARGET_PANE_MAX, 250);

        f32* srtAry = reinterpret_cast<f32*>(&mTranslate);
        srtAry[idx] = value;
    }

    void SetTranslate(const math::VEC2& value) { SetTranslate(math::VEC3(value.x, value.y, 0.0f)); }

    void SetTranslate(const math::VEC3& value) { mTranslate = value; }
    void SetSize(const Size& value) { mSize = value; }
    void SetVisible(bool bVisible) { detail::SetBit(&mFlag, 0, bVisible); }
    void SetAlpha(u8 alpha) { mAlpha = alpha; }

    void SetName(const char* name);
    void SetUserData(const char* userData);

    void Init();

    void InsertChild(LinkList::Iterator next, Pane* pChild);
    void AppendChild(Pane* pChild);
    void CalculateMtxChild(const DrawInfo& drawInfo);

    void DoAlphaThing() {
        mAlpha = 0xff;
        detail::SetBit(&mFlag, 1, true);
    }

    void AddAnimationLink(AnimationLink* pAnimationLink);

    // members
  protected: // Bounding::DrawSelf
    /* base PaneBase */ // size 0x0c, offset 0x00
    Pane* mpParent; // size 0x04, offset 0x0c
    LinkList mChildList; // size 0x0c, offset 0x10
    AnimationLink::LinkList mAnimList; // size 0x0c, offset 0x1c
    Material* mpMaterial; // size 0x04, offset 0x28
    math::VEC3 mTranslate; // size 0x0c, offset 0x2c
    math::VEC3 mRotate; // size 0x0c, offset 0x38
    math::VEC2 mScale; // size 0x08, offset 0x44
    Size mSize; // size 0x08, offset 0x4c
    math::MTX34 mMtx; // size 0x30, offset 0x54
    math::MTX34 mGlbMtx; // size 0x30, offset 0x84
    char mName[16]; // size 0x10, offset 0xb4
    char mUserData[8]; // size 0x08, offset 0xc4
    u8 mBasePosition; // size 0x01, offset 0xcc
    u8 mAlpha; // size 0x01, offset 0xcd
    u8 mGlbAlpha; // size 0x01, offset 0xce
    u8 mFlag; // size 0x01, offset 0xcf
    bool mbUserAllocated; // size 0x01, offset 0xd0
    /* 3 bytes padding */

    // static members
  public:
    static const ut::detail::RuntimeTypeInfo typeInfo;
}; // size 0xd4
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_PANE_HPP
