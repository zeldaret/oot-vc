#ifndef NW4R_UT_NAND_FILE_STREAM_H
#define NW4R_UT_NAND_FILE_STREAM_H

#include "revolution/hbm/nw4hbm/ut/ut_FileStream.hpp"
#include "revolution/nand.h"
#include "revolution/types.h"

namespace nw4hbm {
namespace ut {

class NandFileStream : public FileStream {
  public:
    virtual const ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; }

    NandFileStream(const char* pPath, u32 mode);
    NandFileStream(const NANDFileInfo* pInfo, u32 mode, bool enableClose);
    virtual ~NandFileStream(); // at 0xC

    bool Open(const char* pPath, u32 mode);
    bool Open(const NANDFileInfo* pInfo, u32 mode, bool enableClose) NO_INLINE;

    virtual void Close(); // at 0x10

    virtual s32 Read(void* pDst, u32 size); // at 0x14
    virtual bool ReadAsync(void* pDst, u32 size, StreamCallback pCallback, void* pCallbackArg); // at 0x18
    virtual void Write(const void* pSrc, u32 size); // at 0x1C
    virtual bool WriteAsync(const void* pSrc, u32 size, StreamCallback pCallback, void* pCallbackArg); // at 0x20
    virtual void Seek(s32 offset, u32 origin); // at 0x44

    virtual bool IsBusy() const { return mIsBusy; } // at 0x24
    virtual u32 Tell() const { return mFilePosition.Tell(); } // at 0x58
    virtual u32 GetSize() const { return mFilePosition.GetFileSize(); } // at 0x40
    virtual bool CanAsync() const { return true; } // at 0x28
    virtual bool CanSeek() const { return true; } // at 0x50
    virtual bool CanRead() const { return mCanRead; } // at 0x2C
    virtual bool CanWrite() const { return mCanWrite; } // at 0x30
    virtual bool CanCancel() const { return false; } // at 0x54
    virtual u32 GetOffsetAlign() const { return 1; } // at 0x34
    virtual u32 GetSizeAlign() const { return 32; } // at 0x38
    virtual u32 GetBufferAlign() const { return 32; } // at 0x3C

    void ReadAsyncSetArgs(StreamCallback pCallback, void* pCallbackArg);
    bool ReadAsyncImpl(void* buf, u32 length, StreamCallback pCallback, void* pCallbackArg);

  private:
    struct NandFileStreamInfo {
        NANDCommandBlock nandBlock; // at 0x0
        NANDFileInfo nandInfo; // at 0xB8
        NandFileStream* stream; // at 0x144
    };

  private:
    static void NandAsyncCallback_(s32 result, NANDCommandBlock* pBlock);

    void Initialize_();

  private:
    FilePosition mFilePosition; // at 0x14
    NandFileStreamInfo mFileInfo; // at 0x1C
    bool mCanRead; // at 0x164
    bool mCanWrite; // at 0x165
    volatile bool mIsBusy; // at 0x166
    bool mCloseOnDestroyFlg; // at 0x167
    bool mCloseEnableFlg; // at 0x168

  public:
    static const ut::detail::RuntimeTypeInfo typeInfo;
};

} // namespace ut
} // namespace nw4hbm

#endif
