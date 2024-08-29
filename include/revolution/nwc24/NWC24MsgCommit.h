#ifndef _RVL_SDK_NWC24_MSG_COMMIT_H
#define _RVL_SDK_NWC24_MSG_COMMIT_H

#include "revolution/nwc24/NWC24Types.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct NWC24MsgObj;

NWC24Err NWC24CommitMsg(struct NWC24MsgObj*);

#ifdef __cplusplus
}
#endif

#endif
