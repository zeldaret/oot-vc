#include "revolution/hbm/nw4hbm/lyt/lyt_bounding.hpp"

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_drawInfo.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_pane.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_Color.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace lyt {
const ut::detail::RuntimeTypeInfo Bounding::typeInfo(&Pane::typeInfo);
}
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace lyt {

Bounding::Bounding(const res::Bounding* pBlock, const ResBlockSet&) : Pane(pBlock) {}

Bounding::~Bounding() {}

void Bounding::DrawSelf(const DrawInfo& drawInfo) {
    if (drawInfo.IsDebugDrawMode()) {
        LoadMtx(drawInfo);
        detail::DrawLine(GetVtxPos(), mSize, ut::Color(0x00ff00ff));
    }
}

} // namespace lyt
} // namespace nw4hbm
