#include "revolution/hbm/nw4hbm/lyt/resourceAccessor.h"

namespace nw4hbm {
namespace lyt {

ResourceAccessor::~ResourceAccessor() {}

ResourceAccessor::ResourceAccessor() {}

ut::Font* ResourceAccessor::GetFont(const char*) { return nullptr; }

} // namespace lyt
} // namespace nw4hbm
