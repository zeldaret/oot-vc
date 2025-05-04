#ifndef RVL_SDK_HBM_NW4HBM_UT_TAG_PROCESSOR_BASE_HPP
#define RVL_SDK_HBM_NW4HBM_UT_TAG_PROCESSOR_BASE_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace ut {
// forward declarations
struct Rect;
template <typename> class TextWriterBase;

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x49ba0b
enum Operation {
    OPERATION_DEFAULT,
    OPERATION_NO_CHAR_SPACE,
    OPERATION_CHAR_SPACE,
    OPERATION_NEXT_LINE,
    OPERATION_END_DRAW,

    NUM_OF_OPERATION
};

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4a555b, 0x49b98c...
template <typename charT> struct PrintContext {
    TextWriterBase<charT>* writer; // offset 0x00, size 0x04
    const charT* str; // offset 0x04, size 0x04
    f32 xOrigin; // offset 0x08, size 0x04
    f32 yOrigin; // offset 0x0c, size 0x04
    u32 flags; // offset 0x10, size 0x04
}; // size 0x14

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4a547f, 0x49b13e...
template <typename charT> class TagProcessorBase {
    // methods
  public:
    // cdtors
    TagProcessorBase();
    virtual ~TagProcessorBase();

    // virtual function ordering
    // vtable TagProcessorBase
    virtual Operation Process(char16_t code, PrintContext<charT>* context);
    virtual Operation CalcRect(Rect* pRect, char16_t code, PrintContext<charT>* context);

    // methods
    void ProcessLinefeed(PrintContext<charT>* context);
    void ProcessTab(PrintContext<charT>* context);

    // members
  private:
    /* vtable */ // offset 0x00, size 0x04
}; // size 0x04
} // namespace ut
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_UT_TAG_PROCESSOR_BASE_HPP
