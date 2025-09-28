#ifndef RVL_SDK_HBM_HOMEBUTTON_ANM_CONTROLLER_HPP
#define RVL_SDK_HBM_HOMEBUTTON_ANM_CONTROLLER_HPP

#include "revolution/hbm/homebutton/HBMFrameController.hpp"

// forward declarations
namespace nw4hbm {
namespace lyt {
class AnimTransform;
class Group;
} // namespace lyt
} // namespace nw4hbm

namespace homebutton {

class GroupAnmController : public FrameController {
    // methods
  public:
    // cdtors
    GroupAnmController();
    virtual ~GroupAnmController();

    // virtual function ordering
    // vtable FrameController
    // virtual void calc();

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
