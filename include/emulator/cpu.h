#ifndef _CPU_H
#define _CPU_H

#include "emulator/xlObject.h"
#include "revolution/os.h"
#include "revolution/types.h"
#include "macros.h"

#ifdef __cplusplus
extern "C" {
#endif

// MIPS instruction encoding:
// R-type: opcode (6 bits) | rs (5 bits) | rt (5 bits) | rd (5 bits) | sa (5 bits) | funct (6 bits)
// I-type: opcode (6 bits) | rs (5 bits) | rt (5 bits) | imm (16 bits)
// J-type: opcode (6 bits) | target (26 bits)
//  float: opcode (6 bits) | fmt (5 bits) | ft (5 bits) | fs (5 bits) | fd (5 bits) | funct (6 bits)
#define MIPS_OP(inst) ((inst) >> 26)
#define MIPS_RS(inst) (((inst) >> 21) & 0x1F)
#define MIPS_RT(inst) (((inst) >> 16) & 0x1F)
#define MIPS_RD(inst) (((inst) >> 11) & 0x1F)
#define MIPS_SA(inst) (((inst) >> 6) & 0x1F)
#define MIPS_FUNCT(inst) ((inst) & 0x3F)
#define MIPS_IMM_S16(inst) ((s16)((inst) & 0xFFFF))
#define MIPS_IMM_U16(inst) ((u16)((inst) & 0xFFFF))
#define MIPS_TARGET(inst) ((inst) & 0x3FFFFFF)

#define MIPS_FMT(inst) (((inst) >> 21) & 0x1F)
#define MIPS_FT(inst) (((inst) >> 16) & 0x1F)
#define MIPS_FS(inst) (((inst) >> 11) & 0x1F)
#define MIPS_FD(inst) (((inst) >> 6) & 0x1F)

#define TLB_PGSZ_MASK 0x01FFE000
#define TLB_PGSZ_4K 0
#define TLB_PGSZ_16K 0x6000
#define TLB_PGSZ_64K 0x1E000
#define TLB_PGSZ_256K 0x7E000
#define TLB_PGSZ_1M 0x1FE000
#define TLB_PGSZ_4M 0x7FE000
#define TLB_PGSZ_16M 0x1FFE000

typedef struct CpuBlock CpuBlock;
typedef bool (*UnknownBlockCallback)(CpuBlock* pBlock, bool bUnknown);

struct CpuBlock {
    /* 0x00 */ struct CpuBlock* pNext;
    /* 0x04 */ u32 nSize;
    /* 0x08 */ UnknownBlockCallback pfUnknown;
    /* 0x0C */ u32 nAddress0;
    /* 0x10 */ u32 nAddress1;
}; // size = 0x14

typedef bool (*GetBlockFunc)(void* pObject, CpuBlock* pBlock);

typedef bool (*Put8Func)(void* pObject, u32 nAddress, s8* pData);
typedef bool (*Put16Func)(void* pObject, u32 nAddress, s16* pData);
typedef bool (*Put32Func)(void* pObject, u32 nAddress, s32* pData);
typedef bool (*Put64Func)(void* pObject, u32 nAddress, s64* pData);

typedef bool (*Get8Func)(void* pObject, u32 nAddress, s8* pData);
typedef bool (*Get16Func)(void* pObject, u32 nAddress, s16* pData);
typedef bool (*Get32Func)(void* pObject, u32 nAddress, s32* pData);
typedef bool (*Get64Func)(void* pObject, u32 nAddress, s64* pData);

typedef enum CpuExceptionCode {
    CEC_NONE = -1,
    CEC_INTERRUPT = 0,
    CEC_TLB_MODIFICATION = 1,
    CEC_TLB_LOAD = 2,
    CEC_TLB_STORE = 3,
    CEC_ADDRESS_LOAD = 4,
    CEC_ADDRESS_STORE = 5,
    CEC_BUS_INSTRUCTION = 6,
    CEC_BUS_DATA = 7,
    CEC_SYSCALL = 8,
    CEC_BREAK = 9,
    CEC_RESERVED = 10,
    CEC_COPROCESSOR = 11,
    CEC_OVERFLOW = 12,
    CEC_TRAP = 13,
    CEC_VCE_INSTRUCTION = 14,
    CEC_FLOAT = 15,
    CEC_RESERVED_16 = 16,
    CEC_RESERVED_17 = 17,
    CEC_RESERVED_18 = 18,
    CEC_RESERVED_19 = 19,
    CEC_RESERVED_20 = 20,
    CEC_RESERVED_21 = 21,
    CEC_RESERVED_22 = 22,
    CEC_WATCH = 23,
    CEC_RESERVED_24 = 24,
    CEC_RESERVED_25 = 25,
    CEC_RESERVED_26 = 26,
    CEC_RESERVED_27 = 27,
    CEC_RESERVED_28 = 28,
    CEC_RESERVED_29 = 29,
    CEC_RESERVED_30 = 30,
    CEC_VCE_DATA = 31,
    CEC_COUNT = 32,
} CpuExceptionCode;

typedef enum CpuMode {
    CM_NONE = -1,
    CM_USER = 0,
    CM_SUPER = 1,
    CM_KERNEL = 2,
} CpuMode;

typedef enum CpuSize {
    CS_NONE = -1,
    CS_32BIT = 0,
    CS_64BIT = 1,
} CpuSize;

typedef union CpuGpr {
    struct {
        /* 0x0 */ s8 _0s8;
        /* 0x1 */ s8 _1s8;
        /* 0x2 */ s8 _2s8;
        /* 0x3 */ s8 _3s8;
        /* 0x4 */ s8 _4s8;
        /* 0x5 */ s8 _5s8;
        /* 0x6 */ s8 _6s8;
        /* 0x7 */ s8 s8;
    };
    struct {
        /* 0x0 */ s16 _0s16;
        /* 0x2 */ s16 _1s16;
        /* 0x4 */ s16 _2s16;
        /* 0x6 */ s16 s16;
    };
    struct {
        /* 0x0 */ s32 _0s32;
        /* 0x4 */ s32 s32;
    };
    struct {
        /* 0x0 */ s64 s64;
    };
    struct {
        /* 0x0 */ u8 _0u8;
        /* 0x1 */ u8 _1u8;
        /* 0x2 */ u8 _2u8;
        /* 0x3 */ u8 _3u8;
        /* 0x4 */ u8 _4u8;
        /* 0x5 */ u8 _5u8;
        /* 0x6 */ u8 _6u8;
        /* 0x7 */ u8 u8;
    };
    struct {
        /* 0x0 */ u16 _0u16;
        /* 0x2 */ u16 _1u16;
        /* 0x4 */ u16 _2u16;
        /* 0x6 */ u16 u16;
    };
    struct {
        /* 0x0 */ u32 _0u32;
        /* 0x4 */ u32 u32;
    };
    struct {
        /* 0x0 */ u64 u64;
    };
} CpuGpr;

typedef union CpuFpr {
    struct {
        /* 0x0 */ f32 _0f32;
        /* 0x4 */ f32 f32;
    };
    struct {
        /* 0x0 */ f64 f64;
    };
    struct {
        /* 0x0 */ s32 _0s32;
        /* 0x4 */ s32 s32;
    };
    struct {
        /* 0x0 */ s64 s64;
    };
    struct {
        /* 0x0 */ u32 _0u32;
        /* 0x4 */ u32 u32;
    };
    struct {
        /* 0x0 */ u64 u64;
    };
} CpuFpr;

typedef struct CpuDevice {
    /* 0x00 */ s32 nType;
    /* 0x04 */ void* pObject;
    /* 0x08 */ s32 nOffsetAddress;
    /* 0x0C */ Get8Func pfGet8;
    /* 0x10 */ Get16Func pfGet16;
    /* 0x14 */ Get32Func pfGet32;
    /* 0x18 */ Get64Func pfGet64;
    /* 0x1C */ Put8Func pfPut8;
    /* 0x20 */ Put16Func pfPut16;
    /* 0x24 */ Put32Func pfPut32;
    /* 0x28 */ Put64Func pfPut64;
    /* 0x2C */ GetBlockFunc pfGetBlock;
    /* 0x30 */ u32 nAddressVirtual0;
    /* 0x34 */ u32 nAddressVirtual1;
    /* 0x38 */ u32 nAddressPhysical0;
    /* 0x3C */ u32 nAddressPhysical1;
} CpuDevice; // size = 0x40

typedef struct CpuJump {
    /* 0x0 */ s32 nOffsetHost;
    /* 0x4 */ s32 nAddressN64;
} CpuJump; // size = 0x8

// cpu_callerID
typedef struct CpuCallerID {
    /* 0x0 */ s32 N64address;
    /* 0x4 */ s32 GCNaddress;
} CpuCallerID; // size = 0x8

typedef struct CpuFunction CpuFunction;

// cpu_function
struct CpuFunction {
    /* 0x00 */ void* pnBase;
    /* 0x04 */ void* pfCode;
    /* 0x08 */ s32 nCountJump;
    /* 0x0C */ CpuJump* aJump;
    /* 0x10 */ s32 nAddress0;
    /* 0x14 */ s32 nAddress1;
    /* 0x18 */ CpuCallerID* block;
    /* 0x1C */ s32 callerID_total;
    /* 0x20 */ s32 callerID_flag;
    /* 0x24 */ u32 nChecksum;
    /* 0x28 */ s32 timeToLive;
    /* 0x2C */ s32 memory_size;
    /* 0x30 */ s32 heapID;
    /* 0x34 */ s32 heapWhere;
    /* 0x38 */ s32 treeheapWhere;
    /* 0x3C */ CpuFunction* prev;
    /* 0x40 */ CpuFunction* left;
    /* 0x44 */ CpuFunction* right;
}; // size = 0x48

// cpu_treeRoot
typedef struct CpuTreeRoot {
    /* 0x00 */ u16 total;
    /* 0x04 */ s32 total_memory;
    /* 0x08 */ s32 root_address;
    /* 0x0C */ s32 start_range;
    /* 0x10 */ s32 end_range;
    /* 0x14 */ s32 cache_miss;
    /* 0x18 */ s32 cache[20];
    /* 0x68 */ CpuFunction* left;
    /* 0x6C */ CpuFunction* right;
    /* 0x70 */ s32 kill_limit;
    /* 0x74 */ s32 kill_number;
    /* 0x78 */ s32 side;
    /* 0x7C */ CpuFunction* restore;
    /* 0x80 */ s32 restore_side;
} CpuTreeRoot; // size = 0x84

// _CPU_ADDRESS
typedef struct CpuAddress {
    /* 0x0 */ s32 nN64;
    /* 0x4 */ s32 nHost;
    /* 0x8 */ CpuFunction* pFunction;
} CpuAddress; // size = 0xC

typedef struct CpuCodeHack {
    /* 0x0 */ u32 nAddress;
    /* 0x4 */ u32 nOpcodeOld;
    /* 0x8 */ u32 nOpcodeNew;
} CpuCodeHack; // size = 0xC

// cpu_optimize
typedef struct CpuOptimize {
    /* 0x00 */ u32 validCheck;
    /* 0x04 */ u32 destGPR_check;
    /* 0x08 */ s32 destGPR;
    /* 0x0C */ s32 destGPR_mapping;
    /* 0x10 */ u32 destFPR_check;
    /* 0x14 */ s32 destFPR;
    /* 0x18 */ u32 addr_check;
    /* 0x1C */ s32 addr_last;
    /* 0x20 */ u32 checkType;
    /* 0x24 */ u32 checkNext;
} CpuOptimize; // size = 0x28

typedef struct Cpu Cpu;
typedef bool (*CpuExecuteFunc)(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN);

// _CPU
struct Cpu {
    /* 0x00000 */ s32 nMode;
    /* 0x00004 */ s32 nTick;
    /* 0x00008 */ s64 nLo;
    /* 0x00010 */ s64 nHi;
    /* 0x00018 */ s32 nCountAddress;
    /* 0x0001C */ s32 iDeviceDefault;
    /* 0x00020 */ u32 nPC;
    /* 0x00024 */ u32 nWaitPC;
    /* 0x00028 */ u32 nCallLast;
    /* 0x0002C */ CpuFunction* pFunctionLast;
    /* 0x00030 */ s32 nReturnAddrLast;
    /* 0x00034 */ s32 survivalTimer;
    /* 0x00038 */ u32 nTickLast;
    /* 0x0003C */ u32 nRetrace;
    /* 0x00040 */ u32 nRetraceUsed;
    /* 0x00044 */ CpuGpr aGPR[32];
    /* 0x00144 */ CpuFpr aFPR[32];
    /* 0x00244 */ u64 aTLB[48][5];
    /* 0x009C4 */ s32 anFCR[32];
    /* 0x00A44 */ s64 anCP0[32];
    /* 0x00B44 */ CpuExecuteFunc pfStep;
    /* 0x00B48 */ CpuExecuteFunc pfJump;
    /* 0x00B4C */ CpuExecuteFunc pfCall;
    /* 0x00B50 */ CpuExecuteFunc pfIdle;
    /* 0x00B54 */ CpuExecuteFunc pfRam;
    /* 0x00B58 */ CpuExecuteFunc pfRamF;
    /* 0x00B64 */ CpuDevice* apDevice[256];
    /* 0x00F64 */ u8 aiDevice[65536];
    /* 0x10F64 */ void* gHeap1;
    /* 0x10F68 */ void* gHeap2;
    /* 0x10F6C */ u32 aHeap1Flag[192];
    /* 0x1126C */ u32 aHeap2Flag[13];
    /* 0x1129C */ void* gHeapTree;
    /* 0x112A0 */ u32 aHeapTreeFlag[125];
    /* 0x11494 */ CpuTreeRoot* gTree;
    /* 0x11498 */ CpuAddress aAddressCache[256];
    /* 0x12098 */ s32 nCountCodeHack;
    /* 0x1209C */ CpuCodeHack aCodeHack[32];
    /* 0x1221C */ u32 nFlagRAM;
    /* 0x12220 */ u32 nFlagCODE;
    /* 0x12224 */ u32 nCompileFlag;
    /* 0x12228 */ s32 unk_12228;

    //! TODO: fix match issue with OSAlarm
    // /* 0x12230 */ OSAlarm alarmRetrace;
    /* 0x12230 */ s32 alarmRetrace[12];

    /* 0x1225C */ s32 unk_1225C;
    /* 0x12260 */ s32 unk_12260;
    /* 0x12264 */ s32 unk_12264;
    /* 0x12268 */ s32 unk_12268;
    /* 0x1226C */ s32 unk_1226C;
    /* 0x12270 */ CpuOptimize nOptimize;
    /* 0x12298 */ s64 nTimeRetrace;
    u8 pad[0x30];
}; // size = 0x122D0

#define CPU_DEVICE(apDevice, aiDevice, nAddress) (apDevice[aiDevice[(u32)(nAddress) >> 16]])

#define CPU_DEVICE_GET8(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                        \
        ->pfGet8(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                 (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s8*)pValue)
#define CPU_DEVICE_GET16(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                         \
        ->pfGet16(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                  (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s16*)pValue)
#define CPU_DEVICE_GET32(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                         \
        ->pfGet32(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                  (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s32*)pValue)
#define CPU_DEVICE_GET64(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                         \
        ->pfGet64(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                  (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s64*)pValue)

#define CPU_DEVICE_PUT8(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                        \
        ->pfPut8(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                 (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s8*)pValue)
#define CPU_DEVICE_PUT16(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                         \
        ->pfPut16(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                  (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s16*)pValue)
#define CPU_DEVICE_PUT32(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                         \
        ->pfPut32(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                  (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s32*)pValue)
#define CPU_DEVICE_PUT64(apDevice, aiDevice, nAddress, pValue)       \
    CPU_DEVICE(apDevice, aiDevice, nAddress)                         \
        ->pfPut64(CPU_DEVICE(apDevice, aiDevice, nAddress)->pObject, \
                  (nAddress) + CPU_DEVICE(apDevice, aiDevice, nAddress)->nOffsetAddress, (s64*)pValue)

bool cpuFreeCachedAddress(Cpu* pCPU, s32 nAddress0, s32 nAddress1);
bool cpuTestInterrupt(Cpu* pCPU, s32 nMaskIP);
bool cpuException(Cpu* pCPU, CpuExceptionCode eCode, s32 nMaskIP);
bool cpuSetRegisterCP0(Cpu* pCPU, s32 iRegister, s64 nData);
bool cpuGetRegisterCP0(Cpu* pCPU, s32 iRegister, s64* pnData);
bool __cpuERET(Cpu* pCPU);
bool __cpuBreak(Cpu* pCPU);
bool cpuFindBranchOffset(Cpu* pCPU, CpuFunction* pFunction, s32* pnOffset, s32 nAddress, s32* anCode);
s32 fn_8000E81C(Cpu* pCPU, s32 arg1, s32 arg2, s32 arg3, s32 arg5, s32* arg6, s32* arg7);
bool cpuExecute(Cpu* pCPU, u64 nAddressBreak);
bool cpuMapObject(Cpu* pCPU, void* pObject, u32 nAddress0, u32 nAddress1, s32 nType);
bool cpuGetBlock(Cpu* pCPU, CpuBlock* pBlock);
bool cpuSetGetBlock(Cpu* pCPU, CpuDevice* pDevice, GetBlockFunc pfGetBlock);
bool cpuSetDeviceGet(Cpu* pCPU, CpuDevice* pDevice, Get8Func pfGet8, Get16Func pfGet16, Get32Func pfGet32,
                     Get64Func pfGet64);
bool cpuSetDevicePut(Cpu* pCPU, CpuDevice* pDevice, Put8Func pfPut8, Put16Func pfPut16, Put32Func pfPut32,
                     Put64Func pfPut64);
bool cpuSetCodeHack(Cpu* pCPU, s32 nAddress, s32 nOpcodeOld, s32 nOpcodeNew);
bool cpuReset(Cpu* pCPU);
bool cpuGetXPC(Cpu* pCPU, s64* pnPC, s64* pnLo, s64* pnHi);
bool cpuSetXPC(Cpu* pCPU, s64 nPC, s64 nLo, s64 nHi);
bool cpuEvent(Cpu* pCPU, s32 nEvent, void* pArgument);
bool cpuGetAddressOffset(Cpu* pCPU, s32* pnOffset, u32 nAddress);
bool cpuGetAddressBuffer(Cpu* pCPU, void** ppBuffer, u32 nAddress) NO_INLINE;
bool cpuGetOffsetAddress(Cpu* pCPU, u32* anAddress, s32* pnCount, u32 nOffset, u32 nSize);
bool cpuInvalidateCache(Cpu* pCPU, s32 nAddress0, s32 nAddress1);
bool cpuGetFunctionChecksum(Cpu* pCPU, u32* pnChecksum, CpuFunction* pFunction);
bool cpuHeapTake(void* heap, Cpu* pCPU, CpuFunction* pFunction, int memory_size);
bool cpuFindFunction(Cpu* pCPU, s32 theAddress, CpuFunction** tree_node);
bool fn_8003F330(Cpu* pCPU, CpuFunction* pFunction);
bool treeCleanUpCheck(Cpu* pCPU, CpuFunction* node);

extern _XL_OBJECTTYPE gClassCPU;

#ifdef __cplusplus
}
#endif

#endif
