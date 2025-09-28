#ifndef NW4HBM_LYT_RESOURCE_ACCESSOR_H
#define NW4HBM_LYT_RESOURCE_ACCESSOR_H

#include "revolution/types.h"

namespace nw4hbm {

namespace ut {
class Font;
}

namespace lyt {
class ResourceAccessor {
  public:
    ResourceAccessor();
    virtual ~ResourceAccessor();

    virtual void* GetResource(u32 resType, const char* name, u32* pSize) = 0;
    virtual ut::Font* GetFont(const char* name);
};
} // namespace lyt
} // namespace nw4hbm

#endif // NW4HBM_LYT_RESOURCE_ACCESSOR_H
