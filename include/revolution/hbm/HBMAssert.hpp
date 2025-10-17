#ifndef REVOLUTION_HBM_ASSERT_H
#define REVOLUTION_HBM_ASSERT_H

/*******************************************************************************
 * NW4HBMAssert/NW4HBMCheck base macros
 */

#ifdef HBM_ASSERT
#define NW4HBMAssertMessage_FileLine(file_, line_, expr_, ...) \
    ((void)((expr_) || (::nw4hbm::db::Panic(file_, line_, __VA_ARGS__), 0)))

#define NW4HBMCheckMessage_FileLine(file_, line_, expr_, ...) \
    ((void)((expr_) || (::nw4hbm::db::Warning(file_, line_, __VA_ARGS__), 0)))
#else
#define NW4HBMAssertMessage_FileLine(file_, line_, expr_, ...) ((void)(0))
#define NW4HBMCheckMessage_FileLine(file_, line_, expr_, ...) ((void)(0))
#endif

/*******************************************************************************
 * Derived macros
 */

/* Main asserts */

// NW4HBMAssert family
#define NW4HBMAssertMessage_Line(expr_, line_, ...) NW4HBMAssertMessage_FileLine(__FILE__, line_, expr_, __VA_ARGS__)
#define NW4HBMAssertMessage(expr_, ...) NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, expr_, __VA_ARGS__)

#define NW4HBMAssert_FileLine(expr_, file_, line_) \
    NW4HBMAssertMessage_FileLine(file_, line_, expr_, "NW4HBM:Failed assertion " #expr_)

#define NW4HBMAssert_Line(expr_, line_) \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, expr_, "NW4HBM:Failed assertion " #expr_)

#define NW4HBMAssert(expr_) NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, expr_, "NW4HBM:Failed assertion " #expr_)

// NW4HBMPanic family
#define NW4HBMPanicMessage_Line(line_, ...) NW4HBMAssertMessage_FileLine(__FILE__, line_, false, __VA_ARGS__)
#define NW4HBMPanic_Line(line_) NW4HBMAssert_FileLine(false, __FILE__, line_)
#define NW4HBMPanic() NW4HBMAssert_FileLine(false, __FILE__, __LINE__)

// NW4HBMCheck family
#define NW4HBMCheckMessage_Line(expr_, line_, ...) NW4HBMCheckMessage_FileLine(__FILE__, line_, expr_, __VA_ARGS__)
#define NW4HBMCheckMessage(expr_, ...) NW4HBMCheckMessage_FileLine(__FILE__, __LINE__, expr_, __VA_ARGS__)

// NW4HBMWarning family
#define NW4HBMWarningMessage_Line(line_, ...) NW4HBMCheckMessage_FileLine(__FILE__, line_, false, __VA_ARGS__)
#define NW4HBMWarningMessage(...) NW4HBMCheckMessage_FileLine(__FILE__, __LINE__, false, __VA_ARGS__)

// Aligned
#define NW4HBM_IS_ALIGNED_(x, align) \
    (((unsigned long)(x) & ((align) - 1)) == 0) // just redefine instead of pulling in <macros.h>

// NW4HBMAlign family
#define NW4HBMAlign2_Line(expr_, line_)                                                                           \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, NW4HBM_IS_ALIGNED_(expr_, 2),                                   \
                                 "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 2 bytes boundary.", \
                                 expr_)

// NW4HBMAlign32
#define NW4HBMAlign32_Line(expr_, line_)                                                                           \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, NW4HBM_IS_ALIGNED_(expr_, 32),                                   \
                                 "NW4HBM:Alignment Error(0x%x)\n" #expr_ " must be aligned to 32 bytes boundary.", \
                                 expr_)

// Generic align
#define NW4HBMAssertAligned_Line(line_, val_, align_)      \
    NW4HBMAssertMessage_FileLine(                          \
        __FILE__, line_, NW4HBM_IS_ALIGNED_(val_, align_), \
        "NW4HBM:Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary.", val_)

/* Extended asserts */

// PointerNonnull
#define NW4HBMAssertPointerNonnull_Line(ptr_, line_) \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, (ptr_) != 0, "NW4HBM:Pointer must not be NULL (" #ptr_ ")")

#define NW4HBMAssertPointerNonnull(ptr_) \
    NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (ptr_) != 0, "NW4HBM:Pointer must not be NULL (" #ptr_ ")")

#define NW4HBMAssertHeaderPointerNonnull_FileLine(ptr_, file_, line_) \
    NW4HBMAssertMessage_FileLine(file_, line_, (ptr_) != 0, "Pointer must not be NULL (" #ptr_ ")")

#define POINTER_VALID_TEST(ptr_)                                                                             \
    (((unsigned long)ptr_ & 0xFF000000) == 0x80000000 || ((unsigned long)ptr_ & 0xFF800000) == 0x81000000 || \
     ((unsigned long)ptr_ & 0xF8000000) == 0x90000000 || ((unsigned long)ptr_ & 0xFF000000) == 0xC0000000 || \
     ((unsigned long)ptr_ & 0xFF800000) == 0xC1000000 || ((unsigned long)ptr_ & 0xF8000000) == 0xD0000000 || \
     ((unsigned long)ptr_ & 0xFFFFC000) == 0xE0000000)

// PointerValid
#define NW4HBMAssertPointerValid_Line(ptr_, line_)                          \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, POINTER_VALID_TEST(ptr_), \
                                 "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)

#define NW4HBMAssertPointerValid(ptr_)                                         \
    NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, POINTER_VALID_TEST(ptr_), \
                                 "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)

// MinimumValue
#define NW4HBMAssertMinimumValue_Line(var_, minValue_, line_)                                                          \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, minValue_ <= var_,                                                   \
                                 "NW4HBM:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), \
                                 (int)minValue_)

#define NW4HBMAssertHeaderMinimumValue_Line(var_, minValue_, line_)                                          \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, minValue_ <= var_,                                         \
                                 #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), \
                                 (int)(minValue_))

#define NW4HBMAssertHeaderMinimumValue(var_, minValue_)                                                      \
    NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, minValue_ <= var_,                                      \
                                 #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), \
                                 (int)(minValue_))

// ClampedLValue
#define NW4HBMAssertHeaderClampedLValue_Line(var_, minValue_, maxValue_, line_)                                   \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, (var_) >= (minValue_) && (var_) < (maxValue_),                  \
                                 #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), \
                                 (int)(minValue_), (int)(maxValue_))

#define NW4HBMAssertHeaderClampedLValue(var_, minValue_, maxValue_)                                               \
    NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (var_) >= (minValue_) && (var_) < (maxValue_),               \
                                 #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied.", (int)(var_), \
                                 (int)(minValue_), (int)(maxValue_))

// ClampedLRValue
#define NW4HBMAssertHeaderClampedLRValue_Line(var_, minValue_, maxValue_, line_)                                   \
    NW4HBMAssertMessage_FileLine(__FILE__, line_, (var_) >= (minValue_) && (var_) <= (maxValue_),                  \
                                 #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), \
                                 (int)(minValue_), (int)(maxValue_))

#define NW4HBMAssertHeaderClampedLRValue(var_, minValue_, maxValue_)                                               \
    NW4HBMAssertMessage_FileLine(__FILE__, __LINE__, (var_) >= (minValue_) && (var_) <= (maxValue_),               \
                                 #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied.", (int)(var_), \
                                 (int)(minValue_), (int)(maxValue_))

/*******************************************************************************
 * Strings, for deadstripping
 */

/* Main asserts */

#define NW4HBMAssert_String(expr_) "NW4HBM:Failed assertion " #expr_

/* Extended asserts */

#define NW4HBMAssertPointerNonnull_String(ptr_) "NW4HBM:Pointer must not be NULL (" #ptr_ ")"
#define NW4HBMAssertPointerValid_String(ptr_) "NW4HBM:Pointer Error\n" #ptr_ "(=%p) is not valid pointer."
#define NW4HBMAssertHeaderClampedLValue_String(var_) #var_ " is out of bounds(%d)\n%d <= " #var_ " < %d not satisfied."

#define NW4HBMAssertHeaderClampedLRValue_String(var_) \
    #var_ " is out of bounds(%d)\n%d <= " #var_ " <= %d not satisfied."

#define NW4HBMAssertAligned_String(val_, align_) \
    "NW4HBM:Alignment Error(0x%x)\n" #val_ " must be aligned to " #align_ " bytes boundary."

namespace nw4hbm {
namespace db {

__attribute__((weak)) extern void Panic(char const* file, int line, char const* msg, ...);
__attribute__((weak)) extern void Warning(char const* file, int line, char const* msg, ...);

} // namespace db
} // namespace nw4hbm

#endif
