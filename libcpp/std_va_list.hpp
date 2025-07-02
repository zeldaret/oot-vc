#ifndef __INTERNAL_STD_VA_LIST_HPP__
#define __INTERNAL_STD_VA_LIST_HPP__

// IWYU pragma: private, include <cstdarg>

extern "C"
{
    typedef struct __va_list_struct
	{
	    char gpr;
	    char fpr;
	    char reserved[2];
	    char *input_arg_area;
	    char *reg_save_area;
	} va_list[1];
} // extern "C"

namespace std
{
    typedef ::va_list va_list;
} // namespace std

#endif // __INTERNAL_STD_VA_LIST_HPP__
