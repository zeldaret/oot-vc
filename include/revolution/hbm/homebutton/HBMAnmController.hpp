#ifndef RVL_SDK_HBM_HOMEBUTTON_ANM_CONTROLLER_HPP
#define RVL_SDK_HBM_HOMEBUTTON_ANM_CONTROLLER_HPP

#include "revolution/hbm/homebutton/HBMFrameController.hpp"

namespace nw4hbm {
namespace lyt {
class AnimTransform;
class Group;
} // namespace lyt
} // namespace nw4hbm

namespace homebutton {

class GroupAnmController : public FrameController {
public:
    /* 0x00 (base) */
    /* 0x20 */ nw4hbm::lyt::Group* mpGroup;
    /* 0x24 */ nw4hbm::lyt::AnimTransform* mpAnimGroup;

    GroupAnmController();
    virtual ~GroupAnmController();

    void do_calc();
}; // size = 0x28

} // namespace homebutton

#endif
