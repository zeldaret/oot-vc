#ifndef _METROTRK_TARGIMPL_H
#define _METROTRK_TARGIMPL_H

#include "metrotrk/msgbuf.h"
#include "metrotrk/nubevent.h"
#include "metrotrk/trk.h"
#include "revolution/types.h"

typedef enum ValidMemoryOptions {
    kValidMemoryReadable = 0,
    kValidMemoryWriteable = 1
} ValidMemoryOptions;

typedef enum MemoryAccessOptions {
    kUserMemory = 0,
    kDebuggerMemory = 1
} MemoryAccessOptions;

typedef struct DSVersions {
    /* 0x00 */ u8 kernelMajor;
    /* 0x01 */ u8 kernelMinor;
    /* 0x02 */ u8 protocolMajor;
    /* 0x03 */ u8 protocolMinor;
} DSVersions; // size = 0x4

typedef struct DSCPUType {
    /* 0x00 */ u8 cpuMajor;
    /* 0x01 */ u8 cpuMinor;
    /* 0x02 */ u8 bigEndian;
    /* 0x03 */ u8 defaultTypeSize;
    /* 0x04 */ u8 fpTypeSize;
    /* 0x05 */ u8 extended1TypeSize;
    /* 0x06 */ u8 extended2TypeSize;
} DSCPUType; // size = 0x7

void TRKTargetSetStopped(bool);
void TRKTargetSetInputPendingPtr(void*);

DSError TRKTargetAccessMemory(void*, void*, size_t*, MemoryAccessOptions, bool);
DSError TRKTargetAccessDefault(u32, u32, MessageBuffer*, size_t*, bool);
DSError TRKTargetAccessFP(u32 firstRegister, u32 lastRegister, MessageBuffer* b, size_t* registerStorageSize, s32 read);
DSError TRKTargetAccessExtended1(u32 firstRegister, u32 lastRegister, MessageBuffer* b, size_t* registerStorageSize,
                                 s32 read);
DSError TRKTargetAccessExtended2(u32, u32, MessageBuffer*, size_t*, bool);

DSError TRKTargetVersions(DSVersions*);
DSError TRKTargetSupportMask(DSSupportMask*);
// DSError TRKTargetCPUType(DSCPUType*);
DSError TRKTargetCheckException(void);
DSError TRKInitializeTarget(void);
DSError TRKTargetContinue(void);

void TRKSwapAndGo(void);

DSError TRKTargetInterrupt(NubEvent*);

DSError TRKTargetAddStopInfo(MessageBuffer*);
void TRKTargetAddExceptionInfo(MessageBuffer*);

DSError TRKTargetSingleStep(u32, bool);
DSError TRKTargetStepOutOfRange(u32, u32, bool);

u32 TRKTargetGetPC(void);

DSError TRKTargetSupportRequest(void);
DSError TRKTargetFlushCache(u8, void*, void*);

bool TRKTargetStopped(void);
void TRKTargetSetStopped(bool);
DSError TRKTargetStop(void);

void* TRKTargetTranslate(u32*);

void TRK_InterruptHandler(u16);

#if TRK_TRANSPORT_INT_DRIVEN
void TRKTargetSetInputPendingPtr(void*);
#endif

/*
** Define aliases for the functions which have both OS and non-OS
** variants. Each alias references the variant which is appropriate
** for the current protocol level.
*/
#if DS_PROTOCOL < DS_PROTOCOL_RTOS

#define XTargetAccessMemory TRKTargetAccessMemory
#define XTargetAccessDefault TRKTargetAccessDefault
#define XTargetAccessFP TRKTargetAccessFP
#define XTargetAccessExtended1 TRKTargetAccessExtended1
#define XTargetAccessExtended2 TRKTargetAccessExtended2
#define XTargetContinue TRKTargetContinue
#define XTargetSingleStep TRKTargetSingleStep
#define XTargetStepOutOfRange TRKTargetStepOutOfRange
#define XTargetGetPC TRKTargetGetPC
#define XTargetStopped TRKTargetStopped
#define XTargetStop TRKTargetStop
#define XTargetAddStopInfo TRKTargetAddStopInfo
#define XTargetAddExceptionInfo TRKTargetAddExceptionInfo

#else /* #if DS_PROTOCOL < DS_PROTOCOL_RTOS */

#define XTargetAccessMemory OsTargetAccessMemory
#define XTargetAccessDefault OsTargetAccessDefault
#define XTargetAccessFP OsTargetAccessFP
#define XTargetAccessExtended1 OsTargetAccessExtended1
#define XTargetAccessExtended2 OsTargetAccessExtended2
#define XTargetContinue OsTargetContinue
#define XTargetSingleStep OsTargetSingleStep
#define XTargetStepOutOfRange OsTargetStepOutOfRange
#define XTargetGetPC OsTargetGetPC
#define XTargetStopped OsTargetStopped
#define XTargetStop OsTargetStop
#define XTargetAddStopInfo OsTargetAddStopInfo
#define XTargetAddExceptionInfo OsTargetAddExceptionInfo

#endif /* #if DS_PROTOCOL < DS_PROTOCOL_RTOS */

#endif
