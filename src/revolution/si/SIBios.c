#include "revolution/os.h"
#include "revolution/pad.h"
#include "revolution/si.h"

const char* __SIVersion = "<< RVL_SDK - SI \trelease build: Sep  7 2006 07:20:53 (0x4200_60422) >>";

static SIMain Si = {SI_CHAN_NONE};
static u32 Type[SI_MAX_CHAN] = {SI_ERROR_NOREP, SI_ERROR_NOREP, SI_ERROR_NOREP, SI_ERROR_NOREP};

static SIPacket Packet[SI_MAX_CHAN];
static s64 XferTime[SI_MAX_CHAN];
static s64 TypeTime[SI_MAX_CHAN];
static SITypeAndStatusCallback TypeCallback[SI_MAX_TYPE][SI_MAX_CHAN];
static bool InputBufferValid[SI_MAX_CHAN];
static u32 InputBuffer[SI_MAX_CHAN][2];
static OSInterruptHandler RDSTHandler[SI_MAX_CHAN];
static vu32 InputBufferVcount[SI_MAX_CHAN];
static OSAlarm Alarm[SI_MAX_CHAN];

u32 __PADFixBits;

static void GetTypeCallback(s32 chan, u32 error, OSContext* context);

bool SIBusy() { return Si.chan != -1 ? true : false; }

bool SIIsChanBusy(s32 chan) { return (Packet[chan].chan != -1 || Si.chan == chan); }

static inline void SIClearTCInterrupt(void) {
    u32 csr = SI_HW_REGS[SI_SICOMSCR];

    csr |= SI_SICOMCSR_TCINT;
    csr &= ~SI_SICOMCSR_TSTART;

    SI_HW_REGS[SI_SICOMSCR] = csr;
}

static u32 CompleteTransfer(void) {
    u32 i;
    u32 sr;
    u8* dst;
    u32 size;
    u32 imm;

    sr = SI_HW_REGS[SI_SISR];
    SIClearTCInterrupt();

    if (Si.chan != SI_CHAN_NONE) {
        XferTime[Si.chan] = __OSGetSystemTime();

        dst = (u8*)Si.inAddr;

        size = Si.inSize / sizeof(u32);
        for (i = 0; i < size; dst += sizeof(u32), i++) {
            *(u32*)dst = SI_HW_REGS[SI_RAM_BASE + i];
        }

        size = Si.inSize % sizeof(u32);
        if (size > 0) {
            imm = SI_HW_REGS[SI_RAM_BASE + i];

            for (i = 0; i < size; dst++, i++) {
                *dst = imm >> (3 - i) * 8;
            }
        }

        if (SI_HW_REGS[SI_SICOMSCR] & SI_SICOMCSR_COMERR) {
            // Flags relative to channel ID
            sr >>= ((SI_MAX_CHAN - 1) - Si.chan) * 8;
            sr &= (SI_UNRUN | SI_OVRUN | SI_COLL | SI_NOREP);

            if ((sr & SI_NOREP) && (Type[Si.chan] & SI_ERROR_BUSY) == 0) {
                Type[Si.chan] = SI_ERROR_NOREP;
            }

            if (sr == 0) {
                sr = SI_COLL;
            }
        } else {
            TypeTime[Si.chan] = __OSGetSystemTime();
            sr = 0;
        }

        Si.chan = SI_CHAN_NONE;
    }

    return sr;
}

static inline void SITransferNext(s32 chan) {
    int i;
    SIPacket* packet;

    for (i = 0; i < SI_MAX_CHAN; ++i) {
        ++chan;
        chan %= SI_MAX_CHAN;
        packet = &Packet[chan];
        if (packet->chan != -1 && packet->fire <= __OSGetSystemTime()) {
            if (__SITransfer(packet->chan, packet->outAddr, packet->outSize, packet->inAddr, packet->inSize,
                             packet->callback)) {
                OSCancelAlarm(&Alarm[chan]);
                packet->chan = -1;
            }
            break;
        }
    }
}

static void SIInterruptHandler(__OSInterrupt interrupt, OSContext* context) {
    u32 reg;

    reg = SI_HW_REGS[SI_SICOMSCR];

    if ((reg & 0xC0000000) == 0xC0000000) {
        s32 chan;
        u32 sr;
        SICallback callback;

        chan = Si.chan;
        sr = CompleteTransfer();
        callback = Si.callback;
        Si.callback = 0;

        SITransferNext(chan);

        if (callback) {
            callback(chan, sr, context); // context
        }

        sr = SI_HW_REGS[SI_SISR];
        sr &= 0xF000000 >> (8 * chan);
        SI_HW_REGS[SI_SISR] = sr;

        if (Type[chan] == SI_ERROR_BUSY && !SIIsChanBusy(chan)) {
            static u32 cmdTypeAndStatus = 0 << 24;
            SITransfer(chan, &cmdTypeAndStatus, 1, &Type[chan], 3, GetTypeCallback, OSMicrosecondsToTicks(65));
        }
    }

    if ((reg & 0x18000000) == 0x18000000) {

        int i;
        u32 vcount;
        u32 x;

        vcount = VIGetCurrentLine() + 1;
        x = (Si.poll & 0x03FF0000) >> 16;

        for (i = 0; i < SI_MAX_CHAN; ++i) {
            if (SIGetResponseRaw(i)) {
                InputBufferVcount[i] = vcount;
            }
        }

        for (i = 0; i < SI_MAX_CHAN; ++i) {
            if (!(Si.poll & (SI_CHAN0_BIT >> (31 - 7 + i)))) {
                continue;
            }
            if (InputBufferVcount[i] == 0 || InputBufferVcount[i] + (x / 2) < vcount) {
                return;
            }
        }

        for (i = 0; i < SI_MAX_CHAN; ++i) {
            InputBufferVcount[i] = 0;
        }

        for (i = 0; i < 4; ++i) {
            if (RDSTHandler[i]) {
                RDSTHandler[i](interrupt, context);
            }
        }
    }
}

static bool SIEnablePollingInterrupt(bool enable) {
    bool enabled;
    bool rc;
    u32 reg;
    int i;

    enabled = OSDisableInterrupts();
    reg = SI_HW_REGS[SI_SICOMSCR];
    rc = (reg & 0x08000000) ? true : false;
    if (enable) {
        reg |= 0x08000000;
        for (i = 0; i < SI_MAX_CHAN; ++i) {
            InputBufferVcount[i] = 0;
        }
    } else {
        reg &= ~0x08000000;
    }
    reg &= ~0x80000001;
    SI_HW_REGS[SI_SICOMSCR] = reg;
    OSRestoreInterrupts(enabled);
    return rc;
}

bool SIUnregisterPollingHandler(OSInterruptHandler handler) {
    bool enabled;
    int i;

    enabled = OSDisableInterrupts();
    for (i = 0; i < 4; ++i) {
        if (RDSTHandler[i] == handler) {
            RDSTHandler[i] = 0;
            for (i = 0; i < 4; ++i) {
                if (RDSTHandler[i]) {
                    break;
                }
            }
            if (i == 4) {
                SIEnablePollingInterrupt(false);
            }
            OSRestoreInterrupts(enabled);
            return true;
            break;
        }
    }
    OSRestoreInterrupts(enabled);
    return false;
}

void SIInit(void) {
    static bool Initialized = false;

    if (Initialized) {
        return;
    }

    OSRegisterVersion(__SIVersion);

    Packet[SI_CHAN_0].chan = Packet[SI_CHAN_1].chan = Packet[SI_CHAN_2].chan = Packet[SI_CHAN_3].chan = SI_CHAN_NONE;

    Si.poll = 0;
    SISetSamplingRate(0);

    while (SI_HW_REGS[SI_SICOMSCR] & SI_SICOMCSR_TSTART) {
        ;
    }

    SI_HW_REGS[SI_SICOMSCR] = SI_SICOMCSR_TCINT;

    __OSSetInterruptHandler(OS_INTR_PI_SI, SIInterruptHandler);
    __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_PI_SI));

    SIGetType(SI_CHAN_0);
    SIGetType(SI_CHAN_1);
    SIGetType(SI_CHAN_2);
    SIGetType(SI_CHAN_3);

    Initialized = true;
}

static bool __SITransfer(s32 chan, void* outAddr, u32 outSize, void* inAddr, u32 inSize, SICallback callback) {
    bool enabled;
    u32 sr;
    u32 alignSize;
    u32 i;

    union {
        struct {
            u32 TCINT : 1;
            u32 TCINTMSK : 1;
            u32 COMERR : 1;
            u32 RDSTINT : 1;
            u32 RDSTINTMSK : 1;
            u32 reserved2 : 4;
            u32 OUTLNGTH : 7;
            u32 reserved1 : 1;
            u32 INLNGTH : 7;
            u32 reserved0 : 5;
            u32 CHANNEL : 2;
            u32 TSTART : 1;
        };

        u32 reg;
    } comscr;

    enabled = OSDisableInterrupts();

    if (Si.chan != SI_CHAN_NONE) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    sr = SI_HW_REGS[SI_SISR];
    sr &= (SI_SISR_UNRUN0 | SI_SISR_OVRUN0 | SI_SISR_COLL0 | SI_SISR_NOREP0) >> (chan * 8);
    SI_HW_REGS[SI_SISR] = sr;

    Si.chan = chan;
    Si.callback = callback;
    Si.inSize = inSize;
    Si.inAddr = inAddr;

    alignSize = (outSize + 3) / 4;
    for (i = 0; i < alignSize; i++) {
        SI_HW_REGS[SI_RAM_BASE + i] = ((u32*)outAddr)[i];
    }

    comscr.reg = SI_HW_REGS[SI_SICOMSCR];

    comscr.TCINT = true;
    comscr.TCINTMSK = callback != NULL;
    // Minimum transfer is 1 byte. 0x00 will transfer 128 bytes
    comscr.OUTLNGTH = outSize == 128 ? 0 : outSize;
    comscr.INLNGTH = inSize == 128 ? 0 : inSize;
    comscr.CHANNEL = chan;
    comscr.TSTART = true;

    SI_HW_REGS[SI_SICOMSCR] = comscr.reg;

    OSRestoreInterrupts(enabled);
    return true;
}

u32 SIGetStatus(s32 chan) {
    bool enabled;
    u32 sr;
    int chanShift;

    enabled = OSDisableInterrupts();
    sr = SI_HW_REGS[SI_SISR];
    chanShift = 8 * (SI_MAX_CHAN - 1 - chan);
    sr >>= chanShift;
    if (sr & SI_ERROR_NO_RESPONSE) {
        if (!(Type[chan] & SI_ERROR_BUSY)) {
            Type[chan] = SI_ERROR_NO_RESPONSE;
        }
    }
    OSRestoreInterrupts(enabled);
    return sr;
}

void SISetCommand(s32 chan, u32 command) { SI_HW_REGS[3 * chan] = command; }

static inline void SITransferCommands(void) { SI_HW_REGS[SI_SISR] = 0x80000000; }

u32 SISetXY(u32 lines, u32 times) {
    bool enabled;
    u32 poll;

    poll = lines << 16;
    poll |= times << 8;

    enabled = OSDisableInterrupts();

    Si.poll &= ~(SI_SIPOLL_X | SI_SIPOLL_Y);
    Si.poll |= poll;

    poll = Si.poll;
    SI_HW_REGS[SI_SIPOLL] = poll;

    OSRestoreInterrupts(enabled);
    return poll;
}

u32 SIEnablePolling(u32 poll) {
    bool enabled;
    u32 en;

    if (poll == 0) {
        return Si.poll;
    }

    enabled = OSDisableInterrupts();

    poll >>= (31 - 7);
    en = poll & 0xF0;

    poll &= (en >> 4) | 0x03FFFFF0;

    poll &= ~0x03FFFF00;

    Si.poll &= ~(en >> 4);

    Si.poll |= poll;

    poll = Si.poll;

    SITransferCommands();

    SI_HW_REGS[SI_SIPOLL] = poll;

    OSRestoreInterrupts(enabled);

    return poll;
}

u32 SIDisablePolling(u32 poll) {
    bool enabled;

    if (poll == 0) {
        return Si.poll;
    }

    enabled = OSDisableInterrupts();

    poll >>= (31 - 7);
    poll &= 0xF0;

    poll = Si.poll & ~poll;

    SI_HW_REGS[SI_SIPOLL] = poll;
    Si.poll = poll;

    OSRestoreInterrupts(enabled);
    return poll;
}

static inline bool SIGetResponseRaw(s32 chan) {
    u32 sr;

    sr = SIGetStatus(chan);
    if (sr & SI_ERROR_RDST) {
        InputBuffer[chan][0] = SI_HW_REGS[3 * chan + 1];
        InputBuffer[chan][1] = SI_HW_REGS[3 * chan + 2];
        InputBufferValid[chan] = true;
        return true;
    }
    return false;
}

bool SIGetResponse(s32 chan, void* data) {
    bool rc;
    bool enabled;

    enabled = OSDisableInterrupts();
    SIGetResponseRaw(chan);
    rc = InputBufferValid[chan];
    InputBufferValid[chan] = false;
    if (rc) {
        ((u32*)data)[0] = InputBuffer[chan][0];
        ((u32*)data)[1] = InputBuffer[chan][1];
    }
    OSRestoreInterrupts(enabled);
    return rc;
}

static void AlarmHandler(OSAlarm* alarm, OSContext* ctx) {
    s32 chan;
    SIPacket* packet;

    chan = alarm - Alarm;
    packet = &Packet[chan];

    if (packet->chan != SI_CHAN_NONE) {
        if (__SITransfer(packet->chan, packet->outAddr, packet->outSize, packet->inAddr, packet->inSize,
                         packet->callback)) {
            packet->chan = SI_CHAN_NONE;
        }
    }
}

bool SITransfer(s32 chan, void* output, u32 outputBytes, void* input, u32 inputBytes, SICallback callback,
                OSTime delay) {
    bool enabled;
    SIPacket* packet = &Packet[chan];
    OSTime now;
    OSTime fire;

    enabled = OSDisableInterrupts();
    if (packet->chan != -1 || Si.chan == chan) {
        OSRestoreInterrupts(enabled);
        return false;
    }

    now = __OSGetSystemTime();
    if (delay == 0) {
        fire = now;
    } else {
        fire = XferTime[chan] + delay;
    }
    if (now < fire) {
        delay = fire - now;
        OSSetAlarm(&Alarm[chan], delay, AlarmHandler);
    } else if (__SITransfer(chan, output, outputBytes, input, inputBytes, callback)) {
        OSRestoreInterrupts(enabled);
        return true;
    }

    packet->chan = chan;
    packet->outAddr = output;
    packet->outSize = outputBytes;
    packet->inAddr = input;
    packet->inSize = inputBytes;
    packet->callback = callback;
    packet->fire = fire;

    OSRestoreInterrupts(enabled);
    return true;
}

static inline void CallTypeAndStatusCallback(s32 chan, u32 type) {
    SITypeAndStatusCallback callback;
    int i;

    for (i = 0; i < 4; ++i) {
        callback = TypeCallback[chan][i];
        if (callback) {
            TypeCallback[chan][i] = 0;
            callback(chan, type);
        }
    }
}

static void GetTypeCallback(s32 chan, u32 error, OSContext* context) {
    static u32 cmdFixDevice[SI_MAX_CHAN];
    u32 type;
    u32 chanBit;
    bool fix;
    u32 id;

    Type[chan] &= ~SI_ERROR_BUSY;
    Type[chan] |= error;
    TypeTime[chan] = __OSGetSystemTime();

    type = Type[chan];

    chanBit = SI_CHAN0_BIT >> chan;
    fix = (bool)(__PADFixBits & chanBit);
    __PADFixBits &= ~chanBit;

    if ((error & (SI_ERROR_UNDER_RUN | SI_ERROR_OVER_RUN | SI_ERROR_NO_RESPONSE | SI_ERROR_COLLISION)) ||
        (type & SI_TYPE_MASK) != SI_TYPE_DOLPHIN || !(type & SI_GC_WIRELESS) || (type & SI_WIRELESS_IR)) {
        OSSetWirelessID(chan, 0);
        CallTypeAndStatusCallback(chan, Type[chan]);
        return;
    }

    id = (u32)(OSGetWirelessID(chan) << 8);

    if (fix && (id & SI_WIRELESS_FIX_ID)) {
        cmdFixDevice[chan] = 0x4Eu << 24 | (id & SI_WIRELESS_TYPE_ID) | SI_WIRELESS_FIX_ID;
        Type[chan] = SI_ERROR_BUSY;
        SITransfer(chan, &cmdFixDevice[chan], 3, &Type[chan], 3, GetTypeCallback, 0);
        return;
    }

    if (type & SI_WIRELESS_FIX_ID) {
        if ((id & SI_WIRELESS_TYPE_ID) != (type & SI_WIRELESS_TYPE_ID)) {
            if (!(id & SI_WIRELESS_FIX_ID)) {
                id = type & SI_WIRELESS_TYPE_ID;
                id |= SI_WIRELESS_FIX_ID;
                OSSetWirelessID(chan, (u16)((id >> 8) & 0xFFff));
            }

            cmdFixDevice[chan] = 0x4E << 24 | id;
            Type[chan] = SI_ERROR_BUSY;
            SITransfer(chan, &cmdFixDevice[chan], 3, &Type[chan], 3, GetTypeCallback, 0);
            return;
        }
    } else if (type & SI_WIRELESS_RECEIVED) {
        id = type & SI_WIRELESS_TYPE_ID;
        id |= SI_WIRELESS_FIX_ID;

        OSSetWirelessID(chan, (u16)((id >> 8) & 0xFFff));

        cmdFixDevice[chan] = 0x4E << 24 | id;
        Type[chan] = SI_ERROR_BUSY;
        SITransfer(chan, &cmdFixDevice[chan], 3, &Type[chan], 3, GetTypeCallback, 0);
        return;
    } else {
        OSSetWirelessID(chan, 0);
    }

    CallTypeAndStatusCallback(chan, Type[chan]);
}

u32 SIGetType(s32 chan) {
    static u32 cmdTypeAndStatus;
    bool enabled;
    u32 type;
    OSTime diff;

    enabled = OSDisableInterrupts();

    type = Type[chan];
    diff = __OSGetSystemTime() - TypeTime[chan];
    if (Si.poll & (0x80 >> chan)) {
        if (type != SI_ERROR_NO_RESPONSE) {
            TypeTime[chan] = __OSGetSystemTime();
            OSRestoreInterrupts(enabled);
            return type;
        } else {
            type = Type[chan] = SI_ERROR_BUSY;
        }
    } else if (diff <= OSMillisecondsToTicks(50) && type != SI_ERROR_NO_RESPONSE) {
        OSRestoreInterrupts(enabled);
        return type;
    } else if (diff <= OSMillisecondsToTicks(75)) {
        Type[chan] = SI_ERROR_BUSY;
    } else {
        type = Type[chan] = SI_ERROR_BUSY;
    }
    TypeTime[chan] = __OSGetSystemTime();

    SITransfer(chan, &cmdTypeAndStatus, 1, &Type[chan], 3, GetTypeCallback, OSMicrosecondsToTicks(65));

    OSRestoreInterrupts(enabled);
    return type;
}

u32 SIGetTypeAsync(s32 chan, SITypeAndStatusCallback callback) {
    bool enabled;
    u32 type;

    enabled = OSDisableInterrupts();
    type = SIGetType(chan);
    if (Type[chan] & SI_ERROR_BUSY) {
        int i;

        for (i = 0; i < 4; ++i) {
            if (TypeCallback[chan][i] == callback) {
                break;
            }
            if (TypeCallback[chan][i] == 0) {
                TypeCallback[chan][i] = callback;
                break;
            }
        }
    } else {
        callback(chan, type);
    }
    OSRestoreInterrupts(enabled);
    return type;
}
