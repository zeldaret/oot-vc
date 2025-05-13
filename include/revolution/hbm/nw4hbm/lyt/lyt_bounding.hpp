#ifndef RVL_SDK_HBM_NW4HBM_LYT_BOUNDING_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_BOUNDING_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/lyt/pane.h"

/*******************************************************************************
 * classes and functions
 */

// forward declarations
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
struct ResBlockSet;

namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x489653
struct Bounding : public Pane {};
// size 0x4c
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47835b
class Bounding : public Pane {
    // methods
  public:
    // cdtors
    Bounding(const res::Bounding* pBlock, const ResBlockSet& resBlockSet);
    virtual ~Bounding();

    // virtual function ordering
    // vtable Pane
    virtual const ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; }

    virtual void DrawSelf(const DrawInfo& drawInfo);

    // members
  private:
    /* base Pane */ // size 0xd4, offset 0x00

    // static members
  public:
    static const ut::detail::RuntimeTypeInfo typeInfo;
}; // size 0xd4
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_BOUNDING_HPP
