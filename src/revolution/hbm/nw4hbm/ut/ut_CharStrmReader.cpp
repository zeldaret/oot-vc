#include "revolution/hbm/nw4hbm/ut/CharStrmReader.h"

#include "revolution/hbm/HBMAssert.hpp"
#include "revolution/types.h"

namespace nw4hbm {
namespace ut {
namespace {
inline bool IsSJISLeadByte(char_t c) { return (0x81 <= c && c < 0xA0) || 0xE0 <= c; }
} // namespace
} // namespace ut
} // namespace nw4hbm

namespace nw4hbm {
namespace ut {

char16_t CharStrmReader::ReadNextCharUTF8() {
    NW4HBMAssertPointerValid_Line(this, 76);
    NW4HBMAssertPointerValid_Line(mCharStrm, 77);
    NW4HBMAssert_Line((GetChar<u8>() & 0xC0) != 0x80, 79);
    byte2_t code;

    if ((GetChar<char8_t>(0) & 0x80) == 0x00) {
        // 1-byte UTF-8 sequence
        code = GetChar<char8_t>(0);
        StepStrm<char8_t>(1);
    } else if ((GetChar<char8_t>(0) & 0xE0) == 0xC0) {
        // 2-byte UTF-8 sequence
        code = (GetChar<char8_t>(0) & 0x1f) << 6 | (GetChar<char8_t>(1) & 0x3f);
        StepStrm<char8_t>(2);
    } else {
        // 3-byte UTF-8 sequence

        NW4HBMAssert_Line((GetChar<u8>() & 0xF0) == 0xE0, 100);
        /* technical ERRATUM: the mask of GetChar<char8_t>(0) should be 0x0f */
        code = (GetChar<char8_t>(0) & 0x1f) << 12 | (GetChar<char8_t>(1) & 0x3f) << 6 | (GetChar<char8_t>(2) & 0x3f);
        StepStrm<char8_t>(3);
    }

    /* NOTE: 4-byte to 7-byte UTF-8 sequences usually encode code points outside
     * of the BMP; I think sticking to the BMP is fine here.
     */

    return code;
}

char16_t CharStrmReader::ReadNextCharUTF16() {
    NW4HBMAssertPointerValid_Line(this, 129);
    NW4HBMAssertPointerValid_Line(mCharStrm, 130);
    NW4HBMAlign2_Line(mCharStrm, 131);
    byte2_t code = GetChar<char16_t>(0);
    StepStrm<char16_t>(1);

    return code;
}

char16_t CharStrmReader::ReadNextCharCP1252() {
    NW4HBMAssertPointerValid_Line(this, 155);
    NW4HBMAssertPointerValid_Line(mCharStrm, 156);
    byte2_t code = GetChar<char_t>(0);
    StepStrm<char_t>(1);

    return code;
}

char16_t CharStrmReader::ReadNextCharSJIS() {
    NW4HBMAssertPointerValid_Line(this, 180);
    NW4HBMAssertPointerValid_Line(mCharStrm, 181);
    byte2_t code;

    if (IsSJISLeadByte(GetChar<char_t>(0))) {
        code = GetChar<char_t>(0) << 8 | GetChar<char_t>(1);
        StepStrm<char_t>(2);
    } else {
        code = GetChar<char_t>(0);
        StepStrm<char_t>(1);
    }

    return code;
}

} // namespace ut
} // namespace nw4hbm
