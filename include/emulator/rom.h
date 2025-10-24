#ifndef _ROM_H
#define _ROM_H

#include "emulator/errordisplay.h"
#include "emulator/xlFileRVL.h"
#include "emulator/xlObject.h"
#include "revolution/types.h"
#include "versions.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ROM_THREAD_SIZE 0x2000

typedef bool UnknownCallbackFunc(void);
typedef bool ProgressCallbackFunc(f32 progressPercent);

typedef enum RomModeLoad {
    RLM_NONE = -1,
    RLM_PART = 0,
    RLM_FULL = 1,
    RLM_COUNT = 2,
} RomModeLoad;

typedef enum RomCacheType {
    RCT_NONE = -1,
    RCT_RAM = 0,
    RCT_ARAM = 1,
} RomCacheType;

typedef struct RomBlock {
    /* 0x00 */ s32 iCache; // Stores cache index `i` if the block is in RAM, or `-(i + 1)` if the block is in ARAM
    /* 0x04 */ u32 nSize;
    /* 0x08 */ u32 nTickUsed;
    /* 0x0C */ s8 keep;
} RomBlock; // size = 0x10

typedef struct RomCopyState {
    /* 0x00 */ bool bWait;
    /* 0x04 */ UnknownCallbackFunc* pCallback;
    /* 0x08 */ u8* pTarget;
    /* 0x0C */ u32 nSize;
    /* 0x10 */ u32 nOffset;
} RomCopyState; // size = 0x14

typedef struct RomLoadState {
    /* 0x00 */ bool bWait;
    /* 0x04 */ bool bDone;
    /* 0x08 */ s32 nResult;
    /* 0x0C */ u8* anData;
    /* 0x10 */ UnknownCallbackFunc* pCallback;
    /* 0x14 */ s32 iCache;
    /* 0x18 */ s32 iBlock;
    /* 0x1C */ s32 nOffset;
    /* 0x20 */ u32 nOffset0;
    /* 0x24 */ u32 nOffset1;
    /* 0x28 */ u32 nSize;
    /* 0x2C */ u32 nSizeRead;
} RomLoadState; // size = 0x30

typedef struct Rom {
    /*   OoT   -   SM64  -   MK64  */
    /* 0x00000 - 0x00000 - 0x00000 */ void* pBuffer;
    /* 0x00004 - 0x00004 - 0x00004 */ bool bFlip;
    /* 0x00008 - 0x00008 - 0x00008 */ bool bLoad;
#if VERSION >= MK64_J
    /*   N/A   - 0x0000C - 0x0000C */ s32 unk_C;
#endif
    /* 0x00010 - 0x0000C - 0x00010 */ char acNameFile[513];
    /* 0x00214 - 0x00210 - 0x00214 */ u32 nSize;
#if VERSION >= OOT_J
    /* 0x00218 */ s32 unk_218;
#endif
    /* 0x0021C - 0x00214 - 0x00218 */ RomModeLoad eModeLoad;
    /* 0x00220 - 0x00218 - 0x0021C */ RomBlock aBlock[VERSION < MK64_J ? 4096 : 6144];
    /* 0x18220 - 0x10218 - 0x1821C */ u32 nTick;
    /* 0x18224 - 0x1021C - 0x18220 */ u8* pCacheRAM;
    /* 0x18228 - 0x10220 - 0x18224 */ u8 anBlockCachedRAM[4096]; // Bitfield, one bit per block
    /* 0x19228 - 0x11220 - 0x19224 */ u8 anBlockCachedARAM[2046]; // Bitfield, one bit per block
    /* 0x19A28 - 0x11A20 - 0x19A24 */ RomCopyState copy;
    /* 0x19A3C - 0x11A34 - 0x19A38 */ RomLoadState load;
    /* 0x19A6C - 0x11A64 - 0x19A68 */ s32 nCountBlockRAM;
    /* 0x19A70 - 0x11A68 - 0x19A6C */ s32 nSizeCacheRAM;
    /* 0x19A74 - 0x11A6C - 0x19A70 */ u8 acHeader[64];
    /* 0x19AB4 - 0x11AAC - 0x19AB0 */ u32* anOffsetBlock;
    /* 0x19AB8 - 0x11AB0 - 0x19AB4 */ s32 nCountOffsetBlocks;
    /* 0x19ABC - 0x11AB4 - 0x19AB8 */ s32 nChecksum;
    /* 0x19AC0 - 0x11AB8 - 0x19ABC */ DVDFileInfo fileInfo;
    /* 0x19AFC - 0x11AF4 - 0x19AF8 */ s32 offsetToRom;
} Rom; // size = 0x19B00 - 0x11AF8 - 0x19AFC

s32 fn_80042E30(EDString* pSTString);
bool romGetPC(Rom* pROM, u64* pnPC);
bool romGetCode(Rom* pROM, s32* acCode);

//! NOTE: The debug informations indicates that `nSize` is unsigned, but the
//! generated code seems to treat it as signed.
bool romCopy(Rom* pROM, void* pTarget, s32 nOffset, s32 nSize, UnknownCallbackFunc* pCallback);

bool romUpdate(Rom* pROM);
bool romSetImage(Rom* pROM, char* szNameFile);
bool romGetImage(Rom* pROM, char* acNameFile);
bool romGetBuffer(Rom* pROM, void** pBuffer, u32 nAddress, s32* pData);
bool romEvent(Rom* pROM, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassROM;

#ifdef __cplusplus
}
#endif

#endif
