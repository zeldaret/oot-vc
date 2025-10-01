#ifndef NW4HBM_LYT_BOUNDING_H
#define NW4HBM_LYT_BOUNDING_H

#include "revolution/hbm/nw4hbm/lyt/pane.h"

namespace nw4hbm {
namespace lyt {

class Bounding : public Pane {
public:
    Bounding(const res::Bounding* pBlock, const ResBlockSet& resBlockSet);

    /* 0x08 */ virtual ~Bounding();
    /* 0x18 */ virtual void DrawSelf(const DrawInfo& drawInfo);

    NW4HBM_UT_RUNTIME_TYPEINFO;
};

} // namespace lyt
} // namespace nw4hbm

#endif
