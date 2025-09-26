#ifndef __PPC_EABI_LINKER
#define __PPC_EABI_LINKER

#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __rom_copy_info {
    char* rom;
    char* addr;
    unsigned int size;
} __rom_copy_info;

INIT extern __rom_copy_info _rom_copy_info[];

typedef struct __bss_init_info {
    char* addr;
    unsigned int size;
} __bss_init_info;

INIT extern __bss_init_info _bss_init_info[];

typedef struct __eti_init_info {
    void* eti_start;
    void* eti_end;
    void* code_start;
    unsigned long code_size;
} __eti_init_info;

INIT extern __eti_init_info _eti_init_info[];

INIT extern const char _f_PPC_EMB_sdata0_rom[];
INIT extern char _f_PPC_EMB_sdata0[];
INIT extern char _e_PPC_EMB_sdata0[];
INIT extern char _f_PPC_EMB_sbss0[];
INIT extern char _e_PPC_EMB_sbss0[];

#ifdef __cplusplus
}
#endif

#endif // __PPC_EABI_LINKER
