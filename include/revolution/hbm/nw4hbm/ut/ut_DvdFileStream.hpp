#ifndef NW4R_UT_DVD_FILE_STREAM_H
#define NW4R_UT_DVD_FILE_STREAM_H

#include "revolution/hbm/nw4hbm/ut/ut_FileStream.hpp"
#include "revolution/types.h"

namespace nw4hbm {
namespace ut {

class DvdFileStream : public FileStream {
  public:
    virtual const ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; }

    static const ut::detail::RuntimeTypeInfo typeInfo;
};

} // namespace ut
} // namespace nw4hbm

#endif
