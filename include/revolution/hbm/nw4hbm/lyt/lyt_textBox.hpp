#ifndef RVL_SDK_HBM_NW4HBM_LYT_TEXT_BOX_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_TEXT_BOX_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/pane.h"
#include "revolution/hbm/nw4hbm/ut/ut_Color.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Rect.hpp"
#include "revolution/types.h"

#define TEXTCOLOR_MAX 2
#define VERTEXCOLOR_MAX 4

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace ut {
class Font;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace ut {
template <typename> class TagProcessorBase;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace ut {
template <typename> class TextWriterBase;
}
} // namespace nw4hbm
namespace nw4hbm {
namespace ut {
namespace detail {
class RuntimeTypeInfo;
}
} // namespace ut
} // namespace nw4hbm

namespace nw4hbm {
namespace lyt {
namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x49b16e
struct Font {
    u32 nameStrOffset; // size 0x04, offset 0x00
    u8 type; // size 0x01, offset 0x04
    byte1_t padding[3];
}; // size 0x08

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48cbdc
struct TextBox : public Pane {
    /* base Pane */ // size 0x4c, offset 0x00
    u16 textBufBytes; // size 0x02, offset 0x4c
    u16 textStrBytes; // size 0x02, offset 0x4e
    u16 materialIdx; // size 0x02, offset 0x50
    u16 fontIdx; // size 0x02, offset 0x52
    u8 textPosition; // size 0x01, offset 0x54
    byte1_t padding[3];
    u32 textStrOffset; // size 0x04, offset 0x58
    u32 textCols[2]; // size 0x08, offset 0x5c
    Size fontSize; // size 0x08, offset 0x64
    f32 charSpace; // size 0x04, offset 0x6c
    f32 lineSpace; // size 0x04, offset 0x70
}; // size 0x74
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x480eb2
class TextBox : public Pane {
    // methods
  public:
    // cdtors
    TextBox(const res::TextBox* pBlock, const ResBlockSet& resBlockSet);
    /* virtual ~TextBox(); */

    // virtual function ordering
    // vtable Pane
    virtual const ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; }
    virtual ~TextBox();
    virtual void DrawSelf(const DrawInfo& drawInfo);
    virtual const ut::Color GetVtxColor(u32 idx) const;
    virtual void SetVtxColor(u32 idx, ut::Color value);
    virtual u8 GetVtxColorElement(u32 idx) const;
    virtual void SetVtxColorElement(u32 idx, u8 value);

    // vtable TextBox
    virtual void AllocStringBuffer(u16 minLen);
    virtual void FreeStringBuffer();
    virtual u16 SetString(const wchar_t* str, u16 dstIdx);
    virtual u16 SetString(const wchar_t* str, u16 dstIdx, u16 strLen);

    // methods
    const ut::Color GetTextColor(u32 type) const {
        NW4HBM_ASSERT3(type < TEXTCOLOR_MAX, "textBox.h", 95);
        return mTextColors[type];
    }
    const Size GetFontSize() const { return mFontSize; }

    void SetFontSize(const Size& fontSize) { mFontSize = fontSize; }
    void SetTagProcessor(ut::TagProcessorBase<wchar_t>* pTagProcessor) { mpTagProcessor = pTagProcessor; }

    u16 GetStringBufferLength() const;
    ut::Rect GetTextDrawRect(ut::TextWriterBase<wchar_t>* pWriter) const;
    f32 GetTextMagH() const;
    f32 GetTextMagV() const;
    u8 GetTextPositionH() const { return detail::GetHorizontalPosition(mTextPosition); }
    u8 GetTextPositionV() const { return detail::GetVerticalPosition(mTextPosition); }

    void SetTextColor(u32 type, ut::Color value);
    void SetTextPositionH(u8 pos) { detail::SetHorizontalPosition(&mTextPosition, pos); }
    void SetTextPositionV(u8 pos) { detail::SetVerticalPosition(&mTextPosition, pos); }
    void SetFont(const ut::Font* pFont);

    void Init(u16 allocStrLen);

    // members
  private:
    /* base Pane */ // size 0xd4, offset 0x00
    wchar_t* mTextBuf; // size 0x04, offset 0xd4
    ut::Color mTextColors[2]; // size 0x08, offset 0xd8
    const ut::Font* mpFont; // size 0x04, offset 0xe0
    Size mFontSize; // size 0x08, offset 0xe4
    f32 mLineSpace; // size 0x04, offset 0xec
    f32 mCharSpace; // size 0x04, offset 0xf0
    ut::TagProcessorBase<wchar_t>* mpTagProcessor; // size 0x04, offset 0xf4
    u16 mTextBufBytes; // size 0x02, offset 0xf8
    u16 mTextLen; // size 0x02, offset 0xfa
    u8 mTextPosition; // size 0x01, offset 0xfc

    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x481010
    struct {
        u8 allocFont : 1; // offset be0 / le7
        /* 7 bits padding */
    } mTextBoxFlag; // size 0x01, offset 0xfd
    /* 2 bytes padding */

    // static members
  public:
    static const ut::detail::RuntimeTypeInfo typeInfo;
}; // size 0x100
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_TEXT_BOX_HPP
