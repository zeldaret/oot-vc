#ifndef RVL_SDK_HBM_NW4HBM_UT_CHAR_STREAM_READER_HPP
#define RVL_SDK_HBM_NW4HBM_UT_CHAR_STREAM_READER_HPP

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/hbm/HBMAssert.hpp"
#include "revolution/hbm/nw4hbm/db/assert.hpp"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace ut {
class CharStrmReader {
    // typedefs
  public:
    typedef char16_t (CharStrmReader::*ReadFunc)();

    // methods
  public:
    // cdtors
    CharStrmReader(ReadFunc func) : mCharStrm(nullptr), mReadFunc(func) {}

    // methods
    const void* GetCurrentPos() const {
        NW4HBMAssertPointerValid_FileLine(this, "CharStrmReader.h", 68);
        return mCharStrm;
    }

    void Set(const char* stream) {
        NW4HBMAssertPointerValid_FileLine(this, "CharStrmReader.h", 59);
        NW4HBMAlign2_FileLine(stream, "CharStrmReader.h", 60);
        NW4HBMAssertPointerValid_FileLine(stream, "CharStrmReader.h", 61);
        mCharStrm = stream;
    }

    void Set(const wchar_t* stream) {
        NW4HBMAssertPointerValid_FileLine(this, "CharStrmReader.h", 59);
        NW4HBMAlign2_FileLine(stream, "CharStrmReader.h", 60);
        NW4HBMAssertPointerValid_FileLine(stream, "CharStrmReader.h", 61);
        mCharStrm = stream;
    }

    char16_t ReadNextCharUTF8();
    char16_t ReadNextCharUTF16();
    char16_t ReadNextCharCP1252();
    char16_t ReadNextCharSJIS();

    char16_t Next() {
        NW4HBMAssertPointerValid_FileLine(this, "CharStrmReader.h", 74);
        NW4HBMAssert_FileLine(mReadFunc == ReadNextCharUTF16, "CharStrmReader.h", 62);
        return (this->*mReadFunc)();
    }

    void Something() {
    }

    template <typename charT> charT GetChar() const {
        const charT* const charStrm = static_cast<const charT* const>(mCharStrm);

        return charStrm[0];
    }

    template <typename charT> charT GetChar(int offset) const {
        const charT* const charStrm = static_cast<const charT* const>(mCharStrm);

        return charStrm[offset];
    }

    template <typename charT> void StepStrm(int step) {
        // ...sure
        const charT*& charStrm = *reinterpret_cast<const charT**>(&mCharStrm);

        charStrm += step;
    }

    // members
  private:
    const void* mCharStrm; // size 0x04, offset 0x00
    ReadFunc mReadFunc; // size 0x0c, offset 0x04
}; // size 0x10
} // namespace ut
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_UT_CHAR_STREAM_READER_HPP
