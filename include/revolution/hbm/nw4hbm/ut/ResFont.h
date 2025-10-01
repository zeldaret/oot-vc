#ifndef NW4HBM_UT_RES_FONT_H
#define NW4HBM_UT_RES_FONT_H

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/Font.h"
#include "revolution/hbm/nw4hbm/ut/binaryFileFormat.h"

namespace nw4hbm {
namespace ut {
namespace detail {
class ResFontBase : public Font {
  public:
    ResFontBase();
    virtual ~ResFontBase();

    virtual int GetWidth() const;
    virtual int GetHeight() const;

    virtual int GetAscent() const;
    virtual int GetDescent() const;

    virtual int GetBaselinePos() const;

    virtual int GetCellHeight() const;
    virtual int GetCellWidth() const;

    virtual int GetMaxCharWidth() const;

    virtual Type GetType() const;

    virtual GXTexFmt GetTextureFormat() const;

    virtual int GetLineFeed() const;

    virtual CharWidths GetDefaultCharWidths() const;
    virtual void SetDefaultCharWidths(const CharWidths& widths);

    virtual bool SetAlternateChar(u16 c);
    virtual void SetLineFeed(int linefeed);

    virtual int GetCharWidth(u16 c) const;
    virtual CharWidths GetCharWidths(u16 c) const;

    virtual void GetGlyph(Glyph* glyph, u16 c) const;

    virtual FontEncoding GetEncoding() const;

    void SetResourceBuffer(void* pUserBuffer, FontInformation* pFontInfo);

    u16 GetGlyphIndex(u16 c) const;
    const CharWidths& GetCharWidthsFromIndex(u16 index) const;
    const CharWidths& GetCharWidthsFromIndex(const FontWidth* pWidth, u16 index) const;
    void GetGlyphFromIndex(Glyph* glyph, u16 index) const;

    u16 FindGlyphIndex(u16 c) const;
    u16 FindGlyphIndex(const FontCodeMap* pMap, u16 c) const;

    bool IsManaging(const void* ptr) const { return mResource == ptr; }

  private:
    /* 0x10 */ void* mResource;
    /* 0x14 */ FontInformation* mFontInfo;
};
} // namespace detail

class ResFont : public detail::ResFontBase {
  public:
    ResFont();
    virtual ~ResFont();

    bool SetResource(void* brfnt);
    static FontInformation* Rebuild(BinaryFileHeader* fileHeader);

  private:
    static const u32 SIGNATURE_FONT = 'RFNT'; /* Revolution FoNT */
    static const u32 SIGNATURE_FONT_UNPACKED = 'RFNU'; /* Revolution FoNt Unpacked */

    static const u32 SIGNATURE_FONT_INFO = 'FINF'; /* Font INFormation */
    static const u32 SIGNATURE_TEX_GLYPH = 'TGLP'; /* Texture GLyPh */
    static const u32 SIGNATURE_CHAR_WIDTH = 'CWDH'; /* Character WiDtH */
    static const u32 SIGNATURE_CODE_MAP = 'CMAP'; /* Code MAP */

    static const u32 SIGNATURE_GLGR = 'GLGR'; /* Uhhh what is this?.... Something related to GLyPh??? */
};
} // namespace ut
} // namespace nw4hbm

#endif
