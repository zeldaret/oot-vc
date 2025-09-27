#include "revolution/hbm/nw4hbm/ut.h"

#include "revolution/hbm/HBMAssert.hpp"

namespace nw4hbm {
namespace ut {

const ut::detail::RuntimeTypeInfo IOStream::typeInfo(nullptr);

s32 IOStream::Read(void* pDst, u32 size) {
#pragma unused(pDst)
#pragma unused(size)
#pragma unused(pCallback)
#pragma unused(pCallbackArg)
    NW4HBMAssertMessage_Line(CanRead(), 41, "Stream don't support READ function\n");
    return 0;
}

bool IOStream::ReadAsync(void* pDst, u32 size, IOStreamCallback pCallback, void* pCallbackArg) {
#pragma unused(pDst)
#pragma unused(size)
#pragma unused(pCallback)
#pragma unused(pCallbackArg)
    NW4HBMAssertMessage_Line(CanRead(), 62, "Stream don't support READ function\n");
    NW4HBMAssertMessage_Line(CanAsync(), 63, "Stream don't support ASYNC function\n");
    return false;
}

void IOStream::Write(const void* pSrc, u32 size) {
#pragma unused(pSrc)
#pragma unused(size)
    NW4HBMAssertMessage_Line(CanWrite(), 82, "Stream don't support WRITE function\n");
}

bool IOStream::WriteAsync(const void* pSrc, u32 size, IOStreamCallback pCallback, void* pCallbackArg) {
#pragma unused(pSrc)
#pragma unused(size)
#pragma unused(pCallback)
#pragma unused(pCallbackArg)
    NW4HBMAssertMessage_Line(CanWrite(), 102, "Stream don't support WRITE function\n");
    NW4HBMAssertMessage_Line(CanAsync(), 103, "Stream don't support ASYNC function\n");
    return false;
}

bool IOStream::IsBusy() const {
    NW4HBMAssertMessage_Line(CanAsync(), 142, "Stream don't support ASYNC function\n");
    return false;
}

} // namespace ut
} // namespace nw4hbm
