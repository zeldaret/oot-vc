#include "emulator/vc64_RVL.h"
#include "emulator/frame.h"
#include "emulator/rom.h"
#include "emulator/system.h"
#include "emulator/xlFileRVL.h"
#include "emulator/xlHeap.h"
#include "emulator/xlObject.h"
#include "macros.h"
#include "revolution/demo.h"
#include "revolution/gx.h"
#include "revolution/vi.h"
#include "string.h"

static char* gaszArgument[12];
System* gpSystem;

static void simulatorDEMOSwapBuffers(void) {
    if ((void*)DemoCurrentBuffer == (void*)DemoFrameBuffer1) {
        DemoCurrentBuffer = DemoFrameBuffer2;
    } else {
        DemoCurrentBuffer = DemoFrameBuffer1;
    }
}

void simulatorDEMODoneRender(void) {
    SYSTEM_FRAME(gpSystem)->nMode = 0;
    SYSTEM_FRAME(gpSystem)->nModeVtx = -1;
    frameDrawReset(SYSTEM_FRAME(gpSystem), 0x5FFED);

    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    GXSetColorUpdate(GX_ENABLE);
    GXCopyDisp(DemoCurrentBuffer, GX_TRUE);
    GXDrawDone();
    VISetNextFrameBuffer(DemoCurrentBuffer);
    VIFlush();
    VIWaitForRetrace();
    simulatorDEMOSwapBuffers();
}

bool simulatorDVDShowError(s32 nStatus, void* anData, s32 nSizeRead, u32 nOffset) { return true; }

bool simulatorDVDOpen(char* szNameFile, DVDFileInfo* pFileInfo) { return false; }

bool simulatorDVDRead(DVDFileInfo* pFileInfo, void* anData, s32 nSizeRead, s32 nOffset, DVDCallback callback) {
    return false;
}

bool simulatorShowLoad(s32 unknown, char* szNameFile, f32 rProgress) { return true; }

static bool simulatorParseArguments(void) {
    char* szText;
    char* szValue;
    s32 iArgument;

    for (iArgument = 0; iArgument < SAT_COUNT; iArgument++) {
        gaszArgument[iArgument] = NULL;
    }

    iArgument = 1;

    while (iArgument < xlCoreGetArgumentCount()) {
        xlCoreGetArgument(iArgument, &szText);
        iArgument++;
        if (szText[0] == '-' || szText[0] == '/' || szText[0] == '\\') {
            if (szText[2] == '\0') {
                xlCoreGetArgument(iArgument, &szValue);
                iArgument++;
            } else {
                szValue = &szText[2];
            }

            switch (szText[1]) {
                case 'L':
                case 'l':
                    gaszArgument[SAT_UNK9] = szValue;
                    break;
                case 'H':
                case 'h':
                    gaszArgument[SAT_UNK10] = szValue;
                    break;
                case 'V':
                case 'v':
                    gaszArgument[SAT_VIBRATION] = szValue;
                    break;
                case 'R':
                case 'r':
                    gaszArgument[SAT_RESET] = szValue;
                    break;
                case 'P':
                case 'p':
                    gaszArgument[SAT_PROGRESSIVE] = szValue;
                    break;
                case 'G':
                case 'g':
                    gaszArgument[SAT_CONTROLLER] = szValue;
                    break;
                case 'C':
                case 'c':
                    gaszArgument[SAT_MEMORYCARD] = szValue;
                    break;
                case 'M':
                case 'm':
                    gaszArgument[SAT_MOVIE] = szValue;
                    break;
                case 'X':
                case 'x':
                    gaszArgument[SAT_XTRA] = szValue;
                    break;
            }
        } else {
            gaszArgument[SAT_NAME] = szText;
        }
    }
    return true;
}

bool simulatorGetArgument(SimulatorArgumentType eType, char** pszArgument) {
    if (eType != SAT_NONE && pszArgument != NULL && gaszArgument[eType] != NULL) {
        *pszArgument = gaszArgument[eType];
        return true;
    }

    return false;
}

static inline bool simulatorRun(SystemMode* peMode) {
    s32 nResult;

    while (systemGetMode(gpSystem, peMode) && *peMode == SM_RUNNING) {
        nResult = systemExecute(gpSystem, 100000);
        if (!nResult) {
            return nResult;
        }
    }

    return true;
}

bool xlMain(void) {
    SystemMode eMode;
    s32 nSize0;
    s32 nSize1;
    GXColor color;
    char acNameROM[32];

    simulatorParseArguments();

    if (!xlHeapGetHeap1Free(&nSize0)) {
        return false;
    }

    if (nSize0 > 0x01800000) {
        OSReport("\n\nERROR: This program MUST be run on a system with 24MB (or less) memory!\n");
        OSPanic("vc64_RVL.c", 1352, "       Please reduce memory-size to 24MB (using 'setsmemsize 0x1800000')\n\n");
    }

#ifdef __MWERKS__
    asm {
        li      r3, 0x706
        oris    r3, r3, 0x706
        mtspr   GQR6, r3
        li      r3, 0x507
        oris    r3, r3, 0x507
        mtspr   GQR7, r3
    }
#endif

    VISetBlack(true);
    VIFlush();
    VIWaitForRetrace();

    color.r = color.g = color.b = 0;
    color.a = 255;

    GXSetCopyClear(color, 0xFFFFFF);

    if (!xlHeapGetHeap1Free(&nSize0)) {
        return false;
    }

    if (!xlObjectMake((void**)&gpSystem, NULL, &gClassSystem)) {
        return false;
    }

    if (!xlFileSetOpen((DVDOpenCallback)simulatorDVDOpen)) {
        return false;
    }

    if (!xlFileSetRead((DVDReadCallback)simulatorDVDRead)) {
        return false;
    }

    strcpy(acNameROM, "rom");

    if (!romSetImage(SYSTEM_ROM(gpSystem), acNameROM)) {
        return false;
    }

    if (!systemReset(gpSystem)) {
        return false;
    }

    if (!frameShow(SYSTEM_FRAME(gpSystem))) {
        return false;
    }

    if (!xlHeapGetHeap1Free(&nSize1)) {
        return false;
    }

    if (!systemSetMode(gpSystem, SM_RUNNING)) {
        return false;
    }

    simulatorRun(&eMode);

    if (!xlObjectFree((void**)&gpSystem)) {
        return false;
    }

    return true;
}
