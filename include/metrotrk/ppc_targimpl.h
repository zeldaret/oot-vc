#ifndef METROTRK_PPC_TARGIMPL
#define METROTRK_PPC_TARGIMPL

#include "metrotrk/m7xx_m603e_reg.h"
#include "metrotrk/ppc_reg.h"
#include "metrotrk/trk.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TRKState_PPC {
    DefaultType GPR[32]; // 0x0
    DefaultType LR; // 0x80
    DefaultType CTR; // 0x84
    DefaultType XER; // 0x88
    Extended1Type MSR; // 0x8c
    Extended1Type DAR; // 0x90
    Extended1Type DSISR; // 0x94
    bool stopped; // 0x98
    bool inputActivated; // 0x9c
    u8* inputPendingPtr; // 0xA0
} TRKState_PPC;

extern TRKState_PPC gTRKState;

typedef struct ProcessorRestoreFlags_PPC {
    u8 TBR;
    u8 DEC;
    u8 linker_padding[0x9 - 0x2];
} ProcessorRestoreFlags_PPC;

extern ProcessorRestoreFlags_PPC gTRKRestoreFlags;
extern ProcessorState_PPC gTRKCPUState;

u32 __TRK_get_MSR();
void __TRK_set_MSR(register u32 val);
u32 __TRK_get_PVR();
u32 __TRK_get_IBAT0U();
u32 __TRK_get_IBAT0L();
u32 __TRK_get_IBAT1U();
u32 __TRK_get_IBAT1L();
u32 __TRK_get_IBAT2U();
u32 __TRK_get_IBAT2L();
u32 __TRK_get_IBAT3U();
u32 __TRK_get_IBAT3L();
u32 __TRK_get_DBAT0U();
u32 __TRK_get_DBAT0L();
u32 __TRK_get_DBAT1U();
u32 __TRK_get_DBAT1L();
u32 __TRK_get_DBAT2U();
u32 __TRK_get_DBAT2L();
u32 __TRK_get_DBAT3U();
u32 __TRK_get_DBAT3L();

DSError TRKPPCAccessSPR(void* srcDestPtr, u32 spr, bool read);
DSError TRKPPCAccessPairedSingleRegister(void* srcDestPtr, u32 psr, bool read);
DSError TRKPPCAccessFPRegister(void* srcDestPtr, u32 fpr, bool read);
DSError TRKPPCAccessSpecialReg(void* srcDestPtr, u32* instructionData, bool read);
void TRKPostInterruptEvent();
u32 ConvertAddress(u32);

#ifdef __cplusplus
}
#endif

#endif
