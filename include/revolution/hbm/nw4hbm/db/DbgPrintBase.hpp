#ifndef NW4R_DB_DEBUG_PRINT_BASE_H
#define NW4R_DB_DEBUG_PRINT_BASE_H

/*******************************************************************************
 * headers
 */

#include "cstdarg.hpp" // std::va_list

#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/Color.hpp"
#include "revolution/hbm/nw4hbm/ut/Font.hpp"
#include "revolution/hbm/nw4hbm/ut/list.h"

#include "revolution/mem/mem_heapCommon.h" // MEMiHeapHead

/*******************************************************************************
 * class
 */

namespace nw4hbm {
namespace db {
// [SPQE7T]/ISpyD.elf:.debug_info::0x39c1f9
template <typename charT> class DbgPrintBase {
    // nested types
  public:
    // [SPQE7T]/ISpyD.elf:.debug_info::0x39c65a
    struct DbgText {
        int x; // size 0x04, offset 0x00
        int y; // size 0x04, offset 0x04
        int time; // size 0x04, offset 0x08
        int length; // size 0x04, offset 0x0c
        ut::Link link; // size 0x08, offset 0x10
        charT text[]; // flexible,  offset 0x18, (unit size 0x01/0x02)
    }; // size 0x18

    // methods
  public:
    // instantiation function ordering

    /* 478 */ void Reset(); // why does this mess up the ordering if i move it???

    /* 373 */ void VRegisterf(int x, int y, int time, charT const* format, std::va_list args);
    /* 410 */ void Register(int x, int y, int time, charT const* string, int length);
    /* 707 */ void Registf(int x, int y, charT const* format, ...);
    /* 719 */ void Registf(int x, int y, int time, charT const* format, ...);
    /* 731 */ void VRegistf(int x, int y, int time, charT const* format, std::va_list args);
    /* 738 */ void Regist(int x, int y, int time, charT const* string, int length);

    /* 510 */ void Flush();
    /* 567 */ void Flush(int x, int y, int w, int h);

    /* 051 */ DbgPrintBase(ut::Color textColor, ut::Font const* font);
    /* 074 */ ~DbgPrintBase();

    /* 448 */ void Unregister(DbgText* dbgText);

    /* 616 */ static void SetBuffer(void* buffer, u32 size);
    /* 634 */ static void* ReleaseBuffer();

    /* 670 */ void* operator new(u32 size);
    /* 688 */ void operator delete(void* ptr);

    /* 347 */ void Registerf(int x, int y, int time, charT const* format, ...);
    /* 321 */ void Registerf(int x, int y, charT const* format, ...);

    /* 293 */ bool IsVisible() const;
    /* 276 */ void SetVisible(bool bVisible);
    /* 225 */ f32 GetFontSize() const;
    /* 242 */ void SetFontSize(f32 size);
    /* 259 */ ut::Color GetTextColor() const;
    /* 208 */ void SetTextColor(ut::Color textColor);
    /* 191 */ ut::Font const* GetFont() const;
    /* 173 */ void SetFont(ut::Font const& font);

  private:
    /* 141 */ static void Initialize(void* buffer, u32 size, ut::Font const& font, ut::Color textColor);
    /* 115 */ static void Initialize(void* buffer, u32 size, ut::Color textColor);
    /* 097 */ static DbgPrintBase<charT>* GetInstance();

#if 1
    // inlines
    DbgText* GetFirstText() { return static_cast<DbgText*>(List_GetNext(&mTextList, nullptr)); }

    DbgText* GetNextText(DbgText* dbgText) { return static_cast<DbgText*>(List_GetNext(&mTextList, dbgText)); }
#else
#define GetFirstText() static_cast<DbgText*>(List_GetNext(&mTextList, nullptr))
#define GetNextText(dbgText) static_cast<DbgText*>(List_GetNext(&mTextList, dbgText))
#endif

    // static members
  private:
    static MEMiHeapHead* mHeapHandle;
    static DbgPrintBase<charT>* mInstance;

    // members
  private:
    ut::Font const* mFont; // size 0x04, offset 0x00
    ut::List mTextList; // size 0x0c, offset 0x04
    ut::Color mTextColor; // size 0x04, offset 0x10
    f32 mFontSize; // size 0x04, offset 0x14
    bool mVisible; // size 0x01, offset 0x18
    byte_t padding_[3];
}; // size 0x1c
} // namespace db
} // namespace nw4hbm

#endif // NW4R_DB_DEBUG_PRINT_BASE_H
