#include "revolution/hbm/nw4hbm/ut/ResFont.h"

#include "revolution/hbm/nw4hbm/config.h"
#include "revolution/hbm/nw4hbm/db/console.h"

#include "revolution/hbm/HBMAssert.hpp"

#define MAGIC_FONT 'RFNT' // Revolution Font
#define MAGIC_FONT_UNPACKED 'RFNU' // Revolution Font, unpacked

#define MAGIC_FONT_INFO 'FINF' // FontInformation
#define MAGIC_FONT_TEX_GLYPH 'TGLP' // FontTextureGlyph
#define MAGIC_FONT_CHAR_WIDTH 'CWDH' // Font[Char]Width
#define MAGIC_FONT_CODE_MAP 'CMAP' // FontCodeMap
#define MAGIC_FONT_GLGR 'GLGR' // Glyph Group?

#define CONVERT_OFFSET_TO_PTR(type_, ptr_, offset_) reinterpret_cast<type_*>(reinterpret_cast<u32>(ptr_) + offset_)

namespace nw4hbm {
namespace ut {
namespace {
template <typename T> void ResolveOffset(T*& ptr, void* base) {
    *reinterpret_cast<u32*>(&ptr) = reinterpret_cast<u32>(base) + reinterpret_cast<u32>(ptr);
}
} // namespace
} // namespace ut
} // namespace nw4hbm

namespace nw4hbm {
namespace ut {

ResFont::ResFont() {}

ResFont::~ResFont() {}

bool ResFont::SetResource(void* brfnt) {
    NW4HBMAssertPointerValid_Line(this, 97);
    NW4HBMAssertPointerValid_Line(brfnt, 98);
    NW4HBMAlign32_Line(brfnt, 99);

    FontInformation* pFontInfo = nullptr;
    BinaryFileHeader* fileHeader = static_cast<BinaryFileHeader*>(brfnt);

    if (!IsManaging(nullptr)) {
        NW4HBMWarningMessage_Line(107, "Font resource already atached.");
        return false;
    }

    if (fileHeader->signature == MAGIC_FONT_UNPACKED) {
        BinaryBlockHeader* blockHeader;
        int nBlocks = 0;

        blockHeader = CONVERT_OFFSET_TO_PTR(BinaryBlockHeader, fileHeader, fileHeader->headerSize);

        while (nBlocks < fileHeader->dataBlocks) {
            NW4HBMAssertPointerValid_Line(blockHeader, 124);
            if (blockHeader->kind == MAGIC_FONT_INFO) {
                pFontInfo = CONVERT_OFFSET_TO_PTR(FontInformation, blockHeader, sizeof *blockHeader);
                break;
            }

            blockHeader = CONVERT_OFFSET_TO_PTR(BinaryBlockHeader, blockHeader, blockHeader->size);
            nBlocks++;
        }
    } else {
        if (fileHeader->version == NW4HBM_VERSION(1, 4)) {
            if (!IsValidBinaryFile(fileHeader, MAGIC_FONT, NW4HBM_VERSION(1, 4), 2)) {
                NW4HBMWarningMessage_Line(150, "Invalid font resource.");
                return false;
            }
        } else {
            if (!IsValidBinaryFile(fileHeader, MAGIC_FONT, NW4HBM_VERSION(1, 2), 2)) {
                NW4HBMWarningMessage_Line(160, "Invalid font resource.");
                return false;
            }
        }

        pFontInfo = Rebuild(fileHeader);
    }

    if (!pFontInfo) {
        return false;
    }

    SetResourceBuffer(brfnt, pFontInfo);
    InitReaderFunc(GetEncoding());

    return true;
}

extern "C" char unused1[] = "ResFont::RemoveResource(): Res font is not loaded.\n";

FontInformation* ResFont::Rebuild(BinaryFileHeader* fileHeader) {
    BinaryBlockHeader* blockHeader;
    FontInformation* info;
    int nBlocks;

    NW4HBMAssertPointerValid_Line(fileHeader, 218);
    NW4HBMAlign32_Line(fileHeader, 219);

    info = nullptr;
    nBlocks = 0;
    blockHeader = CONVERT_OFFSET_TO_PTR(BinaryBlockHeader, fileHeader, fileHeader->headerSize);

    while (nBlocks < fileHeader->dataBlocks) {
        NW4HBMAssertPointerValid_Line(blockHeader, 230);

        switch (blockHeader->kind) {
            case MAGIC_FONT_INFO: {
                NW4HBMAssert_Line(info == NULL, 237);
                info = CONVERT_OFFSET_TO_PTR(FontInformation, blockHeader, sizeof *blockHeader);

                NW4HBMAssert_Line(info->fontType == FONT_TYPE_NNGCTEXTURE, 243);
                NW4HBMAssert_Line(info->alterCharIndex != GLYPH_INDEX_NOT_FOUND, 244);

                // no check
                NW4HBMAssertPointerNonnull_Line(info->pGlyph, 247);
                ResolveOffset(info->pGlyph, fileHeader);
                NW4HBMAssertPointerValid_Line(info->pGlyph, 249);

                if (info->pWidth) {
                    ResolveOffset(info->pWidth, fileHeader);
                    NW4HBMAssertPointerValid_Line(info->pWidth, 255);
                }

                if (info->pMap) {
                    ResolveOffset(info->pMap, fileHeader);
                    NW4HBMAssertPointerValid_Line(info->pMap, 260);
                }
            } break;

            case MAGIC_FONT_TEX_GLYPH: {
                FontTextureGlyph* glyph = CONVERT_OFFSET_TO_PTR(FontTextureGlyph, blockHeader, sizeof *blockHeader);

                NW4HBMAssertPointerNonnull_Line(glyph->sheetImage, 274);
                // no check
                ResolveOffset(glyph->sheetImage, fileHeader);
                NW4HBMAssertPointerValid_Line(glyph->sheetImage, 276);
                NW4HBMAssertHeaderMinimumValue_Line(glyph->cellWidth, 1, 279);
                NW4HBMAssertHeaderMinimumValue_Line(glyph->cellHeight, 1, 280);
                NW4HBMAssertHeaderClampedLRValue_Line(glyph->sheetSize, 0x200, 0x400000, 281);
                NW4HBMAssertHeaderMinimumValue_Line(glyph->sheetNum, 1, 282);
                NW4HBMAssertHeaderMinimumValue_Line(glyph->sheetRow, 1, 283);
                NW4HBMAssertHeaderMinimumValue_Line(glyph->sheetLine, 1, 284);
                NW4HBMAssertHeaderClampedLRValue_Line(glyph->sheetWidth, 0x20, 0x400, 285);
                NW4HBMAssertHeaderClampedLRValue_Line(glyph->sheetHeight, 0x20, 0x400, 286);
            } break;

            case MAGIC_FONT_CHAR_WIDTH: {
                FontWidth* width = CONVERT_OFFSET_TO_PTR(FontWidth, blockHeader, sizeof *blockHeader);

                NW4HBMAssert_Line(width->indexBegin <= width->indexEnd, 298);

                if (width->pNext) {
                    ResolveOffset(width->pNext, fileHeader);
                    NW4HBMAssertPointerValid_Line(width->pNext, 303);
                }
            } break;

            case MAGIC_FONT_CODE_MAP: {
                FontCodeMap* map = CONVERT_OFFSET_TO_PTR(FontCodeMap, blockHeader, sizeof *blockHeader);

                NW4HBMAssert_Line(map->ccodeBegin <= map->ccodeEnd, 316);
                NW4HBMAssert_Line((map->mappingMethod == FONT_MAPMETHOD_DIRECT) ||
                                      (map->mappingMethod == FONT_MAPMETHOD_TABLE) ||
                                      (map->mappingMethod == FONT_MAPMETHOD_SCAN),
                                  319);

                if (map->pNext) {
                    ResolveOffset(map->pNext, fileHeader);
                    NW4HBMAssertPointerValid_Line(map->pNext, 324);
                }
            } break;

            case MAGIC_FONT_GLGR:
                break;

            default:
                nw4hbm::db::Panic(__FILE__, 345, "The font has unknown block('%c%c%c%c').", blockHeader->kind >> 24,
                                  (blockHeader->kind >> 16) & 0xFF, (blockHeader->kind >> 8) & 0xFF,
                                  blockHeader->kind & 0xFF);
                return nullptr;
        }

        blockHeader = CONVERT_OFFSET_TO_PTR(BinaryBlockHeader, blockHeader, blockHeader->size);
        nBlocks++;
    }

    fileHeader->signature = MAGIC_FONT_UNPACKED;

    return info;
}

} // namespace ut
} // namespace nw4hbm
