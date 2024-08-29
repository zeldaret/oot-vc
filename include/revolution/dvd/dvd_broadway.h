#ifndef _RVL_SDK_DVD_BROADWAY_H
#define _RVL_SDK_DVD_BROADWAY_H

#include "revolution/os/OSAlarm.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DVD_LOW_OFFSET(x) ((x) >> 2)
#define DVD_LOW_SPEED(x) (((x) & 3) << 16)

// Forward declarations
typedef struct DVDDiskID;
typedef struct DVDDriveInfo;
typedef struct ESPTicket;
typedef struct ESPTmd;

typedef enum {
    DVD_INTTYPE_TC = (1 << 0), // Transaction callback?
    DVD_INTTYPE_DE = (1 << 1), // Drive error
    DVD_INTTYPE_CVR = (1 << 2), // Something with DVD cover
    DVD_INTTYPE_BR = (1 << 3), // Break requested
    DVD_INTTYPE_TIME = (1 << 4), // Time out
    DVD_INTTYPE_SERR = (1 << 5), // Security error
    DVD_INTTYPE_VERR = (1 << 6), // Verify error
    DVD_INTTYPE_ARGS = (1 << 7), // Bad arguments
} DVDLowIntType;

// DICVR - DI Cover Register (via DVDLowGetCoverRegister)
#define DVD_DICVR_CVR (1 << 0)
#define DVD_DICVR_CVRINTMASK (1 << 1)
#define DVD_DICVR_CVRINT (1 << 2)

typedef void (*DVDLowCallback)(u32 intType);

bool DVDLowInit(void);
bool DVDLowReadDiskID(struct DVDDiskID* out, DVDLowCallback callback);
bool DVDLowOpenPartition(u32 offset, const struct ESPTicket* ticket, u32 certsSize, const void* certs,
                         struct ESPTmd* tmd, DVDLowCallback callback);
bool DVDLowClosePartition(DVDLowCallback callback);
bool DVDLowUnencryptedRead(void* dst, u32 size, u32 offset, DVDLowCallback callback);
bool DVDLowStopMotor(bool eject, bool kill, DVDLowCallback callback);
bool DVDLowInquiry(struct DVDDriveInfo* out, DVDLowCallback callback);
bool DVDLowRequestError(DVDLowCallback callback);
bool DVDLowSetSpinupFlag(bool enable);
bool DVDLowReset(DVDLowCallback callback);
bool DVDLowAudioBufferConfig(bool enable, u32 size, DVDLowCallback callback);
bool DVDLowSetMaximumRotation(u32 speed, DVDLowCallback callback);
bool DVDLowRead(void* dst, u32 size, u32 offset, DVDLowCallback callback);
bool DVDLowSeek(u32 offset, DVDLowCallback callback);
u32 DVDLowGetCoverRegister(void);
bool DVDLowPrepareCoverRegister(DVDLowCallback callback);
u32 DVDLowGetImmBufferReg(void);
bool DVDLowUnmaskStatusInterrupts(void);
bool DVDLowMaskCoverInterrupt(void);
bool DVDLowClearCoverInterrupt(DVDLowCallback callback);
bool __DVDLowTestAlarm(const struct OSAlarm* alarm);

#ifdef __cplusplus
}
#endif

#endif
