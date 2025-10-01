#ifndef NW4HBM_UT_DVD_LOCKED_FILE_STREAM_H
#define NW4HBM_UT_DVD_LOCKED_FILE_STREAM_H

#include "revolution/hbm/nw4hbm/ut/DvdFileStream.h"

namespace nw4hbm {
namespace ut {
class DvdLockedFileStream : public DvdFileStream {
  public:
    NW4HBM_UT_RUNTIME_TYPEINFO;

    explicit DvdLockedFileStream(s32 entrynum);
    DvdLockedFileStream(const DVDFileInfo* info, bool close);
    /* 0x0C */ virtual ~DvdLockedFileStream();

    /* 0x14 */ virtual s32 Read(void* pDst, u32 size);
    virtual bool ReadAsync(void* pDst, u32 size, IOStreamCallback pCallback, void* pCallbackArg) {
        return false;
    } // 0x18

    /* 0x5C */ virtual s32 Peek(void* pDst, u32 size);
    virtual bool PeekAsync(void* pDst, u32 size, IOStreamCallback pCallback, void* pCallbackArg) {
        return false;
    } // 0x60

    virtual bool CanAsync() const { return false; } // 0x28

  private:
    static void InitMutex_();

  private:
    /* 0x6F */ bool mCancelFlag;

    static bool sInitialized;
    static OSMutex sMutex;
};
} // namespace ut
} // namespace nw4hbm

#endif
