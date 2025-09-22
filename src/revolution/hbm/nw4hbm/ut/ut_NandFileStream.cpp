#include "macros.h"
#include "revolution.h"

namespace nw4hbm {
namespace ut {

const ut::detail::RuntimeTypeInfo NandFileStream::typeInfo(&FileStream::typeInfo);

void NandFileStream::NandAsyncCallback_(s32 result, NANDCommandBlock* commandBlock) {
    NW4HBMAssertPointerNonnull_Line(commandBlock, 44);
    NandFileStream* p = reinterpret_cast<NandFileStreamInfo*>(commandBlock)->stream;

    p->mIsBusy = false;
    p->mAsyncResult = result;

    if (p->mCallback != nullptr) {
        p->mCallback(result, p, p->mArg);
    }
}

void NandFileStream::Initialize_() {
    mCanRead = false;
    mCanWrite = false;

    mCloseOnDestroyFlg = false;
    mCloseEnableFlg = false;

    mAvailable = false;
    mIsBusy = false;

    mCallback = nullptr;
    mArg = nullptr;
    mAsyncResult = NAND_RESULT_OK;

    mFileInfo.stream = this;
}

NandFileStream::NandFileStream(const char* path, u32 mode) {
    NW4HBMAssertPointerNonnull_Line(path, 113);
    Initialize_();
    Open(path, mode);
}

NandFileStream::NandFileStream(const NANDFileInfo* pInfo, u32 mode, bool enableClose) {
    Initialize_();
    Open(pInfo, mode, enableClose);
}

NandFileStream::~NandFileStream() {
    if (mCloseOnDestroyFlg) {
        Close();
    }
}

bool NandFileStream::Open(const char* path, u32 mode) {
    NW4HBMAssertPointerNonnull_Line(path, 173);

    if (mCloseOnDestroyFlg) {
        Close();
    }

    mCanRead = mode & NAND_ACCESS_READ;
    mCanWrite = mode & NAND_ACCESS_WRITE;

    if (NANDOpen(path, &mFileInfo.nandInfo, mode) != NAND_RESULT_OK) {
        return false;
    }

    if (mCanRead) {
        u32 fileSize;

        if (NANDGetLength(&mFileInfo.nandInfo, &fileSize) != NAND_RESULT_OK) {
            NANDClose(&mFileInfo.nandInfo);
            return false;
        }

        mFilePosition.SetFileSize(fileSize);
    }

    mFilePosition.Seek(0, SEEK_BEG);

    mCloseOnDestroyFlg = true;
    mCloseEnableFlg = true;
    mAvailable = true;

    return true;
}

bool NandFileStream::Open(const NANDFileInfo* pInfo, u32 mode, bool enableClose) {
    if (mCloseOnDestroyFlg) {
        Close();
    }

    mCanRead = mode & NAND_ACCESS_READ;
    mCanWrite = mode & NAND_ACCESS_WRITE;

    mFileInfo.nandInfo = *pInfo;

    u32 fileSize;
    if (NANDGetLength(&mFileInfo.nandInfo, &fileSize) != NAND_RESULT_OK) {
        if (enableClose) {
            NANDClose(&mFileInfo.nandInfo);
        }

        return false;
    }

    mFilePosition.SetFileSize(fileSize);
    mFilePosition.Seek(0, SEEK_BEG);

    mCloseOnDestroyFlg = false;
    mCloseEnableFlg = enableClose;
    mAvailable = true;

    return true;
}

void NandFileStream::Close() {
    if (mCloseEnableFlg && mAvailable) {
        NW4HBMAssertMessage_Line(NANDClose(&mFileInfo.nandInfo) == NAND_RESULT_OK, 264,
                                 "Can't Close NAND File. It still has been used\n");
        mAvailable = false;
    }
}

s32 NandFileStream::Read(void* buf, u32 length) {
    NW4HBMAlign32_Line(buf, 284);
    NW4HBMAlign32_Line(length, 285);
    NW4HBMAssertMessage_Line(this->IsAvailable() != 0, 286, "NandFileStream is not opened");
    NANDSeek(&mFileInfo.nandInfo, mFilePosition.Tell(), NAND_SEEK_BEG);

    s32 result = NANDRead(&mFileInfo.nandInfo, buf, length);
    mFilePosition.Skip(result);

    return result;
}

bool NandFileStream::ReadAsync(void* buf, u32 length, StreamCallback pCallback, void* pCallbackArg) {
    NW4HBMAlign32_2_Line(buf, 313);
    NW4HBMAlign32_2_Line(length, 314);
    NW4HBMAssertMessage_Line(this->IsAvailable() != 0, 315, "NandFileStream is not opened");

    return ReadAsyncImpl(buf, length, pCallback, pCallbackArg);
}

// fake? ReadAsync requires inlines
void NandFileStream::ReadAsyncSetArgs(StreamCallback pCallback, void* pCallbackArg) {
    mCallback = pCallback;
    mArg = pCallbackArg;
    mIsBusy = true;
}

// fake? ReadAsync requires inlines
bool NandFileStream::ReadAsyncImpl(void* buf, u32 length, StreamCallback pCallback, void* pCallbackArg) {
    NW4HBMAlign32_Line(buf, 370);
    NW4HBMAlign32_Line(length, 371);
    NW4HBMAssertMessage_Line(this->IsAvailable() != 0, 372, "NandFileStream is not opened");

    ReadAsyncSetArgs(pCallback, pCallbackArg);

    NANDSeek(&mFileInfo.nandInfo, mFilePosition.Tell(), NAND_SEEK_BEG);

    bool success =
        NANDReadAsync(&mFileInfo.nandInfo, buf, length, NandAsyncCallback_, &mFileInfo.nandBlock) == NAND_RESULT_OK;

    if (success) {
        mFilePosition.Skip(length);
    } else {
        mIsBusy = false;
    }

    return success;
}

void NandFileStream::Write(const void* buf, u32 length) {
    NW4HBMAlign32_Line(buf, 396);
    NW4HBMAlign32_Line(length, 397);
    NW4HBMAssertMessage_Line(this->IsAvailable() != 0, 398, "NandFileStream is not opened");

    NANDSeek(&mFileInfo.nandInfo, mFilePosition.Tell(), NAND_SEEK_BEG);
    s32 result = NANDWrite(&mFileInfo.nandInfo, buf, length);

    //! @bug: Error code will be interpreted as a negative size
    mFilePosition.Append(result);
}

bool NandFileStream::WriteAsync(const void* buf, u32 length, StreamCallback pCallback, void* pCallbackArg) {
    NW4HBMAlign32_Line(buf, 423);
    NW4HBMAlign32_Line(length, 424);
    NW4HBMAssertMessage_Line(this->IsAvailable() != 0, 425, "NandFileStream is not opened");

    mCallback = pCallback;
    mArg = pCallbackArg;
    mIsBusy = true;

    NANDSeek(&mFileInfo.nandInfo, mFilePosition.Tell(), NAND_SEEK_BEG);

    s32 result = NANDWriteAsync(&mFileInfo.nandInfo, buf, length, NandAsyncCallback_, &mFileInfo.nandBlock);

    if (result == NAND_RESULT_OK) {
        mFilePosition.Append(length);
    } else {
        mIsBusy = false;
    }

    return result == NAND_RESULT_OK;
}

void NandFileStream::Seek(s32 offset, u32 origin) {
    NW4HBMAssertMessage_Line(this->IsAvailable() != 0, 462, "NandFileStream is not opened");
    mFilePosition.Seek(offset, origin);
}

} // namespace ut
} // namespace nw4hbm
