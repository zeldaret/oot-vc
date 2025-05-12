#ifndef RVL_SDK_HBM_NW4HBM_LYT_DRAW_INFO_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_DRAW_INFO_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/math/math_types.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Rect.hpp"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace lyt {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x477c6a
class DrawInfo {
    // methods
  public:
    // cdtors
    DrawInfo();
    virtual ~DrawInfo();

    // methods
    const math::MTX34& GetViewMtx() const { return mViewMtx; }
    const math::VEC2& GetLocationAdjustScale() const { return mLocationAdjustScale; }
    f32 GetGlobalAlpha() const { return mGlobalAlpha; }

    bool IsMultipleViewMtxOnDraw() const { return mFlag.mulViewDraw; }
    bool IsInfluencedAlpha() const { return mFlag.influencedAlpha; }
    bool IsLocationAdjust() const { return mFlag.locationAdjust; }
    bool IsInvisiblePaneCalculateMtx() const { return mFlag.invisiblePaneCalculateMtx; }
    bool IsDebugDrawMode() const { return mFlag.debugDrawMode; }

    void SetViewMtx(const math::MTX34& mtx) { mViewMtx = mtx; }
    void SetViewRect(const ut::Rect& rect) { mViewRect = rect; }
    void SetLocationAdjustScale(const math::VEC2& scale) { mLocationAdjustScale = scale; }
    void SetGlobalAlpha(f32 alpha) { mGlobalAlpha = alpha; }

    void SetInfluencedAlpha(bool bEnable) { mFlag.influencedAlpha = bEnable; }
    void SetLocationAdjust(bool bEnable) { mFlag.locationAdjust = bEnable; }

    bool IsYAxisUp() const { return mViewRect.bottom - mViewRect.top < 0.0f; }

    // members
  public: // homebutton::HomeButton::draw
    math::MTX34 mViewMtx; // size 0x30, offset 0x04
    ut::Rect mViewRect; // size 0x10, offset 0x34
    math::VEC2 mLocationAdjustScale; // size 0x08, offset 0x44
    f32 mGlobalAlpha; // size 0x04, offset 0x4c

    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x477d31
    struct /* explicitly untagged */
    {
        u8 mulViewDraw : 1; // offset be0 / le7
        u8 influencedAlpha : 1; // offset be1 / le6
        u8 locationAdjust : 1; // offset be2 / le5
        u8 invisiblePaneCalculateMtx : 1; // offset be3 / le4
        u8 debugDrawMode : 1; // offset be4 / le3
    } mFlag; // size 0x01, offset 0x50
    /* 3 bytes padding */
}; // size 0x54
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_DRAW_INFO_HPP
