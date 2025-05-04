#include "revolution/hbm/nw4hbm/ut/ut_ResFont.hpp"

/*******************************************************************************
 * headers
 */

#include "macros.h" // NW4HBM_VERSION
#include "revolution.h"

/*******************************************************************************
 * macros
 */

#define MAGIC_FONT 'RFNT' // Revolution Font
#define MAGIC_FONT_UNPACKED 'RFNU' // Revolution Font, unpacked

#define MAGIC_FONT_INFO 'FINF' // FontInformation
#define MAGIC_FONT_TEX_GLYPH 'TGLP' // FontTextureGlyph
#define MAGIC_FONT_CHAR_WIDTH 'CWDH' // Font[Char]Width
#define MAGIC_FONT_CODE_MAP 'CMAP' // FontCodeMap
#define MAGIC_FONT_GLGR 'GLGR' // Glyph Group?

#define CONVERT_OFFSET_TO_PTR(type_, ptr_, offset_) reinterpret_cast<type_*>(reinterpret_cast<u32>(ptr_) + offset_)

/*******************************************************************************
 * local function declarations
 */

namespace nw4hbm {
namespace ut {
namespace {
template <typename T> void ResolveOffset(T*& ptr, void* base) {
    *reinterpret_cast<u32*>(&ptr) = reinterpret_cast<u32>(base) + reinterpret_cast<u32>(ptr);
}
} // namespace
} // namespace ut
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

//! TODO: remove once matched
extern "C" void fn_8010CB20(char*, int, ...);
extern "C" void fn_8010CBAC(char*, int, ...);

namespace nw4hbm {
namespace ut {

ResFont::ResFont() {}

ResFont::~ResFont() {}

bool ResFont::SetResource(void* brfnt) {
    NW4HBM_ASSERT_PTR(this, 97);
    NW4HBM_ASSERT_PTR(brfnt, 98);
    NW4HBM_ASSERT_ALIGN(brfnt, 99);

    FontInformation* pFontInfo = nullptr;
    BinaryFileHeader* fileHeader = static_cast<BinaryFileHeader*>(brfnt);

    if (!IsManaging(nullptr)) {
        fn_8010CBAC(__FILE__, 107, "Font resource already atached.");
        return false;
    }

    if (fileHeader->signature == MAGIC_FONT_UNPACKED) {
        BinaryBlockHeader* blockHeader;
        int nBlocks = 0;

        blockHeader = CONVERT_OFFSET_TO_PTR(BinaryBlockHeader, fileHeader, fileHeader->headerSize);

        while (nBlocks < fileHeader->dataBlocks) {
            NW4HBM_ASSERT_PTR(blockHeader, 124);
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
                fn_8010CBAC(__FILE__, 150, "Invalid font resource.");
                return false;
            }
        } else {
            if (!IsValidBinaryFile(fileHeader, MAGIC_FONT, NW4HBM_VERSION(1, 2), 2)) {
                fn_8010CBAC(__FILE__, 160, "Invalid font resource.");
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
    FontInformation* info = nullptr;
    int nBlocks = 0;

    NW4HBM_ASSERT_PTR(fileHeader, 218);
    NW4HBM_ASSERT_ALIGN(fileHeader, 219);

    while (nBlocks < fileHeader->dataBlocks) {
        NW4HBM_ASSERT_PTR(blockHeader, 230);
        blockHeader = CONVERT_OFFSET_TO_PTR(BinaryBlockHeader, fileHeader, fileHeader->headerSize);

        switch (blockHeader->kind) {
            case MAGIC_FONT_INFO: {
                info = CONVERT_OFFSET_TO_PTR(FontInformation, blockHeader, sizeof *blockHeader);

                NW4HBM_ASSERT(info == NULL, 237);
                NW4HBM_ASSERT(info->fontType == FONT_TYPE_NNGCTEXTURE, 243);
                NW4HBM_ASSERT(info->alterCharIndex != GLYPH_INDEX_NOT_FOUND, 244);

                // no check
                NW4HBM_ASSERT_PTR_NULL(info->pGlyph, 247);
                NW4HBM_ASSERT_PTR(info->pGlyph, 249);
                ResolveOffset(info->pGlyph, fileHeader);

                if (info->pWidth) {
                    NW4HBM_ASSERT_PTR(info->pWidth, 255);
                    ResolveOffset(info->pWidth, fileHeader);
                }

                if (info->pMap) {
                    NW4HBM_ASSERT_PTR(info->pMap, 255);
                    ResolveOffset(info->pMap, fileHeader);
                }
            } break;

            case MAGIC_FONT_TEX_GLYPH: {
                FontTextureGlyph* glyph = CONVERT_OFFSET_TO_PTR(FontTextureGlyph, blockHeader, sizeof *blockHeader);

                NW4HBM_ASSERT_PTR_NULL(glyph->sheetImage, 274);
                NW4HBM_ASSERT_PTR(glyph->sheetImage, 276);
                NW4HBM_PANIC(1 <= glyph->cellWidth, 279,
                             "glyph->cellWidth is out of bounds(%d)\n%d <= glyph->cellWidth not satisfied.", 0, 1);
                NW4HBM_PANIC(1 <= glyph->cellHeight, 280,
                             "glyph->cellHeight is out of bounds(%d)\n%d <= glyph->cellHeight not satisfied.", 0, 1);
                NW4HBM_PANIC(0x200 <= glyph->sheetSize <= 0x400000, 281,
                             "glyph->sheetSize is out of bounds(%d)\n%d <= glyph->sheetSize <= %d not satisfied.",
                             glyph->sheetSize, 0x200, 0x400000);
                NW4HBM_PANIC(1 <= glyph->sheetNum, 282,
                             "glyph->sheetNum is out of bounds(%d)\n%d <= glyph->sheetNum not satisfied.", 0, 1);
                NW4HBM_PANIC(1 <= glyph->sheetRow, 283,
                             "glyph->sheetRow is out of bounds(%d)\n%d <= glyph->sheetRow not satisfied.", 0, 1);
                NW4HBM_PANIC(1 <= glyph->sheetLine, 284,
                             "glyph->sheetLine is out of bounds(%d)\n%d <= glyph->sheetLine not satisfied.", 0, 1);
                NW4HBM_PANIC(0x20 <= glyph->sheetWidth <= 0x400, 285,
                             "glyph->sheetWidth is out of bounds(%d)\n%d <= glyph->sheetWidth <= %d not satisfied.",
                             glyph->sheetWidth, 0x20, 0x400);
                NW4HBM_PANIC(0x20 <= glyph->sheetHeight <= 0x400, 286,
                             "glyph->sheetHeight is out of bounds(%d)\n%d <= glyph->sheetHeight <= %d not satisfied.",
                             glyph->sheetHeight, 0x20, 0x400);
                // no check
                // ResolveOffset(glyph->sheetImage, fileHeader);
            } break;

            case MAGIC_FONT_CHAR_WIDTH: {
                FontWidth* width = CONVERT_OFFSET_TO_PTR(FontWidth, blockHeader, sizeof *blockHeader);

                NW4HBM_ASSERT(width->indexBegin <= width->indexEnd, 237);

                if (width->pNext) {
                    ResolveOffset(width->pNext, fileHeader);
                    NW4HBM_ASSERT_PTR(width->pNext, 303);
                }
            } break;

            case MAGIC_FONT_CODE_MAP: {
                FontCodeMap* map = CONVERT_OFFSET_TO_PTR(FontCodeMap, blockHeader, sizeof *blockHeader);

                NW4HBM_ASSERT2(map->ccodeBegin <= map->ccodeEnd, 316);
                NW4HBM_ASSERT((map->mappingMethod == FONT_MAPMETHOD_DIRECT) ||
                                  (map->mappingMethod == FONT_MAPMETHOD_TABLE) ||
                                  (map->mappingMethod == FONT_MAPMETHOD_SCAN),
                              319);

                if (map->pNext) {
                    ResolveOffset(map->pNext, fileHeader);
                    NW4HBM_ASSERT_PTR(map->pNext, 324);
                }
            } break;

            case MAGIC_FONT_GLGR:
                break;

            default:
                fn_8010CB20(__FILE__, 345, "The font has unknown block('%c%c%c%c').", blockHeader->kind >> 24,
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
