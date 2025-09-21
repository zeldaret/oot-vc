#ifndef RVL_SDK_HBM_NW4HBM_LYT_COMMON_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_COMMON_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/gx/GXTypes.h"
#include "revolution/hbm/nw4hbm/math/math_types.hpp" // math::VEC2
#include "revolution/hbm/nw4hbm/ut/Color.hpp"
#include "revolution/tpl/TPL.h"
#include "revolution/types.h"
#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * macros
 */

#define TEXTCOLOR_MAX 2
#define VERTEXCOLOR_MAX 4
#define ANIMTARGET_VERTEXCOLOR_MAX 16
#define TEXTUREFLIP_MAX 6
#define WINDOWFRAME_MAX 8

// TODO: change to constexpr variables, like snd

#define FILE_HEADER_SIGNATURE_RLAN 'RLAN' // 0x524c414e
#define FILE_HEADER_SIGNATURE_LAYOUT 'RLYT' // 0x524c5954

#define OBJECT_KIND_PANE 'pan1' // 0x70696331

#define OBJECT_KIND_PICTURE 'pic1' // 0x70696331
#define OBJECT_KIND_BOUNDING 'bnd1' // 0x626e6431
#define OBJECT_KIND_WINDOW 'wnd1' // 0x776e6431
#define OBJECT_KIND_TEXT_BOX 'txt1' // 0x74787431

// painting?
#define OBJECT_KIND_pai1 'pai1' // 0x70616931

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace lyt {
// forward declarations
class ResourceAccessor;

namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x489d4a
struct BinaryFileHeader {
    char signature[4]; // size 0x04, offset 0x00
    byte2_t byteOrder; // size 0x02, offset 0x04
    u16 version; // size 0x02, offset 0x06
    u32 fileSize; // size 0x04, offset 0x08
    u16 headerSize; // size 0x02, offset 0x0c
    u16 dataBlocks; // size 0x02, offset 0x0e
}; // size 0x10

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4760d9
struct DataBlockHeader {
    char kind[4]; // size 0x04, offset 0x00
    u32 size; // size 0x04, offset 0x04
}; // size 0x08

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48c867
struct TextureList {
    DataBlockHeader blockHeader; // size 0x08, offset 0x00
    u16 texNum; // size 0x02, offset 0x08
    byte1_t padding[2];
}; // size 0x0c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48c8c8
struct FontList {
    DataBlockHeader blockHeader; // size 0x08, offset 0x00
    u16 fontNum; // size 0x02, offset 0x08
    byte1_t padding[2];
}; // size 0x0c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48c927
struct MaterialList {
    DataBlockHeader blockHeader; // size 0x08, offset 0x00
    u16 materialNum; // size 0x02, offset 0x08
    byte1_t padding[2];
}; // size 0x0c
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48c7f4
struct ResBlockSet {
    const res::TextureList* pTextureList; // size 0x04, offset 0x00
    const res::FontList* pFontList; // size 0x04, offset 0x04
    const res::MaterialList* pMaterialList; // size 0x04, offset 0x08
    ResourceAccessor* pResAccessor; // size 0x04, offset 0x0c
}; // size 0x10

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x476709
struct Size {
    // methods
  public:
    // cdtors
    Size() : width(), height() {}
    Size(f32 aWidth, f32 aHeight) : width(aWidth), height(aHeight) {}
    Size(const Size& other) : width(other.width), height(other.height) {}

    // operators
    friend bool operator==(const Size& a, const Size& b) { return a.width == b.width && a.height == b.height; }

    // members
  public:
    f32 width; // size 0x04, offset 0x00
    f32 height; // size 0x04, offset 0x04
}; // size 0x08

namespace detail {
typedef math::VEC2 TexCoords[4];

class TexCoordAry {
    // methods
  public:
    // cdtors
    TexCoordAry();

    // methods
    u8 GetSize() const { return mNum; }
    const TexCoords* GetArray() const { return mpData; }

    void SetSize(u8 num);

    bool IsEmpty() const { return mCap == 0; }

    void Reserve(u8 num);
    void Free();
    void Copy(const void* pResTexCoord, u8 texCoordNum);

    // members
  private:
    u8 mCap; // size 0x01, offset 0x00
    u8 mNum; // size 0x01, offset 0x01
    /* 2 bytes padding */
    TexCoords* mpData; // size 0x04, offset 0x04
}; // size 0x08

bool EqualsPaneName(const char* name1, const char* name2);
bool EqualsMaterialName(const char* name1, const char* name2);
bool TestFileHeader(const res::BinaryFileHeader& fileHeader);
bool TestFileHeader(const res::BinaryFileHeader& fileHeader, byte4_t testSig);

// TexCoordAry happens here

bool IsModulateVertexColor(ut::Color* vtxColors, u8 glbAlpha);
ut::Color MultipleAlpha(const ut::Color col, u8 alpha);
void MultipleAlpha(ut::Color* dst, const ut::Color* src, u8 alpha);
void SetVertexFormat(bool bModulate, u8 texCoordNum);

void DrawQuad(const math::VEC2& basePt, const Size& size, u8 texCoordNum, const TexCoords* texCoords,
              const ut::Color* vtxColors);
void DrawQuad(const math::VEC2& basePt, const Size& size, u8 texCoordNum, const TexCoords* texCoords,
              const ut::Color* vtxColors, u8 alpha);
void DrawLine(const math::VEC2& pos, const Size& size, ut::Color color);
void InitGXTexObjFromTPL(GXTexObj* to, TPLPalette* pal, u32 id);

// Inlines

inline long GetSignatureInt(const char* sig) { return *reinterpret_cast<const long*>(sig); }

inline const char* GetStrTableStr(const void* pStrTable, int index) {
    const u32* offsets = static_cast<const u32*>(pStrTable);
    const char* stringPool = static_cast<const char*>(pStrTable);

    return &stringPool[offsets[index]];
}

// But why
inline u8 GetVtxColorElement(const ut::Color* cols, u32 idx) {
    NW4HBMAssert_FileLine(idx < ANIMTARGET_VERTEXCOLOR_MAX, "common.h", 199);
    return reinterpret_cast<const u8*>(cols + idx / 4)[idx % 4];
}

inline void SetVtxColorElement(ut::Color* cols, u32 idx, u8 value) {
    NW4HBMAssert_FileLine(idx < ANIMTARGET_VERTEXCOLOR_MAX, "common.h", 212);
    reinterpret_cast<u8*>(cols + idx / 4)[idx % 4] = value;
}

// positions of what?
inline u8 GetHorizontalPosition(u8 var) { return var % 3; }

inline u8 GetVerticalPosition(u8 var) { return var / 3; }

inline void SetHorizontalPosition(u8* pVar, u8 newVal) { *pVar = GetVerticalPosition(*pVar) * 3 + newVal; }

inline void SetVerticalPosition(u8* pVar, u8 newVal) { *pVar = newVal * 3 + GetHorizontalPosition(*pVar); }
} // namespace detail
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_COMMON_HPP
