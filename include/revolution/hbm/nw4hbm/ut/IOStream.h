#ifndef NW4HBM_UT_IO_STREAM_H
#define NW4HBM_UT_IO_STREAM_H

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/RuntimeTypeInfo.h"

namespace nw4hbm {
namespace ut {
class IOStream {
  public:
    typedef void (*IOStreamCallback)(s32 result, IOStream* pStream, void* pCallbackArg);

    NW4HBM_UT_RUNTIME_TYPEINFO;

    IOStream() : mAvailable(false), mCallback(nullptr), mArg(nullptr) {}

    virtual ~IOStream() {} // 0x0C

    /* 0x10 */ virtual void Close() = 0;

    /* 0x14 */ virtual s32 Read(void* pDst, u32 size);
    /* 0x18 */ virtual bool ReadAsync(void* pDst, u32 size, IOStreamCallback pCallback, void* pCallbackArg);

    /* 0x1C */ virtual void Write(const void* pSrc, u32 size);
    /* 0x20 */ virtual bool WriteAsync(const void* pSrc, u32 size, IOStreamCallback pCallback, void* pCallbackArg);

    /* 0x24 */ virtual bool IsBusy() const;

    /* 0x28 */ virtual bool CanAsync() const = 0;
    /* 0x2C */ virtual bool CanRead() const = 0;
    /* 0x30 */ virtual bool CanWrite() const = 0;

    virtual u32 GetOffsetAlign() const { return 1; } // 0x34
    virtual u32 GetSizeAlign() const { return 1; } // 0x38
    virtual u32 GetBufferAlign() const { return 1; } // 0x3C

    bool IsAvailable() const { return mAvailable; }

  protected:
    /* 0x04 */ bool mAvailable;

    /* 0x08 */ s32 mAsyncResult;

    /* 0x0C */ IOStreamCallback mCallback;
    /* 0x10 */ void* mArg;
};
}; // namespace ut
}; // namespace nw4hbm

#endif
