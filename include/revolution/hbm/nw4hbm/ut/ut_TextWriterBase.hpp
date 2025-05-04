#ifndef RVL_SDK_HBM_NW4HBM_UT_TEXT_WRITER_BASE_HPP
#define RVL_SDK_HBM_NW4HBM_UT_TEXT_WRITER_BASE_HPP

/*******************************************************************************
 * headers
 */

#include "cstdarg.hpp" // std::va_list
#include "cstdio.hpp" // std::vsnprintf
#include "cstring.hpp" // std::strlen
#include "cwchar.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_CharWriter.hpp"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace ut {
// forward declarations
struct Rect;
template <typename> class TagProcessorBase;

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x49b65b, 0x4a55da...
template <typename charT> class TextWriterBase : public CharWriter {
    // methods
  public:
    // cdtors
    TextWriterBase();
    ~TextWriterBase();

    // explicit instantiation function ordering
    void SetLineHeight(f32 lineHeight);
    f32 GetLineHeight() const;
    void SetLineSpace(f32 lineSpace);
    void SetCharSpace(f32 charSpace);
    f32 GetLineSpace() const;
    f32 GetCharSpace() const;
    void SetTabWidth(int tabWidth);
    int GetTabWidth() const;
    void SetDrawFlag(u32 flags);
    u32 GetDrawFlag() const;
    void SetTagProcessor(TagProcessorBase<charT>* tagProcessor);
    void ResetTagProcessor();
    TagProcessorBase<charT>& GetTagProcessor() const;
    f32 CalcFormatStringWidth(const charT* format, ...) const;
    f32 CalcFormatStringHeight(const charT* format, ...) const;
    void CalcFormatStringRect(Rect* pRect, const charT* format, ...) const;
    void CalcVStringRect(Rect* pRect, const charT* format, std::va_list args) const;
    f32 CalcStringWidth(const charT* str, int length) const;
    f32 CalcStringWidth(const charT* str) const;
    f32 CalcStringHeight(const charT* str, int length) const;
    f32 CalcStringHeight(const charT* str) const;
    void CalcStringRect(Rect* pRect, const charT* str, int length) const;
    void CalcStringRect(Rect* pRect, const charT* str) const;
    f32 Printf(const charT* format, ...);
    f32 VPrintf(const charT* format, std::va_list args);
    f32 Print(const charT* str, int length);
    f32 Print(const charT* str);

    static charT* SetBuffer(charT* buf, u32 size);
    static charT* SetBuffer(u32 size);
    static u32 GetBufferSize();
    static charT* GetBuffer();

    // I fucking hate that this works
    static int VSNPrintf(charT* buffer, u32 count, const charT* format, std::va_list arg) {
        return sizeof(charT) == sizeof(char) ? std::vsnprintf((char*)buffer, count, (const char*)format, arg)
                                             : std::vswprintf((wchar_t*)buffer, count, (const wchar_t*)format, arg);
    }
    static int StrLen(const charT* str) {
        return sizeof(charT) == sizeof(char) ? std::strlen((const char*)str) : std::wcslen((const wchar_t*)str);
    }

    f32 CalcLineWidth(const charT* str, int length);
    int CalcLineRectImpl(Rect* pRect, const charT* str, int length);
    void CalcStringRectImpl(Rect* pRect, const charT* str, int length);
    f32 PrintImpl(const charT* str, int length);
    f32 AdjustCursor(f32* xOrigin, f32* yOrigin, const charT* str, int length);
    bool IsDrawFlagSet(u32 mask, u32 flag) const { return (mDrawFlag & mask) == flag; }

    // members
  private:
    /* base CharWriter */ // offset 0x00, size 0x4c
    f32 mCharSpace; // offset 0x4c, size 0x04
    f32 mLineSpace; // offset 0x50, size 0x04
    int mTabWidth; // offset 0x54, size 0x04
    u32 mDrawFlag; // offset 0x58, size 0x04
    TagProcessorBase<charT>* mTagProcessor; // offset 0x5c, size 0x04

    // static members
  private:
    static charT* mFormatBuffer;
    static u32 mFormatBufferSize;
    static TagProcessorBase<charT> mDefaultTagProcessor;
}; // size 0x60
} // namespace ut
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_UT_TEXT_WRITER_BASE_HPP
