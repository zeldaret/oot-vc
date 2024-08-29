#include "emulator/store.h"
#include "emulator/eeprom.h"
#include "emulator/flash.h"
#include "emulator/sram.h"
#include "emulator/vc64_RVL.h"
#include "emulator/xlHeap.h"
#include "mem_funcs.h"
#include "revolution/nand.h"
#include "revolution/os.h"

#define STORE_OBJ (*(Store**)ppObject)

static bool fn_800616F4(Store* pStore, s32 unknown, s32 nSize) NO_INLINE;
static bool fn_800618D4(Store* pStore, void* arg1, s32 arg2, s32 arg3);
static void fn_80061C08(s32 nResult, NANDCommandBlock* block);
static void fn_80061C4C(s32 nResult, NANDCommandBlock* block);
static bool fn_80061CAC(Store* pStore);
static inline void fn_80061DB8_Inline(Store* pStore);

_XL_OBJECTTYPE gClassStore = {
    "Store",
    sizeof(Store),
    NULL,
    (EventFunc)storeEvent,
};

static inline bool unknownInline(Store* pStore, s32 unknown) {
    if (pStore->unk_A4 == 0) {
        return true;
    }

    if (!fn_800641CC(&pStore->nandFileInfo, pStore->szFileName, pStore->unk_00, 0xAA, unknown)) {
        pStore->unk_A4 = 0;
    }

    return true;
}

static bool fn_800616F4(Store* pStore, s32 unknown, s32 nSize) {
    if (pStore->unk_A8 != 0) {
        return true;
    }

    if (xlHeapTake((void**)&pStore->unk_A8, nSize | 0x30000000) == false) {
        return false;
    }

    return !!xlHeapTake(&pStore->unk_AC, nSize | 0x30000000);
}

bool fn_80061770(void** ppObject, char* szName, SystemRomType eTypeROM, void* pArgument) {
    if (!xlObjectMake(ppObject, NULL, &gClassStore)) {
        return false;
    }

    STORE_OBJ->unk_00 = (s32)pArgument;
    STORE_OBJ->eTypeROM = eTypeROM;

    // example: "RAM_CZLJ"
    STORE_OBJ->szFileName[0] = szName[0];
    STORE_OBJ->szFileName[1] = szName[1];
    STORE_OBJ->szFileName[2] = szName[2];
    STORE_OBJ->szFileName[3] = '_';
    STORE_OBJ->szFileName[4] = GET_GAME_MEDIUM(eTypeROM);
    STORE_OBJ->szFileName[5] = GET_GAME_ID_1(eTypeROM);
    STORE_OBJ->szFileName[6] = GET_GAME_ID_2(eTypeROM);
    STORE_OBJ->szFileName[7] = GET_GAME_REGION(eTypeROM);
    STORE_OBJ->szFileName[8] = '\0';

    STORE_OBJ->unk_B4 = 0;
    STORE_OBJ->unk_B0 = 0;

    fn_800616F4(*ppObject, eTypeROM, (s32)pArgument);

    STORE_OBJ->unk_A4 = 1;
    STORE_OBJ->unk_B9 = 1;
    STORE_OBJ->unk_B8 = 0;
    STORE_OBJ->unk_BA = 1;
    STORE_OBJ->unk_BC = (s32)pArgument;

    fn_800618D4(*ppObject, (void*)STORE_OBJ->unk_A8, 0, (s32)pArgument);
    return true;
}

bool storeFreeObject(void** ppObject) { return !!xlObjectFree(ppObject); }

static bool fn_800618D4(Store* pStore, void* arg1, s32 arg2, s32 arg3) {
    s32 var_r4;
    s32 var_r5;
    s32 var_r7;
    s32 var_r26;
    s32 var_r27;
    s32 var_r28;
    s32 i;

    for (var_r26 = arg3; var_r26 > 0; var_r26--) {
        while (true) {
            if (!unknownInline(pStore, 1)) {
                return false;
            }

            if (pStore->unk_A4 == 0) {
                // probably fn_80061B88 inlined?
                xlHeapCopy((s32*)arg1, (void*)(pStore->unk_A8 + arg2), arg3);
                return true;
            }

            if (arg2 == 0) {
                DCInvalidateRange(arg1, arg3);

                if (NANDRead(&pStore->nandFileInfo, arg1, arg3) >= 0) {
                    break;
                }

                fn_80064600(&pStore->nandFileInfo, 1);
                var_r28 = (arg2 / 32) << 5;

                if (NANDSeek(&pStore->nandFileInfo, var_r28, NAND_SEEK_BEG) < 0) {
                    break;
                }
            }
        }

        var_r27 = arg2;
        DCInvalidateRange(pStore->unk_9C, 0x20);

        if (NANDRead(&pStore->nandFileInfo, pStore->unk_9C, 0x20) < 0) {
            fn_80064600(&pStore->nandFileInfo, 1);
        }

        var_r5 = var_r27 - var_r28 - 0x20;
        if (var_r5 > var_r26) {
            var_r5 = var_r26;
        }

        if (var_r5 > 8) {
            // if (var_r5 >= 8 && var_r5 <= 0x7FFFFFFE) {
            //     var_r7 = 1;
            // }

            // if (var_r7 != 0) {
            var_r4 = 0;
            for (i = 0; i < ((var_r5 - 8) + 7) >> 3; i++) {
                *((u8*)arg1 + i) = *((u8*)pStore->unk_9C + i);
            }
            // }
        }
    }

    fn_80064600(&pStore->nandFileInfo, 1);
    return true;
}

bool fn_80061B88(Store* pStore, void* pHeapTarget, s32 nOffset, s32 nByteCount) {
    xlHeapCopy((s32*)pHeapTarget, (void*)(pStore->unk_A8 + nOffset), nByteCount);
    return true;
}

bool fn_80061BC0(Store* pStore, void* pHeapTarget, s32 nOffset, s32 nByteCount) {
    xlHeapCopy((void*)(pStore->unk_A8 + nOffset), (s32*)pHeapTarget, nByteCount);
    pStore->unk_B8 = 1;
    return true;
}

static void fn_80061C08(s32 nResult, NANDCommandBlock* block) {
    Store* pStore = (Store*)NANDGetUserData(block);

    if (nResult != 0) {
        pStore->unk_BC = nResult;
    }

    pStore->unk_B9 = 1;
}

static void fn_80061C4C(s32 nResult, NANDCommandBlock* block) {
    Store* pStore = (Store*)NANDGetUserData(block);
    bool bSuccess;

    pStore->unk_BC = nResult;
    bSuccess = NANDCloseAsync(&pStore->nandFileInfo, fn_80061C08, &pStore->nandCmdBlock);

    if (bSuccess) {
        pStore->unk_BC = bSuccess;
        pStore->unk_B9 = 1;
    }
}

static bool fn_80061CAC(Store* pStore) {
    pStore->unk_B9 = 0;
    pStore->unk_B8 = 0;

    memcpy(pStore->unk_AC, (void*)pStore->unk_A8, pStore->unk_00);
    DCFlushRange(pStore->unk_AC, pStore->unk_00);
    NANDSetUserData(&pStore->nandCmdBlock, &pStore->unk_00);

    while (true) {
        if (!unknownInline(pStore, 3)) {
            return false;
        }

        if (pStore->unk_A4 == 0) {
            return true;
        }

        if (NANDWriteAsync(&pStore->nandFileInfo, pStore->unk_AC, pStore->unk_00, fn_80061C4C, &pStore->nandCmdBlock) >=
            0) {
            break;
        }

        fn_80064600(&pStore->nandFileInfo, 3);
    }

    pStore->unk_BA = 0;
    return true;
}

static inline void fn_80061DB8_Inline(Store* pStore) {
    bool interrupts;
    u8 unk_B9;

    interrupts = OSDisableInterrupts();
    unk_B9 = pStore->unk_B9;
    OSRestoreInterrupts(interrupts);

    if (unk_B9 == 1 && pStore->unk_BC != pStore->unk_00) {
        pStore->unk_B8 = 1;
    }

    if (pStore->unk_B8 != 0 && unk_B9 == 1) {
        fn_80061CAC(pStore);
    }
}

bool fn_80061DB8(void) {
    Flash* pFlash;
    Sram* pSram;
    EEPROM* pEEPROM;
    bool interrupts;
    u8 unk_B9;
    Store* pStore;

    pSram = SYSTEM_SRAM(gpSystem);
    pEEPROM = SYSTEM_EEPROM(gpSystem);
    pFlash = SYSTEM_FLASH(gpSystem);

    if (pFlash != NULL) {
        if (pFlash->pStore != NULL) {
            pStore = pFlash->pStore;

            interrupts = OSDisableInterrupts();
            unk_B9 = pStore->unk_B9;
            OSRestoreInterrupts(interrupts);

            if (unk_B9 == 1 && pStore->unk_BC != pStore->unk_00) {
                pStore->unk_B8 = 1;
            }

            if (pStore->unk_B8 != 0 && unk_B9 == 1) {
                fn_80061CAC(pStore);
            }
        }
    }

    if (pSram != NULL) {
        if (pSram->pStore != NULL) {
            fn_80061DB8_Inline(pSram->pStore);
        }
    }

    if (pEEPROM != NULL) {
        //! TODO: possible bug? it's using `pSram` instead of `pEEPROM` there
        if (pSram->pStore != NULL) {
            fn_80061DB8_Inline(pSram->pStore);
        }
    }

    return true;
}

bool storeEvent(Store* pStore, s32 nEvent, void* pArgument) {
    switch (nEvent) {
        case 2:
            if (!xlHeapTake((void**)&pStore->unk_9C, 0x20 | 0x30000000)) {
                return false;
            }
            break;
        case 3:
            if (!xlHeapFree((void**)&pStore->unk_9C)) {
                return false;
            }
        case 0:
        case 1:
        case 5:
        case 6:
            break;
        default:
            return false;
    }

    return true;
}
