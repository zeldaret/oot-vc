#ifndef _RVL_SDK_OS_PPC_EABI_INIT_H
#define _RVL_SDK_OS_PPC_EABI_INIT_H

#include "macros.h"
#include "revolution/types.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma section ".init"
INIT void __init_hardware(void);
INIT void __flush_cache(void*, size_t);

void __init_user(void);
void __init_cpp(void);
void __fini_cpp(void);
WEAK void exit(void);
void _ExitProcess(void);

/**
 * Linker Generated Symbols
 */

// Declare linker symbols for a section in the ROM
#define DECL_ROM_SECTION(x)  \
    extern u8 _f##x[];       \
    extern u8 _f##x##_rom[]; \
    extern u8 _e##x[];

// Declare linker symbols for a BSS section
#define DECL_BSS_SECTION(x) \
    extern u8 _f##x[];      \
    extern u8 _e##x[];

// Debugger stack
extern u8 _db_stack_addr[];
extern u8 _db_stack_end[];

// Program arena
extern u8 __ArenaLo[];
extern u8 __ArenaHi[];

// Program stack
extern u8 _stack_addr[];
extern u8 _stack_end[];

// Small data bases
extern u8 _SDA_BASE_[];
extern u8 _SDA2_BASE_[];

// ROM sections
DECL_ROM_SECTION(_init);
DECL_ROM_SECTION(extab);
DECL_ROM_SECTION(extabindex);
DECL_ROM_SECTION(_text);
DECL_ROM_SECTION(_ctors);
DECL_ROM_SECTION(_dtors);
DECL_ROM_SECTION(_rodata);
DECL_ROM_SECTION(_data);
DECL_ROM_SECTION(_sdata);
DECL_ROM_SECTION(_sdata2);
DECL_ROM_SECTION(_stack);

// BSS sections
DECL_BSS_SECTION(_bss);
DECL_BSS_SECTION(_sbss);
DECL_BSS_SECTION(_sbss2);

typedef struct RomSection {
    /* 0x4 */ void* phys;
    /* 0x0 */ void* virt;
    /* 0x8 */ size_t size;
} RomSection;

typedef struct BssSection {
    /* 0x0 */ void* virt;
    /* 0x8 */ size_t size;
} BssSection;

typedef struct ExtabIndexInfo {
    /* 0x0 */ void* section;
    /* 0x4 */ struct ExtabIndexInfo* extab;
    /* 0x8 */ void* codeStart;
    /* 0xC */ u32 codeSize;
} ExtabIndexInfo;

INIT extern const RomSection _rom_copy_info[];
INIT extern const BssSection _bss_init_info[];
INIT extern const ExtabIndexInfo _eti_init_info[];

#ifdef __cplusplus
}
#endif

#endif
