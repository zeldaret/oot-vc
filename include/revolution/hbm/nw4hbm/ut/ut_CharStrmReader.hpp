#ifndef RVL_SDK_HBM_NW4HBM_UT_CHAR_STREAM_READER_HPP
#define RVL_SDK_HBM_NW4HBM_UT_CHAR_STREAM_READER_HPP

/*******************************************************************************
 * headers
 */

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
    const void* GetCurrentPos() const { return mCharStrm; }

    void Set(const char* stream) { mCharStrm = stream; }
    void Set(const wchar_t* stream) { mCharStrm = stream; }

    char16_t ReadNextCharUTF8();
    char16_t ReadNextCharUTF16();
    char16_t ReadNextCharCP1252();
    char16_t ReadNextCharSJIS();

    char16_t Next() { return (this->*mReadFunc)(); }

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
