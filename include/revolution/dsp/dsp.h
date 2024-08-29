#ifndef _RVL_SDK_DSP_H
#define _RVL_SDK_DSP_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct DSPTask;

// General-purpose typedef
typedef void* DSPMail;

bool DSPCheckMailToDSP(void);
bool DSPCheckMailFromDSP(void);
DSPMail DSPReadMailFromDSP(void);
void DSPSendMailToDSP(DSPMail mail);
void DSPAssertInt(void);
void DSPInit(void);
bool DSPCheckInit(void);
struct DSPTask* DSPAddTask(struct DSPTask* task);
struct DSPTask* DSPCancelTask(struct DSPTask* task);
struct DSPTask* DSPAssertTask(struct DSPTask* task);

#ifdef __cplusplus
}
#endif

#endif
