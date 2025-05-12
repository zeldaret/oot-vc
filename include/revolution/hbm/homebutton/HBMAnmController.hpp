#ifndef RVL_SDK_HBM_HOMEBUTTON_ANM_CONTROLLER_HPP
#define RVL_SDK_HBM_HOMEBUTTON_ANM_CONTROLLER_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/homebutton/HBMFrameController.hpp"

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace lyt {
class AnimTransform;
class Group;
} // namespace lyt
} // namespace nw4hbm

namespace homebutton {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x475e60
class GroupAnmController : public FrameController {
    // methods
  public:
    // cdtors
    GroupAnmController();
    virtual ~GroupAnmController();

    // virtual function ordering
    // vtable FrameController
    virtual void calc();

    // methods
    void do_calc();

    // members
  public: // HomeButton::create
    /* base FrameController */ // size 0x20, offset 0x00
    nw4hbm::lyt::Group* mpGroup; // size 0x04, offset 0x20
    nw4hbm::lyt::AnimTransform* mpAnimGroup; // size 0x04, offset 0x24
}; // size 0x28
} // namespace homebutton

#endif // RVL_SDK_HBM_HOMEBUTTON_ANM_CONTROLLER_HPP
