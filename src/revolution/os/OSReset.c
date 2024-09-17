#include "revolution/dvd.h"
#include "revolution/os.h"
#include "revolution/pad.h"
#include "revolution/sc.h"
#include "revolution/vi.h"
#include "string.h"

static OSShutdownFunctionQueue ShutdownFunctionQueue;
static u32 bootThisDol = 0;

static inline void KillThreads(void) {
    OSThread* iter;
    OSThread* next;

    for (iter = OS_THREAD_QUEUE.head; iter != NULL; iter = next) {
        next = iter->nextActive;

        switch (iter->state) {
            case OS_THREAD_STATE_SLEEPING:
            case OS_THREAD_STATE_READY:
                OSCancelThread(iter);
                break;
        }
    }
}

void OSRegisterShutdownFunction(OSShutdownFunctionInfo* info) {
    OSShutdownFunctionInfo* it;
    OSShutdownFunctionInfo* prev;
    OSShutdownFunctionInfo* tail;

    for (it = ShutdownFunctionQueue.head; it != NULL && it->prio <= info->prio; it = it->next) {
        ;
    }

    if (it == NULL) {

        tail = ShutdownFunctionQueue.tail;
        if (tail == NULL) {
            ShutdownFunctionQueue.head = info;
        } else {
            ShutdownFunctionQueue.tail->next = info;
        }

        info->prev = tail;
        info->next = NULL;

        it = ShutdownFunctionQueue.head;
        ShutdownFunctionQueue.tail = info;
    } else {
        info->next = it;

        prev = it->prev;
        it->prev = info;
        info->prev = prev;

        if (prev == NULL) {
            ShutdownFunctionQueue.head = info;
        } else {
            prev->next = info;
        }
    }
}

bool __OSCallShutdownFunctions(u32 pass, u32 event) {
    OSShutdownFunctionInfo* iter;
    bool failure;
    u32 prio;

    prio = 0;
    failure = false;

    for (iter = ShutdownFunctionQueue.head; iter != NULL; iter = iter->next) {
        if (failure && prio != iter->prio) {
            break;
        }

        failure |= !iter->func(pass, event);
        prio = iter->prio;
    }

    failure |= !__OSSyncSram();

    return !failure;
}

void __OSShutdownDevices(u32 event) {
    bool padIntr;
    bool osIntr;
    bool keepEnable;

    switch (event) {
        case 0:
        case OS_SD_EVENT_RESTART:
        case OS_SD_EVENT_RETURN_TO_MENU:
        case OS_SD_EVENT_LAUNCH_APP:
            keepEnable = false;
            break;
        case 1:
        case OS_SD_EVENT_SHUTDOWN:
        case 3:
        default:
            keepEnable = true;
            break;
    }

    __OSStopAudioSystem();

    if (!keepEnable) {
        padIntr = __PADDisableRecalibration(true);
    }

    while (!__OSCallShutdownFunctions(false, event)) {
        ;
    }

    while (!__OSSyncSram()) {
        ;
    }

    osIntr = OSDisableInterrupts();
    __OSCallShutdownFunctions(true, event);
    LCDisable();

    if (!keepEnable) {
        __PADDisableRecalibration(padIntr);
    }

    KillThreads();
}

static inline void __OSGetDiscState(u8* out) {
    u32 flags;

    if (__DVDGetCoverStatus() != DVD_COVER_CLOSED) {
        *out = 3;
    } else {
        __OSGetRTCFlags(&flags);
        if (flags != 0) {
            *out = 2;
        } else {
            *out = 1;
        }
    }
}

void OSShutdownSystem(void) {
    SCIdleModeInfo idleMode;
    OSStateFlags stateFlags;
    OSIOSRev iosRev;

    memset(&idleMode, 0, sizeof(SCIdleModeInfo));
    SCInit();
    while (SCCheckStatus() == SC_STATUS_BUSY) {
        ;
    }
    SCGetIdleMode(&idleMode);

    __OSStopPlayRecord();
    __OSUnRegisterStateEvent();
    __DVDPrepareReset();
    __OSReadStateFlags(&stateFlags);

    __OSGetDiscState(&stateFlags.discState);
    if (idleMode.wc24 == true) {
        stateFlags.BYTE_0x5 = 5;
    } else {
        stateFlags.BYTE_0x5 = 1;
    }

    __OSClearRTCFlags();
    __OSWriteStateFlags(&stateFlags);
    __OSGetIOSRev(&iosRev);

    if (idleMode.wc24 == true) {
        OSDisableScheduler();
        __OSShutdownDevices(OS_SD_EVENT_RETURN_TO_MENU);
        OSEnableScheduler();
        __OSLaunchMenu();
    } else {
        OSDisableScheduler();
        __OSShutdownDevices(OS_SD_EVENT_SHUTDOWN);
        __OSShutdownToSBY();
    }
}

void OSRestart(u32 resetCode) {
    u8 type = OSGetAppType();

    __OSStopPlayRecord();
    __OSUnRegisterStateEvent();

    if (type == 0x81) {
        OSDisableScheduler();
        __OSShutdownDevices(4);
        OSEnableScheduler();
        __OSRelaunchTitle();
    } else if (type == 0x80) {
        OSDisableScheduler();
        __OSShutdownDevices(4);
        OSEnableScheduler();
        __OSReboot(resetCode, bootThisDol);
    }

    OSDisableScheduler();
    __OSShutdownDevices(1);
    __OSHotReset();
}

void OSReturnToMenu(void) {
    OSStateFlags stateFlags;

    __OSStopPlayRecord();
    __OSUnRegisterStateEvent();
    __DVDPrepareReset();

    __OSReadStateFlags(&stateFlags);
    __OSGetDiscState(&stateFlags.discState);
    stateFlags.BYTE_0x5 = 3;
    __OSClearRTCFlags();
    __OSWriteStateFlags(&stateFlags);

    OSDisableScheduler();
    __OSShutdownDevices(OS_SD_EVENT_RETURN_TO_MENU);
    OSEnableScheduler();

    __OSLaunchMenu();

    OSPanic("OSReset.c", 783, "OSReturnToMenu(): Falied to boot system menu.\n");
}

u32 OSGetResetCode(void) {
    if (__OSRebootParams.valid != 0) {
        return __OSRebootParams.restartCode | 0x80000000;
    }

    return PI_HW_REGS[PI_RESET] >> 3;
}

void OSResetSystem(u32 arg0, u32 arg1, u32 arg2) {
#pragma unused(arg0)
#pragma unused(arg1)
#pragma unused(arg2)

    OSPanic("OSReset.c", 935, "OSResetSystem() is obsoleted. It doesn't work any longer.\n");
}
