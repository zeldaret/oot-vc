#ifndef REVOLUTION_DEMO_H_
#define REVOLUTION_DEMO_H_

#include "revolution/gx.h"
#include "revolution/pad.h"
#include "revolution/types.h"

typedef enum {
    DMTF_POINTSAMPLE,
    DMTF_BILERP,
} DMTexFlt;

typedef enum {
    DM_FT_OPQ,
    DM_FT_RVS,
    DM_FT_XLU
} DMFontType;

typedef enum DEMO_STAT_TYPE {
    DEMO_STAT_GP0 = 0,
    DEMO_STAT_GP1 = 1,
    DEMO_STAT_MEM = 2,
    DEMO_STAT_PIX = 3,
    DEMO_STAT_VC = 4,
    DEMO_STAT_FR = 5,
    DEMO_STAT_TBW = 6,
    DEMO_STAT_TBP = 7,
    DEMO_STAT_MYC = 8,
    DEMO_STAT_MYR = 9,
} DEMO_STAT_TYPE;

typedef enum {
    DEMO_STAT_TL = 0,
    DEMO_STAT_BL = 1,
    DEMO_STAT_TLD = 2,
    DEMO_STAT_BLD = 3,
    DEMO_STAT_IO = 4,
} DEMO_STAT_DISP;

typedef struct DemoStatData {
    char text[50];
    DEMO_STAT_TYPE stat_type;
    unsigned long stat;
    unsigned long count;
} DemoStatData;

typedef struct DEMODMPad {
    /* 0x00 */ PADStatus pst;
    /* 0x0C */ u16 buttonDown;
    /* 0x0E */ u16 buttonUp;
    /* 0x10 */ u16 dirs;
    /* 0x12 */ u16 dirsNew;
    /* 0x14 */ u16 dirsReleased;
    /* 0x16 */ s16 stickDeltaX;
    /* 0x18 */ s16 stickDeltaY;
    /* 0x1A */ s16 substickDeltaX;
    /* 0x1C */ s16 substickDeltaY;
} DEMODMPad; // size = 0x1E

extern void* DemoCurrentBuffer;
extern void* DemoFrameBuffer2;
extern void* DemoFrameBuffer1;

extern DEMODMPad DemoPad[4];
extern u8 DemoStatEnable;

void DEMOInit(GXRenderModeObj* mode);
void DEMOBeforeRender(void);
void DEMODoneRender(void);
void DEMOSwapBuffers(void);
GXRenderModeObj* DEMOGetRenderModeObj(void);
void DEMOReInit(GXRenderModeObj* mode);
void DEMOInitCaption(s32, s32, s32);
void DEMOPuts(s16, s16, s16, char*);
void DEMOPrintf(s16, s16, s16, char*, ...);
void DEMOPadRead(void);
void DEMOPadInit(void);
void DEMOUpdateStats(u8 inc);
void DEMOPrintStats(void);
void DEMOSetGPHangMetric(GXBool enable);

#endif
