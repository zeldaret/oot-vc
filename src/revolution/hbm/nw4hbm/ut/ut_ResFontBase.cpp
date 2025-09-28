#include "revolution/hbm/nw4hbm/ut/ResFont.h"

#include "macros.h"
#include "revolution/hbm/HBMAssert.hpp"

#define GLYPH_INDEX_NOT_FOUND 0xFFFF

struct CMapScanEntry {
    char16_t ccode; // size 0x02, offset 0x00
    u16 index; // size 0x02, offset 0x02
}; // size 0x04

struct CMapInfoScan {
    u16 num; // size 0x02, offset 0x00
    CMapScanEntry entries[]; // flexible,  offset 0x02 (unit size 0x04)
}; // size 0x02

namespace nw4hbm {
namespace ut {
namespace detail {

ResFontBase::ResFontBase() : mResource(nullptr), mFontInfo(nullptr) {}

ResFontBase::~ResFontBase() {}

void ResFontBase::SetResourceBuffer(void* pUserBuffer, FontInformation* pFontInfo) {
    NW4HBMAssertPointerValid_Line(pUserBuffer, 79);
    NW4HBMAssertPointerValid_Line(pFontInfo, 80);
    NW4HBMAssert_Line(mResource == NULL, 81);
    NW4HBMAssert_Line(mFontInfo == NULL, 82);
    mResource = pUserBuffer;
    mFontInfo = pFontInfo;
}

int ResFontBase::GetWidth() const {
    NW4HBMAssertPointerValid_Line(this, 128);
    NW4HBMAssertPointerValid_Line(mFontInfo, 129);
    return mFontInfo->width;
}

int ResFontBase::GetHeight() const {
    NW4HBMAssertPointerValid_Line(this, 145);
    NW4HBMAssertPointerValid_Line(mFontInfo, 146);
    return mFontInfo->height;
}

int ResFontBase::GetAscent() const {
    NW4HBMAssertPointerValid_Line(this, 162);
    NW4HBMAssertPointerValid_Line(mFontInfo, 163);
    return mFontInfo->ascent;
}

int ResFontBase::GetDescent() const {
    NW4HBMAssertPointerValid_Line(this, 179);
    NW4HBMAssertPointerValid_Line(mFontInfo, 180);
    return mFontInfo->height - mFontInfo->ascent;
}

int ResFontBase::GetBaselinePos() const {
    NW4HBMAssertPointerValid_Line(this, 196);
    NW4HBMAssertPointerValid_Line(mFontInfo, 197);
    return mFontInfo->pGlyph->baselinePos;
}

int ResFontBase::GetCellHeight() const {
    NW4HBMAssertPointerValid_Line(this, 213);
    NW4HBMAssertPointerValid_Line(mFontInfo, 214);
    return mFontInfo->pGlyph->cellHeight;
}

int ResFontBase::GetCellWidth() const {
    NW4HBMAssertPointerValid_Line(this, 230);
    NW4HBMAssertPointerValid_Line(mFontInfo, 231);
    return mFontInfo->pGlyph->cellWidth;
}

int ResFontBase::GetMaxCharWidth() const {
    NW4HBMAssertPointerValid_Line(this, 247);
    NW4HBMAssertPointerValid_Line(mFontInfo, 248);
    return mFontInfo->pGlyph->maxCharWidth;
}

Font::Type ResFontBase::GetType() const { return TYPE_RESOURCE; }

GXTexFmt ResFontBase::GetTextureFormat() const {
    NW4HBMAssertPointerValid_Line(this, 279);
    NW4HBMAssertPointerValid_Line(mFontInfo, 280);
    return static_cast<GXTexFmt>(mFontInfo->pGlyph->sheetFormat);
}

int ResFontBase::GetLineFeed() const {
    NW4HBMAssertPointerValid_Line(this, 296);
    NW4HBMAssertPointerValid_Line(mFontInfo, 297);
    return mFontInfo->linefeed;
}

CharWidths ResFontBase::GetDefaultCharWidths() const {
    NW4HBMAssertPointerValid_Line(this, 313);
    NW4HBMAssertPointerValid_Line(mFontInfo, 314);
    return mFontInfo->defaultWidth;
}

void ResFontBase::SetDefaultCharWidths(const CharWidths& widths) {
    // clang-format off
    NW4HBMAssertPointerValid_Line(this, 330);
    NW4HBMAssertPointerValid_Line(mFontInfo, 331);
    NW4HBMAssertPointerValid_Line(& widths, 332);
    mFontInfo->defaultWidth = widths;
    // clang-format on
}

bool ResFontBase::SetAlternateChar(char16_t c) {
    NW4HBMAssertPointerValid_Line(this, 350);
    NW4HBMAssertPointerValid_Line(mFontInfo, 351);
    u16 index = FindGlyphIndex(c);

    if (index != GLYPH_INDEX_NOT_FOUND) {
        mFontInfo->alterCharIndex = index;
        return true;
    } else {
        return false;
    }
}

void ResFontBase::SetLineFeed(int linefeed) {
    NW4HBMAssertPointerValid_Line(this, 375);
    NW4HBMAssertPointerValid_Line(mFontInfo, 376);
    NW4HBMAssertHeaderClampedLRValue_Line(linefeed, -128, 127, 377);
    mFontInfo->linefeed = linefeed;
}

int ResFontBase::GetCharWidth(char16_t c) const { return GetCharWidths(c).charWidth; }

CharWidths ResFontBase::GetCharWidths(char16_t c) const {
    u16 index = GetGlyphIndex(c);

    return GetCharWidthsFromIndex(index);
}

void ResFontBase::GetGlyph(Glyph* glyph, char16_t c) const {
    u16 index = GetGlyphIndex(c);

    GetGlyphFromIndex(glyph, index);
}

FontEncoding ResFontBase::GetEncoding() const {
    NW4HBMAssertPointerValid_Line(this, 456);
    NW4HBMAssertPointerValid_Line(mFontInfo, 457);
    return static_cast<FontEncoding>(mFontInfo->encoding);
}

u16 ResFontBase::GetGlyphIndex(char16_t c) const {
    NW4HBMAssertPointerValid_Line(this, 482);
    NW4HBMAssertPointerValid_Line(mFontInfo, 483);
    u16 index = FindGlyphIndex(c);

    return index != GLYPH_INDEX_NOT_FOUND ? index : mFontInfo->alterCharIndex;
}

u16 ResFontBase::FindGlyphIndex(char16_t c) const {
    NW4HBMAssertPointerValid_Line(this, 502);
    NW4HBMAssertPointerValid_Line(mFontInfo, 503);
    FontCodeMap* pMap;
    for (pMap = mFontInfo->pMap; pMap; pMap = pMap->pNext) {
        if (pMap->ccodeBegin <= c && c <= pMap->ccodeEnd) {
            return FindGlyphIndex(pMap, c);
        }
    }

    return GLYPH_INDEX_NOT_FOUND;
}

u16 ResFontBase::FindGlyphIndex(const FontCodeMap* pMap, char16_t c) const {
    NW4HBMAssertPointerValid_Line(this, 539);
    NW4HBMAssertPointerValid_Line(pMap, 540);
    u16 index = GLYPH_INDEX_NOT_FOUND;

    switch (pMap->mappingMethod) {
        case FONT_MAPMETHOD_DIRECT: {
            u16 offset = *pMap->mapInfo;

            index = c - pMap->ccodeBegin + offset;
        } break;

        case FONT_MAPMETHOD_TABLE: {
            int table_index = c - pMap->ccodeBegin;

            index = pMap->mapInfo[table_index];
        } break;

        case FONT_MAPMETHOD_SCAN: {
            const CMapInfoScan* scanInfo = reinterpret_cast<const CMapInfoScan*>(pMap->mapInfo);

            const CMapScanEntry* first = scanInfo->entries;
            const CMapScanEntry* last = scanInfo->entries + (scanInfo->num - 1);

            while (first <= last) {
                const CMapScanEntry* mid = first + (last - first) / 2;

                if (mid->ccode < c) {
                    first = mid + 1;
                } else if (c < mid->ccode) {
                    last = mid - 1;
                } else {
                    index = mid->index;
                    break;
                }
            }
        } break;

        default:
            nw4hbm::db::Panic(__FILE__, 597, "unknwon MAPMETHOD");
            break;
    }

    return index;
}

const CharWidths& ResFontBase::GetCharWidthsFromIndex(u16 index) const {

    const FontWidth* pWidth;

    NW4HBMAssertPointerValid_Line(this, 615);
    NW4HBMAssertPointerValid_Line(mFontInfo, 616);
    for (pWidth = mFontInfo->pWidth; pWidth; pWidth = pWidth->pNext) {
        if (pWidth->indexBegin <= index && index <= pWidth->indexEnd) {
            return GetCharWidthsFromIndex(pWidth, index);
        }
    }

    return mFontInfo->defaultWidth;
}

const CharWidths& ResFontBase::GetCharWidthsFromIndex(const FontWidth* pWidth, u16 index) const {
    NW4HBMAssertPointerValid_Line(pWidth, 651);
    return pWidth->widthTable[index - pWidth->indexBegin];
}

void ResFontBase::GetGlyphFromIndex(Glyph* glyph, u16 index) const {
    NW4HBMAssertPointerValid_Line(this, 671);
    NW4HBMAssertPointerValid_Line(mFontInfo, 672);
    FontTextureGlyph& tg = *mFontInfo->pGlyph;

    u32 cellsInASheet = tg.sheetRow * tg.sheetLine;
    u32 sheetNo = index / cellsInASheet;
    u32 cellNo = index % cellsInASheet;
    u32 cellUnitX = cellNo % tg.sheetRow;
    u32 cellUnitY = cellNo / tg.sheetRow;
    u32 cellPixelX = cellUnitX * (tg.cellWidth + 1);
    u32 cellPixelY = cellUnitY * (tg.cellHeight + 1);
    u32 offsetBytes = sheetNo * tg.sheetSize;
    void* pSheet = tg.sheetImage + offsetBytes;

    glyph->pTexture = pSheet;
    glyph->widths = GetCharWidthsFromIndex(index);
    glyph->height = static_cast<u8>(tg.cellHeight);
    glyph->texFormat = static_cast<GXTexFmt>(tg.sheetFormat);
    glyph->texWidth = static_cast<u16>(tg.sheetWidth);
    glyph->texHeight = static_cast<u16>(tg.sheetHeight);
    glyph->cellX = cellPixelX + 1;
    glyph->cellY = cellPixelY + 1;
}

} // namespace detail
} // namespace ut
} // namespace nw4hbm
