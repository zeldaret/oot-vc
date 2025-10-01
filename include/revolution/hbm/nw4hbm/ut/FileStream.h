#ifndef NW4HBM_UT_FILE_STREAM_H
#define NW4HBM_UT_FILE_STREAM_H

#include "revolution/hbm/nw4hbm/ut/IOStream.h"

namespace nw4hbm {
namespace ut {
class FileStream : public IOStream {
public:
    enum SeekOrigin {
        SEEK_BEG,
        SEEK_CUR,
        SEEK_END
    };

public:
    NW4HBM_UT_RUNTIME_TYPEINFO;

    FileStream() {}
    virtual ~FileStream() {} // 0x0C

    /* 0x40 */ virtual u32 GetSize() const = 0;

    /* 0x44 */ virtual void Seek(s32 offset, u32 origin);

    /* 0x48 */ virtual void Cancel();
    /* 0x4C */ virtual bool CancelAsync(IOStreamCallback pCallback, void* pCallbackArg);

    /* 0x50 */ virtual bool CanSeek() const = 0;
    /* 0x54 */ virtual bool CanCancel() const = 0;

    /* 0x58 */ virtual u32 Tell() const = 0;

protected:
    class FilePosition {
    public:
        FilePosition() :
            mFileSize(0),
            mPosition(0) {}

        u32 GetFileSize() const { return mFileSize; }
        void SetFileSize(u32 size) { mFileSize = size; }

        u32 Tell() const { return mPosition; }

        u32 Skip(s32 offset);
        u32 Append(s32 offset);
        void Seek(s32 offset, u32 origin);

    private:
        /* 0x0 */ u32 mFileSize;
        /* 0x4 */ u32 mPosition;
    };
};
} // namespace ut
} // namespace nw4hbm

#endif
