#include "revolution/hbm/nw4hbm/snd/NandSoundArchive.h"

#include "revolution/hbm/nw4hbm/snd/PlayerHeap.h"
#include "revolution/hbm/nw4hbm/ut.h"

#include "revolution/nand.h"

#include "cstring.hpp"
#include "new.hpp"

#include "decomp.h"

namespace nw4hbm {
namespace snd {

class NandSoundArchive::NandFileStream : public ut::NandFileStream {
public:
    NandFileStream(const NANDFileInfo* pFileInfo, u32 offset, u32 size);
    NandFileStream(const char* path, u32 offset, u32 size);

    /* 0x14 */ virtual s32 Read(void* buf, u32 size);
    /* 0x44 */ virtual void Seek(s32 offset, u32 origin);

    /* 0x58 */ virtual u32 Tell() const { return ut::NandFileStream::Tell() - mOffset; }

    /* 0x40 */ virtual u32 GetSize() const { return mSize; }

private:
    /* 0x16C */ s32 mOffset;
    /* 0x170 */ s32 mSize;
};

NandSoundArchive::NandSoundArchive() :
    mOpen(false) {}

NandSoundArchive::~NandSoundArchive() { Close(); }

bool NandSoundArchive::Open(const char* path) {
    s32 result;

    if (mOpen) {
        Close();
    }

    result = NANDOpen(path, &mFileInfo, NAND_ACCESS_READ);
    if (result != NAND_RESULT_OK) {
        NW4HBMCheckMessage_Line(result != NAND_RESULT_OK, 88, "Cannot find file %s\n", path);
        return false;
    }

    mOpen = true;

    bool bHeader = LoadFileHeader();
    if (!bHeader) {
        NW4HBMCheckMessage_Line(bHeader, 97, "Cannot load header\n");
        return false;
    }

    char currentDir[64];
    NANDGetCurrentDir(currentDir);
    u32 nandLen = std::strlen(currentDir);

    // clang-format off
    NW4HBMAssert_Line(nandLen+1 < FILE_PATH_MAX, 105);
    // clang-format on

    char extRoot[FILE_PATH_MAX];
    std::strncpy(extRoot, currentDir, nandLen + 1);

    for (int i = std::strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            NW4HBMAssert_Line(i < FILE_PATH_MAX, 115);

            // @bug Long path can overflow extRoot buffer
            std::strncat(extRoot, path, i);
            extRoot[nandLen + i] = '\0';
            break;
        }
    }

    SetExternalFileRoot(extRoot);
    return true;
}

void NandSoundArchive::Close() {
    if (mOpen) {
        s32 result = NANDClose(&mFileInfo);
        NW4HBMCheckMessage_Line(result == NAND_RESULT_OK, 141, "Failed close mcs file. ErrID=%d\n", result);
        mOpen = false;
    }

    Shutdown();
}

ut::FileStream* NandSoundArchive::OpenStream(void* buffer, int size, u32 offset, u32 length) const {
    if (!mOpen) {
        return nullptr;
    }

    if (size < sizeof(NandFileStream)) {
        return nullptr;
    }

    return new (buffer) NandFileStream(&mFileInfo, offset, length);
}

ut::FileStream* NandSoundArchive::OpenExtStream(void* buffer, int size, const char* extPath, u32 offset,
                                                u32 length) const {
    if (!mOpen) {
        return nullptr;
    }

    if (size < sizeof(NandFileStream)) {
        return nullptr;
    }

    NandFileStream* pExtStream = new (buffer) NandFileStream(extPath, offset, length);

    if (!pExtStream->IsAvailable()) {
        pExtStream->~NandFileStream();
        return nullptr;
    }

    return pExtStream;
}

int NandSoundArchive::detail_GetRequiredStreamBufferSize() const { return sizeof(NandFileStream); }

bool NandSoundArchive::LoadFileHeader() {
    NW4HBMAssert_Line(mOpen, 188);
    u8 headerArea[detail::SoundArchiveFile::HEADER_AREA_SIZE];

    static const u32 headerAlignSize = ut::RoundUp(sizeof(detail::SoundArchiveFile::Header), 32);

    void* pFile = ut::RoundUp<u8>(headerArea, 32);

    if (NANDSeek(&mFileInfo, 0, NAND_SEEK_BEG) != NAND_RESULT_OK) {
        // wrong function name in the message
        NW4HBMWarningMessage_Line(205, "NandSoundArchive::LoadHeader cannot seek file.\n");
        return false;
    }

    s32 bytesRead = NANDRead(&mFileInfo, pFile, headerAlignSize);
    if (bytesRead != headerAlignSize) {
        // wrong function name in the message
        NW4HBMWarningMessage_Line(216, "NandSoundArchive::LoadHeader cannot read file.\n");
        return false;
    }

    mFileReader.Init(pFile);
    Setup(&mFileReader);
    return true;
}

bool NandSoundArchive::LoadHeader(void* buffer, u32 size) {
    NW4HBMAssert_Line(mOpen, 239);

    u32 infoOffsetU = mFileReader.GetInfoChunkOffset();
    u32 infoSize = mFileReader.GetInfoChunkSize();

    if (size < infoSize) {
        NW4HBMCheckMessage_Line(size >= infoSize, 249, "SoundArchive::LoadHeader buffer size is too small.\n");
        return false;
    }

    s32 infoOffset = infoOffsetU;
    s32 currOffset = NANDSeek(&mFileInfo, infoOffset, NAND_SEEK_BEG);
    if (currOffset != infoOffset) {
        NW4HBMWarningMessage_Line(262, "NandSoundArchive::LoadHeader cannot seek file.\n");
        return false;
    }

    s32 bytesRead = NANDRead(&mFileInfo, buffer, infoSize);
    if (bytesRead != infoSize) {
        NW4HBMWarningMessage_Line(273, "NandSoundArchive::LoadHeader cannot read file.\n");
        return false;
    }

    mFileReader.SetInfoChunk(buffer, infoSize);
    return true;
}

bool NandSoundArchive::LoadLabelStringData(void* buffer, u32 size) {
    u32 labelSize = mFileReader.GetLabelStringChunkSize();

    u32 labelOffsetU = mFileReader.GetLabelStringChunkOffset();
    s32 labelOffset = *reinterpret_cast<s32*>(&labelOffsetU);

    if (size < labelSize) {
        NW4HBMCheckMessage(size >= labelSize, "SoundArchive::LoadLabelStringData buffer size is too small.");
        return false;
    }

    s32 currOffset = NANDSeek(&mFileInfo, labelOffset, NAND_SEEK_BEG);
    if (currOffset != labelOffset) {
        NW4HBMWarningMessage("NandSoundArchive::LoadLabelStringData cannot seek file.\n");
        return false;
    }

    s32 bytesRead = NANDRead(&mFileInfo, buffer, labelSize);
    if (bytesRead != labelSize) {
        NW4HBMWarningMessage("NandSoundArchive::LoadLabelStringData cannot read file.\n");
        return false;
    }

    mFileReader.SetStringChunk(buffer, labelSize);
    return true;
}

NandSoundArchive::NandFileStream::NandFileStream(const NANDFileInfo* pFileInfo, u32 offset, u32 size) :
    ut::NandFileStream(pFileInfo, NAND_ACCESS_READ, false),
    mOffset(offset),
    mSize(size) {

    if (IsAvailable()) {
        if (mSize == 0) {
            mSize = ut::NandFileStream::GetSize();
        }

        ut::NandFileStream::Seek(mOffset, SEEK_BEG);
    }
}

NandSoundArchive::NandFileStream::NandFileStream(const char* path, u32 offset, u32 size) :
    ut::NandFileStream(path, NAND_ACCESS_READ),
    mOffset(offset),
    mSize(size) {

    if (IsAvailable()) {
        if (mSize == 0) {
            mSize = ut::NandFileStream::GetSize();
        }

        ut::NandFileStream::Seek(mOffset, SEEK_BEG);
    }
}

s32 NandSoundArchive::NandFileStream::Read(void* buf, u32 size) {
    NW4HBMAssertPointerNonnull_Line(buf, 384);
    u32 endOffset = mOffset + mSize;
    u32 startOffset = ut::NandFileStream::Tell();

    if (startOffset + size > endOffset) {
        size = ut::RoundUp(endOffset - ut::NandFileStream::Tell(), 32);
    }

    s32 bytesRead = ut::NandFileStream::Read(buf, size);
    DCStoreRange(buf, size);

    return bytesRead;
}

void NandSoundArchive::NandFileStream::Seek(s32 offset, u32 origin) {
    switch (origin) {
        case SEEK_BEG: {
            offset += mOffset;
            break;
        }
        case SEEK_CUR: {
            offset += ut::NandFileStream::Tell();
            break;
        }
        case SEEK_END: {
            offset = mOffset + mSize - offset;
            break;
        }
        default: {
            NW4HBMPanicMessage_Line(413, "Unsupported Seek origin");
            return;
        }
    }

    if (offset < mOffset) {
        offset = mOffset;
    } else if (offset > mOffset + mSize) {
        offset = mOffset + mSize;
    }

    ut::NandFileStream::Seek(offset, SEEK_BEG);
}

DECOMP_FORCE_CLASS_METHOD(detail::PlayerHeapList, GetNodeFromPointer(nullptr));

} // namespace snd
} // namespace nw4hbm
