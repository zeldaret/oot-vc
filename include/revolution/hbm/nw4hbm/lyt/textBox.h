#ifndef NW4HBM_LYT_TEXTBOX_H
#define NW4HBM_LYT_TEXTBOX_H

#include "revolution/hbm/nw4hbm/lyt/common.h"
#include "revolution/hbm/nw4hbm/lyt/pane.h"

#include "revolution/hbm/nw4hbm/ut/TagProcessorBase.h"

namespace nw4hbm {
namespace lyt {
class TextBox : public Pane {
  public:
    NW4HBM_UT_RUNTIME_TYPEINFO;

    TextBox(u16 allocStrLen, const wchar_t* str, const ut::Font* pFont);
    TextBox(const res::TextBox* pBlock, const ResBlockSet& resBlockSet);

    virtual ~TextBox();

    virtual void DrawSelf(const DrawInfo& drawInfo);

    virtual ut::Color GetVtxColor(u32 idx) const;
    virtual void SetVtxColor(u32 idx, ut::Color value);

    virtual u8 GetVtxColorElement(u32 idx) const;
    virtual void SetVtxColorElement(u32 idx, u8 value);

    virtual void AllocStringBuffer(u16 size); // 0x64
    virtual void FreeStringBuffer(); // 0x68

    virtual u16 SetString(const wchar_t* str, u16 dstIdx = 0); // 0x6C
    virtual u16 SetString(const wchar_t* str, u16 dstIdx, u16 strLen); // 0x70

    const Size& GetFontSize() const { return mFontSize; }

    void SetFontSize(const Size& fontSize) { mFontSize = fontSize; }
    void SetTagProcessor(ut::WideTagProcessor* pTagProcessor) { mpTagProcessor = pTagProcessor; }

    u16 GetStringBufferLength() const;

    f32 GetTextMagH() const;
    f32 GetTextMagV() const;

    u8 GetTextPositionH() const { return detail::GetHorizontalPosition(mTextPosition); }
    u8 GetTextPositionV() const { return detail::GetVerticalPosition(mTextPosition); }

    const ut::Color GetTextColor(u32 type) const {
        NW4HBMAssert_Line(type < TEXTCOLOR_MAX, 95);
        return mTextColors[type];
    }
    void SetTextColor(u32 type, ut::Color value) {
        NW4HBMAssert_Line(type < TEXTCOLOR_MAX, 96);
        mTextColors[type] = value;
    }

    void SetTextPositionH(u8 pos) { detail::SetHorizontalPosition(&mTextPosition, pos); }
    void SetTextPositionV(u8 pos) { detail::SetVerticalPosition(&mTextPosition, pos); }

    const ut::Font* GetFont() const;
    void SetFont(const ut::Font* pFont);

    void Init(u16 allocStrLen);

    const ut::Rect GetTextDrawRect(const DrawInfo& drawInfo) const;
    const ut::Rect GetTextDrawRect(ut::WideTextWriter* pWriter) const;

  private:
    wchar_t* mTextBuf; // 0xD4
    ut::Color mTextColors[TEXTCOLOR_MAX]; // 0xD8

    const ut::Font* mpFont; // 0xE0
    Size mFontSize; // 0xE4

    f32 mLineSpace; // 0xEC
    f32 mCharSpace; // 0xF0

    ut::WideTagProcessor* mpTagProcessor; // 0xF4

    u16 mTextBufBytes; // 0xF8
    u16 mTextLen; // 0xFA
    u8 mTextPosition; // 0xFC

    struct {
        u8 allocFont : 1;
    } mTextBoxFlag; // 0xFD
};
} // namespace lyt
} // namespace nw4hbm

#endif // NW4HBM_LYT_TEXTBOX_H
