#ifndef NW4R_SND_DISPOSE_CALLBACK_H
#define NW4R_SND_DISPOSE_CALLBACK_H
#include "revolution/types.h"

#include "revolution/hbm/ut.hpp"

namespace nw4hbm {
namespace snd {
namespace detail {

class DisposeCallback {
  public:
    NW4R_UT_LINKLIST_NODE_DECL(); // at 0x0

  public:
    virtual ~DisposeCallback() {} // at 0x8

    virtual void InvalidateData(const void* pStart,
                                const void* pEnd) = 0; // at 0xC
    virtual void InvalidateWaveData(const void* pStart,
                                    const void* pEnd) = 0; // at 0x10
};

NW4R_UT_LINKLIST_TYPEDEF_DECL(DisposeCallback);

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
