#ifndef _VC64_RVL_H
#define _VC64_RVL_H

#include "emulator/system.h"
#include "revolution/dvd.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SimulatorArgumentType {
    SAT_NONE = -1,
    SAT_NAME = 0,
    SAT_PROGRESSIVE = 1,
    SAT_VIBRATION = 2,
    SAT_RESET = 3,
    SAT_CONTROLLER = 4,
    SAT_XTRA = 5,
    SAT_MEMORYCARD = 6,
    SAT_MOVIE = 7,
    SAT_UNK8 = 8,
    SAT_UNK9 = 9,
    SAT_UNK10 = 10,
    SAT_COUNT = 11
} SimulatorArgumentType;

void fn_80007020(void);
;
bool simulatorDVDShowError(s32 nStatus, void* anData, s32 nSizeRead, u32 nOffset);
bool simulatorDVDOpen(char* szNameFile, DVDFileInfo* pFileInfo);
bool simulatorDVDRead(DVDFileInfo* pFileInfo, void* anData, s32 nSizeRead, s32 nOffset, DVDCallback callback);
bool simulatorShowLoad(s32 unknown, char* szNameFile, f32 rProgress);
bool simulatorGetArgument(SimulatorArgumentType eType, char** pszArgument);
bool xlMain(void);

extern System* gpSystem;

#ifdef __cplusplus
}
#endif

#endif
