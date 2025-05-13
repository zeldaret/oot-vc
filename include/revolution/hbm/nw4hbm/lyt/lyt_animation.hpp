#ifndef RVL_SDK_HBM_NW4HBM_LYT_ANIMATION_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_ANIMATION_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/ut/LinkList.h"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

#define ANIMTARGET_PANE_COLOR_MAX 17
#define ANIMTARGET_MATCOLOR_MAX 32
#define ANIMTARGET_TEXSRT_MAX 5
#define ANIMTARGET_PANE_MAX 10
#define ANIMCURVE_HERMITE 2
#define ANIMCURVE_STEP 1
#define TexMtxMax 10
#define IndTexMtxMax 3

namespace nw4hbm {
namespace lyt {
// forward declarations
class AnimTransform;
class Material;
class Pane;
class ResourceAccessor;

namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48513a
struct StepKey {
    f32 frame; // size 0x04, offset 0x00
    u16 value; // size 0x02, offset 0x04
    byte2_t padding;
}; // size 0x08

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x485187
struct HermiteKey {
    f32 frame; // size 0x04, offset 0x00
    f32 value; // size 0x04, offset 0x04
    f32 slope; // size 0x04, offset 0x08
}; // size 0x0c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4851f4
struct AnimationInfo {
    // members
  public:
    u32 kind; // size 0x04, offset 0x00
    u8 num; // size 0x01, offset 0x04
    byte1_t padding[3];

    // static members
  public:
    static const byte4_t ANIM_INFO_PANE_PAIN_SRT = 'RLPA';
    static const byte4_t ANIM_INFO_PANE_VERTEX_COLOR = 'RLVC';
    static const byte4_t ANIM_INFO_PANE_VISIBILITY = 'RLVI';

    static const byte4_t ANIM_INFO_MATERIAL_COLOR = 'RLMC';
    static const byte4_t ANIM_INFO_MATERIAL_TEXTURE_PATTERN = 'RLTP';
    static const byte4_t ANIM_INFO_MATERIAL_TEXTURE_SRT = 'RLTS';
    static const byte4_t ANIM_INFO_MATERIAL_IND_TEX_SRT = 'RLIM';
}; // size 0x08

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x485259
struct AnimationTarget {
    u8 id; // size 0x01, offset 0x00
    u8 target; // size 0x01, offset 0x01
    u8 curveType; // size 0x01, offset 0x02
    byte1_t padding1;
    u16 keyNum; // size 0x02, offset 0x04
    byte1_t padding2[2];
    u32 keysOffset; // size 0x04, offset 0x08
}; // size 0x0c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47602f
struct AnimationBlock {
    DataBlockHeader blockHeader; // size 0x08, offset 0x00
    u16 frameSize; // size 0x02, offset 0x08
    u8 loop; // size 0x01, offset 0x0a
    byte1_t padding1;
    u16 fileNum; // size 0x02, offset 0x0c
    u16 animContNum; // size 0x02, offset 0x0e
    u32 animContOffsetsOffset; // size 0x04, offset 0x10
}; // size 0x14

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4855c9
struct AnimationContent {
    // enums
  public:
    enum ACType {
        ACType_Pane,
        ACType_Material
    };

    // members
  public:
    char name[20]; // size 0x14, offset 0x00
    u8 num; // size 0x01, offset 0x14
    u8 type; // size 0x01, offset 0x15
    byte1_t padding[2];
}; // size 0x18
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4854e8
class AnimationLink {
    // typedefs
  public:
    typedef ut::LinkList<AnimationLink, 0> LinkList;

    // methods
  public:
    // cdtors
    AnimationLink() : mLink(), mbDisable(false) { Reset(); }
    ~AnimationLink() {}

    // methods
    AnimTransform* GetAnimTransform() const { return mAnimTrans; }
    u16 GetIndex() const { return mIdx; }
    bool IsEnable() const { return !mbDisable; }

    void SetAnimTransform(AnimTransform* animTrans, u16 idx) {
        mAnimTrans = animTrans;
        mIdx = idx;
    }
    void SetEnable(bool bEnable) { mbDisable = !bEnable; }

    void Reset() { SetAnimTransform(nullptr, 0); }

    // members
  private:
    ut::LinkListNode mLink; // size 0x08, offset 0x00
    AnimTransform* mAnimTrans; // size 0x04, offset 0x08
    u16 mIdx; // size 0x02, offset 0x0c
    bool mbDisable; // size 0x01, offset 0x0e
    /* 1 byte padding */
}; // size 0x10

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x475fe3
class AnimTransform {
    // typedefs
  public:
    typedef ut::LinkList<AnimTransform, 4> LinkList;

    // methods
  public:
    // cdtors
    AnimTransform();
    virtual ~AnimTransform();

    // virtual function ordering
    // vtable AnimTransform
    virtual void SetResource(const res::AnimationBlock* pRes, ResourceAccessor* pResAccessor) = 0;
    virtual void Bind(Pane* pPane, bool bRecursive) = 0;
    virtual void Bind(Material* pMaterial) = 0;
    virtual void Animate(u32 idx, Pane* pPane) = 0;
    virtual void Animate(u32 idx, Material* pMaterial) = 0;

    // methods
    f32 GetFrameMax() const { return GetFrameSize(); }
    u16 GetFrameSize() const;

    void SetFrame(f32 frame) { mFrame = frame; }

    // members
  protected: // AnimTransformBasic::SetResource
    /* vtable */ // size 0x04, offset 0x00
    ut::LinkListNode mLink; // size 0x08, offset 0x04
    const res::AnimationBlock* mpRes; // size 0x04, offset 0x0c
    f32 mFrame; // size 0x04, offset 0x10
}; // size 0x14

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48546b
class AnimTransformBasic : public AnimTransform {
    // methods
  public:
    // cdtors
    AnimTransformBasic();
    virtual ~AnimTransformBasic();

    // virtual function ordering
    // vtable AnimTransform
    virtual void SetResource(const res::AnimationBlock* pRes, ResourceAccessor* pResAccessor);
    virtual void Bind(Pane* pPane, bool bRecursive);
    virtual void Bind(Material* pMaterial);
    virtual void Animate(u32 idx, Pane* pPane);
    virtual void Animate(u32 idx, Material* pMaterial);

    // members
  private:
    /* base AnimTransform */ // size 0x14, offset 0x00
    void** mpFileResAry; // size 0x04, offset 0x14
    AnimationLink* mAnimLinkAry; // size 0x04, offset 0x18
    u16 mAnimLinkNum; // size 0x02, offset 0x1c
    /* 2 bytes padding */
}; // size 0x20

namespace detail {
AnimationLink* FindAnimationLink(AnimationLink::LinkList* pAnimList, AnimTransform* pAnimTrans);
} // namespace detail
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_ANIMATION_HPP
