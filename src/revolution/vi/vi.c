//! TODO: figure out why the order matters here
// clang-format off
#include "macros.h"
#include "revolution/dvd.h"
#include "revolution/vi.h"
#include "revolution/os.h"
#include "revolution/sc.h"
// clang-format on

// for some reasons this is required
#define VI_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define MARK_CHANGED(index) (changed |= 1LL << (63 - (index)))

const char* __VIVersion = "<< RVL_SDK - VI \trelease build: Sep 26 2006 17:27:57 (0x4200_60422) >>";

static void __VIRetraceHandler(__OSInterrupt, OSContext*);
extern bool __OSIsDiag;

static volatile VITimeToDIM g_current_time_to_dim;
static volatile u32 retraceCount;
static volatile u32 flushFlag;
static volatile u32 flushFlag3in1;

static bool IsInitialized = false;
static vu32 vsync_timing_err_cnt = 0;
static vu32 vsync_timing_test_flag = 0;
static volatile bool __VIDimming_All_Clear = false;
static volatile bool __VIDimmingFlag_Enable;
static volatile bool __VIDVDStopFlag_Enable;
static vu32 NEW_TIME_TO_DIMMING = 0;
static vu32 THD_TIME_TO_DVD_STOP = 0;
static vu32 _gIdleCount_dimming = 0;
static vu32 _gIdleCount_dvd = 0;
static VIPositionCallback PositionCallback = NULL;
static s16 displayOffsetH = 0;
static s16 displayOffsetV = 0;
static vu32 changeMode = 0;
static vu32 THD_TIME_TO_DIMMING = 0;
static vu64 changed = 0;
static vu32 shdwChangeMode = 0;
static vu64 shdwChanged = 0;
static u32 FBSet = 0;
static timing_s* timingExtra = NULL;

static vu32 __VIDimmingFlag_RF_IDLE;
static vu32 __VIDimmingFlag_SI_IDLE;

extern VIVideo Vdac_Flag_Region;
extern volatile u32 Vdac_Flag_Changed;

static OSThreadQueue retraceQueue;

#define ToPhysical(fb) (u32)(((u32)(fb)) & 0x3FFFFFFF)
#define IS_LOWER_16MB(x) ((x) < 16 * 1024 * 1024)
#define ONES(x) ((1 << (x)) - 1)

#define VI_NUMREGS ARRAY_COUNT(VI_HW_REGS)

static u16 shdwRegs[VI_NUMREGS] = {0};
static vu32 __VIDimmingFlag_DEV_IDLE[10];
static u16 regs[VI_NUMREGS];
static HorVer_s HorVer;

static VIRetraceCallback PreCB;
static VIRetraceCallback PostCB;
static u32 encoderType;
static timing_s* CurrTiming;
static u32 CurrTvMode;
static u32 NextBufAddr;
static u32 CurrBufAddr;

static bool OnShutdown(bool final, u32 event);

static timing_s timing[] = {
    {6, 240, 24, 25, 3, 2, 12, 13, 12, 13, 520, 519, 520, 519, 525, 429, 64, 71, 105, 162, 373, 122, 412},
    {6, 240, 24, 24, 4, 4, 12, 12, 12, 12, 520, 520, 520, 520, 526, 429, 64, 71, 105, 162, 373, 122, 412},
    {5, 287, 35, 36, 1, 0, 13, 12, 11, 10, 619, 618, 617, 620, 625, 432, 64, 75, 106, 172, 380, 133, 420},
    {5, 287, 33, 33, 2, 2, 13, 11, 13, 11, 619, 621, 619, 621, 624, 432, 64, 75, 106, 172, 380, 133, 420},
    {6, 240, 24, 25, 3, 2, 16, 15, 14, 13, 518, 517, 516, 519, 525, 429, 64, 78, 112, 162, 373, 122, 412},
    {6, 240, 24, 24, 4, 4, 16, 14, 16, 14, 518, 520, 518, 520, 526, 429, 64, 78, 112, 162, 373, 122, 412},
    {12, 480, 48, 48, 6, 6, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 162, 373, 122, 412},
    {12, 480, 44, 44, 10, 10, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 168, 379, 122, 412},
    {6, 241, 24, 25, 1, 0, 12, 13, 12, 13, 520, 519, 520, 519, 525, 429, 64, 71, 105, 159, 370, 122, 412},
    {12, 480, 48, 48, 6, 6, 24, 24, 24, 24, 1038, 1038, 1038, 1038, 1050, 429, 64, 71, 105, 180, 391, 122, 412},
    {10, 576, 62, 62, 6, 6, 20, 20, 20, 20, 1240, 1240, 1240, 1240, 1250, 432, 64, 75, 106, 172, 380, 122, 412},
};

static u16 taps[] = {
    0x1F0, 0x1DC, 0x1AE, 0x174, 0x129, 0x0DB, 0x08E, 0x046, 0x0C, 0xE2, 0xCB, 0xC0, 0xC4,
    0xCF,  0xDE,  0xEC,  0xFC,  0x08,  0x0F,  0x13,  0x13,  0x0F, 0x0C, 0x08, 0x01,
};

static OSShutdownFunctionInfo ShutdownFunctionInfo = {OnShutdown, 127};

FORCE_ACTIVE(vi, THD_TIME_TO_DIMMING);

static bool OnShutdown(bool final, u32 event) {
    bool retval = final;

    if (retval == false) {
        switch (event) {
            case 3:
            case 1:
            case 2:
                __VISetRGBModeImm();
                retval = true;
                break;
            case 4:
            case 0:
            case 5:
            case 6:
                retval = true;
                break;
        }
    } else {
        retval = true;
    }

    return retval;
}

static void GetCurrentDisplayPosition(u32* hct, u32* vct) {
    u32 hcount, vcount0, vcount;

    vcount = (u32)((VI_HW_REGS[0x16]) & 0x7FF);

    do {
        vcount0 = vcount;
        hcount = (u32)(VI_HW_REGS[0x17] & 0x7FF);
        vcount = (u32)(VI_HW_REGS[0x16] & 0x7FF);
    } while (vcount0 != vcount);

    *hct = hcount;
    *vct = vcount;
}

static u32 getCurrentFieldEvenOdd(void);
static s32 cntlzd(u64);

static bool VISetRegs(void) {
    s32 regIndex;

    if (!((shdwChangeMode == 1) && (getCurrentFieldEvenOdd() == 0))) {
        while (shdwChanged) {
            regIndex = cntlzd(shdwChanged);
            VI_HW_REGS[regIndex] = shdwRegs[regIndex];
            shdwChanged &= ~(1ull << (63 - (regIndex)));
        }

        shdwChangeMode = 0;
        CurrTiming = HorVer.timing;
        CurrTvMode = HorVer.tv;
        CurrBufAddr = NextBufAddr;

        return true;
    } else {
        return false;
    }
}

extern DVDCommandBlock __DVDStopMotorCommandBlock;

static void __VIRetraceHandler(__OSInterrupt interrupt, OSContext* context) {
    OSContext exceptionContext;
    u16 reg;
    u32 inter = 0;
    u32 regIndex;
    static u32 old_dtvStatus = 999;
    static u32 old_tvtype = 999;
    u32 now_dtvStatus = 0;
    u32 now_tvtype = 0;
    static bool __VIDimmingFlag_Enable_old = true;
    static bool __VIDVDStopFlag_Enable_old = true;
    u32 i;
    static u32 DimmingON_Pending = 0;
    static u32 DimmingOFF_Pending = 0;

    reg = VI_HW_REGS[0x18];
    if (reg & 0x8000) {
        VI_HW_REGS[0x18] = (u16)(reg & ~0x8000);
        inter |= 1;
    }

    reg = VI_HW_REGS[0x1A];
    if (reg & 0x8000) {
        VI_HW_REGS[0x1A] = (u16)(reg & ~0x8000);
        inter |= 2;
    }

    reg = VI_HW_REGS[0x1C];
    if (reg & 0x8000) {
        VI_HW_REGS[0x1C] = (u16)(reg & ~0x8000);
        inter |= 4;
    }

    reg = VI_HW_REGS[0x1E];
    if (reg & 0x8000) {
        VI_HW_REGS[0x1E] = (u16)(reg & ~0x8000);
        inter |= 8;
    }

    reg = VI_HW_REGS[0x1E];

    if ((inter & 4) || (inter & 8)) {
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);

        if (PositionCallback) {
            s16 x, y;
            __VIGetCurrentPosition(&x, &y);
            (*PositionCallback)(x, y);
        }

        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
        return;
    }

    retraceCount++;
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);

    if (PreCB) {
        (*PreCB)(retraceCount);
    }

    if (vsync_timing_test_flag) {
        u32 hcount, vcount;

        GetCurrentDisplayPosition(&hcount, &vcount);

        if (!((vcount == 1) || (vcount == (CurrTiming->nhlines / 2 + 1)))) {
            vsync_timing_err_cnt++;
        }
    }

    if (flushFlag) {
        if (VISetRegs()) {
            flushFlag = 0;
            SIRefreshSamplingRate();
        }
    }

    now_dtvStatus = VIGetDTVStatus();
    if (now_dtvStatus != old_dtvStatus) {
        __VISetYUVSEL((VIBool)now_dtvStatus);
    }

    old_dtvStatus = now_dtvStatus;
    now_tvtype = VIGetTvFormat();

    if (now_tvtype != old_tvtype) {
        if (now_tvtype == VI_EURGB60) {
            __VISetFilter4EURGB60(VI_ENABLE);
        } else {
            __VISetFilter4EURGB60(VI_DISABLE);
        }

        switch (now_tvtype) {
            case VI_PAL:
                NEW_TIME_TO_DIMMING = 15000;
                THD_TIME_TO_DVD_STOP = 90000;
                break;
            default:
                NEW_TIME_TO_DIMMING = 18000;
                THD_TIME_TO_DVD_STOP = 108000;
                break;
        }

        _gIdleCount_dimming = 0;
        _gIdleCount_dvd = 0;
    }

    old_tvtype = now_tvtype;

    if (flushFlag3in1) {
        while (Vdac_Flag_Changed) {
            regIndex = (u32)__cntlzw(Vdac_Flag_Changed);
            regIndex = (u32)(1 << (31 - regIndex));

            switch (regIndex) {
                case 1:
                    __VISetCGMS();
                    break;
                case 2:
                    __VISetWSS();
                    break;
                case 4:
                    __VISetClosedCaption();
                    break;
                case 8:
                    __VISetMacrovision();
                    break;
                case 0x10:
                    __VISetGamma();
                    break;
                case 0x20:
                    __VISetTrapFilter();
                    break;
                case 0x40:
                    __VISetRGBOverDrive();
                    break;
            }

            Vdac_Flag_Changed &= ~regIndex;
        }

        flushFlag3in1 = 0;
    }

    if (PostCB) {
        OSClearContext(&exceptionContext);
        (*PostCB)(retraceCount);
    }

    OSWakeupThread(&retraceQueue);
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);

    if (__VIDimming_All_Clear == true) {
        if (__OSSetVIForceDimming(false, 0, 0) == true) {
            __VIDimming_All_Clear = false;
            _gIdleCount_dimming = 0;
        }
    }

    for (i = 0; i < 10; i++) {
        if (__VIDimmingFlag_DEV_IDLE[i] == 0) {
            __VIDimmingFlag_DEV_IDLE[0] = 0;
            break;
        }
    }

    if (__VIDimmingFlag_RF_IDLE && __VIDimmingFlag_SI_IDLE && __VIDimmingFlag_DEV_IDLE[0]) {
        if ((__VIDimmingFlag_Enable == true) && (_gIdleCount_dimming < 0xFFFFFFFF)) {
            _gIdleCount_dimming++;
        }

        if ((__VIDVDStopFlag_Enable == true) && (_gIdleCount_dvd < 0xFFFFFFFF)) {
            _gIdleCount_dvd++;
        }
    } else {
        if (_gIdleCount_dimming >= NEW_TIME_TO_DIMMING) {
            DimmingOFF_Pending = 1;
        }
        if (_gIdleCount_dvd >= THD_TIME_TO_DVD_STOP) {
            __DVDRestartMotor();
        }

        _gIdleCount_dimming = 0;
        _gIdleCount_dvd = 0;
    }

    if (__VIDimmingFlag_Enable_old != __VIDimmingFlag_Enable) {
        if (__VIDimmingFlag_Enable == false) {
            if (_gIdleCount_dimming >= NEW_TIME_TO_DIMMING) {
                DimmingOFF_Pending = 1;
            }
        }

        _gIdleCount_dimming = 0;
    }

    if (_gIdleCount_dimming == NEW_TIME_TO_DIMMING) {
        DimmingON_Pending = 1;
    }

    if (DimmingOFF_Pending) {
        if (__OSSetVIForceDimming(false, 2, 2) == true) {
            DimmingOFF_Pending = 0;
        }
    }

    if (DimmingON_Pending) {
        if (__OSSetVIForceDimming(true, 2, 2) == true) {
            DimmingON_Pending = 0;
        }
    }

    if (__VIDVDStopFlag_Enable_old != __VIDVDStopFlag_Enable) {
        if (__VIDVDStopFlag_Enable == false) {
            if (_gIdleCount_dvd >= THD_TIME_TO_DVD_STOP) {
                __DVDRestartMotor();
            }
        }
        _gIdleCount_dvd = 0;
    }

    if (_gIdleCount_dvd == THD_TIME_TO_DVD_STOP) {
        __DVDStopMotorAsync(&__DVDStopMotorCommandBlock, NULL);
    }

    __VIDimmingFlag_RF_IDLE = 1;
    __VIDimmingFlag_SI_IDLE = 1;

    for (i = 0; i < 10; i++) {
        __VIDimmingFlag_DEV_IDLE[i] = 1;
    }

    __VIDimmingFlag_Enable_old = __VIDimmingFlag_Enable;
    __VIDVDStopFlag_Enable_old = __VIDVDStopFlag_Enable;
}

static void setScalingRegs(u16 panSizeX, u16 dispSizeX, bool threeD) {
    u32 scale;

    panSizeX = (u16)(threeD ? panSizeX * 2 : panSizeX);

    if (panSizeX < dispSizeX) {
        scale = (256 * (u32)panSizeX + (u32)dispSizeX - 1) / (u32)dispSizeX;

        regs[0x25] = (u16)((((unsigned long)(scale))) | (((unsigned long)(1)) << 12));
        changed |= (1ull << (63 - (0x25)));

        regs[0x38] = (u16)((((unsigned long)(panSizeX))));
        changed |= (1ull << (63 - (0x38)));
    } else {
        regs[0x25] = (u16)((((unsigned long)(256))) | (((unsigned long)(0)) << 12));
        changed |= (1ull << (63 - (0x25)));
    }
}

static void calcFbbs(u32 bufAddr, u16 panPosX, u16 panPosY, u8 wordPerLine, VIXFBMode xfbMode, u16 dispPosY, u32* tfbb,
                     u32* bfbb) {
    u32 bytesPerLine, xoffInWords;
    xoffInWords = (u32)panPosX / 16;
    bytesPerLine = (u32)wordPerLine * 32;

    *tfbb = bufAddr + xoffInWords * 32 + bytesPerLine * panPosY;
    *bfbb = (xfbMode == VI_XFB_MODE_SF) ? *tfbb : (*tfbb + bytesPerLine);

    if (dispPosY % 2 == 1) {
        u32 tmp = *tfbb;
        *tfbb = *bfbb;
        *bfbb = tmp;
    }

    *tfbb = ToPhysical(*tfbb);
    *bfbb = ToPhysical(*bfbb);
}

static inline void setHorizontalRegs(timing_s* tm, u16 dispPosX, u16 dispSizeX) {
    u32 hbe;
    u32 hbs;
    u32 hbeLo;
    u32 hbeHi;

    regs[3] = (u16)(u32)tm->hlw;
    MARK_CHANGED(3);
    regs[2] = tm->hce | (tm->hcs << 8);
    MARK_CHANGED(2);
    hbe = tm->hbe640 - 40 + dispPosX;
    hbs = tm->hbs640 + 40 + dispPosX - (720 - dispSizeX);
    hbeLo = hbe & 0x1FF;
    hbeHi = hbe >> 9;
    regs[5] = tm->hsy | (hbeLo << 7);
    MARK_CHANGED(5);
    regs[4] = hbeHi | (hbs * 2);
    MARK_CHANGED(4);
}

static u32 getCurrentHalfLine(void) {
    u32 hcount;
    u32 vcount;

    GetCurrentDisplayPosition(&hcount, &vcount);

    return ((vcount - 1) << 1) + ((hcount - 1) / CurrTiming->hlw);
}

static u32 getCurrentFieldEvenOdd(void) { return (getCurrentHalfLine() < CurrTiming->nhlines) ? 1u : 0u; }

VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback cb) {
    bool enabled;
    VIRetraceCallback oldcb = PreCB;
    enabled = OSDisableInterrupts();
    PreCB = cb;
    OSRestoreInterrupts(enabled);
    return oldcb;
}

VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback cb) {
    bool enabled;
    VIRetraceCallback oldcb = PostCB;
    enabled = OSDisableInterrupts();
    PostCB = cb;
    OSRestoreInterrupts(enabled);
    return oldcb;
}

static timing_s* getTiming(VITVMode mode) {
    switch (mode) {
        case VI_TVMODE_NTSC_INT:
            return &timing[0];
        case VI_TVMODE_NTSC_DS:
            return &timing[1];
        case VI_TVMODE_PAL_INT:
            return &timing[2];
        case VI_TVMODE_PAL_DS:
            return &timing[3];
        case VI_TVMODE_EURGB60_INT:
            return &timing[0];
        case VI_TVMODE_EURGB60_DS:
            return &timing[1];
        case VI_TVMODE_MPAL_INT:
            return &timing[4];
        case VI_TVMODE_MPAL_DS:
            return &timing[5];
        case VI_TVMODE_NTSC_PROG:
        case VI_TVMODE_MPAL_PROG:
        case VI_TVMODE_EURGB60_PROG:
            return &timing[6];
        case VI_TVMODE_NTSC_3D:
            return &timing[7];
        case VI_TVMODE_DEBUG_PAL_INT:
            return &timing[2];
        case VI_TVMODE_DEBUG_PAL_DS:
            return &timing[3];
        case VI_TVMODE_GCA_INT:
            return &timing[8];
        case VI_TVMODE_GCA_PROG:
            return &timing[9];
        case VI_TVMODE_PAL_PROG:
            break;
        case VI_TVMODE_EXTRA_INT:
        case VI_TVMODE_EXTRA_DS:
        case VI_TVMODE_EXTRA_PROG:
            return timingExtra;
    }

    return NULL;
}

void __VIInit(VITVMode mode) {
    timing_s* tm;
    u32 nonInter;
    u32 tv, tvForReg;
    vu32 a;
    u16 hct;
    u16 vct;

    nonInter = (u32)mode & 3;
    tv = (u32)mode >> 2;

    *(u32*)OSPhysicalToCached(0xCC) = tv;

    tm = getTiming(mode);

    VI_HW_REGS[1] = 2;
    for (a = 0; a < 1000; a++) {}

    VI_HW_REGS[1] = 0;
    VI_HW_REGS[3] = (u16)((((unsigned long)(tm->hlw)) << 0));
    VI_HW_REGS[2] = (u16)((((unsigned long)(tm->hce)) << 0) | (((unsigned long)(tm->hcs)) << 8));
    VI_HW_REGS[5] = (u16)((((unsigned long)(tm->hsy)) << 0) | (((unsigned long)(tm->hbe640 & ((1 << (9)) - 1))) << 7));
    VI_HW_REGS[4] = (u16)((((unsigned long)(tm->hbe640 >> 9)) << 0) | (((unsigned long)(tm->hbs640)) << 1));

    if (encoderType == 0) {
        VI_HW_REGS[0x39] = (u16)((((unsigned long)(tm->hbeCCIR656)) << 0) | (((unsigned long)(1)) << 15));
        VI_HW_REGS[0x3A] = (u16)((((unsigned long)(tm->hbsCCIR656)) << 0));
    }

    VI_HW_REGS[0] = (u16)((((unsigned long)(tm->equ)) << 0) | (((unsigned long)(0)) << 4));
    VI_HW_REGS[7] = (u16)((((unsigned long)(tm->prbOdd + tm->acv * 2 - 2)) << 0));
    VI_HW_REGS[6] = (u16)((((unsigned long)(tm->psbOdd + 2)) << 0));
    VI_HW_REGS[9] = (u16)((((unsigned long)(tm->prbEven + tm->acv * 2 - 2)) << 0));
    VI_HW_REGS[8] = (u16)((((unsigned long)(tm->psbEven + 2)) << 0));
    VI_HW_REGS[0xB] = (u16)((((unsigned long)(tm->bs1)) << 0) | (((unsigned long)(tm->be1)) << 5));
    VI_HW_REGS[0xA] = (u16)((((unsigned long)(tm->bs3)) << 0) | (((unsigned long)(tm->be3)) << 5));
    VI_HW_REGS[0xD] = (u16)((((unsigned long)(tm->bs2)) << 0) | (((unsigned long)(tm->be2)) << 5));
    VI_HW_REGS[0xC] = (u16)((((unsigned long)(tm->bs4)) << 0) | (((unsigned long)(tm->be4)) << 5));
    VI_HW_REGS[0x24] = (u16)((((unsigned long)(40)) << 0) | (((unsigned long)(40)) << 8));
    VI_HW_REGS[0x1B] = 1;
    VI_HW_REGS[0x1A] = (u16)((((unsigned long)(1)) << 0) | (((unsigned long)(1)) << 12) | (((unsigned long)(0)) << 15));

    hct = (u16)(tm->hlw + 1);
    vct = (u16)(tm->nhlines / 2 + 1);
    VI_HW_REGS[0x19] = (u16)hct;
    VI_HW_REGS[0x18] =
        (u16)((((unsigned long)(vct)) << 0) | (((unsigned long)(1)) << 12) | (((unsigned long)(0)) << 15));

    switch (tv) {
        case VI_PAL:
        case VI_MPAL:
        case VI_DEBUG:
            tvForReg = tv;
            break;

        default:
            tvForReg = VI_NTSC;
            break;
    }

    if ((nonInter == VI_INTERLACE) || (nonInter == VI_NON_INTERLACE)) {
        VI_HW_REGS[1] =
            (u16)((((unsigned long)(1)) << 0) | (((unsigned long)(0)) << 1) | (((unsigned long)(nonInter & 1)) << 2) |
                  (((unsigned long)(0)) << 3) | (((unsigned long)(0)) << 4) | (((unsigned long)(0)) << 6) |
                  (((unsigned long)(tvForReg)) << 8));
        VI_HW_REGS[0x36] = 0;
    } else {
        VI_HW_REGS[1] = (u16)((((unsigned long)(1)) << 0) | (((unsigned long)(0)) << 1) | (((unsigned long)(1)) << 2) |
                              (((unsigned long)(0)) << 3) | (((unsigned long)(0)) << 4) | (((unsigned long)(0)) << 6) |
                              (((unsigned long)(tvForReg)) << 8));
        VI_HW_REGS[0x36] = 1;
    }
}

static void AdjustPosition(u16 acv) {
    s32 coeff, frac;

    HorVer.AdjustedDispPosX = (u16)CLAMP((s16)HorVer.DispPosX + displayOffsetH, 0, 720 - HorVer.DispSizeX);

    coeff = (HorVer.FBMode == VI_XFB_MODE_SF) ? 2 : 1;
    frac = HorVer.DispPosY & 1;

    HorVer.AdjustedDispPosY = (u16)VI_MAX((s16)HorVer.DispPosY + displayOffsetV, frac);

    HorVer.AdjustedDispSizeY =
        (u16)(HorVer.DispSizeY + MIN((s16)HorVer.DispPosY + displayOffsetV - frac, 0) -
              VI_MAX((s16)HorVer.DispPosY + (s16)HorVer.DispSizeY + displayOffsetV - ((s16)acv * 2 - frac), 0));

    HorVer.AdjustedPanPosY = (u16)(HorVer.PanPosY - MIN((s16)HorVer.DispPosY + displayOffsetV - frac, 0) / coeff);

    HorVer.AdjustedPanSizeY =
        (u16)(HorVer.PanSizeY + MIN((s16)HorVer.DispPosY + displayOffsetV - frac, 0) / coeff -
              VI_MAX((s16)HorVer.DispPosY + (s16)HorVer.DispSizeY + displayOffsetV - ((s16)acv * 2 - frac), 0) / coeff);
}

static void ImportAdjustingValues(void) {
    displayOffsetH = SCGetDisplayOffsetH();
    displayOffsetV = 0;
}

void VIInit(void) {
    u16 dspCfg;
    u32 value, tv, tvInBootrom;

    if (IsInitialized) {
        return;
    }

    OSRegisterVersion(__VIVersion);
    IsInitialized = true;

    if (!(VI_HW_REGS[1] & 1)) {
        __VIInit(VI_TVMODE_NTSC_INT);
    }

    retraceCount = 0;
    changed = 0;
    shdwChanged = 0;
    changeMode = 0;
    shdwChangeMode = 0;
    flushFlag = 0;
    flushFlag3in1 = 0;

    VI_HW_REGS[0x27] = (u16)((((unsigned long)(taps[0])) << 0) | (((unsigned long)(taps[1] & ((1 << (6)) - 1))) << 10));
    VI_HW_REGS[0x26] = (u16)((((unsigned long)(taps[1] >> 6)) << 0) | (((unsigned long)(taps[2])) << 4));
    VI_HW_REGS[0x29] = (u16)((((unsigned long)(taps[3])) << 0) | (((unsigned long)(taps[4] & ((1 << (6)) - 1))) << 10));
    VI_HW_REGS[0x28] = (u16)((((unsigned long)(taps[4] >> 6)) << 0) | (((unsigned long)(taps[5])) << 4));
    VI_HW_REGS[0x2B] = (u16)((((unsigned long)(taps[6])) << 0) | (((unsigned long)(taps[7] & ((1 << (6)) - 1))) << 10));
    VI_HW_REGS[0x2A] = (u16)((((unsigned long)(taps[7] >> 6)) << 0) | (((unsigned long)(taps[8])) << 4));
    VI_HW_REGS[0x2D] = (u16)((((unsigned long)(taps[9])) << 0) | (((unsigned long)(taps[10])) << 8));
    VI_HW_REGS[0x2C] = (u16)((((unsigned long)(taps[11])) << 0) | (((unsigned long)(taps[12])) << 8));
    VI_HW_REGS[0x2F] = (u16)((((unsigned long)(taps[13])) << 0) | (((unsigned long)(taps[14])) << 8));
    VI_HW_REGS[0x2E] = (u16)((((unsigned long)(taps[15])) << 0) | (((unsigned long)(taps[16])) << 8));
    VI_HW_REGS[0x31] = (u16)((((unsigned long)(taps[17])) << 0) | (((unsigned long)(taps[18])) << 8));
    VI_HW_REGS[0x30] = (u16)((((unsigned long)(taps[19])) << 0) | (((unsigned long)(taps[20])) << 8));
    VI_HW_REGS[0x33] = (u16)((((unsigned long)(taps[21])) << 0) | (((unsigned long)(taps[22])) << 8));
    VI_HW_REGS[0x32] = (u16)((((unsigned long)(taps[23])) << 0) | (((unsigned long)(taps[24])) << 8));

    VI_HW_REGS[0x38] = (u16)640;
    ImportAdjustingValues();
    tvInBootrom = *(u32*)OSPhysicalToCached(0xCC);
    dspCfg = VI_HW_REGS[1];
    HorVer.nonInter = VIGetScanMode();

    HorVer.tv = ((((unsigned long)(dspCfg)) & 0x00000300) >> 8);
    if ((tvInBootrom == VI_EURGB60) || ((tvInBootrom == VI_PAL) && (HorVer.tv == VI_NTSC))) {
        HorVer.tv = VI_EURGB60;
    }

    tv = (HorVer.tv == VI_DEBUG) ? VI_NTSC : HorVer.tv;
    HorVer.timing = getTiming((VITVMode)VI_TVMODE(tv, HorVer.nonInter));
    regs[1] = dspCfg;
    CurrTiming = HorVer.timing;
    CurrTvMode = HorVer.tv;
    HorVer.DispSizeX = 640;
    HorVer.DispSizeY = (u16)(CurrTiming->acv * 2);
    HorVer.DispPosX = (u16)((720 - HorVer.DispSizeX) / 2);
    HorVer.DispPosY = 0;
    AdjustPosition(CurrTiming->acv);
    HorVer.FBSizeX = 640;
    HorVer.FBSizeY = (u16)(CurrTiming->acv * 2);
    HorVer.PanPosX = 0;
    HorVer.PanPosY = 0;
    HorVer.PanSizeX = 640;
    HorVer.PanSizeY = (u16)(CurrTiming->acv * 2);
    HorVer.FBMode = VI_XFB_MODE_SF;
    HorVer.wordPerLine = 40;
    HorVer.std = 40;
    HorVer.wpl = 40;
    HorVer.xof = 0;
    HorVer.black = true;
    HorVer.threeD = false;

    OSInitThreadQueue(&retraceQueue);

    value = VI_HW_REGS[0x18];
    value = (((unsigned long)(value)) & ~0x00008000) | (((unsigned long)(0)) << 15);
    VI_HW_REGS[0x18] = (u16)value;

    value = VI_HW_REGS[0x1A];
    value = (((unsigned long)(value)) & ~0x00008000) | (((unsigned long)(0)) << 15);
    VI_HW_REGS[0x1A] = (u16)value;

    PreCB = (VIRetraceCallback)NULL;
    PostCB = (VIRetraceCallback)NULL;

    __OSSetInterruptHandler(24, __VIRetraceHandler);
    __OSUnmaskInterrupts((0x80000000u >> (24)));
    OSRegisterShutdownFunction(&ShutdownFunctionInfo);

    switch (VIGetTvFormat()) {
        case VI_PAL:
            NEW_TIME_TO_DIMMING = 15000;
            THD_TIME_TO_DVD_STOP = 90000;
            break;
        default:
            NEW_TIME_TO_DIMMING = 18000;
            THD_TIME_TO_DVD_STOP = 108000;
            break;
    }

    _gIdleCount_dimming = 0;
    _gIdleCount_dvd = 0;
    __VIDimming_All_Clear = VI_DM_10M;

    VIEnableDimming(true);

    VIEnableDVDStopMotor(false);
    __VISetRevolutionModeSimple();
}

void VIWaitForRetrace(void) {
    bool enabled;
    u32 count;

    enabled = OSDisableInterrupts();
    count = retraceCount;

    do {
        OSSleepThread(&retraceQueue);
    } while (count == retraceCount);

    OSRestoreInterrupts(enabled);
}

static void setInterruptRegs(timing_s* tm) {
    u16 vct, hct, borrow;

    vct = (u16)(tm->nhlines / 2);
    borrow = (u16)(tm->nhlines % 2);
    hct = (u16)((borrow) ? tm->hlw : (u16)0);

    vct++;
    hct++;

    regs[0x19] = (u16)hct;
    changed |= (1ull << (63 - (0x19)));

    regs[0x18] = (u16)((((unsigned long)(vct))) | (((unsigned long)(1)) << 12) | (((unsigned long)(0)) << 15));
    changed |= (1ull << (63 - (0x18)));
}

static void setPicConfig(u16 fbSizeX, VIXFBMode xfbMode, u16 panPosX, u16 panSizeX, u8* wordPerLine, u8* std, u8* wpl,
                         u8* xof) {
    *wordPerLine = (u8)((fbSizeX + 15) / 16);
    *std = (u8)((xfbMode == VI_XFB_MODE_SF) ? *wordPerLine : (u8)(2 * *wordPerLine));
    *xof = (u8)(panPosX % 16);
    *wpl = (u8)((*xof + panSizeX + 15) / 16);

    regs[0x24] = (u16)((((unsigned long)(*std))) | (((unsigned long)(*wpl)) << 8));
    changed |= (1ull << (63 - (0x24)));
}

static void setBBIntervalRegs(timing_s* tm) {
    u16 val;

    val = (u16)((((unsigned long)(tm->bs1))) | (((unsigned long)(tm->be1)) << 5));
    regs[0xB] = val;
    changed |= (1ull << (63 - (0x0B)));

    val = (u16)((((unsigned long)(tm->bs3))) | (((unsigned long)(tm->be3)) << 5));
    regs[0xA] = val;
    changed |= (1ull << (63 - (0x0A)));

    val = (u16)((((unsigned long)(tm->bs2))) | (((unsigned long)(tm->be2)) << 5));
    regs[0xD] = val;
    changed |= (1ull << (63 - (0x0D)));

    val = (u16)((((unsigned long)(tm->bs4))) | (((unsigned long)(tm->be4)) << 5));
    regs[0xC] = val;
    changed |= (1ull << (63 - (0x0C)));
}

static s32 cntlzd(u64 bit) {
    u32 hi, lo;
    s32 value;

    hi = (u32)(bit >> 32);
    lo = (u32)(bit & 0xFFFFFFFF);
    value = __cntlzw(hi);

    if (value < 32) {
        return value;
    }

    return (32 + __cntlzw(lo));
}

static void PrintDebugPalCaution(void) {
    static u32 message = 0;

    if (message == 0) {
        message = 1;
        OSReport("***************************************\n");
        OSReport(" ! ! ! C A U T I O N ! ! !             \n");
        OSReport("This TV format \"DEBUG_PAL\" is only for \n");
        OSReport("temporary solution until PAL DAC board \n");
        OSReport("is available. Please do NOT use this   \n");
        OSReport("mode in real games!!!                  \n");
        OSReport("***************************************\n");
    }
}

void setFbbRegs(HorVer_s* HorVer, u32* tfbb, u32* bfbb, u32* rtfbb, u32* rbfbb) {
    u32 shifted;
    calcFbbs(HorVer->bufAddr, HorVer->PanPosX, HorVer->AdjustedPanPosY, HorVer->wordPerLine, HorVer->FBMode,
             HorVer->AdjustedDispPosY, tfbb, bfbb);

    if (HorVer->threeD) {
        calcFbbs(HorVer->rbufAddr, HorVer->PanPosX, HorVer->AdjustedPanPosY, HorVer->wordPerLine, HorVer->FBMode,
                 HorVer->AdjustedDispPosY, rtfbb, rbfbb);
    }

    if (IS_LOWER_16MB(*tfbb) && IS_LOWER_16MB(*bfbb) && IS_LOWER_16MB(*rtfbb) && IS_LOWER_16MB(*rbfbb)) {
        shifted = 0;
    } else {
        shifted = 1;
    }

    if (shifted) {
        *tfbb >>= 5;
        *bfbb >>= 5;
        *rtfbb >>= 5;
        *rbfbb >>= 5;
    }

    regs[0xF] = (u16)(*tfbb & 0xFFFF);
    changed |= (1ull << (63 - (0xF)));

    regs[0xE] = (u16)((((*tfbb >> 16))) | HorVer->xof << 8 | shifted << 12);
    changed |= (1ull << (63 - (0xE)));

    regs[0x13] = (u16)(*bfbb & 0xFFFF);
    changed |= (1ull << (63 - (0x13)));

    regs[0x12] = (u16)(*bfbb >> 16);
    changed |= (1ull << (63 - (0x12)));

    if (HorVer->threeD) {
        regs[0x11] = *rtfbb & 0xFFFF;
        changed |= (1ull << (63 - (0x11)));

        regs[0x10] = *rtfbb >> 16;
        changed |= (1ull << (63 - (0x10)));

        regs[0x15] = *rbfbb & 0xFFFF;
        changed |= (1ull << (63 - (0x15)));

        regs[0x14] = *rbfbb >> 16;
        changed |= (1ull << (63 - (0x14)));
    }
}

void setVerticalRegs(u16 dispPosY, u16 dispSizeY, u8 equ, u16 acv, u16 prbOdd, u16 prbEven, u16 psbOdd, u16 psbEven,
                     bool black) {
    u16 actualPrbOdd;
    u16 actualPrbEven;
    u16 actualPsbOdd;
    u16 actualPsbEven;
    u16 actualAcv;
    u16 c;
    u16 d;

    if (regs[0x36] & 1) {
        c = 1;
        d = 2;
    } else {
        c = 2;
        d = 1;
    }

    if (dispPosY % 2 == 0) {
        actualPrbOdd = (u16)(prbOdd + d * dispPosY);
        actualPsbOdd = (u16)(psbOdd + d * ((c * acv - dispSizeY) - dispPosY));
        actualPrbEven = (u16)(prbEven + d * dispPosY);
        actualPsbEven = (u16)(psbEven + d * ((c * acv - dispSizeY) - dispPosY));
    } else {
        actualPrbOdd = (u16)(prbEven + d * dispPosY);
        actualPsbOdd = (u16)(psbEven + d * ((c * acv - dispSizeY) - dispPosY));
        actualPrbEven = (u16)(prbOdd + d * dispPosY);
        actualPsbEven = (u16)(psbOdd + d * ((c * acv - dispSizeY) - dispPosY));
    }

    actualAcv = (u16)(dispSizeY / c);

    if (black) {
        actualPrbOdd += 2 * actualAcv - 2;
        actualPsbOdd += 2;
        actualPrbEven += 2 * actualAcv - 2;
        actualPsbEven += 2;
        actualAcv = 0;
    }

    regs[0] = (u16)(equ | actualAcv << 4);
    changed |= (1ull << (63 - (0x00)));

    regs[7] = (u16)actualPrbOdd;
    changed |= (1ull << (63 - (0x07)));

    regs[6] = (u16)actualPsbOdd;
    changed |= (1ull << (63 - (0x06)));

    regs[9] = (u16)actualPrbEven;
    changed |= (1ull << (63 - (0x09)));

    regs[8] = (u16)actualPsbEven;
    changed |= (1ull << (63 - (0x08)));
}

void VIConfigure(const GXRenderModeObj* rm) {
    timing_s* tm;
    u32 regDspCfg;
    u32 regClksel;
    int enabled;
    u32 newNonInter;
    u32 tvInBootrom;
    u32 tvInGame;

    enabled = OSDisableInterrupts();
    newNonInter = rm->viTVmode & 3;

    if (HorVer.nonInter != newNonInter) {
        changeMode = 1;
        HorVer.nonInter = newNonInter;
    }

    tvInGame = (u32)rm->viTVmode >> 2;
    tvInBootrom = *(u32*)OSPhysicalToCached(0xCC);

    if (tvInGame == VI_DEBUG_PAL) {
        PrintDebugPalCaution();
    }

    switch (tvInBootrom) {
        case VI_MPAL:
        case VI_NTSC:
        case VI_GCA:
        case 7:
            if (tvInGame == VI_NTSC || tvInGame == VI_MPAL || tvInGame == 6 || tvInGame == 7) {
                break;
            }
            goto panic;
        case VI_PAL:
        case VI_EURGB60:
            if (tvInGame == VI_PAL || tvInGame == VI_EURGB60) {
                break;
            }
        default:
        panic:
            OSPanic("vi.c", 2465, "VIConfigure(): Tried to change mode from (%d) to (%d), which is forbidden\n",
                    tvInBootrom, tvInGame);
    }

    if ((tvInGame == VI_NTSC) || (tvInGame == VI_MPAL)) {
        HorVer.tv = tvInBootrom;
    } else {
        HorVer.tv = tvInGame;
    }

    HorVer.DispPosX = rm->viXOrigin;
    HorVer.DispPosY = (HorVer.nonInter == VI_NON_INTERLACE) ? (u16)(rm->viYOrigin * 2) : rm->viYOrigin;
    HorVer.DispSizeX = rm->viWidth;
    HorVer.FBSizeX = rm->fbWidth;
    HorVer.FBSizeY = rm->xfbHeight;
    HorVer.FBMode = rm->xfbMode;
    HorVer.PanSizeX = HorVer.FBSizeX;
    HorVer.PanSizeY = HorVer.FBSizeY;
    HorVer.PanPosX = 0;
    HorVer.PanPosY = 0;
    HorVer.DispSizeY = (HorVer.nonInter == VI_PROGRESSIVE) ? HorVer.PanSizeY
                       : (HorVer.nonInter == VI_3D)        ? HorVer.PanSizeY
                       : (HorVer.FBMode == VI_XFB_MODE_SF) ? (u16)(HorVer.PanSizeY * 2)
                                                           : HorVer.PanSizeY;
    HorVer.threeD = (HorVer.nonInter == VI_3D) ? true : false;

    tm = getTiming((VITVMode)VI_TVMODE(HorVer.tv, HorVer.nonInter));
    HorVer.timing = tm;

    AdjustPosition(tm->acv);
    setInterruptRegs(tm);

    regDspCfg = regs[1];
    regClksel = regs[54];

    if (HorVer.nonInter == VI_PROGRESSIVE || HorVer.nonInter == VI_3D) {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000004) | (((u32)(1)) << 2);
        regClksel = (((u32)(regClksel)) & ~0x00000001) | (((u32)(1)) << 0);
    } else {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000004) | (((u32)(HorVer.nonInter & 1)) << 2);
        regClksel = (((u32)(regClksel)) & ~0x00000001);
    }

    regDspCfg = (((unsigned long)(regDspCfg)) & ~0x00000008) | (((unsigned long)(HorVer.threeD)) << 3);

    if ((HorVer.tv == VI_PAL) || (HorVer.tv == VI_MPAL) || (HorVer.tv == VI_DEBUG)) {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000300) | (((u32)(HorVer.tv)) << 8);
    } else {
        regDspCfg = (((u32)(regDspCfg)) & ~0x00000300);
    }

    regs[1] = (u16)regDspCfg;
    regs[54] = (u16)regClksel;
    MARK_CHANGED(1);
    MARK_CHANGED(54);

    setScalingRegs(HorVer.PanSizeX, HorVer.DispSizeX, HorVer.threeD);
    setHorizontalRegs(tm, HorVer.AdjustedDispPosX, HorVer.DispSizeX);
    setBBIntervalRegs(tm);
    setPicConfig(HorVer.FBSizeX, HorVer.FBMode, HorVer.PanPosX, HorVer.PanSizeX, &HorVer.wordPerLine, &HorVer.std,
                 &HorVer.wpl, &HorVer.xof);

    if (FBSet) {
        setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    }

    setVerticalRegs(HorVer.AdjustedDispPosY, HorVer.AdjustedDispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven,
                    tm->psbOdd, tm->psbEven, HorVer.black);
    OSRestoreInterrupts(enabled);
}

void VIConfigurePan(u16 xOrg, u16 yOrg, u16 width, u16 height) {
    bool enabled;
    timing_s* tm;

    enabled = OSDisableInterrupts();

    HorVer.PanPosX = xOrg;
    HorVer.PanPosY = yOrg;
    HorVer.PanSizeX = width;
    HorVer.PanSizeY = height;
    HorVer.DispSizeY = (u16)((HorVer.nonInter == VI_PROGRESSIVE) ? HorVer.PanSizeY
                             : (HorVer.nonInter == VI_3D)        ? HorVer.PanSizeY
                             : (HorVer.FBMode == VI_XFB_MODE_SF) ? (u16)(2 * HorVer.PanSizeY)
                                                                 : HorVer.PanSizeY);

    tm = HorVer.timing;

    AdjustPosition(tm->acv);
    setScalingRegs(HorVer.PanSizeX, HorVer.DispSizeX, HorVer.threeD);
    setPicConfig(HorVer.FBSizeX, HorVer.FBMode, HorVer.PanPosX, HorVer.PanSizeX, &HorVer.wordPerLine, &HorVer.std,
                 &HorVer.wpl, &HorVer.xof);

    if (FBSet) {
        setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    }

    setVerticalRegs(HorVer.AdjustedDispPosY, HorVer.DispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven, tm->psbOdd,
                    tm->psbEven, HorVer.black);

    OSRestoreInterrupts(enabled);
}

void VIFlush(void) {
    bool enabled;
    s32 regIndex;

    enabled = OSDisableInterrupts();
    shdwChangeMode |= changeMode;
    changeMode = 0;
    shdwChanged |= changed;

    while (changed) {
        regIndex = cntlzd(changed);
        shdwRegs[regIndex] = regs[regIndex];
        changed &= ~(1ull << (63 - (regIndex)));
    }

    flushFlag = 1;
    flushFlag3in1 = 1;
    NextBufAddr = HorVer.bufAddr;
    OSRestoreInterrupts(enabled);
}

void VISetNextFrameBuffer(void* fb) {
    bool enabled = OSDisableInterrupts();
    HorVer.bufAddr = (u32)fb;
    FBSet = 1;
    setFbbRegs(&HorVer, &HorVer.tfbb, &HorVer.bfbb, &HorVer.rtfbb, &HorVer.rbfbb);
    OSRestoreInterrupts(enabled);
}

void* VIGetCurrentFrameBuffer() { return *(void**)(&CurrBufAddr); }

void VISetBlack(bool black) {
    bool enabled;
    timing_s* tm;

    enabled = OSDisableInterrupts();
    HorVer.black = black;
    tm = HorVer.timing;
    setVerticalRegs(HorVer.AdjustedDispPosY, HorVer.DispSizeY, tm->equ, tm->acv, tm->prbOdd, tm->prbEven, tm->psbOdd,
                    tm->psbEven, HorVer.black);
    OSRestoreInterrupts(enabled);
}

u32 VIGetRetraceCount(void) { return retraceCount; }

u32 VIGetNextField(void) {
    s32 nextField;
    bool enabled;

    enabled = OSDisableInterrupts();
    nextField = getCurrentFieldEvenOdd() ^ 1;
    OSRestoreInterrupts(enabled);

    return nextField ^ (HorVer.AdjustedDispPosY & 1);
}

u32 VIGetCurrentLine(void) {
    u32 halfLine;
    timing_s* tm;
    bool enabled;

    tm = CurrTiming;
    enabled = OSDisableInterrupts();
    halfLine = getCurrentHalfLine();
    OSRestoreInterrupts(enabled);

    if (halfLine >= tm->nhlines) {
        halfLine -= tm->nhlines;
    }

    return (halfLine >> 1);
}

VITvFormat VIGetTvFormat(void) {
    u32 format;
    bool enabled;

    enabled = OSDisableInterrupts();

    switch (CurrTvMode) {
        case 0:
        case 3:
        case 6:
        case 7:
            format = 0;
            break;
        case 1:
        case 4:
            format = 1;
            break;
        case 5:
        case 2:
            format = CurrTvMode;
            break;
        default:
            break;
    }

    OSRestoreInterrupts(enabled);

    return format;
}

static inline VIScanMode VIGetScanMode(void) {
    u32 scanMode;
    bool enabled;

    enabled = OSDisableInterrupts();

    if (((((unsigned long)(VI_HW_REGS[0x36])) & 0x00000001) >> 0) == 1) {
        scanMode = VI_PROGRESSIVE;
    } else {
        if (((((unsigned long)(VI_HW_REGS[0x01])) & 0x00000004) >> 2) == 0) {
            scanMode = VI_INTERLACE;
        } else {
            scanMode = VI_NON_INTERLACE;
        }
    }
    OSRestoreInterrupts(enabled);

    return scanMode;
}

u32 VIGetDTVStatus(void) {
    u32 dtvStatus;
    bool enabled;

    enabled = OSDisableInterrupts();
    dtvStatus = ((((unsigned long)(VI_HW_REGS[0x37])) & 0x00000003) >> 0);
    OSRestoreInterrupts(enabled);

    return (dtvStatus & 0x01);
}

void __VIDisplayPositionToXY(u32 hct, u32 vct, s16* x, s16* y) {
    u32 halfLine = ((vct - 1) << 1) + ((hct - 1) / CurrTiming->hlw);

    if (HorVer.nonInter == VI_INTERLACE) {
        if (halfLine < CurrTiming->nhlines) {
            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbOdd) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbOdd) {
                *y = -1;
            } else {
                *y = (s16)((halfLine - CurrTiming->equ * 3 - CurrTiming->prbOdd) & ~1);
            }
        } else {
            halfLine -= CurrTiming->nhlines;

            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbEven) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbEven) {
                *y = -1;
            } else {
                *y = (s16)(((halfLine - CurrTiming->equ * 3 - CurrTiming->prbEven) & ~1) + 1);
            }
        }
    } else if (HorVer.nonInter == VI_NON_INTERLACE) {
        if (halfLine >= CurrTiming->nhlines) {
            halfLine -= CurrTiming->nhlines;
        }

        if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbOdd) {
            *y = -1;
        } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbOdd) {
            *y = -1;
        } else {
            *y = (s16)((halfLine - CurrTiming->equ * 3 - CurrTiming->prbOdd) & ~1);
        }
    } else if (HorVer.nonInter == VI_PROGRESSIVE) {
        if (halfLine < CurrTiming->nhlines) {
            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbOdd) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbOdd) {
                *y = -1;
            } else {
                *y = (s16)(halfLine - CurrTiming->equ * 3 - CurrTiming->prbOdd);
            }
        } else {
            halfLine -= CurrTiming->nhlines;

            if (halfLine < CurrTiming->equ * 3 + CurrTiming->prbEven) {
                *y = -1;
            } else if (halfLine >= CurrTiming->nhlines - CurrTiming->psbEven) {
                *y = -1;
            } else {
                *y = (s16)((halfLine - CurrTiming->equ * 3 - CurrTiming->prbEven) & ~1);
            }
        }
    }

    *x = (s16)(hct - 1);
}

static inline void __VIGetCurrentPosition(s16* x, s16* y) {
    u32 hcount;
    u32 vcount;

    GetCurrentDisplayPosition(&hcount, &vcount);
    __VIDisplayPositionToXY(hcount, vcount, x, y);
}

static inline bool VIEnableDVDStopMotor(bool enable) {
    bool old = __VIDVDStopFlag_Enable;
    __VIDVDStopFlag_Enable = enable;
    return old;
}

static inline bool VIEnableDimming(bool enable) {
    u8 value;
    bool old = __VIDimmingFlag_Enable;

    if (enable == true) {
        value = SCGetScreenSaverMode();

        if (value == 0) {
            enable = false;
        }
    }

    __VIDimmingFlag_Enable = enable;
    return old;
}

bool fn_8009AA34(void) {
    __VIDimmingFlag_RF_IDLE = 0;
    return true;
}

bool __VIResetSIIdle(void) {
    __VIDimmingFlag_SI_IDLE = 0;
    return true;
}
