#ifndef RVL_SDK_HBM_NW4HBM_LYT_RESOURCE_ACCESSOR_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_RESOURCE_ACCESSOR_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // u32

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace ut {
class Font;
}
} // namespace nw4hbm

namespace nw4hbm {
namespace lyt {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47f902
class ResourceAccessor {
    // methods
  public:
    // cdtors
    ResourceAccessor();
    virtual ~ResourceAccessor();

    // virtual function ordering
    // vtable ResourceAccessor
    virtual void* GetResource(u32 resType, const char* name, u32* pSize) = 0;
    virtual ut::Font* GetFont(const char* name);

    // members
  private:
    /* vtable */ // offset 0x00, size 0x04
}; // size 0x04
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_RESOURCE_ACCESSOR_HPP
