#ifndef _LIBRARY_H
#define _LIBRARY_H

#include "emulator/cpu.h"
#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LibraryFuncImpl)(Cpu*);

typedef struct LibraryFunc {
    /* 0x0 */ char* szName;
    /* 0x4 */ LibraryFuncImpl pfLibrary;
    /* 0x8 */ u32 anData[17];
} LibraryFunc; // size = 0x4C

typedef struct Library {
    /* 0x00 */ s32 nFlag;
    /* 0x04 */ s32 nAddStackSwap;
    /* 0x08 */ s32 nCountFunction;
    /* 0x0C */ s32 nAddressException;
    /* 0x10 */ LibraryFunc* aFunction;
    /* 0x14 */ void* apData[10];
    /* 0x3C */ s32 anAddress[10];
} Library; // size = 0x64

extern _XL_OBJECTTYPE gClassLibrary;

bool libraryTestFunction(Library* pLibrary, CpuFunction* pFunction);
bool libraryFunctionReplaced(Library* pLibrary, s32 iFunction);
bool libraryCall(Library* pLibrary, Cpu* pCPU, s32 iFunction);
bool libraryEvent(Library* pLibrary, s32 nEvent, void* pArgument);

#ifdef __cplusplus
}
#endif

#endif
