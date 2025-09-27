#include "revolution/hbm/nw4hbm/lyt/drawInfo.h"

/*******************************************************************************
 * headers
 */

#include "cstring.hpp" // memset

#include "revolution/hbm/nw4hbm/math/types.h" // math::MTX34Identity

/*******************************************************************************
 * functions
 */

namespace nw4hbm {
namespace lyt {

DrawInfo::DrawInfo() : mLocationAdjustScale(1.0f, 1.0f), mGlobalAlpha(1.0f) {
    std::memset(&mFlag, 0, sizeof mFlag);

    math::MTX34Identity(&mViewMtx);
}

DrawInfo::~DrawInfo() {}

} // namespace lyt
} // namespace nw4hbm
