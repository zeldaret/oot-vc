#include "revolution/hbm/nw4hbm/lyt/resourceAccessor.h"

// forward declarations
namespace nw4hbm {
namespace ut {
class Font;
}
} // namespace nw4hbm

// these guys were on something when they decided to make this file

namespace nw4hbm {
namespace lyt {

// destructor is first for some reason
ResourceAccessor::~ResourceAccessor() {}

ResourceAccessor::ResourceAccessor() {}

ut::Font* ResourceAccessor::GetFont(const char*) { return nullptr; }

} // namespace lyt
} // namespace nw4hbm
