#ifndef RVL_SDK_HBM_NW4HBM_LYT_WINDOW_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_WINDOW_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/pane.h"
#include "revolution/hbm/nw4hbm/ut/Color.hpp"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace math {
struct VEC2;
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
class Material;

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4780d3
struct InflationLRTB {
    f32 l; // size 0x04, offset 0x00
    f32 r; // size 0x04, offset 0x04
    f32 t; // size 0x04, offset 0x08
    f32 b; // size 0x04, offset 0x0c
}; // size 0x10

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x49de70
struct WindowFrameSize {
    f32 l; // size 0x04, offset 0x00
    f32 r; // size 0x04, offset 0x04
    f32 t; // size 0x04, offset 0x08
    f32 b; // size 0x04, offset 0x0c
}; // size 0x10

namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x49e048
struct WindowFrame {
    u16 materialIdx; // size 0x02, offset 0x00
    u8 textureFlip; // size 0x01, offset 0x02
    byte1_t padding1;
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x49dfad
struct WindowContent {
    u32 vtxCols[4]; // size 0x10, offset 0x00
    u16 materialIdx; // size 0x02, offset 0x10
    u8 texCoordNum; // size 0x01, offset 0x12
    byte1_t padding[1];
}; // size 0x14

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48cd00
struct Window : public Pane {
    /* base Pane */ // size 0x4c, offset 0x00
    InflationLRTB inflation; // size 0x10, offset 0x4c
    u8 frameNum; // size 0x01, offset 0x5c
    byte1_t padding1;
    byte1_t padding2;
    byte1_t padding3;
    u32 contentOffset; // size 0x04, offset 0x60
    u32 frameOffsetTableOffset; // size 0x04, offset 0x64
}; // size 0x68
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x478060
class Window : public Pane {
    // nested types
  private:
    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x478118
    struct Content {
        ut::Color vtxColors[4]; // size 0x10, offset 0x00
        detail::TexCoordAry texCoordAry; // size 0x08, offset 0x10
    }; // size 0x18

    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x4781bd
    struct Frame {
        u8 textureFlip; // size 0x01, offset 0x00
        /* 3 bytes padding */
        Material* pMaterial; // size 0x04, offset 0x04
    }; // size 0x08

    // methods
  public:
    // cdtors
    Window(const res::Window* pBlock, const ResBlockSet& resBlockSet);
    virtual ~Window();

    // virtual function ordering
    // vtable Pane
    virtual const ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; }
    virtual void DrawSelf(const DrawInfo& drawInfo);
    virtual void AnimateSelf(u32 option);
    virtual const ut::Color GetVtxColor(u32 idx) const;
    virtual void SetVtxColor(u32 idx, ut::Color value);
    virtual u8 GetVtxColorElement(u32 idx) const;
    virtual void SetVtxColorElement(u32 idx, u8 value);
    virtual Material* FindMaterialByName(const char* findName, bool bRecursive);
    virtual void UnbindAnimationSelf(AnimTransform* pAnimTrans);
    virtual AnimationLink* FindAnimationLink(AnimTransform* pAnimTrans);
    virtual void SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable, bool bRecursive);

    // vtable Window
    virtual Material* GetContentMaterial() const;
    virtual Material* GetFrameMaterial(u32 frameIdx) const;
    virtual void DrawContent(const math::VEC2& basePt, const WindowFrameSize& frameSize, u8 alpha);
    virtual void DrawFrame(const math::VEC2& basePt, const Frame& frame, const WindowFrameSize& frameSize, u8 alpha);
    virtual void DrawFrame4(const math::VEC2& basePt, const Frame* frames, const WindowFrameSize& frameSize, u8 alpha);
    virtual void DrawFrame8(const math::VEC2& basePt, const Frame* frames, const WindowFrameSize& frameSize, u8 alpha);

    // get methods
    WindowFrameSize GetFrameSize(u8 frameNum, const Frame* frames);

    // members
  private:
    /* base Pane */ // size 0x0d4, offset 0x000
    InflationLRTB mContentInflation; // size 0x010, offset 0x0d4
    Content mContent; // size 0x018, offset 0x0e4
    Frame* mFrames; // size 0x004, offset 0x0fc
    u8 mFrameNum; // size 0x001, offset 0x100
    /* 3 bytes padding */

    // static members
  public:
    static const ut::detail::RuntimeTypeInfo typeInfo;
}; // size 0x104
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_WINDOW_HPP
