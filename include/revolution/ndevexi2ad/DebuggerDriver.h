#ifndef _RVL_SDK_NDEV_EXI2_AD_DEBUGGER_DRIVER_H
#define _RVL_SDK_NDEV_EXI2_AD_DEBUGGER_DRIVER_H

#include "revolution/ndevexi2ad/exi2.h"
#include "revolution/os.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void DBInitComm(u8** flagOut, OSInterruptHandler handler);
void DBInitInterrupts(void);
u32 DBQueryData(void);
bool DBRead(void* dst, u32 size);
bool DBWrite(const void* src, u32 size);
void DBOpen(void);
void DBClose(void);

static inline bool __DBReadMailbox(u32* mailOut) { return __DBEXIReadReg(0x34000200, mailOut, sizeof(*mailOut)); }

static inline bool __DBRead(u32 ofs, void* dest, u32 size) {
    return __DBEXIReadRam(((ofs + 0xD10000) * 0x40) & 0x3FFFFF00, dest, size);
}

static inline bool __DBWriteMailbox(u32 mail) { return __DBEXIWriteReg(0xB4000100, &mail, sizeof(mail)); }

static inline bool __DBWrite(u32 ofs, const void* src, u32 size) {
    return __DBEXIWriteRam((((ofs + 0xD10000) * 0x40) & 0x3FFFFF00) | 0x80000000, src, size);
}

#ifdef __cplusplus
}
#endif

#endif
