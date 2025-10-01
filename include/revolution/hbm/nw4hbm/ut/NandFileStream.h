#ifndef NW4HBM_UT_NAND_FILE_STREAM_H
#define NW4HBM_UT_NAND_FILE_STREAM_H

#include "revolution/hbm/nw4hbm/ut/FileStream.h"

#include "revolution/nand.h"

namespace nw4hbm {
namespace ut {
class NandFileStream : public FileStream {
  public:
    NW4HBM_UT_RUNTIME_TYPEINFO;

    NandFileStream(const char* path, u32 mode);
    NandFileStream(const NANDFileInfo* info, u32 mode, bool enableClose);
    /* 0x0C */ virtual ~NandFileStream();

    bool Open(const char* path, u32 mode);
    bool Open(const NANDFileInfo* info, u32 mode, bool enableClose) NO_INLINE;

    /* 0x10 */ virtual void Close();

    /* 0x14 */ virtual s32 Read(void* pDst, u32 size);
    /* 0x18 */ virtual bool ReadAsync(void* pDst, u32 size, IOStreamCallback pCallback, void* pCallbackArg);

    /* 0x1C */ virtual void Write(const void* pSrc, u32 size);
    /* 0x20 */ virtual bool WriteAsync(const void* pSrc, u32 size, IOStreamCallback pCallback, void* pCallbackArg);

    /* 0x44 */ virtual void Seek(s32 offset, u32 origin);

    virtual bool IsBusy() const { return mIsBusy; } // 0x24

    virtual u32 Tell() const { return mFilePosition.Tell(); } // 0x58
    virtual u32 GetSize() const { return mFilePosition.GetFileSize(); } // 0x40

    virtual bool CanAsync() const { return true; } // 0x28
    virtual bool CanSeek() const { return true; } // 0x50
    virtual bool CanRead() const { return mCanRead; } // 0x2C
    virtual bool CanWrite() const { return mCanWrite; } // 0x30
    virtual bool CanCancel() const { return false; } // 0x54

    virtual u32 GetOffsetAlign() const { return 1; } // 0x34
    virtual u32 GetSizeAlign() const { return DEFAULT_ALIGN; } // 0x38
    virtual u32 GetBufferAlign() const { return DEFAULT_ALIGN; } // 0x3C

  private:
    typedef struct NandFileStreamInfo {
        /* 0x00 */ NANDCommandBlock nandBlock;
        /* 0xB8 */ NANDFileInfo nandInfo;
        /* 0x144 */ NandFileStream* stream;
    } NandFileStreamInfo;

    static void NandAsyncCallback_(s32 result, NANDCommandBlock* pBlock);

    void Initialize_();
    bool ReadAsyncImpl(void* buf, u32 length, IOStreamCallback pCallback, void* pCallbackArg);
    void ReadAsyncSetArgs(IOStreamCallback pCallback, void* pCallbackArg);

    /* 0x14 */ FilePosition mFilePosition;
    /* 0x1C */ NandFileStreamInfo mFileInfo;

    /* 0x164 */ bool mCanRead;
    /* 0x165 */ bool mCanWrite;

    /* 0x166 */ volatile bool mIsBusy;

    /* 0x167 */ bool mCloseOnDestroyFlg;
    /* 0x168 */ bool mCloseEnableFlg;
};
} // namespace ut
} // namespace nw4hbm

#endif
