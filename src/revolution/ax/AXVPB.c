#include "math.h"
#include "revolution/ax.h"
#include "revolution/os.h"
#include "stdint.h"
#include "string.h"

/**
 * It's really even worse than this: what appears to be manually unrolled
 * copies.
 *
 * I hope that it's just some compiler pattern that I'm unaware of, but in the
 * meantime I've managed to make a macro that still works.
 *
 * (If you compile for -O0 this will totally break functions)
 */
#define FUNNY_COPY(dst, src, type, n)            \
    do {                                         \
                                                 \
        type* __dst = (type*)dst;                \
        type* __src = (type*)src;                \
        u16 i;                                   \
                                                 \
        for (i = 0; i < n / sizeof(type); i++) { \
            *__dst = *__src;                     \
            __dst++;                             \
            __src++;                             \
        }                                        \
                                                 \
    } while (0)

#define ADDRHI(x) (((uintptr_t)x) >> 16 & 0xFFFF)
#define ADDRLO(x) (((uintptr_t)x) & 0xFFFF)

static AXPB __AXPB[AX_VOICE_MAX] ATTRIBUTE_ALIGN(32);
static AXPBU __AXUpdates[AX_VOICE_MAX] ATTRIBUTE_ALIGN(32);
static AXITD __AXITD[AX_VOICE_MAX] ATTRIBUTE_ALIGN(32);
static AXVPB __AXVPB[AX_VOICE_MAX] ATTRIBUTE_ALIGN(32);

static s32 __AXNumVoices = 0;
static u32 __AXRecDspCycles = 0;
static u32 __AXMaxDspCycles = 0;

static u32 __AXMixCycles[] = {
    0, 760, 760, 1470, 0, 1265, 1265, 2470, 760,  1520, 1520, 2230, 760,  2025, 2025, 3230,
    0, 760, 760, 1470, 0, 1265, 1265, 2470, 1265, 2025, 2025, 2735, 1265, 2530, 2530, 3735,
};

s32 __AXGetNumVoices(void) { return __AXNumVoices; }

void __AXServiceVPB(AXVPB* vpb) {
    AXPB* dst;
    AXPB* src;
    u32 sync;

    __AXNumVoices++;

    dst = &__AXPB[vpb->index];
    src = &vpb->pb;
    sync = vpb->sync;

    if (sync == 0) {
        src->state = dst->state;
        src->ve.currentVolume = dst->ve.currentVolume;
        src->addr.currentAddressHi = dst->addr.currentAddressHi;
        src->addr.currentAddressLo = dst->addr.currentAddressLo;
        return;
    }

    if (sync & AX_PBSYNC_ALL) {
        memcpy(dst, src, sizeof(AXPB));

        if (vpb->updateCounter != 0) {
            u32 count;
            u32* dst = (void*)&__AXUpdates[vpb->index];
            u32* src = (void*)&vpb->updateData;
            for (count = vpb->updateCounter; count; count--) {
                *dst++ = *src++;
            }
        }

        return;
    }

    if (sync & AX_PBSYNC_SELECT) {
        dst->srcSelect = src->srcSelect;
        dst->coefSelect = src->coefSelect;
    }

    if (sync & AX_PBSYNC_MIXER_CTRL) {
        dst->mixerCtrl = src->mixerCtrl;
    }

    if (sync & AX_PBSYNC_STATE) {
        dst->state = src->state;
    } else {
        src->state = dst->state;
    }

    if (sync & AX_PBSYNC_TYPE) {
        dst->type = src->type;
    }

    if (sync & AX_PBSYNC_MIX) {
        memcpy(&dst->mix, &src->mix, sizeof(AXPBMIX));
    }

    if (sync & AX_PBSYNC_ITD_SHIFT) {
        dst->itd.targetShiftL = src->itd.targetShiftL;
        dst->itd.targetShiftR = src->itd.targetShiftR;
    } else if (sync & AX_PBSYNC_ITD) {
        FUNNY_COPY(&dst->itd, &src->itd, u16, sizeof(AXPBITD));
        __memclr(vpb->itdBuffer, sizeof(AXITD));
    }

    if (sync & AX_PBSYNC_UNK7) {
        memcpy(&dst->unk052, &src->unk052, sizeof(dst->unk052));

        if (vpb->updateCounter != 0) {
            u32 count;
            u32* dst = (void*)&__AXUpdates[vpb->index];
            u32* src = (void*)&vpb->updateData;
            for (count = vpb->updateCounter; count; count--) {
                *dst++ = *src++;
            }
        }
    }

    if (sync & AX_PBSYNC_DPOP) {
        memcpy(&dst->dpop, &src->dpop, sizeof(AXPBDPOP));
    }

    if (sync & AX_PBSYNC_VE_DELTA) {
        src->ve.currentVolume = dst->ve.currentVolume;
        dst->ve.currentDelta = src->ve.currentDelta;
    } else if (sync & AX_PBSYNC_VE) {
        dst->ve.currentVolume = src->ve.currentVolume;
        dst->ve.currentDelta = src->ve.currentDelta;
    }

    if (sync & (AX_PBSYNC_LOOP_FLAG | AX_PBSYNC_LOOP_ADDR | AX_PBSYNC_END_ADDR | AX_PBSYNC_CURR_ADDR)) {
        if (sync & AX_PBSYNC_LOOP_FLAG) {
            dst->addr.loopFlag = src->addr.loopFlag;
        }

        if (sync & AX_PBSYNC_LOOP_ADDR) {
            *(u32*)&dst->addr.loopAddressHi = *(u32*)&src->addr.loopAddressHi;
        }

        if (sync & AX_PBSYNC_END_ADDR) {
            *(u32*)&dst->addr.endAddressHi = *(u32*)&src->addr.endAddressHi;
        }

        if (sync & AX_PBSYNC_CURR_ADDR) {
            *(u32*)&dst->addr.currentAddressHi = *(u32*)&src->addr.currentAddressHi;
        } else {
            *(u32*)&src->addr.currentAddressHi = *(u32*)&dst->addr.currentAddressHi;
        }
    } else if (sync & AX_PBSYNC_ADDR) {
        FUNNY_COPY(&dst->addr, &src->addr, u32, sizeof(AXPBADDR));
    } else {
        src->addr.currentAddressHi = dst->addr.currentAddressHi;
        src->addr.currentAddressLo = dst->addr.currentAddressLo;
    }

    if (sync & AX_PBSYNC_ADPCM) {
        FUNNY_COPY(&dst->adpcm, &src->adpcm, u32, sizeof(AXPBADPCM));
    }

    if (sync & AX_PBSYNC_SRC_RATIO) {
        dst->src.ratioHi = src->src.ratioHi;
        dst->src.ratioLo = src->src.ratioLo;
    } else if (sync & AX_PBSYNC_SRC) {
        FUNNY_COPY(&dst->src, &src->src, u16, sizeof(AXPBSRC));
    }

    if (sync & AX_PBSYNC_ADPCM_LOOP) {
        FUNNY_COPY(&dst->adpcmLoop, &src->adpcmLoop, u16, sizeof(AXPBADPCMLOOP));
    }

    if (sync & AX_PBSYNC_LPF_COEFS) {
        dst->lpf.a0 = src->lpf.a0;
        dst->lpf.b0 = src->lpf.b0;
    } else if (sync & AX_PBSYNC_LPF) {
        FUNNY_COPY(&dst->lpf, &src->lpf, u16, sizeof(AXPBLPF));
    }

    if (sync & AX_PBSYNC_UNK23) {
        dst->unk0CC.unk4 = src->unk0CC.unk4;
        dst->unk0CC.unk6 = src->unk0CC.unk6;
    } else if (sync & AX_PBSYNC_UNK22) {
        dst->unk0CC.unk0 = src->unk0CC.unk0;
        dst->unk0CC.unk2 = src->unk0CC.unk2;
        dst->unk0CC.unk4 = src->unk0CC.unk4;
        dst->unk0CC.unk6 = src->unk0CC.unk6;
    }

    if (sync & AX_PBSYNC_REMOTE) {
        dst->remote = src->remote;
    }

    if (sync & AX_PBSYNC_RMT_MIXER_CTRL) {
        dst->rmtMixerCtrl = src->rmtMixerCtrl;
    }

    if (sync & AX_PBSYNC_RMTMIX) {
        memcpy(&dst->rmtMix, &src->rmtMix, sizeof(AXPBRMTMIX));
    }

    if (sync & AX_PBSYNC_RMTDPOP) {
        memcpy(&dst->rmtDpop, &src->rmtDpop, sizeof(AXPBRMTDPOP));
    }

    if (sync & AX_PBSYNC_RMTSRC) {
        memcpy(&dst->rmtSrc, &src->rmtSrc, sizeof(AXPBRMTSRC));
    }
}

void __AXDumpVPB(AXVPB* vpb) {
    AXPB* pb = &__AXPB[vpb->index];

    if (pb->state == AX_VOICE_RUN) {
        __AXDepopVoice(pb);
    }

    pb->unk052.unk0 = pb->unk052.unk2 = pb->unk052.unk4 = 0;
    pb->state = AX_VOICE_STOP;
    vpb->pb.state = AX_VOICE_STOP;

    __AXPushCallbackStack(vpb);
}

void __AXSyncPBs(u32 baseCycles) {
    u32 cycles;
    u32 prio;
    AXVPB* head;

    __AXNumVoices = 0;

    DCInvalidateRange(__AXPB, AX_VOICE_MAX * sizeof(AXPB));
    DCInvalidateRange(__AXITD, AX_VOICE_MAX * sizeof(AXITD));

    cycles = 16 + __AXGetCommandListCycles() + AX_VOICE_MAX * 680 + baseCycles;

    for (prio = AX_PRIORITY_MAX; prio > AX_PRIORITY_FREE; prio--) {
        for (head = __AXGetStackHead(prio); head; head = head->next) {
            if (head->depop) {
                __AXDepopVoice(&__AXPB[head->index]);
            }

            if (head->pb.state == AX_VOICE_RUN || head->updateCounter != 0) {
                if (head->pb.srcSelect != AX_SRC_TYPE_4TAP_8K) {
                    u32 ratio = head->pb.src.ratioHi << 16 | head->pb.src.ratioLo;

                    cycles += (((ratio * 132 + 0x8000) >> 16) + 728) * 6;
                }

                if (head->pb.lpf.on) {
                    cycles += 555;
                }

                if (head->pb.unk0CC.unk0) {
                    cycles += 555;
                }

                cycles += __AXMixCycles[head->pb.mixerCtrl >> 26 & 31] + __AXMixCycles[head->pb.mixerCtrl >> 0 & 31] +
                          __AXMixCycles[head->pb.mixerCtrl >> 16 & 31] + __AXMixCycles[head->pb.mixerCtrl >> 21 & 31];
                cycles += 140;

                if (__AXMaxDspCycles > cycles) {
                    __AXServiceVPB(head);
                } else {
                    __AXDumpVPB(head);
                }
            } else {
                __AXServiceVPB(head);
            }

            head->sync = 0;
            head->depop = false;
            head->updateMS = head->updateCounter = 0;
            head->updateWrite = head->updateData.UNK_0x0;
        }
    }

    __AXRecDspCycles = cycles;

    for (head = __AXGetStackHead(AX_PRIORITY_FREE); head; head = head->next) {
        if (head->depop) {
            __AXDepopVoice(&__AXPB[head->index]);
        }

        head->depop = false;
        __AXPB[head->index].unk052.unk0 = __AXPB[head->index].unk052.unk2 = __AXPB[head->index].unk052.unk4 = 0;
        __AXPB[head->index].state = AX_VOICE_STOP;
    }

    DCFlushRange(__AXPB, AX_VOICE_MAX * sizeof(AXPB));
    DCFlushRange(__AXITD, AX_VOICE_MAX * sizeof(AXITD));
    DCFlushRange(__AXUpdates, AX_VOICE_MAX * sizeof(AXPBU));
}

AXPB* __AXGetPBs(void) { return __AXPB; }

void __AXSetPBDefault(AXVPB* vpb) {
    vpb->pb.state = AX_VOICE_STOP;
    vpb->pb.itd.flag = 0;
    vpb->sync = AX_PBSYNC_STATE | AX_PBSYNC_ITD | AX_PBSYNC_UNK7 | AX_PBSYNC_LPF | AX_PBSYNC_UNK22 | AX_PBSYNC_REMOTE |
                AX_PBSYNC_RMTSRC;
    vpb->updateMS = vpb->updateCounter = 0;
    vpb->updateWrite = vpb->updateData.UNK_0x0;
    vpb->pb.unk052.unk0 = vpb->pb.unk052.unk2 = vpb->pb.unk052.unk4 = 0;
    vpb->pb.lpf.on = 0;
    vpb->pb.unk0CC.unk0 = 0;
    vpb->pb.remote = 0;
    vpb->pb.rmtSrc.currentAddressFrac = 0;
    vpb->pb.rmtSrc.last_samples[0] = 0;
    vpb->pb.rmtSrc.last_samples[1] = 0;
    vpb->pb.rmtSrc.last_samples[2] = 0;
    vpb->pb.rmtSrc.last_samples[3] = 0;
}

void __AXVPBInit(void) {
    u32 i;
    u32* dst;

    __AXMaxDspCycles = OS_BUS_CLOCK_SPEED / 667;
    __AXRecDspCycles = 0;

    for (dst = (u32*)__AXPB, i = AX_VOICE_MAX * (sizeof(AXPB) / sizeof(u32)); i > 0; i--) {
        *dst++ = 0;
    }

    for (dst = (u32*)__AXITD, i = AX_VOICE_MAX * (sizeof(AXITD) / sizeof(u32)); i > 0; i--) {
        *dst++ = 0;
    }

    for (dst = (u32*)__AXVPB, i = AX_VOICE_MAX * (sizeof(AXVPB) / sizeof(u32)); i > 0; i--) {
        *dst++ = 0;
    }

    for (i = 0; i < AX_VOICE_MAX; i++) {
        AXPB* pb = &__AXPB[i];
        AXITD* itd = &__AXITD[i];
        AXPBU* update = &__AXUpdates[i];
        AXVPB* vpb = &__AXVPB[i];

        vpb->index = i;
        vpb->itdBuffer = itd;
        __AXSetPBDefault(vpb);

        if (i == AX_VOICE_MAX - 1) {
            pb->nextHi = pb->nextLo = 0;
            vpb->pb.nextHi = vpb->pb.nextLo = 0;
        } else {
            vpb->pb.nextHi = ADDRHI((uintptr_t)pb + sizeof(AXPB));
            vpb->pb.nextLo = ADDRLO((uintptr_t)pb + sizeof(AXPB));

            pb->nextHi = ADDRHI((uintptr_t)pb + sizeof(AXPB));
            pb->nextLo = ADDRLO((uintptr_t)pb + sizeof(AXPB));
        }

        vpb->pb.currHi = ADDRHI(pb);
        vpb->pb.currLo = ADDRLO(pb);

        pb->currHi = ADDRHI(pb);
        pb->currLo = ADDRLO(pb);

        vpb->pb.itd.bufferHi = ADDRHI(itd);
        vpb->pb.itd.bufferLo = ADDRLO(itd);

        pb->itd.bufferHi = ADDRHI(itd);
        pb->itd.bufferLo = ADDRLO(itd);

        vpb->pb.unk052.unk6 = ADDRHI(update);
        vpb->pb.unk052.unk8 = ADDRLO(update);

        pb->unk052.unk6 = ADDRHI(update);
        pb->unk052.unk8 = ADDRLO(update);

        vpb->priority = AX_PRIORITY_MIN;
        __AXPushFreeStack(vpb);
    }

    DCFlushRange(__AXPB, AX_VOICE_MAX * sizeof(AXPB));
}

void __AXVPBQuit(void) {}

void AXSetVoiceSrcType(AXVPB* vpb, u32 type) {
    bool enabled = OSDisableInterrupts();

    switch (type) {
        case AX_SRC_TYPE_NONE:
            vpb->pb.srcSelect = 2;
            break;
        case AX_SRC_TYPE_LINEAR:
            vpb->pb.srcSelect = 1;
            break;
        case AX_SRC_TYPE_4TAP_8K:
            vpb->pb.srcSelect = 0;
            vpb->pb.coefSelect = 0;
            break;
        case AX_SRC_TYPE_4TAP_12K:
            vpb->pb.srcSelect = 0;
            vpb->pb.coefSelect = 1;
            break;
        case AX_SRC_TYPE_4TAP_16K:
            vpb->pb.srcSelect = 0;
            vpb->pb.coefSelect = 2;
            break;
    }

    vpb->sync |= AX_PBSYNC_SELECT;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceState(AXVPB* vpb, u16 state) {
    bool enabled = OSDisableInterrupts();

    if (vpb->pb.state == state) {
        OSRestoreInterrupts(enabled);
        return;
    }

    vpb->pb.state = state;
    vpb->sync |= AX_PBSYNC_STATE;

    if (state == AX_VOICE_STOP) {
        vpb->depop = true;
    }

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceType(AXVPB* vpb, u16 type) {
    bool enabled = OSDisableInterrupts();

    vpb->pb.type = type;
    vpb->sync |= AX_PBSYNC_TYPE;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceMix(AXVPB* vpb, AXPBMIX* mix) {
    u32 mixerCtrl = 0;
    bool enabled = OSDisableInterrupts();
    u16* dst = (u16*)&vpb->pb.mix;
    u16* src = (u16*)mix;

    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x1;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x5;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x2;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x6;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x10000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x50000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x20000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x60000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x200000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0xA00000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x400000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0xC00000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x4000000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x14000000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x8000000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x18000000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x8;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x18;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x80000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x180000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x1000000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x3000000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x20000000;
    }
    if ((*dst++ = *src++) != 0) {
        mixerCtrl |= 0x60000000;
    }

    vpb->pb.mixerCtrl = mixerCtrl;
    vpb->sync |= AX_PBSYNC_MIX | AX_PBSYNC_MIXER_CTRL;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceVe(AXVPB* vpb, AXPBVE* ve) {
    bool enabled = OSDisableInterrupts();

    vpb->pb.ve.currentVolume = ve->currentVolume;
    vpb->pb.ve.currentDelta = ve->currentDelta;
    vpb->sync |= AX_PBSYNC_VE;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceAddr(AXVPB* vpb, AXPBADDR* addr) {
    bool enabled = OSDisableInterrupts();
    u32* dst = (u32*)&vpb->pb.addr;
    u32* src = (u32*)addr;

    // clang-format off
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    // clang-format on

    switch (addr->format) {
        case AX_SAMPLE_FORMAT_PCM_S16:
            // clang-format off
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0x08000000; dst++;
            *dst = 0; dst++;
            // clang-format on
            break;
        case AX_SAMPLE_FORMAT_PCM_S8:
            // clang-format off
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0; dst++;
            *dst = 0x01000000; dst++;
            *dst = 0; dst++;
            // clang-format on
            break;
        case AX_SAMPLE_FORMAT_DSP_ADPCM:
        default:
            break;
    }

    vpb->sync &= ~(AX_PBSYNC_LOOP_FLAG | AX_PBSYNC_LOOP_ADDR | AX_PBSYNC_END_ADDR | AX_PBSYNC_CURR_ADDR);
    vpb->sync |= AX_PBSYNC_ADDR | AX_PBSYNC_ADPCM;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceAdpcm(AXVPB* vpb, AXPBADPCM* adpcm) {
    bool enabled = OSDisableInterrupts();
    u32* dst = (u32*)&vpb->pb.adpcm;
    u32* src = (u32*)adpcm;

    // clang-format off
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    // clang-format on

    vpb->sync |= AX_PBSYNC_ADPCM;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceSrc(AXVPB* vpb, AXPBSRC* src_) {
    bool enabled = OSDisableInterrupts();
    u16* dst = (u16*)&vpb->pb.src;
    u16* src = (u16*)src_;

    // clang-format off
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    // clang-format on

    vpb->sync &= ~AX_PBSYNC_SRC_RATIO;
    vpb->sync |= AX_PBSYNC_SRC;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceSrcRatio(AXVPB* p, f32 ratio) {
    bool enabled = OSDisableInterrupts();
    u32 r;

    r = 65536.0f * ratio;
    p->pb.src.ratioHi = ((u32)r >> 16);
    p->pb.src.ratioLo = ((u32)r);
    p->sync |= AX_PBSYNC_SRC_RATIO;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceAdpcmLoop(AXVPB* vpb, AXPBADPCMLOOP* adpcmloop) {
    bool enabled = OSDisableInterrupts();
    u16* dst = (u16*)&vpb->pb.adpcmLoop;
    u16* src = (u16*)adpcmloop;

    // clang-format off
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    // clang-format on

    vpb->sync |= AX_PBSYNC_ADPCM_LOOP;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceLpf(AXVPB* vpb, AXPBLPF* lpf) {
    bool enabled = OSDisableInterrupts();
    u16* dst = (u16*)&vpb->pb.lpf;
    u16* src = (u16*)lpf;

    // clang-format off
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    *dst = *src; dst++; src++;
    // clang-format on

    vpb->sync |= AX_PBSYNC_LPF;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceLpfCoefs(AXVPB* vpb, u16 a0, u16 b0) {
    bool enabled = OSDisableInterrupts();

    vpb->pb.lpf.a0 = a0;
    vpb->pb.lpf.b0 = b0;
    vpb->sync |= AX_PBSYNC_LPF_COEFS;

    OSRestoreInterrupts(enabled);
}

void AXGetLpfCoefs(u16 freq, u16* a, u16* b) {
    f32 rf31 = 2.0f - cosf(2.0f * (f32)M_PI * freq / AX_SAMPLE_RATE);
    f32 rf30 = sqrtf(rf31 * rf31 - 1.0f) - rf31;

    *b = 32768 * -rf30;
    *a = 32767 - *b;
}

void AXSetVoiceRmtOn(AXVPB* vpb, u16 on) {
    bool enabled = OSDisableInterrupts();

    vpb->pb.remote = on;
    vpb->sync |= AX_PBSYNC_REMOTE;

    OSRestoreInterrupts(enabled);
}

void AXSetVoiceRmtMix(AXVPB* vpb, AXPBRMTMIX* rmtmix) {
    u16 rmtMixerCtrl = 0;
    bool enabled = OSDisableInterrupts();
    u16* dst = (u16*)&vpb->pb.rmtMix;
    u16* src = (u16*)rmtmix;

    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x1;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x2;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x4;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x8;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x10;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x20;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x40;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x80;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x100;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x200;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x400;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x800;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x1000;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x2000;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x4000;
    }
    if ((*dst++ = *src++) != 0) {
        rmtMixerCtrl |= 0x8000;
    }

    vpb->pb.rmtMixerCtrl = rmtMixerCtrl;
    vpb->sync |= AX_PBSYNC_RMTMIX | AX_PBSYNC_RMT_MIXER_CTRL;

    OSRestoreInterrupts(enabled);
}
