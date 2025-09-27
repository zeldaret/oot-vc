#ifndef NW4HBM_LYT_BOUNDING_H
#define NW4HBM_LYT_BOUNDING_H

#include "revolution/hbm/nw4hbm/lyt/pane.h"

namespace nw4hbm {
    namespace lyt {
        class Bounding : public Pane {
            public:
                Bounding(const res::Bounding* pBlock, const ResBlockSet& resBlockSet);
                virtual ~Bounding();

                NW4HBM_UT_RUNTIME_TYPEINFO;

                virtual void    DrawSelf(const DrawInfo& drawInfo);
        };
    }
}

#endif // NW4HBM_LYT_BOUNDING_H
