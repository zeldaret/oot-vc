#include "revolution/dsp.h"
#include "revolution/os.h"

const char* __DSPVersion = "<< RVL_SDK - DSP \trelease build: Sep 20 2006 22:25:51 (0x4200_60422) >>";

static bool __DSP_init_flag = false;

bool DSPCheckMailToDSP(void) { return (DSP_HW_REGS[DSP_DSPMBOX_H] & DSP_DSPMBOX_H_STATUS) != 0; }

bool DSPCheckMailFromDSP(void) { return (DSP_HW_REGS[DSP_CPUMBOX_H] & DSP_CPUMBOX_H_STATUS) != 0; }

DSPMail DSPReadMailFromDSP(void) { return (DSPMail)(DSP_HW_REGS[DSP_CPUMBOX_H] << 16 | DSP_HW_REGS[DSP_CPUMBOX_L]); }

void DSPSendMailToDSP(DSPMail mail) {
    DSP_HW_REGS[DSP_DSPMBOX_H] = ((uintptr_t)mail) >> 16 & 0xFFFF;
    DSP_HW_REGS[DSP_DSPMBOX_L] = ((uintptr_t)mail) & 0xFFFF;
}

static inline void DSPAssertInt(void) {
    bool enabled;

    enabled = OSDisableInterrupts();

    DSP_HW_REGS[DSP_CSR] = (DSP_HW_REGS[DSP_CSR] & ~(DSP_CSR_AIDINT | DSP_CSR_ARINT | DSP_CSR_DSPINT)) | DSP_CSR_PIINT;

    OSRestoreInterrupts(enabled);
}

void DSPInit(void) {
    bool enabled;

    __DSP_debug_printf("DSPInit(): Build Date: %s %s\n", "Sep 20 2006", "22:25:51");

    if (__DSP_init_flag == true) {
        return;
    }

    OSRegisterVersion(__DSPVersion);

    enabled = OSDisableInterrupts();

    __OSSetInterruptHandler(OS_INTR_DSP_DSP, __DSPHandler);
    __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_DSP_DSP));

    DSP_HW_REGS[DSP_CSR] = (DSP_HW_REGS[DSP_CSR] & ~(DSP_CSR_AIDINT | DSP_CSR_ARINT | DSP_CSR_DSPINT)) | DSP_CSR_RES;

    DSP_HW_REGS[DSP_CSR] &= ~(DSP_CSR_HALT | DSP_CSR_AIDINT | DSP_CSR_ARINT | DSP_CSR_DSPINT);

    __DSP_tmp_task = NULL;
    __DSP_curr_task = NULL;
    __DSP_last_task = NULL;
    __DSP_first_task = NULL;

    __DSP_init_flag = true;

    OSRestoreInterrupts(enabled);
}

bool DSPCheckInit(void) { return __DSP_init_flag; }

DSPTask* DSPAddTask(DSPTask* task) {
    bool enabled;

    enabled = OSDisableInterrupts();

    __DSP_insert_task(task);
    task->state = DSP_TASK_STATE_0;
    task->flags = DSP_TASK_ACTIVE;

    OSRestoreInterrupts(enabled);

    if (task == __DSP_first_task) {
        __DSP_boot_task(task);
    }

    return task;
}

DSPTask* DSPCancelTask(DSPTask* task) {
    bool old;

    old = OSDisableInterrupts();

    task->flags |= 2;

    OSRestoreInterrupts(old);
    return task;
}

DSPTask* DSPAssertTask(DSPTask* task) {
    bool enabled;

    enabled = OSDisableInterrupts();

    if (__DSP_curr_task == task) {
        __DSP_rude_task_pending = true;
        __DSP_rude_task = task;
        OSRestoreInterrupts(enabled);
        return task;
    }

    if (task->prio < __DSP_curr_task->prio) {
        __DSP_rude_task_pending = true;
        __DSP_rude_task = task;

        if (__DSP_curr_task->state == DSP_TASK_STATE_1) {
            DSPAssertInt();
        }

        OSRestoreInterrupts(enabled);
        return task;
    }

    OSRestoreInterrupts(enabled);
    return NULL;
}
