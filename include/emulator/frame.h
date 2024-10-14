#ifndef _FRAME_H
#define _FRAME_H

#include "emulator/rdp.h"
#include "emulator/rsp.h"
#include "emulator/xlObject.h"
#include "revolution/gx.h"
#include "revolution/mtx.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FRAME_SYNC_TOKEN 0x7D00

// N64 frame buffer dimensions
#define N64_FRAME_WIDTH 320
#define N64_FRAME_HEIGHT 240

// GC is rendered at double the resolution
#define GC_FRAME_WIDTH (N64_FRAME_WIDTH * 2)
#define GC_FRAME_HEIGHT (N64_FRAME_HEIGHT * 2)
#define GC_FRAME_HEIGHT_PAL (GC_FRAME_HEIGHT + 48)

// Dimensions of the player preview on the equipment menu of the Zelda pause screen
#define ZELDA_PAUSE_EQUIP_PLAYER_WIDTH 64
#define ZELDA_PAUSE_EQUIP_PLAYER_HEIGHT 112

#define ZELDA2_CAMERA_WIDTH 160
#define ZELDA2_CAMERA_HEIGHT 128

typedef bool (*FrameDrawFunc)(void*, void*);

typedef enum FrameMatrixType {
    FMT_MODELVIEW = 0,
    FMT_PROJECTION = 1,
} FrameMatrixType;

typedef enum FrameModeType {
    FMT_NONE = -1,
    FMT_FOG = 0,
    FMT_GEOMETRY = 1,
    FMT_TEXTURE1 = 2,
    FMT_TEXTURE2 = 3,
    FMT_OTHER0 = 4,
    FMT_OTHER1 = 5,
    FMT_COMBINE_COLOR1 = 6,
    FMT_COMBINE_COLOR2 = 7,
    FMT_COMBINE_ALPHA1 = 8,
    FMT_COMBINE_ALPHA2 = 9,
    FMT_COUNT = 10,
} FrameModeType;

typedef enum FrameSize {
    FS_NONE = -1,
    FS_SOURCE = 0,
    FS_TARGET = 1,
    FS_COUNT = 2,
} FrameSize;

typedef enum FrameBufferType {
    FBT_NONE = -1,
    FBT_DEPTH = 0,
    FBT_IMAGE = 1,
    FBT_COLOR_SHOW = 2,
    FBT_COLOR_DRAW = 3,
    FBT_COUNT = 4,
} FrameBufferType;

typedef enum FrameResetType {
    FRT_NONE = -1,
    FRT_COLD = 0,
    FRT_WARM = 1,
} FrameResetType;

typedef enum FrameLoadType {
    FLT_NONE = -1,
    FLT_TILE = 0,
    FLT_BLOCK = 1,
} FrameLoadType;

typedef enum FrameMatrixProjection {
    FMP_NONE = -1,
    FMP_PERSPECTIVE = 0,
    FMP_ORTHOGRAPHIC = 1,
} FrameMatrixProjection;

typedef enum FrameColorType {
    FCT_NONE = -1,
    FCT_FOG,
    FCT_FILL,
    FCT_BLEND,
    FCT_PRIMITIVE,
    FCT_ENVIRONMENT,
    FCT_COUNT
} FrameColorType;

typedef struct Texture64 {
    /* 0x00 */ s32 magic; // 'VC64'
    /* 0x04 */ u32 nTypeROM; // `SystemRomType`
    /* 0x08 */ u32 unk_34;
    /* 0x0C */ s32 nSizeX;
    /* 0x10 */ s32 nSizeY;
    /* 0x14 */ GXTexWrapMode eWrapS;
    /* 0x18 */ GXTexWrapMode eWrapT;
    /* 0x1C */ s32 nMode;
    /* 0x20 */ GXTexFmt eFormat;
    /* 0x24 */ u32 nAddress;
    /* 0x28 */ u32 nCodePixel;
    /* 0x2C */ u32 nCodeColor;
    /* 0x30 */ u32 nData0;
    /* 0x34 */ u32 nData1;
} Texture64; // size = 0x38

typedef struct Primitive {
    /* 0x0 */ s32 nCount;
    /* 0x4 */ u8 anData[768];
} Primitive; // size = 0x304

typedef struct Viewport {
    /* 0x0 */ f32 rX;
    /* 0x4 */ f32 rY;
    /* 0x8 */ f32 rSizeX;
    /* 0xC */ f32 rSizeY;
} Viewport; // size = 0x10

typedef struct FrameBuffer {
    /* 0x00 */ s32 nSize;
    /* 0x04 */ s32 nWidth;
    /* 0x08 */ s32 nFormat;
    /* 0x0C */ void* pData;
    /* 0x10 */ s32 nAddress;
} FrameBuffer; // size = 0x14

typedef struct Vec3f {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
} Vec3f; // size = 0xC

typedef struct Light {
    /* 0x00 */ bool bTransformed;
    /* 0x04 */ Vec3f rVecOrigTowards;
    /* 0x10 */ f32 rColorR;
    /* 0x14 */ f32 rColorG;
    /* 0x18 */ f32 rColorB;
    /* 0x1C */ f32 rVectorX;
    /* 0x20 */ f32 rVectorY;
    /* 0x24 */ f32 rVectorZ;
    /* 0x28 */ f32 kc;
    /* 0x2C */ f32 kl;
    /* 0x30 */ f32 kq;
    /* 0x34 */ s16 coordX;
    /* 0x36 */ s16 coordY;
    /* 0x38 */ s16 coordZ;
} Light; // size = 0x3C

typedef struct LookAt {
    /* 0x00 */ bool bTransformed;
    /* 0x04 */ Vec3f rS;
    /* 0x10 */ Vec3f rT;
    /* 0x1C */ Vec3f rSRaw;
    /* 0x28 */ Vec3f rTRaw;
} LookAt; // size = 0x34

typedef struct Vertex {
    /* 0x00 */ f32 rSum;
    /* 0x04 */ f32 rS;
    /* 0x08 */ f32 rT;
    /* 0x0C */ Vec3f vec;
    /* 0x18 */ u8 anColor[4];
} Vertex; // size = 0x1C

typedef union TMEM_Block {
    /* 0x0 */ u8 u8[4096];
    /* 0x0 */ u16 u16[2048];
    /* 0x0 */ u32 u32[1024];
    /* 0x0 */ u64 u64[512];
} TMEM_Block;

typedef struct TextureMemory {
    /* 0x0 */ TMEM_Block data;
} TextureMemory; // size = 0x1000

typedef struct TextureInfo {
    /* 0x0 */ s32 nSizeTextures;
    /* 0x4 */ s32 nCountTextures;
} TextureInfo; // size = 0x8

// _FRAME_TEXTURE
typedef struct FrameTexture FrameTexture;

struct FrameTexture {
    /* 0x00 */ s32 nMode;
    /* 0x04 */ s32 iPackPixel;
    /* 0x08 */ s32 iPackColor;
    /* 0x0C */ s32 nFrameLast;
    /* 0x10 */ s16 nSizeX;
    /* 0x12 */ s16 nSizeY;
    /* 0x14 */ u32 nAddress;
    /* 0x18 */ FrameTexture* pTextureNext;
    /* 0x1C */ u32 nCodePixel;
    /* 0x20 */ u32 nCodeColor;
    /* 0x24 */ u32 nData0;
    /* 0x28 */ u32 nData1;
    /* 0x2C */ u32 nData2;
    /* 0x30 */ u32 nData3;
    /* 0x34 */ u32 unk_34;
    /* 0x38 */ GXTexFmt eFormat;
    /* 0x3C */ GXTlutObj objectTLUT;
    /* 0x48 */ GXTexObj objectTexture;
    /* 0x68 */ GXTexWrapMode eWrapS;
    /* 0x6C */ GXTexWrapMode eWrapT;
}; // size = 0x70

typedef struct Tile {
    /* 0x00 */ s32 nSize;
    /* 0x04 */ s32 nTMEM;
    /* 0x08 */ s32 iTLUT;
    /* 0x0C */ s32 nSizeX;
    /* 0x10 */ s32 nFormat;
    /* 0x14 */ s16 nMaskS;
    /* 0x16 */ s16 nMaskT;
    /* 0x18 */ s16 nModeS;
    /* 0x1A */ s16 nModeT;
    /* 0x1C */ s16 nShiftS;
    /* 0x1E */ s16 nShiftT;
    /* 0x20 */ s16 nX0;
    /* 0x22 */ s16 nY0;
    /* 0x24 */ s16 nX1;
    /* 0x26 */ s16 nY1;
    /* 0x28 */ u32 nCodePixel;
} Tile; // size = 0x2C

typedef struct MatrixHint {
    /* 0x00 */ s32 nCount;
    /* 0x04 */ f32 rScale;
    /* 0x08 */ f32 rAspect;
    /* 0x0C */ f32 rFieldOfViewY;
    /* 0x10 */ f32 rClipNear;
    /* 0x14 */ f32 rClipFar;
    /* 0x18 */ u32 nAddressFloat;
    /* 0x1C */ u32 nAddressFixed;
    /* 0x20 */ FrameMatrixProjection eProjection;
} MatrixHint; // size = 0x24

typedef struct Rectangle {
    /* 0x00 */ bool bFlip;
    /* 0x04 */ s32 iTile;
    /* 0x08 */ s32 nX0;
    /* 0x0C */ s32 nY0;
    /* 0x10 */ s32 nX1;
    /* 0x14 */ s32 nY1;
    /* 0x18 */ f32 rS;
    /* 0x1C */ f32 rT;
    /* 0x20 */ f32 rDeltaS;
    /* 0x24 */ f32 rDeltaT;
} Rectangle; // size = 0x28

//! TODO: fix the offsets
typedef struct Frame {
    /* 0x00000 */ u32 anCIMGAddresses[8];
    /* 0x00020 */ u16 nNumCIMGAddresses;
    /* 0x00024 */ bool bBlurOn;
    /* 0x00028 */ bool bHackPause;
    /* 0x0002C */ s32 nHackCount;
    /* 0x00030 */ s32 nFrameCounter;
    /* 0x00034 */ bool bPauseThisFrame;
    /* 0x00038 */ bool bCameFromBomberNotes;
    /* 0x0003C */ bool bInBomberNotes;
    /* 0x00040 */ s32 bShrinking; // bitfield (not a bool)
    /* 0x00044 */ bool bSnapShot;
    /* 0x00048 */ bool bUsingLens;
    /* 0x0004C */ u32 cBlurAlpha;
    /* 0x00050 */ bool bBlurredThisFrame;
    /* 0x00054 */ s32 nFrameCIMGCalls;
    /* 0x00058 */ bool bModifyZBuffer;
    /* 0x0005C */ bool bOverrideDepth;
    /* 0x00060 */ s32 nZBufferSets;
    /* 0x00064 */ s32 nLastFrameZSets;
    /* 0x00068 */ bool bPauseBGDrawn;
    /* 0x0006C */ bool bFrameOn;
    /* 0x00070 */ bool bBackBufferDrawn;
    /* 0x00074 */ bool bGrabbedFrame;
    /* 0x00078 */ s32 pad1[7];
    /* 0x00094 */ u64* pnGBI;
    /* 0x00098 */ u32 nFlag;
    /* 0x0009C */ f32 rScaleX;
    /* 0x000A0 */ f32 rScaleY;
    /* 0x000A4 */ f32 unk_A4;
    /* 0x000A8 */ f32 unk_A8;
    /* 0x000AC */ u32 nCountFrames;
    /* 0x000B0 */ u32 nMode;
    /* 0x000B4 */ u32 aMode[FMT_COUNT];
    /* 0x000DC */ Viewport viewport;
    /* 0x000EC */ FrameBuffer aBuffer[FBT_COUNT];
    /* 0x0013C */ u32 nOffsetDepth0;
    /* 0x00140 */ u32 nOffsetDepth1;
    /* 0x00144 */ s32 nWidthLine;
    /* 0x00148 */ f32 rDepth;
    /* 0x0014C */ f32 rDelta;
    /* 0x00150 */ FrameDrawFunc aDraw[4];
    /* 0x00160 */ s32 nCountLight;
    /* 0x00164 */ Light aLight[8];
    /* 0x00344 */ LookAt lookAt;
    /* 0x00378 */ s32 nCountVertex;
    /* 0x0037C */ Vertex aVertex[80];
    /* 0x00C40 */ TextureMemory TMEM;
    /* 0x01C40 */ void* aPixelData;
    /* 0x01C44 */ void* aColorData;
    /* 0x01C48 */ s32 nBlocksPixel;
    /* 0x01C4C */ s32 nBlocksMaxPixel;
    /* 0x01C50 */ s32 nBlocksColor;
    /* 0x01C54 */ s32 nBlocksMaxColor;
    /* 0x01C58 */ s32 nBlocksTexture;
    /* 0x01C5C */ s32 nBlocksMaxTexture;
    /* 0x01C60 */ u32 anPackPixel[48];
    /* 0x01D20 */ u32 anPackColor[320];
    /* 0x02220 */ u32 nAddressLoad;
    /* 0x02224 */ u32 nCodePixel;
    /* 0x02228 */ s32 unk_2228;
    /* 0x0222C */ u32 nTlutCode[16];
    /* 0x0226C */ FrameTexture aTexture[2048];
    /* 0x3A26C */ u32 anTextureUsed[64];
    /* 0x3A36C */ FrameTexture* apTextureCached[4096];
    /* 0x3E36C */ FrameTexture* unk_3E36C;
    /* 0x3E370 */ s32 iTileLoad;
    /* 0x3E374 */ u32 n2dLoadTexType;
    /* 0x3E378 */ s32 nLastX0;
    /* 0x3E37C */ s32 nLastY0;
    /* 0x3E380 */ s32 nLastX1;
    /* 0x3E384 */ s32 nLastY1;
    /* 0x3E388 */ Tile aTile[8];
    /* 0x3E4E8 */ s32 anSizeX[FS_COUNT];
    /* 0x3E4F0 */ s32 anSizeY[FS_COUNT];
    /* 0x3E4F8 */ s32 iHintMatrix;
    /* 0x3E4FC */ s32 iMatrixModel;
    /* 0x3E500 */ s32 iHintProjection;
    /* 0x3E504 */ Mtx44 matrixView;
    /* 0x3E544 */ s32 iHintLast;
    /* 0x3E548 */ s32 iHintHack;
    /* 0x3E54C */ FrameMatrixProjection eTypeProjection;
    /* 0x3E550 */ Mtx44 aMatrixModel[10];
    /* 0x3E7D0 */ Mtx44 matrixProjection;
    /* 0x3E810 */ Mtx44 matrixProjectionExtra;
    /* 0x3E850 */ MatrixHint aMatrixHint[64];
    /* 0x3F150 */ Mtx44 unknown;
    /* 0x3F190 */ Mtx44 unknown2;
    /* 0x3F1D0 */ Mtx44 unknown3;
    /* 0x3F210 */ f32 unk_3F210; // rNear
    /* 0x3F214 */ f32 unk_3F214; // rFar
    /* 0x3F218 */ u8 primLODmin;
    /* 0x3F219 */ u8 primLODfrac;
    /* 0x3F21A */ u8 lastTile;
    /* 0x3F21B */ u8 iTileDrawn;
    /* 0x3F21C */ GXColor aColor[FCT_COUNT];
    /* 0x3F230 */ u32 nModeVtx;
    /* 0x3F234 */ u16* nTempBuffer;
    /* 0x3F238 */ u16* nCopyBuffer;
    /* 0x3F23C */ u8* nCameraBuffer;
} Frame; // size = 0x3F240

extern _XL_OBJECTTYPE gClassFrame;
extern bool gNoSwapBuffer;
extern GXTexMapID ganNamePixel[];
extern GXTexCoordID ganNameTexCoord[];

bool frameDrawSetup2D(Frame* pFrame);
bool _frameDrawRectangle(Frame* pFrame, u32 nColor, s32 nX, s32 nY, s32 nSizeX, s32 nSizeY);
bool frameEvent(Frame* pFrame, s32 nEvent, void* pArgument);

void ZeldaDrawFrameNoBlend(Frame* pFrame, u16* pData);
void ZeldaDrawFrame(Frame* pFrame, u16* pData);
bool frameHackTIMG_Zelda(Frame* pFrame, u64** pnGBI, u32* pnCommandLo, u32* pnCommandHi);
bool frameHackCIMG_Zelda2(Frame* pFrame, FrameBuffer* pBuffer, u64* pnGBI, u32 nCommandLo, u32 nCommandHi);
bool frameHackCIMG_Zelda(Frame* pFrame, FrameBuffer* pBuffer, u64* pnGBI, u32 nCommandLo, u32 nCommandHi);
bool frameHackCIMG_Zelda2_Shrink(Rdp* pRDP, Frame* pFrame, u64** ppnGBI);
bool frameHackCIMG_Zelda2_Camera(Frame* pFrame, FrameBuffer* pBuffer, u32 nCommandHi, u32 nCommandLo);
bool frameHackTIMG_Panel(Frame* pFrame, FrameBuffer* pBuffer);
bool frameHackCIMG_Panel(Rdp* pRDP, Frame* pFrame, FrameBuffer* pBuffer, u64** ppnGBI);

bool frameGetDepth(Frame* pFrame, u16* pnData, s32 nAddress);
bool frameShow(Frame* pFrame);
bool frameSetScissor(Frame* pFrame, Rectangle* pScissor);
bool frameSetDepth(Frame* pFrame, f32 rDepth, f32 rDelta);
bool frameSetColor(Frame* pFrame, FrameColorType eType, u32 nRGBA);

bool frameBeginOK(Frame* pFrame);
bool frameBegin(Frame* pFrame, s32 nCountVertex);
bool frameEnd(Frame* pFrame);
bool frameDrawReset(Frame* pFrame, s32 nFlag);

bool frameSetFill(Frame* pFrame, bool bFill);
bool frameSetSize(Frame* pFrame, FrameSize eSize, s32 nSizeX, s32 nSizeY);
bool frameSetMode(Frame* pFrame, FrameModeType eType, u32 nMode);
bool frameGetMode(Frame* pFrame, FrameModeType eType, u32* pnMode);
bool frameSetMatrix(Frame* pFrame, Mtx44 matrix, FrameMatrixType eType, bool bLoad, bool bPush, s32 nAddressN64);
bool frameGetMatrix(Frame* pFrame, Mtx44 matrix, FrameMatrixType eType, bool bPull);
bool frameLoadVertex(Frame* pFrame, void* pBuffer, s32 iVertex0, s32 nCount);
bool frameCullDL(Frame* pFrame, s32 nVertexStart, s32 nVertexEnd);
bool frameLoadTLUT(Frame* pFrame, s32 nCount, s32 iTile);
bool frameLoadTMEM(Frame* pFrame, FrameLoadType eType, s32 iTile);
bool frameSetLightCount(Frame* pFrame, s32 nCount);
bool frameSetLight(Frame* pFrame, s32 iLight, s8* pData);
bool frameSetLookAt(Frame* pFrame, s32 iLookAt, s8* pData);
bool frameSetViewport(Frame* pFrame, s16* pData);
bool frameResetUCode(Frame* pFrame, FrameResetType eType);
bool frameSetBuffer(Frame* pFrame, FrameBufferType eType);
bool frameFixMatrixHint(Frame* pFrame, s32 nAddressFloat, s32 nAddressFixed);
bool frameSetMatrixHint(Frame* pFrame, FrameMatrixProjection eProjection, s32 nAddressFloat, s32 nAddressFixed,
                        f32 rNear, f32 rFar, f32 rFOVY, f32 rAspect, f32 rScale, void* mf);
bool frameInvalidateCache(Frame* pFrame, s32 nOffset0, s32 nOffset1);

//! TODO: document this
int fn_8005329C(Frame*, s32, s32, s32);

// _frameGCNcc.c
void SetNumTexGensChans(Frame* pFrame, s32 numCycles);
void SetTevStages(Frame* pFrame, s32 cycle, s32 numCycles);
bool SetTevStageTable(Frame* pFrame, s32 numCycles);

#ifdef __cplusplus
}
#endif

#endif
