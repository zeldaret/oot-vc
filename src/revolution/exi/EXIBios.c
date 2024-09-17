#include "revolution/dvd.h"
#include "revolution/exi.h"
#include "revolution/os.h"
#include "string.h"

const char* __EXIVersion = "<< RVL_SDK - EXI \trelease build: Sep  7 2006 07:16:20 (0x4200_60422) >>";

static EXIData Ecb[EXI_MAX_CHAN];
static u32 IDSerialPort1;

volatile s32 __EXIProbeStartTime[2] AT_ADDRESS(OS_BASE_CACHED | 0x30C0);

static bool __EXIProbe(EXIChannel chan);

static void SetExiInterruptMask(EXIChannel chan, EXIData* exi) {
    EXIData* exi2 = &Ecb[EXI_CHAN_2];

    switch (chan) {
        case EXI_CHAN_0:
            if (exi->exiCallback == NULL && exi2->exiCallback == NULL || exi->state & EXI_STATE_LOCKED) {
                __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_0_EXI) | OS_INTR_MASK(OS_INTR_EXI_2_EXI));
            } else {
                __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_0_EXI) | OS_INTR_MASK(OS_INTR_EXI_2_EXI));
            }
            break;
        case EXI_CHAN_1:
            if (exi->exiCallback == NULL || exi->state & EXI_STATE_LOCKED) {
                __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_1_EXI));
            } else {
                __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_1_EXI));
            }
            break;
        case EXI_CHAN_2:
            if (__OSGetInterruptHandler(OS_INTR_PI_DEBUG) == NULL || exi->state & EXI_STATE_LOCKED) {
                __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_PI_DEBUG));
            } else {
                __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_PI_DEBUG));
            }
    }
}

static inline void CompleteTransfer(EXIChannel chan) {
    EXIData* exi = &Ecb[chan];
    s32 len;

    if (exi->state & EXI_STATE_BUSY) {
        if (exi->state & EXI_STATE_IMM_ACCESS && (len = exi->bytesRead) != 0) {
            u8* buf;
            u32 data;
            int i;

            buf = (u8*)exi->buffer;
            data = EXI_CHAN_PARAMS[chan].data;
            for (i = 0; i < len; i++) {
                *buf++ = data >> (3 - i) * 8;
            }
        }

        exi->state &= ~EXI_STATE_BUSY;
    }
}

bool EXIImm(EXIChannel chan, void* buf, s32 len, u32 type, EXICallback callback) {
    EXIData* exi = &Ecb[chan];
    bool enabled = OSDisableInterrupts();

    if (exi->state & EXI_STATE_BUSY || !(exi->state & EXI_STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    exi->tcCallback = callback;
    if (callback != NULL) {
        u32 mask = OS_INTR_MASK(OS_INTR_EXI_0_TC);
        EXIClearInterrupts(chan, false, true, false);
        __OSUnmaskInterrupts(mask >> (chan * 3));
    }

    exi->state |= EXI_STATE_IMM_ACCESS;

    if (type != EXI_READ) {
        int i;
        u32 word = 0;
        for (i = 0; i < len; i++) {
            word |= ((u8*)buf)[i] << (3 - i) * 8;
        }

        EXI_CHAN_PARAMS[chan].data = word;
    }

    exi->buffer = buf;
    exi->bytesRead = (type != EXI_WRITE) ? len : 0;
    EXI_CHAN_PARAMS[chan].cr = EXI_CR_TSTART | type << 2 | (len - 1) << 4;

    OSRestoreInterrupts(enabled);
    return true;
}

bool EXIImmEx(EXIChannel chan, void* buf, s32 len, u32 type) {
    while (len != 0) {
        s32 immLen = (len < 4) ? len : 4;

        if (!EXIImm(chan, buf, immLen, type, false)) {
            return false;
        }

        if (!EXISync(chan)) {
            return false;
        }

        buf = (u8*)buf + immLen;
        len -= immLen;
    }

    return true;
}

bool EXIDma(EXIChannel chan, void* buf, s32 len, u32 type, EXICallback callback) {
    EXIData* exi = &Ecb[chan];
    bool enabled = OSDisableInterrupts();

    if (exi->state & EXI_STATE_BUSY || !(exi->state & EXI_STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    exi->tcCallback = callback;
    if (callback != NULL) {
        u32 mask = OS_INTR_MASK(OS_INTR_EXI_0_TC);
        EXIClearInterrupts(chan, false, true, false);
        __OSUnmaskInterrupts(mask >> (chan * 3));
    }

    exi->state |= EXI_STATE_DMA_ACCESS;

    EXI_CHAN_PARAMS[chan].mar = ROUND_DOWN_PTR(buf, 32);
    EXI_CHAN_PARAMS[chan].length = len;
    EXI_CHAN_PARAMS[chan].cr = type << 2 | EXI_CR_TSTART | EXI_CR_DMA;

    OSRestoreInterrupts(enabled);
    return true;
}

bool EXISync(EXIChannel chan) {
    EXIData* exi = &Ecb[chan];
    bool ret = false;

    while (exi->state & EXI_STATE_SELECTED) {
        if (!(EXI_CHAN_PARAMS[chan].cr & EXI_CR_TSTART)) {
            bool enabled = OSDisableInterrupts();

            if (exi->state & EXI_STATE_SELECTED) {
                CompleteTransfer(chan);

                if (__OSGetDIConfig() != 0xFF || (OSGetConsoleType() & OS_CONSOLE_MASK) == OS_CONSOLE_MASK_TDEV ||
                    exi->bytesRead != 4 || EXI_CHAN_PARAMS[chan].cpr & EXI_CPR_CLK ||
                    (EXI_CHAN_PARAMS[chan].data != EXI_ID_USB_ADAPTER &&
                     EXI_CHAN_PARAMS[chan].data != EXI_ID_IS_DOL_VIEWER && EXI_CHAN_PARAMS[chan].data != 0x04220001) ||
                    OS_DVD_DEVICE_CODE == MAKE_DVD_DEVICE_CODE(0x0200)) {
                    ret = true;
                }
            }

            OSRestoreInterrupts(enabled);
            break;
        }
    }

    return ret;
}

static inline void EXIClearInterrupts(EXIChannel chan, bool exi, bool tc, bool ext) {
    u32 cpr = EXI_CHAN_PARAMS[chan].cpr & 0xFFF & ~(EXI_CPR_EXIINT | EXI_CPR_TCINT | EXI_CPR_EXTINT);

    if (exi) {
        cpr |= EXI_CPR_EXIINT;
    }

    if (tc) {
        cpr |= EXI_CPR_TCINT;
    }

    if (ext) {
        cpr |= EXI_CPR_EXTINT;
    }

    EXI_CHAN_PARAMS[chan].cpr = cpr;
}

EXICallback EXISetExiCallback(EXIChannel chan, EXICallback callback) {
    EXIData* exi;
    EXICallback old;
    bool enabled;

    exi = &Ecb[chan];
    enabled = OSDisableInterrupts();

    old = exi->exiCallback;
    exi->exiCallback = callback;

    if (chan != EXI_CHAN_2) {
        SetExiInterruptMask(chan, exi);
    } else {
        SetExiInterruptMask(EXI_CHAN_0, &Ecb[EXI_CHAN_0]);
    }

    OSRestoreInterrupts(enabled);
    return old;
}

static inline void EXIProbeReset(void) {
    __EXIProbeStartTime[0] = __EXIProbeStartTime[1] = 0;
    Ecb[EXI_CHAN_0].lastInsert = Ecb[EXI_CHAN_1].lastInsert = 0;
    __EXIProbe(EXI_CHAN_0);
    __EXIProbe(EXI_CHAN_1);
}

static bool __EXIProbe(EXIChannel chan) {
    EXIData* exi = &Ecb[chan];
    bool enabled;
    bool ret;
    u32 cpr;

    if (chan == EXI_CHAN_2) {
        return true;
    }

    ret = true;
    enabled = OSDisableInterrupts();
    cpr = EXI_CHAN_PARAMS[chan].cpr;

    if (!(exi->state & EXI_STATE_ATTACHED)) {
        if (cpr & EXI_CPR_EXTINT) {
            EXIClearInterrupts(chan, false, false, true);
            exi->lastInsert = 0;
            __EXIProbeStartTime[chan] = 0;
        }

        if (cpr & EXI_CPR_EXT) {
            s32 time = (s32)(OS_TICKS_TO_MSEC(OSGetTime()) / 100) + 1;
            if (__EXIProbeStartTime[chan] == 0) {
                __EXIProbeStartTime[chan] = time;
            }

            if (time - __EXIProbeStartTime[chan] < 3) {
                ret = false;
            }
        } else {
            exi->lastInsert = 0;
            __EXIProbeStartTime[chan] = 0;
            ret = false;
        }
    } else if (!(cpr & EXI_CPR_EXT) || cpr & EXI_CPR_EXTINT) {
        exi->lastInsert = 0;
        __EXIProbeStartTime[chan] = 0;
        ret = false;
    }

    OSRestoreInterrupts(enabled);
    return ret;
}

static inline bool EXIProbe(EXIChannel chan) {
    EXIData* exi = &Ecb[chan];

    bool ret = __EXIProbe(chan);
    if (ret && exi->lastInsert == 0) {
        u32 id;
        return EXIGetID(chan, EXI_DEV_EXT, &id);
    }

    return false;
}

static inline bool __EXIAttach(EXIChannel chan, EXICallback callback) {
    EXIData* exi = &Ecb[chan];
    bool enabled;
    u32 mask;

    enabled = OSDisableInterrupts();
    if (exi->state & EXI_STATE_ATTACHED || !__EXIProbe(chan)) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    EXIClearInterrupts(chan, true, false, false);
    exi->extCallback = callback;
    mask = OS_INTR_MASK(OS_INTR_EXI_0_EXT);
    __OSUnmaskInterrupts(mask >> (chan * 3));
    exi->state |= EXI_STATE_ATTACHED;
    OSRestoreInterrupts(enabled);
    return true;
}

bool EXIAttach(EXIChannel chan, EXICallback callback) {
    EXIData* exi = &Ecb[chan];
    bool enabled;
    bool ret;

    EXIProbe(chan);
    enabled = OSDisableInterrupts();

    if (exi->lastInsert == 0) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    ret = __EXIAttach(chan, callback);
    OSRestoreInterrupts(enabled);
    return ret;
}

bool EXIDetach(EXIChannel chan) {
    EXIData* exi = &Ecb[chan];
    bool enabled;
    u32 mask;

    enabled = OSDisableInterrupts();
    if (!(exi->state & EXI_STATE_ATTACHED)) {
        OSRestoreInterrupts(enabled);
        return true;
    }

    if (exi->state & EXI_STATE_LOCKED && exi->dev == EXI_DEV_EXT) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    exi->state &= ~EXI_STATE_ATTACHED;
    mask = OS_INTR_MASK(OS_INTR_EXI_0_EXI) | OS_INTR_MASK(OS_INTR_EXI_0_EXT);
    __OSMaskInterrupts(mask >> (chan * 3));
    OSRestoreInterrupts(enabled);
    return true;
}

bool EXISelect(EXIChannel chan, u32 dev, u32 freq) {
    EXIData* exi = &Ecb[chan];
    bool enabled;
    u32 cpr;

    enabled = OSDisableInterrupts();

    if (exi->state & EXI_STATE_SELECTED ||
        chan != EXI_CHAN_2 && (dev == EXI_DEV_EXT && !(exi->state & EXI_STATE_ATTACHED) && !__EXIProbe(chan) ||
                               !(exi->state & EXI_STATE_LOCKED) || exi->dev != dev)) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    exi->state |= EXI_STATE_SELECTED;

    cpr = EXI_CHAN_PARAMS[chan].cpr;
    cpr &= (EXI_CPR_EXIINTMASK | EXI_CPR_TCINTMASK | EXI_CPR_EXTINTMASK);
    cpr |= (1 << dev) << 7 | freq << 4;
    EXI_CHAN_PARAMS[chan].cpr = cpr;

    if (exi->state & EXI_STATE_ATTACHED) {
        switch (chan) {
            case EXI_CHAN_0:
                __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_0_EXT));
                break;
            case EXI_CHAN_1:
                __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_1_EXT));
                break;
        }
    }

    OSRestoreInterrupts(enabled);
    return true;
}

bool EXIDeselect(EXIChannel chan) {
    EXIData* exi = &Ecb[chan];
    u32 cpr;
    bool enabled;

    enabled = OSDisableInterrupts();

    if (!(exi->state & EXI_STATE_SELECTED)) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    exi->state &= ~EXI_STATE_SELECTED;
    cpr = EXI_CHAN_PARAMS[chan].cpr;
    EXI_CHAN_PARAMS[chan].cpr = cpr & (EXI_CPR_EXIINTMASK | EXI_CPR_TCINTMASK | EXI_CPR_EXTINTMASK);

    if (exi->state & EXI_STATE_ATTACHED) {
        switch (chan) {
            case EXI_CHAN_0:
                __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_0_EXT));
                break;
            case EXI_CHAN_1:
                __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_1_EXT));
                break;
        }
    }

    OSRestoreInterrupts(enabled);

    if (chan != EXI_CHAN_2 && cpr & EXI_CPR_CS0B) {
        return __EXIProbe(chan) != false;
    }

    return true;
}

static void EXIIntrruptHandler(__OSInterrupt intr, OSContext* ctx) {
    EXIData* exi;
    EXIChannel chan;
    EXICallback callback;
    OSContext temp;

    chan = (EXIChannel)((intr - OS_INTR_EXI_0_EXI) / 3);
    exi = &Ecb[chan];

    EXIClearInterrupts(chan, true, false, false);

    callback = exi->exiCallback;
    if (callback != NULL) {
        OSClearContext(&temp);
        OSSetCurrentContext(&temp);
        callback(chan, ctx);
        OSClearContext(&temp);
        OSSetCurrentContext(ctx);
    }
}

static void TCIntrruptHandler(__OSInterrupt intr, OSContext* ctx) {
    EXIData* exi;
    EXIChannel chan;
    EXICallback callback;
    OSContext temp;

    chan = (EXIChannel)((intr - OS_INTR_EXI_0_TC) / 3);
    exi = &Ecb[chan];

    __OSMaskInterrupts(0x80000000 >> intr);
    EXIClearInterrupts(chan, false, true, false);

    callback = exi->tcCallback;
    if (callback != NULL) {
        exi->tcCallback = NULL;
        CompleteTransfer(chan);
        OSClearContext(&temp);
        OSSetCurrentContext(&temp);
        callback(chan, ctx);
        OSClearContext(&temp);
        OSSetCurrentContext(ctx);
    }
}

static void EXTIntrruptHandler(__OSInterrupt intr, OSContext* ctx) {
    EXIChannel chan;
    EXIData* exi;
    EXICallback callback;
    OSContext temp;
    u32 mask;

    chan = (EXIChannel)((intr - OS_INTR_EXI_0_EXT) / 3);
    mask = OS_INTR_MASK(OS_INTR_EXI_0_EXI) | OS_INTR_MASK(OS_INTR_EXI_0_EXT);
    __OSMaskInterrupts(mask >> (chan * 3));

    exi = &Ecb[chan];
    callback = exi->extCallback;
    exi->state &= ~EXI_STATE_ATTACHED;

    if (callback != NULL) {
        OSClearContext(&temp);
        OSSetCurrentContext(&temp);
        exi->extCallback = NULL;
        callback(chan, ctx);
        OSClearContext(&temp);
        OSSetCurrentContext(ctx);
    }
}

void EXIInit(void) {
    u32 id;

    do {
        do {
            ;
        } while ((EXI_CHAN_PARAMS[EXI_CHAN_0].cr & EXI_CR_TSTART) == 1);
    } while ((EXI_CHAN_PARAMS[EXI_CHAN_1].cr & EXI_CR_TSTART) == 1 ||
             (EXI_CHAN_PARAMS[EXI_CHAN_2].cr & EXI_CR_TSTART) == 1);

    __OSMaskInterrupts(OS_INTR_MASK(OS_INTR_EXI_0_EXI) | OS_INTR_MASK(OS_INTR_EXI_0_TC) |
                       OS_INTR_MASK(OS_INTR_EXI_0_EXT) | OS_INTR_MASK(OS_INTR_EXI_1_EXI) |
                       OS_INTR_MASK(OS_INTR_EXI_1_TC) | OS_INTR_MASK(OS_INTR_EXI_1_EXT) |
                       OS_INTR_MASK(OS_INTR_EXI_2_EXI) | OS_INTR_MASK(OS_INTR_EXI_2_TC));

    EXI_CHAN_PARAMS[EXI_CHAN_0].cpr = 0;
    EXI_CHAN_PARAMS[EXI_CHAN_1].cpr = 0;
    EXI_CHAN_PARAMS[EXI_CHAN_2].cpr = 0;

    EXI_CHAN_PARAMS[EXI_CHAN_0].cpr = EXI_CPR_ROMDIS;

    __OSSetInterruptHandler(OS_INTR_EXI_0_EXI, EXIIntrruptHandler);
    __OSSetInterruptHandler(OS_INTR_EXI_0_TC, TCIntrruptHandler);
    __OSSetInterruptHandler(OS_INTR_EXI_0_EXT, EXTIntrruptHandler);
    __OSSetInterruptHandler(OS_INTR_EXI_1_EXI, EXIIntrruptHandler);
    __OSSetInterruptHandler(OS_INTR_EXI_1_TC, TCIntrruptHandler);
    __OSSetInterruptHandler(OS_INTR_EXI_1_EXT, EXTIntrruptHandler);
    __OSSetInterruptHandler(OS_INTR_EXI_2_EXI, EXIIntrruptHandler);
    __OSSetInterruptHandler(OS_INTR_EXI_2_TC, TCIntrruptHandler);

    EXIGetID(EXI_CHAN_0, EXI_DEV_NET, &IDSerialPort1);

    if (__OSInIPL) {
        EXIProbeReset();
    } else {
        if ((EXIGetID(EXI_CHAN_0, EXI_DEV_EXT, &id) && id == 0x07010000)) {
            __OSEnableBarnacle(EXI_CHAN_1, EXI_DEV_EXT);
        } else if (EXIGetID(EXI_CHAN_1, EXI_DEV_EXT, &id) && id == 0x07010000) {
            __OSEnableBarnacle(EXI_CHAN_0, EXI_DEV_NET);
        }
    }

    OSRegisterVersion(__EXIVersion);
}

bool EXILock(EXIChannel chan, u32 dev, EXICallback callback) {
    EXIData* exi = &Ecb[chan];
    bool enabled;
    int i;

    enabled = OSDisableInterrupts();

    if (exi->state & EXI_STATE_LOCKED) {
        if (callback != NULL) {
            for (i = 0; i < exi->numItems; i++) {
                if (exi->items[i].dev == dev) {
                    OSRestoreInterrupts(enabled);
                    return false;
                }
            }

            exi->items[exi->numItems].callback = callback;
            exi->items[exi->numItems].dev = dev;
            exi->numItems++;
        }

        OSRestoreInterrupts(enabled);
        return false;
    } else {
        exi->state |= EXI_STATE_LOCKED;
        exi->dev = dev;
        SetExiInterruptMask(chan, exi);
        OSRestoreInterrupts(enabled);
        return true;
    }
}

bool EXIUnlock(EXIChannel chan) {
    EXIData* exi = &Ecb[chan];
    bool enabled;
    EXICallback callback;

    enabled = OSDisableInterrupts();

    if (!(exi->state & EXI_STATE_LOCKED)) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    exi->state &= ~EXI_STATE_LOCKED;
    SetExiInterruptMask(chan, exi);

    if (exi->numItems > 0) {
        callback = exi->items[0].callback;

        if (--exi->numItems > 0) {
            memmove(&exi->items[0], &exi->items[1], exi->numItems * sizeof(EXIItem));
        }

        callback(chan, NULL);
    }

    OSRestoreInterrupts(enabled);
    return true;
}

// Does nothing???
static void UnlockedHandler(EXIChannel chan, OSContext* ctx) {
#pragma unused(ctx)
    u32 id;
    EXIGetID(chan, EXI_DEV_EXT, &id);
}

s32 EXIGetID(EXIChannel chan, u32 dev, u32* out) {
    EXIData* exi = &Ecb[chan];
    u32 imm;
    s32 ret;
    s32 time;
    bool enabled;

    if (chan == EXI_CHAN_0 && dev == EXI_DEV_NET && IDSerialPort1 != 0) {
        *out = IDSerialPort1;
        return 1;
    }

    if (chan < EXI_CHAN_2 && dev == EXI_DEV_EXT) {
        if (!__EXIProbe(chan)) {
            return 0;
        }

        if (exi->lastInsert == __EXIProbeStartTime[chan]) {
            *out = exi->id;
            return exi->lastInsert;
        }

        if (!__EXIAttach(chan, NULL)) {
            return 0;
        }

        time = __EXIProbeStartTime[chan];
    }

    enabled = OSDisableInterrupts();
    ret = !EXILock(chan, dev, (chan < EXI_CHAN_2 && dev == EXI_DEV_EXT) ? UnlockedHandler : 0);
    if (ret == 0) {
        ret = !EXISelect(chan, dev, EXI_FREQ_1MHZ);
        if (ret == 0) {
            imm = 0x00000000;
            ret |= !EXIImm(chan, &imm, sizeof(u16), EXI_WRITE, NULL);
            ret |= !EXISync(chan);
            ret |= !EXIImm(chan, out, sizeof(u32), EXI_READ, NULL);
            ret |= !EXISync(chan);
            ret |= !EXIDeselect(chan);
        }
        EXIUnlock(chan);
    }

    OSRestoreInterrupts(enabled);

    if (chan < EXI_CHAN_2 && dev == EXI_DEV_EXT) {
        EXIDetach(chan);

        enabled = OSDisableInterrupts();
        ret |= time != __EXIProbeStartTime[chan];
        if (ret == 0) {
            exi->id = *out;
            exi->lastInsert = time;
        }

        OSRestoreInterrupts(enabled);
        return ret != 0 ? 0 : exi->lastInsert;
    }

    return ret == 0;
}
