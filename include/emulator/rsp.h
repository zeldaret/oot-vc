#ifndef _RSP_H
#define _RSP_H

#include "emulator/vc64_RVL.h"
#include "emulator/xlList.h"
#include "emulator/xlObject.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SP_IMEM_SIZE 0x1000
#define SP_DMEM_SIZE 0x1000

#define SP_DMEM_START 0x04000000
#define SP_IMEM_START 0x04001000

#define SP_BASE_REG 0x04040000
#define SP_MEM_ADDR_REG (SP_BASE_REG | 0x0000)
#define SP_DRAM_ADDR_REG (SP_BASE_REG | 0x0004)
#define SP_RD_LEN_REG (SP_BASE_REG | 0x0008)
#define SP_WR_LEN_REG (SP_BASE_REG | 0x000C)
#define SP_STATUS_REG (SP_BASE_REG | 0x0010)
#define SP_DMA_FULL_REG (SP_BASE_REG | 0x0014)
#define SP_DMA_BUSY_REG (SP_BASE_REG | 0x0018)
#define SP_SEMAPHORE_REG (SP_BASE_REG | 0x001C)

#define SP_PC_REG 0x04080000
#define SP_IBIST_REG 0x04080004

#define RSP_REG_ADDR_HI(addr) (((addr) >> 12) & 0xFFF)
#define RSP_REG_ADDR_LO(addr) ((addr) & 0x1F)
#define RSP_TASK(pRSP) ((RspTask*)((u8*)pRSP->pDMEM + (SP_DMEM_SIZE - sizeof(RspTask))))

#define GBI_COMMAND_HI(p) (((u32*)(p))[0])
#define GBI_COMMAND_LO(p) (((u32*)(p))[1])

#define SEGMENT_ADDRESS(pRSP, nOffsetRDRAM) \
    (pRSP->anBaseSegment[((nOffsetRDRAM) >> 24) & 0xF] + ((nOffsetRDRAM) & 0xFFFFFF))
#define AUDIO_SEGMENT_ADDRESS(pRSP, nOffsetRDRAM) \
    (pRSP->anAudioBaseSegment[((nOffsetRDRAM) >> 24) & 0xF] + ((nOffsetRDRAM) & 0xFFFFFF))

#define G_OBJLT_TXTRBLOCK 0x00001033
#define G_OBJLT_TXTRTILE 0x00FC1034
#define G_OBJLT_TLUT 0x00000030

#define G_IM_SIZ_4b 0
#define G_IM_SIZ_8b 1
#define G_IM_SIZ_16b 2
#define G_IM_SIZ_32b 3

#define G_IM_FMT_RGBA 0
#define G_IM_FMT_YUV 1
#define G_IM_FMT_CI 2
#define G_IM_FMT_IA 3
#define G_IM_FMT_I 4

#define G_TX_WRAP (1 << 0)
#define G_TX_CLAMP (1 << 1)

typedef enum RspAudioUCodeType {
    RUT_NOCODE = -1,
    RUT_ABI1 = 0,
    RUT_ABI2 = 1,
    RUT_ABI3 = 2,
    RUT_ABI4 = 3,
    RUT_UNKNOWN = 4,
} RspAudioUCodeType;

typedef enum RspUCodeType {
    RUT_NONE = -1,
    RUT_TURBO = 0,
    RUT_SPRITE2D = 1,
    RUT_FAST3D = 2,
    RUT_ZSORT = 3,
    RUT_LINE3D = 4,
    RUT_F3DEX1 = 5,
    RUT_F3DEX2 = 6,
    RUT_S2DEX1 = 7,
    RUT_S2DEX2 = 8,
    RUT_L3DEX1 = 9,
    RUT_L3DEX2 = 10,
    RUT_AUDIO1 = 11,
    RUT_AUDIO2 = 12,
    RUT_JPEG = 13,
} RspUCodeType;

typedef enum RspUpdateMode {
    RUM_NONE = 0,
    RUM_IDLE = 1,
} RspUpdateMode;

typedef struct RspTask {
    /* 0x00 */ s32 nType;
    /* 0x04 */ s32 nFlag;
    /* 0x08 */ s32 nOffsetBoot;
    /* 0x0C */ s32 nLengthBoot;
    /* 0x10 */ s32 nOffsetCode;
    /* 0x14 */ s32 nLengthCode;
    /* 0x18 */ s32 nOffsetData;
    /* 0x1C */ s32 nLengthData;
    /* 0x20 */ s32 nOffsetStack;
    /* 0x24 */ s32 nLengthStack;
    /* 0x28 */ s32 nOffsetBuffer;
    /* 0x2C */ s32 nLengthBuffer;
    /* 0x30 */ s32 nOffsetMBI;
    /* 0x34 */ s32 nLengthMBI;
    /* 0x38 */ s32 nOffsetYield;
    /* 0x3C */ s32 nLengthYield;
} RspTask; // size = 0x40

typedef struct RspYield {
    /* 0x00 */ s32 iDL;
    /* 0x04 */ bool bValid;
    /* 0x08 */ RspTask task;
    /* 0x48 */ s32 nCountVertex;
    /* 0x4C */ RspUCodeType eTypeUCode;
    /* 0x50 */ u32 n2TriMult;
    /* 0x54 */ u32 nVersionUCode;
    /* 0x58 */ s32 anBaseSegment[16];
    /* 0x98 */ u64* apDL[16];
} RspYield; // size = 0xD8

typedef struct __anon_0x57AB1 {
    /* 0x00 */ f32 aRotations[2][2];
    /* 0x10 */ f32 fX;
    /* 0x14 */ f32 fY;
    /* 0x18 */ f32 fBaseScaleX;
    /* 0x1C */ f32 fBaseScaleY;
} __anon_0x57AB1; // size = 0x20

typedef struct __anon_0x57BBE {
    /* 0x0 */ f32 rS;
    /* 0x4 */ f32 rT;
    /* 0x8 */ s16 nX;
    /* 0xA */ s16 nY;
    /* 0xC */ s16 nZ;
    /* 0xE */ u8 anData[4];
} __anon_0x57BBE; // size = 0x14

typedef struct __anon_0x57CD6 {
    /* 0x0 */ char anNormal[3];
} __anon_0x57CD6; // size = 0x3

typedef struct __anon_0x57D55 {
    /* 0x0 */ u8 anMaterial[4];
} __anon_0x57D55; // size = 0x4

typedef struct __anon_0x57DF8 {
    /* 0x0 */ f32 aMatrix[4][4];
} __anon_0x57DF8; // size = 0x40

typedef struct __anon_0x57E56 {
    /* 0x0 */ u8 nRed;
    /* 0x1 */ u8 nGreen;
    /* 0x2 */ u8 nBlue;
    /* 0x3 */ s8 rVectorX;
    /* 0x4 */ s8 rVectorY;
    /* 0x5 */ s8 rVectorZ;
} __anon_0x57E56; // size = 0x6

typedef struct __anon_0x58107 {
    /* 0x0 */ s16 anSlice[8];
} __anon_0x58107; // size = 0x10

typedef struct __anon_0x58360 {
    /* 0x0 */ s16 r;
    /* 0x2 */ s16 g;
    /* 0x4 */ s16 b;
    /* 0x6 */ s16 a;
} __anon_0x58360; // size = 0x8

typedef struct __anon_0x583EE {
    /* 0x0 */ s16 y;
    /* 0x2 */ s16 u;
    /* 0x4 */ s16 v;
} __anon_0x583EE; // size = 0x6

typedef struct RspUCode {
    /* 0x00 */ s32 nOffsetCode;
    /* 0x04 */ s32 nLengthCode;
    /* 0x08 */ s32 nOffsetData;
    /* 0x0C */ s32 nLengthData;
    /* 0x10 */ char acUCodeName[64];
    /* 0x50 */ u64 nUCodeCheckSum;
    /* 0x58 */ s32 nCountVertex;
    /* 0x5C */ RspUCodeType eType;
} RspUCode; // size = 0x60

typedef struct Rsp {
    /* 0x0000 */ s32 nMode;
    /* 0x0004 */ s32 nBIST;
    /* 0x0008 */ s32 nStatus;
    /* 0x000C */ s32 unk000C;
    /* 0x0010 */ u8 pIMEM[SP_IMEM_SIZE];
    /* 0x1010 */ u8 pDMEM[SP_DMEM_SIZE];
    /* 0x2010 */ s32 nSemaphore;
    /* 0x2014 */ s32 nFullDMA;
    /* 0x2018 */ s32 nBusyDMA;
    /* 0x201C */ s32 nSizeGet;
    /* 0x2020 */ s32 nSizePut;
    /* 0x2024 */ s32 nAddressSP;
    /* 0x2028 */ s32 nAddressRDRAM;
    /* 0x202C */ s32 nPC;
    /* 0x2030 */ s32 unk2030;
    /* 0x2034 */ u8 unk2034[0x2574 - 0x2034];
    /* 0x2574 */ s32 nGeometryMode;
    /* 0x2578 */ RspYield yield;
    /* 0x2650 */ u32 nTickLast;
    /* 0x2654 */ s32 (*pfUpdateWaiting)(void);
    /* 0x2658 */ u32 n2TriMult;
    /* 0x265C */ s32 aStatus[4];
    /* 0x266C */ f32 aMatrixOrtho[4][4];
    /* 0x26AC */ u32 nMode2D;
    /* 0x26B0 */ struct __anon_0x57AB1 twoDValues;
    /* 0x26D0 */ s32 nPass;
    /* 0x26D4 */ u32 nZSortSubDL;
    /* 0x26D8 */ u32 nStatusSubDL;
    /* 0x26DC */ u32 nNumZSortLights;
    /* 0x26E0 */ s32 aLightAddresses[8];
    /* 0x2700 */ s32 nAmbientLightAddress;
    /* 0x2704 */ struct __anon_0x57BBE aZSortVertex[128];
    /* 0x3104 */ struct __anon_0x57CD6 aZSortNormal[128];
    /* 0x3284 */ struct __anon_0x57D55 aZSortMaterial[128];
    /* 0x3484 */ struct __anon_0x57DF8 aZSortMatrix[128];
    /* 0x5484 */ struct __anon_0x57E56 aZSortLight[8];
    /* 0x54B4 */ s32 aZSortInvW[128];
    /* 0x56B4 */ s16 aZSortWiVal[128];
    /* 0x57B4 */ u32 nNumZSortMatrices;
    /* 0x57B8 */ u32 nNumZSortVertices;
    /* 0x57BC */ u32 nTotalZSortVertices;
    /* 0x57C0 */ u32 nNumZSortNormals;
    /* 0x57C4 */ u32 nNumZSortMaterials;
    /* 0x57C8 */ s32 anAudioBaseSegment[16];
    /* 0x5808 */ s16* anAudioBuffer;
    /* 0x580C */ s16 anADPCMCoef[5][2][8];
    /* 0x58AC */ u16 nAudioDMOutR[2];
    /* 0x58B0 */ u16 nAudioDMauxL[2];
    /* 0x58B4 */ u16 nAudioDMauxR[2];
    /* 0x58B8 */ u16 nAudioCount[2];
    /* 0x58BC */ u16 nAudioFlags;
    /* 0x58BE */ u16 nAudioDMEMIn[2];
    /* 0x58C2 */ u16 nAudioDMEMOut[2];
    /* 0x58C8 */ u32 nAudioLoopAddress;
    /* 0x58CC */ u32 nAudioDryAmt;
    /* 0x58D0 */ u32 nAudioWetAmt;
    /* 0x58D4 */ u32 nAudioVolL;
    /* 0x58D8 */ u32 nAudioVolR;
    /* 0x58DC */ u32 nAudioVolTGTL;
    /* 0x58E0 */ u32 nAudioVolRateLM;
    /* 0x58E4 */ u32 nAudioVolRateLL;
    /* 0x58E8 */ u32 nAudioVolTGTR;
    /* 0x58EC */ u32 nAudioVolRateRM;
    /* 0x58F0 */ u32 nAudioVolRateRL;
    /* 0x58F4 */ struct __anon_0x58107 vParams;
    /* 0x5904 */ s16 stepF;
    /* 0x5906 */ s16 stepL;
    /* 0x5908 */ s16 stepR;
    /* 0x590A */ u16 nAudioMemOffset;
    /* 0x590C */ u16 nAudioADPCMOffset;
    /* 0x590E */ u16 nAudioScratchOffset;
    /* 0x5910 */ u16 nAudioParBase;
    /* 0x5914 */ s32 iDL;
    /* 0x5918 */ s32 nCountVertex;
    /* 0x591C */ struct tXL_LIST* pListUCode;
    /* 0x5920 */ RspUCodeType eTypeUCode;
    /* 0x5924 */ u32 nVersionUCode;
    /* 0x5928 */ RspAudioUCodeType eTypeAudioUCode;
    /* 0x592C */ s32 anBaseSegment[16];
    /* 0x596C */ u64* apDL[16];
    /* 0x59AC */ s32* Coeff;
    /* 0x59B0 */ int* dctBuf;
    /* 0x59B4 */ int* Zigzag;
    /* 0x59B8 */ s16* QTable;
    /* 0x59BC */ struct __anon_0x583EE* yuvBuf;
    /* 0x59C0 */ s16* QYTable;
    /* 0x59C4 */ s16* QCbTable;
    /* 0x59C8 */ s16* QCrTable;
    /* 0x59CC */ struct __anon_0x58360* rgbaBuf;
} Rsp; // size = 0x59D0

typedef struct uObjBg_t {
    /* 0x00 */ u16 imageX;
    /* 0x02 */ u16 imageW;
    /* 0x04 */ s16 frameX;
    /* 0x06 */ u16 frameW;
    /* 0x08 */ u16 imageY;
    /* 0x0A */ u16 imageH;
    /* 0x0C */ s16 frameY;
    /* 0x0E */ u16 frameH;
    /* 0x10 */ u32 imagePtr;
    /* 0x14 */ u16 imageLoad;
    /* 0x16 */ u8 imageFmt;
    /* 0x17 */ u8 imageSiz;
    /* 0x18 */ u16 imagePal;
    /* 0x1A */ u16 imageFlip;
    /* 0x1C */ u16 tmemW;
    /* 0x1E */ u16 tmemH;
    /* 0x20 */ u16 tmemLoadSH;
    /* 0x22 */ u16 tmemLoadTH;
    /* 0x24 */ u16 tmemSizeW;
    /* 0x26 */ u16 tmemSize;
} uObjBg_t; // size = 0x28

typedef struct uObjScaleBg_t {
    /* 0x00 */ u16 imageX;
    /* 0x02 */ u16 imageW;
    /* 0x04 */ s16 frameX;
    /* 0x06 */ u16 frameW;
    /* 0x08 */ u16 imageY;
    /* 0x0A */ u16 imageH;
    /* 0x0C */ s16 frameY;
    /* 0x0E */ u16 frameH;
    /* 0x10 */ u32 imagePtr;
    /* 0x14 */ u16 imageLoad;
    /* 0x16 */ u8 imageFmt;
    /* 0x17 */ u8 imageSiz;
    /* 0x18 */ u16 imagePal;
    /* 0x1A */ u16 imageFlip;
    /* 0x1C */ u16 scaleW;
    /* 0x1E */ u16 scaleH;
    /* 0x20 */ s32 imageYorig;
    /* 0x24 */ u8 padding[4];
} uObjScaleBg_t; // size = 0x28

typedef union uObjBg {
    uObjBg_t b;
    uObjScaleBg_t s;
    s64 force_structure_alignment;
} uObjBg;

typedef struct uObjSprite_t {
    /* 0x00 */ s16 objX;
    /* 0x02 */ u16 scaleW;
    /* 0x04 */ u16 imageW;
    /* 0x06 */ u16 paddingX;
    /* 0x08 */ s16 objY;
    /* 0x0A */ u16 scaleH;
    /* 0x0C */ u16 imageH;
    /* 0x0E */ u16 paddingY;
    /* 0x10 */ u16 imageStride;
    /* 0x12 */ u16 imageAdrs;
    /* 0x14 */ u8 imageFmt;
    /* 0x15 */ u8 imageSiz;
    /* 0x16 */ u8 imagePal;
    /* 0x17 */ u8 imageFlags;
} uObjSprite_t; // size = 0x18

typedef union uObjSprite {
    uObjSprite_t s;
    s64 force_structure_alignment;
} uObjSprite;

typedef struct uObjTxtrBlock_t {
    /* 0x00 */ u32 type;
    /* 0x04 */ u32 image;
    /* 0x08 */ u16 tmem;
    /* 0x0A */ u16 tsize;
    /* 0x0C */ u16 tline;
    /* 0x0E */ u16 sid;
    /* 0x10 */ u32 flag;
    /* 0x14 */ u32 mask;
} uObjTxtrBlock_t; // size = 0x18

typedef struct uObjTxtrTile_t {
    /* 0x00 */ u32 type;
    /* 0x04 */ u32 image;
    /* 0x08 */ u16 tmem;
    /* 0x0A */ u16 twidth;
    /* 0x0C */ u16 theight;
    /* 0x0E */ u16 sid;
    /* 0x10 */ u32 flag;
    /* 0x14 */ u32 mask;
} uObjTxtrTile_t; // size = 0x18

typedef struct uObjTxtrTLUT_t {
    /* 0x00 */ u32 type;
    /* 0x04 */ u32 image;
    /* 0x08 */ u16 phead;
    /* 0x0A */ u16 pnum;
    /* 0x0C */ u16 zero;
    /* 0x0E */ u16 sid;
    /* 0x10 */ u32 flag;
    /* 0x14 */ u32 mask;
} uObjTxtrTLUT_t; // size = 0x18

typedef union uObjTxtr {
    uObjTxtrBlock_t block;
    uObjTxtrTile_t tile;
    uObjTxtrTLUT_t tlut;
    s64 force_structure_alignment;
} uObjTxtr;

typedef struct zVtxDest {
    /* 0x0 */ s16 sx;
    /* 0x2 */ s16 sy;
    /* 0x4 */ int invw;
    /* 0x8 */ s16 xi;
    /* 0xA */ s16 yi;
    /* 0xC */ u8 cc;
    /* 0xD */ u8 fog;
    /* 0xE */ s16 wi;
} zVtxDest; // size = 0x10

bool rspFillObjBgScale(Rsp* pRSP, s32 nAddress, uObjBg* pBg);
bool rspPut32(Rsp* pRSP, u32 nAddress, s32* pData);
bool rspGet32(Rsp* pRSP, u32 nAddress, s32* pData);
bool rspInvalidateCache(Rsp* pRSP, s32 nOffset0, s32 nOffset1);
bool rspEnableABI(Rsp* pRSP, bool bFlag);
bool rspFrameComplete(Rsp* pRSP);
bool rspUpdate(Rsp* pRSP, RspUpdateMode eMode);
bool rspGetDMEM(Rsp* pRSP, void** pBuffer, s32 nOffset, u32 nSize);
bool rspGetIMEM(Rsp* pRSP, void** pBuffer, s32 nOffset, u32 nSize);
bool rspGetBuffer(Rsp* pRSP, void** pBuffer, s32 nOffset, u32* pnSize);
bool rspEvent(Rsp* pRSP, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassRSP;

#ifdef __cplusplus
}
#endif

#endif
