#ifndef NW4HBM_UT_CHAR_STREAM_READER_H
#define NW4HBM_UT_CHAR_STREAM_READER_H

// required to fix data order in `lyt_textBox.cpp`
#include "revolution/hbm/nw4hbm/lyt/textBox.h"

namespace nw4hbm {
namespace ut {
class CharStrmReader {
  public:
    typedef u16 (CharStrmReader::*ReadFunc)();

    CharStrmReader(ReadFunc func) : mCharStrm(nullptr), mReadFunc(func) {}

    void Set(const char* stream) {
        NW4HBMAssertPointerValid_Line(this, 49);
        NW4HBMAssertPointerValid_Line(stream, 50);
        NW4HBMAssert_Line(
            mReadFunc == ReadNextCharUTF8 || mReadFunc == ReadNextCharCP1252 || mReadFunc == ReadNextCharSJIS, 53);
        mCharStrm = stream;
    }

    void Set(const wchar_t* stream) {
        NW4HBMAssertPointerValid_Line(this, 59);
        NW4HBMAlign2_Line(stream, 60);
        NW4HBMAssertPointerValid_Line(stream, 61);
        NW4HBMAssert_Line(mReadFunc == ReadNextCharUTF16, 62);
        mCharStrm = stream;
    }

    const void* GetCurrentPos() const {
        NW4HBMAssertPointerValid_Line(this, 68);
        return mCharStrm;
    }

    u16 Next() {
        NW4HBMAssertPointerValid_Line(this, 74);
        return (this->*mReadFunc)();
    }

    u16 ReadNextCharUTF8();
    u16 ReadNextCharUTF16();
    u16 ReadNextCharCP1252();
    u16 ReadNextCharSJIS();

    template <typename T> T GetChar() const {
        const T* const charStrm = static_cast<const T* const>(mCharStrm);
        return charStrm[0];
    }

    template <typename T> T GetChar(int offset) const {
        const T* const charStrm = static_cast<const T* const>(mCharStrm);
        return charStrm[offset];
    }

    template <typename T> void StepStrm(int step) {
        const T*& charStrm = *reinterpret_cast<const T**>(&mCharStrm);
        charStrm += step;
    }

    /* 0x00 */ const void* mCharStrm;
    /* 0x04 */ const ReadFunc mReadFunc;
};

} // namespace ut
} // namespace nw4hbm

#endif
