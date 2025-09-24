#ifndef NW4R_UT_DVD_FILE_STREAM_H
#define NW4R_UT_DVD_FILE_STREAM_H

#include "revolution/hbm/nw4hbm/ut/ut_FileStream.hpp"
#include "revolution/types.h"

namespace nw4hbm {
namespace ut {

class DvdFileStream : public FileStream {
  public:
    virtual const ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; }

    void SetPriority(s32 priority) { mPriority = priority; }

    static const ut::detail::RuntimeTypeInfo typeInfo;

	private:
		char pad[0x68 - 0x14];
		s32 mPriority;
};

} // namespace ut
} // namespace nw4hbm

#endif
