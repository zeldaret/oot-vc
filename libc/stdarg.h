#ifndef _STDARG_H
#define _STDARG_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__INTELLISENSE__) && defined(__cplusplus)
#define __builtin_va_info(v) 0 /* definition for IDEs */
#endif

typedef enum _va_arg_type {
    arg_ARGPOINTER,
    arg_WORD,
    arg_DOUBLEWORD,
    arg_ARGREAL
} _va_arg_type;

typedef struct __va_list_struct {
    char gpr;
    char fpr;
    char reserved[2];
    char* input_arg_area;
    char* reg_save_area;
} __va_list_struct;

typedef __va_list_struct va_list[1];

void* __va_arg(__va_list_struct* list, int type);

#ifdef __MWERKS__
#define va_start(ap, fmt) ((void)fmt, __builtin_va_info(&ap))
#define va_arg(ap, t) (*((t*)__va_arg(ap, _var_arg_typeof(t))))
#else
#define va_start(ap, fmt) 0
#define va_arg(ap, t) 0
#endif

#define va_end(ap) (void)0

#ifdef __cplusplus
}
#endif

#endif
