// clang-format off

/*******************************************************************************
 * anti-header guard
 */

#ifdef __MWERKS__
# pragma notonce
#endif

/*******************************************************************************
 * NW4HBMAssert/NW4HBMCheck base macros
 */

// <assert.h> behavior
#undef NW4HBMAssertMessage_FileLine
#undef NW4HBMCheckMessage_FileLine

#if defined(HBM_ASSERT)
# define NW4HBMAssertMessage_FileLine(file_, line_, expr_, ...)    ((void)((expr_) || (::nw4hbm::db::Panic(file_, line_, __VA_ARGS__), 0)))
# define NW4HBMAssertMessage2_FileLine(file_, line_, expr_, ...) { \
    if ((expr_)) { \
        ::nw4hbm::db::Panic(file_, line_, __VA_ARGS__); \
    } \
}
# define NW4HBMCheckMessage_FileLine(file_, line_, expr_, ...)    ((void)((expr_) || (::nw4hbm::db::Warning(file_, line_, __VA_ARGS__), 0)))
#else
# define NW4HBMAssertMessage_FileLine(file_, line_, expr_, ...)    ((void)(0))
# define NW4HBMCheckMessage_FileLine(file_, line_, expr_, ...)    ((void)(0))
#endif // defined(HBM_ASSERT)

/*******************************************************************************
 * Derived macros
 */

/* Main asserts */

// NW4HBMAssert family
#define NW4HBMAssertMessage_File(expr_, file_, ...)   NW4HBMAssertMessage_FileLine(  file_ , __LINE__, expr_, __VA_ARGS__)
#define NW4HBMAssertMessage_Line(expr_, line_, ...)   NW4HBMAssertMessage_FileLine(__FILE__,   line_ , expr_, __VA_ARGS__)
#define NW4HBMAssertMessage(expr_, ...)               NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, expr_, __VA_ARGS__)

#define NW4HBMAssert_FileLine(expr_, file_, line_)    NW4HBMAssertMessage_FileLine(  file_ ,   line_ , expr_, "NW4HBM:Failed assertion " #expr_)
#define NW4HBMAssert_File(expr_, file_)               NW4HBMAssertMessage_FileLine(  file_ , __LINE__, expr_, "NW4HBM:Failed assertion " #expr_)
#define NW4HBMAssert_Line(expr_, line_)               NW4HBMAssertMessage_FileLine(__FILE__,   line_ , expr_, "NW4HBM:Failed assertion " #expr_)
#define NW4HBMAssert2_Line(expr_, line_)              NW4HBMAssertMessage_FileLine(__FILE__,   line_ , !expr_, "NW4HBM:Failed assertion " #expr_)
#define NW4HBMAssert(expr_)                           NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, expr_, "NW4HBM:Failed assertion " #expr_)

#define NW4HBMAssertHeader_FileLine(file_, line_, expr_)  NW4HBMAssertMessage_FileLine(  file_ ,   line_ , expr_, "Failed assertion " #expr_)
#define NW4HBMAssertHeader_File(file_, expr_)             NW4HBMAssertMessage_FileLine(  file_ , __LINE__, expr_, "Failed assertion " #expr_)
#define NW4HBMAssertHeader_Line(line_, expr_)             NW4HBMAssertMessage_FileLine(__FILE__,   line_ , expr_, "Failed assertion " #expr_)
#define NW4HBMAssertHeader(expr_)                         NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, expr_, "Failed assertion " #expr_)

// NW4HBMPanic family
#define NW4HBMPanicMessage_FileLine(file_, line_, ...)    NW4HBMAssertMessage_FileLine(  file_ ,   line_ , false, __VA_ARGS__)
#define NW4HBMPanicMessage_File(file_, ...)               NW4HBMAssertMessage_FileLine(  file_ , __LINE__, false, __VA_ARGS__)
#define NW4HBMPanicMessage_Line(line_, ...)               NW4HBMAssertMessage_FileLine(__FILE__,   line_ , false, __VA_ARGS__)
#define NW4HBMPanicMessage(...)                           NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, false, __VA_ARGS__)

#define NW4HBMPanic_FileLine(file_, line_)                NW4HBMAssert_FileLine(  file_ ,   line_ , false)
#define NW4HBMPanic_File(file_)                           NW4HBMAssert_FileLine(  file_ , __LINE__, false)
#define NW4HBMPanic_Line(line_)                           NW4HBMAssert_FileLine(__FILE__,   line_ , false)
#define NW4HBMPanic()                                     NW4HBMAssert_FileLine(__FILE__, __LINE__, false)

// NW4HBMCheck family
#define NW4HBMCheckMessage_File(file_, expr_, ...)        NW4HBMCheckMessage_FileLine(  file_ , __LINE__, expr_, __VA_ARGS__)
#define NW4HBMCheckMessage_Line(line_, expr_, ...)        NW4HBMCheckMessage_FileLine(__FILE__,   line_ , expr_, __VA_ARGS__)
#define NW4HBMCheckMessage(expr_, ...)                    NW4HBMCheckMessage_FileLine(__FILE__, __LINE__, expr_, __VA_ARGS__)

#define NW4HBMCheck_FileLine(file_, line_, expr_)         NW4HBMCheckMessage_FileLine(  file_ ,   line_ , expr_, "NW4HBM:Failed check " #expr_)
#define NW4HBMCheck_File(file_, expr_)                    NW4HBMCheckMessage_FileLine(  file_ , __LINE__, expr_, "NW4HBM:Failed check " #expr_)
#define NW4HBMCheck_Line(line_, expr_)                    NW4HBMCheckMessage_FileLine(__FILE__,   line_ , expr_, "NW4HBM:Failed check " #expr_)
#define NW4HBMCheck(expr_)                                NW4HBMCheckMessage_FileLine(__FILE__, __LINE__, expr_, "NW4HBM:Failed check " #expr_)

#define NW4HBMCheckHeader_FileLine(file_, line_, expr_)   NW4HBMCheckMessage_FileLine(  file_ ,   line_ , expr_, "Failed check " #expr_)
#define NW4HBMCheckHeader_File(file_, expr_)              NW4HBMCheckMessage_FileLine(  file_ , __LINE__, expr_, "Failed check " #expr_)
#define NW4HBMCheckHeader_Line(line_, expr_)              NW4HBMCheckMessage_FileLine(__FILE__,   line_ , expr_, "Failed check " #expr_)
#define NW4HBMCheckHeader(expr_)                          NW4HBMCheckMessage_FileLine(__FILE__, __LINE__, expr_, "Failed check " #expr_)

// NW4HBMWarning family
#define NW4HBMWarningMessage_FileLine(file_, line_, ...)  NW4HBMCheckMessage_FileLine(  file_ ,   line_ , false, __VA_ARGS__)
#define NW4HBMWarningMessage_File(file_, ...)             NW4HBMCheckMessage_FileLine(  file_ , __LINE__, false, __VA_ARGS__)
#define NW4HBMWarningMessage_Line(line_, ...)             NW4HBMCheckMessage_FileLine(__FILE__,   line_ , false, __VA_ARGS__)
#define NW4HBMWarningMessage(...)                         NW4HBMCheckMessage_FileLine(__FILE__, __LINE__, false, __VA_ARGS__)

#define NW4HBMAssertWarningMessage_FileLine(expr_, file_, line_, ...)  NW4HBMCheckMessage_FileLine(  file_ ,   line_ , expr_, __VA_ARGS__)
#define NW4HBMAssertWarningMessage_File(expr_, file_,  ...)             NW4HBMCheckMessage_FileLine(  file_ , __LINE__, expr_, __VA_ARGS__)
#define NW4HBMAssertWarningMessage_Line(expr_, line_,  ...)             NW4HBMCheckMessage_FileLine(__FILE__,   line_ , expr_, __VA_ARGS__)
#define NW4HBMAssertWarningMessage(expr_, ...)                          NW4HBMCheckMessage_FileLine(__FILE__, __LINE__, expr_, __VA_ARGS__)

#define NW4HBMWarning_FileLine(file_, line_)              NW4HBMCheck_FileLine(  file_ ,   line_ , false)
#define NW4HBMWarning_File(file_)                         NW4HBMCheck_FileLine(  file_ , __LINE__, false)
#define NW4HBMWarning_Line(line_)                         NW4HBMCheck_FileLine(__FILE__,   line_ , false)
#define NW4HBMWarning()                                   NW4HBMCheck_FileLine(__FILE__, __LINE__, false)

// NW4HBMAlign family
#define NW4HBMAlign2_FileLine(expr_, file_, line_)   NW4HBMAssertMessage_FileLine(  file_ ,   line_ , !((u32)expr_ & 0x01), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 2 bytes boundary.", expr_)
#define NW4HBMAlign2_File(expr_, file_)              NW4HBMAssertMessage_FileLine(  file_ , __LINE__, !((u32)expr_ & 0x01), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 2 bytes boundary.", expr_)
#define NW4HBMAlign2_Line(expr_, line_)              NW4HBMAssertMessage_FileLine(__FILE__,   line_ , !((u32)expr_ & 0x01), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 2 bytes boundary.", expr_)
#define NW4HBMAlign2(expr_)                          NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, !((u32)expr_ & 0x01), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 2 bytes boundary.", expr_)

// NW4HBMAlign32 family
#define NW4HBMAlign32_FileLine(file_, line_, expr_)   NW4HBMAssertMessage_FileLine(  file_ ,   line_ , !((u32)expr_ & 0x1F), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)
#define NW4HBMAlign32_File(expr_, file_)              NW4HBMAssertMessage_FileLine(  file_ , __LINE__, !((u32)expr_ & 0x1F), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)
#define NW4HBMAlign32_Line(expr_, line_)              NW4HBMAssertMessage_FileLine(__FILE__,   line_ , !((u32)expr_ & 0x1F), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)
#define NW4HBMAlign32(expr_)                          NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, !((u32)expr_ & 0x1F), "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)
#define NW4HBMAlign32_2_FileLine(file_, line_, expr_)   NW4HBMAssertMessage_FileLine(  file_ ,   line_ , ((u32)expr_ & 0x1F) ? 0 : 1, "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)
#define NW4HBMAlign32_2_File(expr_, file_)              NW4HBMAssertMessage_FileLine(  file_ , __LINE__, ((u32)expr_ & 0x1F) ? 0 : 1, "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)
#define NW4HBMAlign32_2_Line(expr_, line_)              NW4HBMAssertMessage_FileLine(__FILE__,   line_ , ((u32)expr_ & 0x1F) ? 0 : 1, "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)
#define NW4HBMAlign32_2(expr_)                          NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, ((u32)expr_ & 0x1F) ? 0 : 1, "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", expr_)

// Generic align
// Aligned
#define NW4HBM_IS_ALIGNED_(x, align)	(((unsigned long)(x) & ((align) - 1)) == 0) // just redefine instead of pulling in <macros.h>

#define NW4HBMAssertAligned_FileLine(file_, line_, val_, align_)								NW4HBMAssertMessage_FileLine(  file_ ,   line_ , NW4HBM_IS_ALIGNED_(val_, align_), "NW4HBM:Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)
#define NW4HBMAssertAligned_File(file_, val_, align_)											NW4HBMAssertMessage_FileLine(  file_ , __LINE__, NW4HBM_IS_ALIGNED_(val_, align_), "NW4HBM:Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)
#define NW4HBMAssertAligned_Line(line_, val_, align_)											NW4HBMAssertMessage_FileLine(__FILE__,   line_ , NW4HBM_IS_ALIGNED_(val_, align_), "NW4HBM:Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)
#define NW4HBMAssertAligned(val_, align_)														NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, NW4HBM_IS_ALIGNED_(val_, align_), "NW4HBM:Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)

#define NW4HBMAssertHeaderAligned_FileLine(file_, line_, val_, align_)						NW4HBMAssertMessage_FileLine(  file_ ,   line_ , NW4HBM_IS_ALIGNED_(val_, align_),      "Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)
#define NW4HBMAssertHeaderAligned_File(file_, val_, align_)									NW4HBMAssertMessage_FileLine(  file_ , __LINE__, NW4HBM_IS_ALIGNED_(val_, align_),      "Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)
#define NW4HBMAssertHeaderAligned_Line(line_, val_, align_)									NW4HBMAssertMessage_FileLine(__FILE__,   line_ , NW4HBM_IS_ALIGNED_(val_, align_),      "Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)
#define NW4HBMAssertHeaderAligned(val_, align_)												NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, NW4HBM_IS_ALIGNED_(val_, align_),      "Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)


/* Extended asserts */

// PointerNonnull
#define NW4HBMAssertPointerNonnull_FileLineMsg(ptr_, file_, line_, msg) NW4HBMAssertMessage_FileLine(  file_ ,   line_ , (ptr_) != 0, msg)
#define NW4HBMAssertPointerNonnull_FileLine(ptr_, file_, line_)         NW4HBMAssertMessage_FileLine(  file_ ,   line_ , (ptr_) != 0, "NW4HBM:Pointer must not be NULL (" #ptr_ ")")
#define NW4HBMAssertPointerNonnull_File(ptr_, file_)                    NW4HBMAssertMessage_FileLine(  file_ , __LINE__, (ptr_) != 0, "NW4HBM:Pointer must not be NULL (" #ptr_ ")")
#define NW4HBMAssertPointerNonnull_Line(ptr_, line_)                    NW4HBMAssertMessage_FileLine(__FILE__,   line_ , (ptr_) != 0, "NW4HBM:Pointer must not be NULL (" #ptr_ ")")
#define NW4HBMAssertPointerNonnull(ptr_)                                NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (ptr_) != 0, "NW4HBM:Pointer must not be NULL (" #ptr_ ")")

#define NW4HBMAssertHeaderPointerNonnull_FileLine(ptr_, file_, line_) NW4HBMAssertMessage_FileLine(  file_ ,   line_ , (ptr_) != 0, "Pointer must not be NULL (" #ptr_ ")")
#define NW4HBMAssertHeaderPointerNonnull_File(ptr_, file_)            NW4HBMAssertMessage_FileLine(  file_ , __LINE__, (ptr_) != 0, "Pointer must not be NULL (" #ptr_ ")")
#define NW4HBMAssertHeaderPointerNonnull_Line(ptr_, line_)            NW4HBMAssertMessage_FileLine(__FILE__,   line_ , (ptr_) != 0, "Pointer must not be NULL (" #ptr_ ")")
#define NW4HBMAssertHeaderPointerNonnull(ptr_)                        NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (ptr_) != 0, "Pointer must not be NULL (" #ptr_ ")")

#define POINTER_VALID_TEST(ptr_)                            \
       (((unsigned long)ptr_ & 0xFF000000) == 0x80000000    \
     || ((unsigned long)ptr_ & 0xFF800000) == 0x81000000    \
     || ((unsigned long)ptr_ & 0xF8000000) == 0x90000000    \
     || ((unsigned long)ptr_ & 0xFF000000) == 0xC0000000    \
     || ((unsigned long)ptr_ & 0xFF800000) == 0xC1000000    \
     || ((unsigned long)ptr_ & 0xF8000000) == 0xD0000000    \
     || ((unsigned long)ptr_ & 0xFFFFC000) == 0xE0000000)

// PointerValid
#define NW4HBMAssertPointerValid_FileLine(ptr_, file_, line_) NW4HBMAssertMessage_FileLine(  file_ ,   line_ , POINTER_VALID_TEST(ptr_), "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4HBMAssertPointerValid_File(ptr_, file_)            NW4HBMAssertMessage_FileLine(  file_ , __LINE__, POINTER_VALID_TEST(ptr_), "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4HBMAssertPointerValid_Line(ptr_, line_)            NW4HBMAssertMessage_FileLine(__FILE__,   line_ , POINTER_VALID_TEST(ptr_), "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4HBMAssertPointerValid(ptr_)                        NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, POINTER_VALID_TEST(ptr_), "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)

#define NW4HBMAssertHeaderPointerValid_FileLine(file_, line_, ptr_)   NW4HBMAssertMessage_FileLine(  file_ ,   line_ , POINTER_VALID_TEST(ptr_), "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4HBMAssertHeaderPointerValid_File(file_, ptr_)              NW4HBMAssertMessage_FileLine(  file_ , __LINE__, POINTER_VALID_TEST(ptr_), "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4HBMAssertHeaderPointerValid_Line(line_, ptr_)              NW4HBMAssertMessage_FileLine(__FILE__,   line_ , POINTER_VALID_TEST(ptr_), "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4HBMAssertHeaderPointerValid(ptr_)                          NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, POINTER_VALID_TEST(ptr_), "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)

// MinimumValue
#define NW4HBMAssertMinimumValue_FileLine(var_, minValue_, file_, line_)  NW4HBMAssertMessage_FileLine(  file_ ,   line_ , minValue_ <= var_, "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)
#define NW4HBMAssertMinimumValue_File(var_, minValue_, file_)             NW4HBMAssertMessage_FileLine(  file_ , __LINE__, minValue_ <= var_, "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)
#define NW4HBMAssertMinimumValue_Line(var_, minValue_, line_)             NW4HBMAssertMessage_FileLine(__FILE__,   line_ , minValue_ <= var_, "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)
#define NW4HBMAssertMinimumValue(var_, minValue_)                         NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, minValue_ <= var_, "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)

#define NW4HBMAssertHeaderMinimumValue_FileLine(file_, line_, var_, minValue_)    NW4HBMAssertMessage_FileLine(  file_ ,   line_ , minValue_ <= var_, #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))
#define NW4HBMAssertHeaderMinimumValue_File(var_, minValue_, file_)               NW4HBMAssertMessage_FileLine(  file_ , __LINE__, minValue_ <= var_, #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))
#define NW4HBMAssertHeaderMinimumValue_Line(var_, minValue_, line_)               NW4HBMAssertMessage_FileLine(__FILE__,   line_ , minValue_ <= var_, #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))
#define NW4HBMAssertHeaderMinimumValue(var_, minValue_)                           NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, minValue_ <= var_, #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))

// Range
#define NW4HBMAssertHeaderRangeValue_FileLine(file_, line_, var_, minValue_, maxValue_)    NW4HBMAssertMessage_FileLine(  file_ ,   line_ , var_ >= minValue_ && var_ <= maxValue_, #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderRangeValue_File(var_, minValue_, maxValue_, file_)               NW4HBMAssertMessage_FileLine(  file_ , __LINE__, var_ >= minValue_ && var_ <= maxValue_, #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderRangeValue_Line(var_, minValue_, maxValue_, line_)               NW4HBMAssertMessage_FileLine(__FILE__,   line_ , var_ >= minValue_ && var_ <= maxValue_, #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderRangeValue(var_, minValue_, maxValue_)                           NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, var_ >= minValue_ && var_ <= maxValue_, #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))

// ClampedLValue
#define NW4HBMAssertClampedLValue_FileLine(file_, line_, var_, minValue_, maxValue_)            NW4HBMAssertMessage_FileLine(  file_ ,   line_ , (var_) >= (minValue_) && (var_) < (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertClampedLValue_File(file_, var_, minValue_, maxValue_)                        NW4HBMAssertMessage_FileLine(  file_ , __LINE__, (var_) >= (minValue_) && (var_) < (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertClampedLValue_Line(line_, var_, minValue_, maxValue_)                        NW4HBMAssertMessage_FileLine(__FILE__,   line_ , (var_) >= (minValue_) && (var_) < (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertClampedLValue(var_, minValue_, maxValue_)                                    NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (var_) >= (minValue_) && (var_) < (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))

#define NW4HBMAssertHeaderClampedLValue_FileLine(file_, line_, var_, minValue_, maxValue_)    NW4HBMAssertMessage_FileLine(  file_ ,   line_ , (var_) >= (minValue_) && (var_) < (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderClampedLValue_File(var_, minValue_, maxValue_, file_)               NW4HBMAssertMessage_FileLine(  file_ , __LINE__, (var_) >= (minValue_) && (var_) < (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderClampedLValue_Line(var_, minValue_, maxValue_, line_)               NW4HBMAssertMessage_FileLine(__FILE__,   line_ , (var_) >= (minValue_) && (var_) < (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderClampedLValue(var_, minValue_, maxValue_)                           NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (var_) >= (minValue_) && (var_) < (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))

// ClampedLRValue
#define NW4HBMAssertClampedLRValue_FileLine(file_, line_, var_, minValue_, maxValue_)       NW4HBMAssertMessage_FileLine(  file_ ,   line_ , (var_) >= (minValue_) && (var_) <= (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertClampedLRValue_File(var_, minValue_, maxValue_, file_)                  NW4HBMAssertMessage_FileLine(  file_ , __LINE__, (var_) >= (minValue_) && (var_) <= (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertClampedLRValue_Line(var_, minValue_, maxValue_, line_)                  NW4HBMAssertMessage_FileLine(__FILE__,   line_ , (var_) >= (minValue_) && (var_) <= (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertClampedLRValue(var_, minValue_, maxValue_)                              NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (var_) >= (minValue_) && (var_) <= (maxValue_), "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))

#define NW4HBMAssertHeaderClampedLRValue_FileLine(file_, line_, var_, minValue_, maxValue_)    NW4HBMAssertMessage_FileLine(  file_ ,   line_ , (var_) >= (minValue_) && (var_) <= (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderClampedLRValue_File(var_, minValue_, maxValue_, file_)               NW4HBMAssertMessage_FileLine(  file_ , __LINE__, (var_) >= (minValue_) && (var_) <= (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderClampedLRValue_Line(var_, minValue_, maxValue_, line_)               NW4HBMAssertMessage_FileLine(__FILE__,   line_ , (var_) >= (minValue_) && (var_) <= (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))
#define NW4HBMAssertHeaderClampedLRValue(var_, minValue_, maxValue_)                           NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (var_) >= (minValue_) && (var_) <= (maxValue_),         #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), (int)(minValue_), (int)(maxValue_))


/*******************************************************************************
 * Strings, for deadstripping
 */

/* Main asserts */

#define NW4HBMAssert_String(expr_)        "NW4HBM:Failed assertion " #expr_
#define NW4HBMAssertHeader_String(expr_)  "Failed assertion " #expr_

#define NW4HBMCheck_String(expr_)         "NW4HBM:Failed check " #expr_
#define NW4HBMCheckHeader_String(expr_)   "Failed check " #expr_

/* Extended asserts */

#define NW4HBMAssertPointerNonnull_String(ptr_)       "NW4HBM:Pointer must not be NULL (" #ptr_ ")"
#define NW4HBMAssertHeaderPointerNonnull_String(ptr_) "Pointer must not be NULL (" #ptr_ ")"

#define NW4HBMAssertPointerValid_String(ptr_)       "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer."
#define NW4HBMAssertHeaderPointerValid_String(ptr_) "Pointer Error\n" #ptr_ "(=%p) is not valid pointer."

#define NW4HBMAssertMinimumValue_String(var_)         "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied."
#define NW4HBMAssertHeaderMinimumValue_String(var_)   #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied."

#define NW4HBMAssertClampedLValue_String(var_)			"NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied."
#define NW4HBMAssertHeaderClampedLValue_String(var_)		        #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied."

#define NW4HBMAssertClampedLRValue_String(var_)			"NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied."
#define NW4HBMAssertHeaderClampedLRValue_String(var_)		        #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied."

/*******************************************************************************
 * Declarations
 */

namespace nw4hbm { namespace db
{
    __attribute__((weak)) extern void Panic(char const *file, int line, char const *msg, ...);
    __attribute__((weak)) extern void Warning(char const *file, int line, char const *msg, ...);
}} // namespace nw4hbm::db

// clang-format on
