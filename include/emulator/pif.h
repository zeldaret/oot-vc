#ifndef _PIF_H
#define _PIF_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PIF_RAM_START 0x7C0
#define PIF_RAM_END 0x7FF

#define PIF_DATA_CRC_MESSAGE_BYTES 32
#define PIF_DATA_CRC_LENGTH 8
#define PIF_DATA_CRC_GENERATOR 0x85

#define PIF_GET_RAM_ADDR(pPIF, iData) (((u8*)(pPIF)->pRAM) + (iData))
#define PIF_GET_RAM_DATA(pPIF, iData) (*PIF_GET_RAM_ADDR(pPIF, iData))

// __anon_0x3C277
typedef enum ControllerType {
    CT_NONE = -1,
    CT_CONTROLLER = 0,
    CT_CONTROLLER_W_PAK = 1, // Controller Pak?
    CT_CONTROLLER_W_RPAK = 2, // Rumble Pak
    CT_MOUSE = 3,
    CT_VOICE = 4,
    CT_4K = 5, // Mem Pak?
    CT_16K = 6, // Mem Pak?
    CT_COUNT = 7,
} ControllerType;

// __anon_0x3C350
typedef struct Pif {
    /* 0x00 */ void* pROM;
    /* 0x04 */ void* pRAM;
    /* 0x16 */ char controllerStatus[4];
    /* 0x0C */ u16 controllerType[4];
    /* 0x1C */ ControllerType eControllerType[4];
} Pif; // size = 0x30

bool pifExecuteCommand(Pif* pPIF, u8* buffer, u8* ptx, u8* prx, s32 channel);
bool pifProcessInputData(Pif* pPIF);
bool pifProcessOutputData(Pif* pPIF);
bool pifSetData(Pif* pPIF, u8* acData);
bool pifGetData(Pif* pPIF, u8* acData);
bool pifSetControllerType(Pif* pPIF, s32 channel, ControllerType type);
bool pifEvent(Pif* pPIF, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassPIF;

#ifdef __cplusplus
}
#endif

#endif
