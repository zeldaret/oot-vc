// clang-format off

/*******************************************************************************
 * anti-header guard
 */

#ifdef __MWERKS__
# pragma notonce
#endif

/*******************************************************************************
 * NW4RAssert/NW4RCheck base macros
 */

// <assert.h> behavior
#undef NW4RAssertMessage_FileLine
#undef NW4RCheckMessage_FileLine

#if defined(HBM_ASSERT)
# define NW4RAssertMessage_FileLine(file_, line_, expr_, ...)	((void)((expr_) || (::nw4hbm::db::Panic  (file_, line_, __VA_ARGS__), 0)))
# define NW4RCheckMessage_FileLine(file_, line_, expr_, ...)	((void)((expr_) || (::nw4hbm::db::Warning(file_, line_, __VA_ARGS__), 0)))
#else
# define NW4RAssertMessage_FileLine(file_, line_, expr_, ...)	((void)(0))
# define NW4RCheckMessage_FileLine(file_, line_, expr_, ...)	((void)(0))
#endif // defined(HBM_ASSERT)

/*******************************************************************************
 * Derived macros
 */

/* Main asserts */

// NW4RAssert family
#define NW4RAssertMessage_File(file_, expr_, ...)								NW4RAssertMessage_FileLine(  file_ , __LINE__, expr_, __VA_ARGS__)
#define NW4RAssertMessage_Line(line_, expr_, ...)								NW4RAssertMessage_FileLine(__FILE__,   line_ , expr_, __VA_ARGS__)
#define NW4RAssertMessage(expr_, ...)											NW4RAssertMessage_FileLine(__FILE__, __LINE__, expr_, __VA_ARGS__)

#define NW4RAssert_FileLine(file_, line_, expr_)								NW4RAssertMessage_FileLine(  file_ ,   line_ , expr_, "NW4R:Failed assertion " #expr_)
#define NW4RAssert_File(file_, expr_)											NW4RAssertMessage_FileLine(  file_ , __LINE__, expr_, "NW4R:Failed assertion " #expr_)
#define NW4RAssert_Line(line_, expr_)											NW4RAssertMessage_FileLine(__FILE__,   line_ , expr_, "NW4R:Failed assertion " #expr_)
#define NW4RAssert(expr_)														NW4RAssertMessage_FileLine(__FILE__, __LINE__, expr_, "NW4R:Failed assertion " #expr_)

#define NW4RAssertHeader_FileLine(file_, line_, expr_)							NW4RAssertMessage_FileLine(  file_ ,   line_ , expr_,      "Failed assertion " #expr_)
#define NW4RAssertHeader_File(file_, expr_)										NW4RAssertMessage_FileLine(  file_ , __LINE__, expr_,      "Failed assertion " #expr_)
#define NW4RAssertHeader_Line(line_, expr_)										NW4RAssertMessage_FileLine(__FILE__,   line_ , expr_,      "Failed assertion " #expr_)
#define NW4RAssertHeader(expr_)													NW4RAssertMessage_FileLine(__FILE__, __LINE__, expr_,      "Failed assertion " #expr_)

// NW4RPanic family
#define NW4RPanicMessage_FileLine(file_, line_, ...)							NW4RAssertMessage_FileLine(  file_ ,   line_ , false, __VA_ARGS__)
#define NW4RPanicMessage_File(file_, ...)										NW4RAssertMessage_FileLine(  file_ , __LINE__, false, __VA_ARGS__)
#define NW4RPanicMessage_Line(line_, ...)										NW4RAssertMessage_FileLine(__FILE__,   line_ , false, __VA_ARGS__)
#define NW4RPanicMessage(...)													NW4RAssertMessage_FileLine(__FILE__, __LINE__, false, __VA_ARGS__)

#define NW4RPanic_FileLine(file_, line_)										NW4RAssert_FileLine(  file_ ,   line_ , false)
#define NW4RPanic_File(file_)													NW4RAssert_FileLine(  file_ , __LINE__, false)
#define NW4RPanic_Line(line_)													NW4RAssert_FileLine(__FILE__,   line_ , false)
#define NW4RPanic()																NW4RAssert_FileLine(__FILE__, __LINE__, false)

// NW4RCheck family
#define NW4RCheckMessage_File(file_, expr_, ...)								NW4RCheckMessage_FileLine(  file_ , __LINE__, expr_, __VA_ARGS__)
#define NW4RCheckMessage_Line(line_, expr_, ...)								NW4RCheckMessage_FileLine(__FILE__,   line_ , expr_, __VA_ARGS__)
#define NW4RCheckMessage(expr_, ...)											NW4RCheckMessage_FileLine(__FILE__, __LINE__, expr_, __VA_ARGS__)

#define NW4RCheck_FileLine(file_, line_, expr_)									NW4RCheckMessage_FileLine(  file_ ,   line_ , expr_, "NW4R:Failed check " #expr_)
#define NW4RCheck_File(file_, expr_)											NW4RCheckMessage_FileLine(  file_ , __LINE__, expr_, "NW4R:Failed check " #expr_)
#define NW4RCheck_Line(line_, expr_)											NW4RCheckMessage_FileLine(__FILE__,   line_ , expr_, "NW4R:Failed check " #expr_)
#define NW4RCheck(expr_)														NW4RCheckMessage_FileLine(__FILE__, __LINE__, expr_, "NW4R:Failed check " #expr_)

#define NW4RCheckHeader_FileLine(file_, line_, expr_)							NW4RCheckMessage_FileLine(  file_ ,   line_ , expr_,      "Failed check " #expr_)
#define NW4RCheckHeader_File(file_, expr_)										NW4RCheckMessage_FileLine(  file_ , __LINE__, expr_,      "Failed check " #expr_)
#define NW4RCheckHeader_Line(line_, expr_)										NW4RCheckMessage_FileLine(__FILE__,   line_ , expr_,      "Failed check " #expr_)
#define NW4RCheckHeader(expr_)													NW4RCheckMessage_FileLine(__FILE__, __LINE__, expr_,      "Failed check " #expr_)

// NW4RWarning family
#define NW4RWarningMessage_FileLine(file_, line_, ...)							NW4RCheckMessage_FileLine(  file_ ,   line_ , false, __VA_ARGS__)
#define NW4RWarningMessage_File(file_, ...)										NW4RCheckMessage_FileLine(  file_ , __LINE__, false, __VA_ARGS__)
#define NW4RWarningMessage_Line(line_, ...)										NW4RCheckMessage_FileLine(__FILE__,   line_ , false, __VA_ARGS__)
#define NW4RWarningMessage(...)													NW4RCheckMessage_FileLine(__FILE__, __LINE__, false, __VA_ARGS__)

#define NW4RWarning_FileLine(file_, line_)										NW4RCheck_FileLine(  file_ ,   line_ , false)
#define NW4RWarning_File(file_)													NW4RCheck_FileLine(  file_ , __LINE__, false)
#define NW4RWarning_Line(line_)													NW4RCheck_FileLine(__FILE__,   line_ , false)
#define NW4RWarning()															NW4RCheck_FileLine(__FILE__, __LINE__, false)

/* Extended asserts */

// PointerNonnull
#define NW4RAssertPointerNonnull_FileLine(file_, line_, ptr_)					NW4RAssertMessage_FileLine(  file_ ,   line_ , (ptr_) != 0, "NW4R:Pointer must not be NULL (" #ptr_ ")")
#define NW4RAssertPointerNonnull_File(file_, ptr_)								NW4RAssertMessage_FileLine(  file_ , __LINE__, (ptr_) != 0, "NW4R:Pointer must not be NULL (" #ptr_ ")")
#define NW4RAssertPointerNonnull_Line(line_, ptr_)								NW4RAssertMessage_FileLine(__FILE__,   line_ , (ptr_) != 0, "NW4R:Pointer must not be NULL (" #ptr_ ")")
#define NW4RAssertPointerNonnull(ptr_)											NW4RAssertMessage_FileLine(__FILE__, __LINE__, (ptr_) != 0, "NW4R:Pointer must not be NULL (" #ptr_ ")")

#define NW4RAssertHeaderPointerNonnull_FileLine(file_, line_, ptr_)				NW4RAssertMessage_FileLine(  file_ ,   line_ , (ptr_) != 0,      "Pointer must not be NULL (" #ptr_ ")")
#define NW4RAssertHeaderPointerNonnull_File(file_, ptr_)						NW4RAssertMessage_FileLine(  file_ , __LINE__, (ptr_) != 0,      "Pointer must not be NULL (" #ptr_ ")")
#define NW4RAssertHeaderPointerNonnull_Line(line_, ptr_)						NW4RAssertMessage_FileLine(__FILE__,   line_ , (ptr_) != 0,      "Pointer must not be NULL (" #ptr_ ")")
#define NW4RAssertHeaderPointerNonnull(ptr_)									NW4RAssertMessage_FileLine(__FILE__, __LINE__, (ptr_) != 0,      "Pointer must not be NULL (" #ptr_ ")")

#define POINTER_VALID_TEST(ptr_)							\
	   (((unsigned long)ptr_ & 0xff000000) == 0x80000000	\
	 || ((unsigned long)ptr_ & 0xff800000) == 0x81000000	\
	 || ((unsigned long)ptr_ & 0xf8000000) == 0x90000000	\
	 || ((unsigned long)ptr_ & 0xff000000) == 0xc0000000	\
	 || ((unsigned long)ptr_ & 0xff800000) == 0xc1000000	\
	 || ((unsigned long)ptr_ & 0xf8000000) == 0xd0000000	\
	 || ((unsigned long)ptr_ & 0xffffc000) == 0xe0000000)

// PointerValid
#define NW4RAssertPointerValid_FileLine(file_, line_, ptr_)						NW4RAssertMessage_FileLine(  file_ ,   line_ , POINTER_VALID_TEST(ptr_), "NW4R:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4RAssertPointerValid_File(file_, ptr_)								NW4RAssertMessage_FileLine(  file_ , __LINE__, POINTER_VALID_TEST(ptr_), "NW4R:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4RAssertPointerValid_Line(line_, ptr_)								NW4RAssertMessage_FileLine(__FILE__,   line_ , POINTER_VALID_TEST(ptr_), "NW4R:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4RAssertPointerValid(ptr_)											NW4RAssertMessage_FileLine(__FILE__, __LINE__, POINTER_VALID_TEST(ptr_), "NW4R:Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)

#define NW4RAssertHeaderPointerValid_FileLine(file_, line_, ptr_)				NW4RAssertMessage_FileLine(  file_ ,   line_ , POINTER_VALID_TEST(ptr_),      "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4RAssertHeaderPointerValid_File(file_, ptr_)							NW4RAssertMessage_FileLine(  file_ , __LINE__, POINTER_VALID_TEST(ptr_),      "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4RAssertHeaderPointerValid_Line(line_, ptr_)							NW4RAssertMessage_FileLine(__FILE__,   line_ , POINTER_VALID_TEST(ptr_),      "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)
#define NW4RAssertHeaderPointerValid(ptr_)										NW4RAssertMessage_FileLine(__FILE__, __LINE__, POINTER_VALID_TEST(ptr_),      "Pointer Error\n" #ptr_ "(=%p) is not valid pointer.", ptr_)

// MinimumValue
#define NW4RAssertMinimumValue_FileLine(file_, line_, var_, minValue_)			NW4RAssertMessage_FileLine(  file_ ,   line_ , minValue_ <= var_, "NW4R:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)
#define NW4RAssertMinimumValue_File(file_, var_, minValue_)						NW4RAssertMessage_FileLine(  file_ , __LINE__, minValue_ <= var_, "NW4R:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)
#define NW4RAssertMinimumValue_Line(line_, var_, minValue_)						NW4RAssertMessage_FileLine(__FILE__,   line_ , minValue_ <= var_, "NW4R:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)
#define NW4RAssertMinimumValue(var_, minValue_)									NW4RAssertMessage_FileLine(__FILE__, __LINE__, minValue_ <= var_, "NW4R:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)minValue_)

#define NW4RAssertHeaderMinimumValue_FileLine(file_, line_, var_, minValue_)	NW4RAssertMessage_FileLine(  file_ ,   line_ , minValue_ <= var_,         #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))
#define NW4RAssertHeaderMinimumValue_File(file_, var_, minValue_)				NW4RAssertMessage_FileLine(  file_ , __LINE__, minValue_ <= var_,         #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))
#define NW4RAssertHeaderMinimumValue_Line(line_, var_, minValue_)				NW4RAssertMessage_FileLine(__FILE__,   line_ , minValue_ <= var_,         #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))
#define NW4RAssertHeaderMinimumValue(var_, minValue_)							NW4RAssertMessage_FileLine(__FILE__, __LINE__, minValue_ <= var_,         #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied.", (int)(var_), (int)(minValue_))

/*******************************************************************************
 * Strings, for deadstripping
 */

/* Main asserts */

#define NW4RAssert_String(expr_)						"NW4R:Failed assertion " #expr_
#define NW4RAssertHeader_String(expr_)					     "Failed assertion " #expr_

#define NW4RCheck_String(expr_)							"NW4R:Failed check " #expr_
#define NW4RCheckHeader_String(expr_)					     "Failed check " #expr_

/* Extended asserts */

#define NW4RAssertPointerNonnull_String(ptr_)			"NW4R:Pointer must not be NULL (" #ptr_ ")"
#define NW4RAssertHeaderPointerNonnull_String(ptr_)		     "Pointer must not be NULL (" #ptr_ ")"

#define NW4RAssertPointerValid_String(val_, ptr_)		"NW4R:Pointer Error\n" #ptr_ "(=%p) is not valid pointer."
#define NW4RAssertHeaderPointerValid_String(val_, ptr_)	     "Pointer Error\n" #ptr_ "(=%p) is not valid pointer."

#define NW4RAssertMinimumValue_String(var_)				"NW4R:" #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied."
#define NW4RAssertHeaderMinimumValue_String(var_)		        #var_ " is out of bounds(%d)\n%d <= " #var_ " not satisfied."

/*******************************************************************************
 * Declarations
 */

namespace nw4hbm { namespace db
{
	__attribute__((weak)) extern void Panic(char const *file, int line, char const *msg, ...);
	__attribute__((weak)) extern void Warning(char const *file, int line, char const *msg, ...);
}} // namespace nw4hbm::db

// clang-format on
