#ifndef _EEPROM_H
#define _EEPROM_H

#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EEPROM {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ void* pRAM;
    /* 0x08 */ struct Store* pStore;
} EEPROM; // size = 0x0C

bool fn_80044708(EEPROM* pEEPROM, s32 arg2, u32 nUnknown, void* pBuffer);
bool fn_8004477C(EEPROM* pEEPROM, s32 arg2, u32 nUnknown, void* pBuffer);
bool eepromEvent(EEPROM* pEEPROM, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassEEPROM;

#ifdef __cplusplus
}
#endif

#endif
