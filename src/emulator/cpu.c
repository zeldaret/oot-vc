#include "emulator/cpu.h"
#include "emulator/cpu_jumptable.h"
#include "emulator/frame.h"
#include "emulator/library.h"
#include "emulator/ram.h"
#include "emulator/rom.h"
#include "emulator/rsp.h"
#include "emulator/system.h"
#include "emulator/vc64_RVL.h"
#include "emulator/vi.h"
#include "emulator/xlCoreRVL.h"
#include "emulator/xlHeap.h"
#include "emulator/xlObject.h"
#include "emulator/xlPostRVL.h"
#include "macros.h"
#include "math.h"
#include "revolution/vi.h"

static inline bool cpuMakeCachedAddress(Cpu* pCPU, s32 nAddressN64, s32 nAddressHost, CpuFunction* pFunction);
static bool cpuFindCachedAddress(Cpu* pCPU, s32 nAddressN64, s32* pnAddressHost);
static bool cpuSetTLB(Cpu* pCPU, s32 iEntry);
static bool cpuHeapReset(u32* array, s32 count);
static bool cpuDMAUpdateFunction(Cpu* pCPU, s32 start, s32 end);
static void treeCallerInit(CpuCallerID* block, s32 total);
static bool treeInit(Cpu* pCPU, s32 root_address);
static bool treeKill(Cpu* pCPU);
static bool treeKillNodes(Cpu* pCPU, CpuFunction* tree);
static bool treeAdjustRoot(Cpu* pCPU, s32 new_start, s32 new_end);
static inline bool treeSearch(Cpu* pCPU, s32 target, CpuFunction** node);
static bool treeSearchNode(CpuFunction* tree, s32 target, CpuFunction** node);
bool treeInsert(Cpu* pCPU, s32 start, s32 end);
static bool treeInsertNode(CpuFunction** tree, s32 start, s32 end, CpuFunction** ppFunction);
static bool treeBalance(CpuTreeRoot* root);
static bool treeKillReason(Cpu* pCPU, s32* value);
static bool treeKillRange(Cpu* pCPU, CpuFunction* tree, s32 start, s32 end);
static bool treeTimerCheck(Cpu* pCPU);
static bool treeCleanUp(Cpu* pCPU, CpuTreeRoot* root);
static bool treeCleanNodes(Cpu* pCPU, CpuFunction* top);
static inline bool treeForceCleanUp(Cpu* pCPU, CpuFunction* tree, s32 kill_limit);
static bool treeForceCleanNodes(Cpu* pCPU, CpuFunction* tree, s32 kill_limit);
static bool treePrintNode(Cpu* pCPU, CpuFunction* tree, s32 print_flag, s32* left, s32* right);
static inline s32 treeMemory(Cpu* pCPU);

s64 ganMaskGetCP0[] = {
    0x000000008000003F, 0x000000000000003F, 0x000000003FFFFFFF, 0x000000003FFFFFFF, 0xFFFFFFFFFFFFFFF0,
    0x0000000001FFE000, 0x000000000000001F, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,
    0x00000000FFFFE0FF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x00000000F000FF7C, 0xFFFFFFFFFFFFFFFF,
    0x000000000000FFFF, 0x00000000FFFFEFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFB, 0x000000000000000F,
    0x00000000FFFFFFF0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x00000000000000FF, 0x00000000FFBFFFFF, 0x00000000FFFFFFFF, 0x0000000000000000,
    0xFFFFFFFFFFFFFFFF, 0x0000000000000000,
};

s64 ganMaskSetCP0[] = {
    0x000000000000003F, 0x000000000000003F, 0x000000003FFFFFFF, 0x000000003FFFFFFF, 0xFFFFFFFFFFFFFFF0,
    0x0000000001FFE000, 0x000000000000001F, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x00000000FFFFFFFF,
    0x00000000FFFFE0FF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFF, 0x0000000000000300, 0xFFFFFFFFFFFFFFFF,
    0x000000000000FFFF, 0x00000000FFFFEFFF, 0x00000000FFFFFFFF, 0x00000000FFFFFFFB, 0x000000000000000F,
    0x00000000FFFFFFF0, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
    0x0000000000000000, 0x00000000000000FF, 0x00000000FFBFFFFF, 0x00000000FFFFFFFF, 0x0000000000000000,
    0xFFFFFFFFFFFFFFFF, 0x0000000000000000,
};

u8 Opcode[] = {
    true, true, true, true,  true, true, true, true, true, true, true, true,  true,  true,  true,  true,
    true, true, true, false, true, true, true, true, true, true, true, true,  false, false, false, true,
    true, true, true, true,  true, true, true, true, true, true, true, true,  true,  true,  true,  true,
    true, true, true, false, true, true, true, true, true, true, true, false, true,  true,  true,  true,
};

u8 SpecialOpcode[] = {
    true, false, true, true, true, false, true, true,  true,  true,  false, false, true, true,  false, true,
    true, true,  true, true, true, false, true, true,  true,  true,  true,  true,  true, true,  true,  true,
    true, true,  true, true, true, true,  true, true,  false, false, true,  true,  true, true,  true,  true,
    true, true,  true, true, true, false, true, false, true,  false, true,  true,  true, false, true,  true,
};

u8 RegimmOpcode[] = {
    true, true, true, true, false, false, false, false, true,  true,  true,  true,  true,  false, true,  false,
    true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false,
};

s32 ganOpcodeSaveFP1[] = {
    0x8F480018, 0x11000014, 0x00000000, 0x4448F800, 0x00000000,
};

s32 ganOpcodeSaveFP2_0[] = {
    0x8CBB0018, 0x1360000A, 0x00000000, 0x445BF800, 0x00000000,
};

s32 ganOpcodeSaveFP2_1[] = {
    0x13600009,
    0xACBF011C,
    0x445BF800,
};

s32 ganOpcodeLoadFP[] = {
    0x8F5B0018, 0x13600013, 0x00000000, 0x8F5B012C, 0x44DBF800,
};

#ifndef NON_MATCHING
void* jumptable_80170A68[] = {
    &lbl_8000E130, &lbl_8000E260, &lbl_8000E25C, &lbl_8000E25C, &lbl_8000E25C, &lbl_8000E25C, &lbl_8000E25C,
    &lbl_8000E260, &lbl_8000E260, &lbl_8000E164, &lbl_8000E25C, &lbl_8000E16C, &lbl_8000E1C8, &lbl_8000E1EC,
    &lbl_8000E234, &lbl_8000E260, &lbl_8000E23C, &lbl_8000E25C, &lbl_8000E25C, &lbl_8000E25C, &lbl_8000E25C,
    &lbl_8000E260, &lbl_8000E260, &lbl_8000E260, &lbl_8000E260, &lbl_8000E260, &lbl_8000E25C, &lbl_8000E260,
    &lbl_8000E25C, &lbl_8000E25C, &lbl_8000E25C, &lbl_8000E260,
};
#else
void* jumptable_80170A68[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80170AE8[] = {
    &lbl_8000E3F4, &lbl_8000E2D4, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4,
    &lbl_8000E37C, &lbl_8000E38C, &lbl_8000E364, &lbl_8000E3F4, &lbl_8000E36C, &lbl_8000E3F4, &lbl_8000E3F4,
    &lbl_8000E374, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4,
    &lbl_8000E394, &lbl_8000E3A4, &lbl_8000E3B4, &lbl_8000E3C4, &lbl_8000E3D4, &lbl_8000E3F4, &lbl_8000E3F4,
    &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3F4, &lbl_8000E3E4,
};
#else
void* jumptable_80170AE8[] = {0};
#endif

/**
 * @brief Mapping of VR4300 to PPC registers.
 *
 * If bit 0x100 is set the VR4300 register is not directly mapped to any PPC register,
 * Instead the register will use the emulated VR4300 object for saving/loading register values.
 */
s32 ganMapGPR[] = {
    0x0000000A, 0x0000000B, 0x0000000C, 0x0000000E, 0x0000000F, 0x00000010, 0x00000011, 0x00000012,
    0x00000013, 0x00000014, 0x00000015, 0x00000016, 0x00000017, 0x00000018, 0x00000019, 0x0000001A,
    0x00000110, 0x00000111, 0x00000112, 0x00000113, 0x00000114, 0x00000115, 0x00000116, 0x00000117,
    0x0000001B, 0x0000001C, 0x0000001D, 0x0000001E, 0x0000011C, 0x0000001F, 0x0000011E, 0x0000011F,
};

#ifndef NON_MATCHING
void* jumptable_80170BE8[] = {
    &lbl_8000E5DC, &lbl_8000E64C, &lbl_8000E64C, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E64C, &lbl_8000E5DC,
    &lbl_8000E5DC, &lbl_8000E64C, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC,
    &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC,
    &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E5DC, &lbl_8000E64C,
};
#else
void* jumptable_80170BE8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80170C4C[] = {
    &lbl_8000E55C, &lbl_8000E588, &lbl_8000E638, &lbl_8000E640, &lbl_8000E648, &lbl_8000E648,
    &lbl_8000E648, &lbl_8000E648, &lbl_8000E64C, &lbl_8000E64C, &lbl_8000E64C, &lbl_8000E64C,
    &lbl_8000E64C, &lbl_8000E64C, &lbl_8000E64C, &lbl_8000E64C, &lbl_8000E5B8, &lbl_8000E60C,
    &lbl_8000E64C, &lbl_8000E64C, &lbl_8000E648, &lbl_8000E648, &lbl_8000E648, &lbl_8000E648,
};
#else
void* jumptable_80170C4C[] = {0};
#endif

char lbl_80170CAC[] = "CALLED: ceil_w single (%p)\n";
char lbl_80170CC8[] = "CALLED: floor_w single (%p)\n";
char lbl_80170CE8[] = "CALLED: ceil_w double (%p)\n";
char lbl_80170D04[] = "CALLED: floor_w double (%p)\n";
char lbl_80170D28[] = "ERROR: CVT_S_D\n";
char lbl_80170D38[] = "ERROR: ADD_W\n";
char lbl_80170D48[] = "ERROR: SUB_W\n";
char lbl_80170D58[] = "ERROR: MUL_W\n";
char lbl_80170D68[] = "ERROR: DIV_W\n";
char lbl_80170D78[] = "ERROR: SQRT_W\n";
char lbl_80170D88[] = "ERROR: ABS_W\n";
char lbl_80170D98[] = "ERROR: MOVE_W\n";
char lbl_80170DA8[] = "ERROR: NEG_W\n";
char lbl_80170DB8[] = "ERROR: ROUND_W_W\n";
char lbl_80170DCC[] = "ERROR: TRUNC_W_W\n";
char lbl_80170DE0[] = "ERROR: CEIL_W_W\n";
char lbl_80170DF4[] = "ERROR: FLOOR_W_W\n";
char lbl_80170E08[] = "ERROR: CVT_W_W\n";
char lbl_80170E18[] = "ERROR: C.F_W\n";
char lbl_80170E28[] = "ERROR: C.UN_W\n";
char lbl_80170E38[] = "ERROR: C.EQ_W\n";
char lbl_80170E48[] = "ERROR: C.UEQ_W\n";
char lbl_80170E58[] = "ERROR: C.OLT_W\n";
char lbl_80170E68[] = "ERROR: C.ULT_W\n";
char lbl_80170E78[] = "ERROR: C.OLE_W\n";
char lbl_80170E88[] = "ERROR: C.ULE_W\n";
char lbl_80170E98[] = "ERROR: C.SF_W\n";
char lbl_80170EA8[] = "ERROR: C.NGLE_W\n";
char lbl_80170EC0[] = "ERROR: C.SEQ_W\n";
char lbl_80170ED0[] = "ERROR: C.NGL_W\n";
char lbl_80170EE0[] = "ERROR: C.LT_W\n";
char lbl_80170EF0[] = "ERROR: C.NGE_W\n";
char lbl_80170F00[] = "ERROR: C.LE_W\n";
char lbl_80170F10[] = "ERROR: C.NGT_W\n";
char lbl_80170F20[] = "ERROR: ADD_L\n";
char lbl_80170F30[] = "ERROR: SUB_L\n";
char lbl_80170F40[] = "ERROR: MUL_L\n";
char lbl_80170F50[] = "ERROR: DIV_L\n";
char lbl_80170F60[] = "ERROR: SQRT_L\n";
char lbl_80170F70[] = "ERROR: ABS_L\n";
char lbl_80170F80[] = "ERROR: MOVE_L\n";
char lbl_80170F90[] = "ERROR: NEG_L\n";
char lbl_80170FA0[] = "ERROR: ROUND_W_L\n";
char lbl_80170FB4[] = "ERROR: TRUNC_W_L\n";
char lbl_80170FC8[] = "ERROR: CEIL_W_L\n";
char lbl_80170FDC[] = "ERROR: FLOOR_W_L\n";
char lbl_80170FF0[] = "ERROR: CVT_W_L\n";
char lbl_80171000[] = "ERROR: C.F_L\n";
char lbl_80171010[] = "ERROR: C.UN_L\n";
char lbl_80171020[] = "ERROR: C.EQ_L\n";
char lbl_80171030[] = "ERROR: C.UEQ_L\n";
char lbl_80171040[] = "ERROR: C.OLT_L\n";
char lbl_80171050[] = "ERROR: C.ULT_L\n";
char lbl_80171060[] = "ERROR: C.OLE_L\n";
char lbl_80171070[] = "ERROR: C.ULE_L\n";
char lbl_80171080[] = "ERROR: C.SF_L\n";
char lbl_80171090[] = "ERROR: C.NGLE_L\n";
char lbl_801710A8[] = "ERROR: C.SEQ_L\n";
char lbl_801710B8[] = "ERROR: C.NGL_L\n";
char lbl_801710C8[] = "ERROR: C.LT_L\n";
char lbl_801710D8[] = "ERROR: C.NGE_L\n";
char lbl_801710E8[] = "ERROR: C.LE_L\n";
char lbl_801710F8[] = "ERROR: C.NGT_L\n";

#ifndef NON_MATCHING
void* jumptable_80171108[] = {
    &lbl_80025B34, &lbl_80025B64, &lbl_80025B94, &lbl_80025BC4, &lbl_80025BF4, &lbl_80025C0C, &lbl_80025C3C,
    &lbl_80025C54, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80025C84, &lbl_80025C9C,
    &lbl_80025CB4, &lbl_80025CCC, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88,
    &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88,
    &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80031A30, &lbl_80025CE4, &lbl_80025F88,
    &lbl_80025F88, &lbl_80025E08, &lbl_80031A30, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88,
    &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025F88, &lbl_80025E20,
    &lbl_80025E30, &lbl_80025E40, &lbl_80025E58, &lbl_80025E70, &lbl_80025E88, &lbl_80025EA0, &lbl_80025EB8,
    &lbl_80025ED0, &lbl_80025EE0, &lbl_80025EF8, &lbl_80025F10, &lbl_80025F28, &lbl_80025F40, &lbl_80025F58,
    &lbl_80025F70,
};
#else
void* jumptable_80171108[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171208[] = {
    &lbl_800256F4, &lbl_80025724, &lbl_80025754, &lbl_80025784, &lbl_800257B4, &lbl_800257CC, &lbl_800257FC,
    &lbl_80025814, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80025844, &lbl_8002585C,
    &lbl_80025874, &lbl_8002588C, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08,
    &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08,
    &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80031A30, &lbl_800258A4, &lbl_80025B08,
    &lbl_80025B08, &lbl_80025988, &lbl_80031A30, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08,
    &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_80025B08, &lbl_800259A0,
    &lbl_800259B0, &lbl_800259C0, &lbl_800259D8, &lbl_800259F0, &lbl_80025A08, &lbl_80025A20, &lbl_80025A38,
    &lbl_80025A50, &lbl_80025A60, &lbl_80025A78, &lbl_80025A90, &lbl_80025AA8, &lbl_80025AC0, &lbl_80025AD8,
    &lbl_80025AF0,
};
#else
void* jumptable_80171208[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171308[] = {
    &lbl_80021CB8, &lbl_80021EBC, &lbl_800220B8, &lbl_800222BC, &lbl_800224B8, &lbl_800225EC, &lbl_80022744,
    &lbl_800227D8, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80022930, &lbl_80022A68,
    &lbl_80022BA0, &lbl_80022CE8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8,
    &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8,
    &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_80031A30, &lbl_80022E30, &lbl_800256C8,
    &lbl_800256C8, &lbl_80022E48, &lbl_80031A30, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8,
    &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_800256C8, &lbl_80022F80,
    &lbl_80023028, &lbl_800230D0, &lbl_80023398, &lbl_80023660, &lbl_8002392C, &lbl_80023BF8, &lbl_80023EF8,
    &lbl_800241F8, &lbl_800242A0, &lbl_800245A0, &lbl_80024868, &lbl_80024B30, &lbl_80024DFC, &lbl_800250C8,
    &lbl_800253C8,
};
#else
void* jumptable_80171308[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171408[] = {
    &lbl_8001E130, &lbl_8001E340, &lbl_8001E548, &lbl_8001E758, &lbl_8001E960, &lbl_8001EA9C, &lbl_8001EBFC,
    &lbl_8001EC98, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_8001EDF8, &lbl_8001EF34,
    &lbl_8001F070, &lbl_8001F1BC, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C,
    &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C,
    &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80031A30, &lbl_8001F308, &lbl_80021C8C,
    &lbl_80021C8C, &lbl_8001F3A0, &lbl_80031A30, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C,
    &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_80021C8C, &lbl_8001F4DC,
    &lbl_8001F584, &lbl_8001F62C, &lbl_8001F8FC, &lbl_8001FBCC, &lbl_8001FEA0, &lbl_80020174, &lbl_8002047C,
    &lbl_80020784, &lbl_8002082C, &lbl_80020B34, &lbl_80020E04, &lbl_800210D4, &lbl_800213A8, &lbl_8002167C,
    &lbl_80021984,
};
#else
void* jumptable_80171408[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171508[] = {
    &lbl_8001D16C, &lbl_8001D2F0, &lbl_8001D484, &lbl_8001D9C0, &lbl_8001D598, &lbl_8001D760, &lbl_8001D8E8,
};
#else
void* jumptable_80171508[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171524[] = {
    &lbl_8001D090, &lbl_80031A30, &lbl_80031A30, &lbl_8001D090, &lbl_8001D090, &lbl_80031A30, &lbl_8001D090,
    &lbl_8001D090, &lbl_80031A30, &lbl_8001D090, &lbl_8001D090, &lbl_8001D090, &lbl_8001D090, &lbl_8001D090,
    &lbl_8001D090, &lbl_8001D090, &lbl_8001D090, &lbl_8001D090, &lbl_8001D090, &lbl_8001D090, &lbl_8001D090,
    &lbl_8001D090, &lbl_8001D090, &lbl_8001D090, &lbl_80031A30,
};
#else
void* jumptable_80171524[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171588[] = {
    &lbl_80018B84, &lbl_80018D94, &lbl_80018FA4, &lbl_800191E0, &lbl_8001A444, &lbl_8001A444, &lbl_8001A444,
    &lbl_8001A444, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_8001A444,
    &lbl_80031A30, &lbl_8001A444, &lbl_8001941C, &lbl_80019810, &lbl_80019C04, &lbl_8001A024,
};
#else
void* jumptable_80171588[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_801715D8[] = {
    &lbl_8000EFB8, &lbl_80018B58, &lbl_8000F20C, &lbl_8000F424, &lbl_8000F60C, &lbl_80018B58, &lbl_8000F904,
    &lbl_8000FBFC, &lbl_8000FEF4, &lbl_80010140, &lbl_80018B58, &lbl_80018B58, &lbl_80031A30, &lbl_80031A30,
    &lbl_80018B58, &lbl_80031A30, &lbl_8001070C, &lbl_80010918, &lbl_80010B08, &lbl_80010D14, &lbl_80010F04,
    &lbl_80018B58, &lbl_800111EC, &lbl_800114D4, &lbl_800117BC, &lbl_800119A0, &lbl_80011B84, &lbl_80011DBC,
    &lbl_80011FF4, &lbl_800125D4, &lbl_80012A64, &lbl_80013044, &lbl_800134D4, &lbl_80013ED0, &lbl_800148CC,
    &lbl_80014BDC, &lbl_80014EEC, &lbl_800151FC, &lbl_80015BF8, &lbl_80015F08, &lbl_80018B58, &lbl_80018B58,
    &lbl_80016218, &lbl_8001659C, &lbl_80016924, &lbl_80016D74, &lbl_800171C4, &lbl_80017614, &lbl_80031A30,
    &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80031A30, &lbl_80018B58, &lbl_80031A30, &lbl_80018B58,
    &lbl_80017A64, &lbl_80018B58, &lbl_80017D20, &lbl_80017FE4, &lbl_8001829C, &lbl_80018B58, &lbl_80018584,
    &lbl_80018874,
};
#else
void* jumptable_801715D8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_801716D8[] = {
    &lbl_8000EF90, &lbl_80018B60, &lbl_8001A44C, &lbl_8001A6BC, &lbl_8001ABD8, &lbl_8001B024, &lbl_8001B310,
    &lbl_8001B520, &lbl_8001B730, &lbl_8001BD1C, &lbl_8001C328, &lbl_8001C5D4, &lbl_8001C8C4, &lbl_8001CAA0,
    &lbl_8001CCDC, &lbl_8001CEF4, &lbl_8001D06C, &lbl_8001D134, &lbl_80025F90, &lbl_80025F98, &lbl_80025FA0,
    &lbl_80026378, &lbl_80026690, &lbl_800268CC, &lbl_80026B08, &lbl_80026FD4, &lbl_800274D8, &lbl_800274F8,
    &lbl_80031A2C, &lbl_80031A2C, &lbl_80031A2C, &lbl_800274A0, &lbl_80027538, &lbl_80027F60, &lbl_80028988,
    &lbl_80028D88, &lbl_800297FC, &lbl_8002A1DC, &lbl_8002ABBC, &lbl_80027518, &lbl_8002AFBC, &lbl_8002BAAC,
    &lbl_8002C59C, &lbl_8002C8DC, &lbl_80031A30, &lbl_80031A30, &lbl_8002D5DC, &lbl_8002D918, &lbl_8002D94C,
    &lbl_8002D96C, &lbl_80031A2C, &lbl_80031A2C, &lbl_8002E740, &lbl_8002E760, &lbl_80031A2C, &lbl_8002F444,
    &lbl_80031A30, &lbl_8002F8E8, &lbl_80031A2C, &lbl_80031A2C, &lbl_80030758, &lbl_80030778, &lbl_80031A2C,
    &lbl_80031594,
};
#else
void* jumptable_801716D8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_801717D8[] = {
    &lbl_80038570, &lbl_800385AC, &lbl_800385E8, &lbl_80038634, &lbl_8003866C, &lbl_8003869C, &lbl_800386CC,
    &lbl_800386F0, &lbl_8003871C, &lbl_80038740, &lbl_80038764, &lbl_80038794, &lbl_800387C4, &lbl_800387E0,
    &lbl_800387FC, &lbl_80038830, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80038864, &lbl_80038888, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_800388AC, &lbl_800388C8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_800388EC,
    &lbl_800388FC, &lbl_8003890C, &lbl_8003895C, &lbl_800389AC, &lbl_80038A08, &lbl_80038A64, &lbl_80038AC0,
    &lbl_80038B1C, &lbl_80038B2C, &lbl_80038B88, &lbl_80038BD8, &lbl_80038C28, &lbl_80038C84, &lbl_80038CE0,
    &lbl_80038D3C,
};
#else
void* jumptable_801717D8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_801718D8[] = {
    &lbl_80037E58, &lbl_80037E84, &lbl_80037EB0, &lbl_80037EDC, &lbl_80037F08, &lbl_80037F48, &lbl_80037F88,
    &lbl_80037FA4, &lbl_80037FC4, &lbl_80037FE8, &lbl_8003800C, &lbl_80038048, &lbl_80038084, &lbl_800380A0,
    &lbl_800380BC, &lbl_800380FC, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_8003813C, &lbl_8003816C, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_8003819C, &lbl_800381B8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_800381DC,
    &lbl_800381EC, &lbl_800381FC, &lbl_8003823C, &lbl_8003827C, &lbl_800382BC, &lbl_800382FC, &lbl_8003833C,
    &lbl_8003837C, &lbl_8003838C, &lbl_800383CC, &lbl_8003840C, &lbl_8003844C, &lbl_8003848C, &lbl_800384CC,
    &lbl_8003850C,
};
#else
void* jumptable_801718D8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_801719D8[] = {
    &lbl_800377AC, &lbl_800377D8, &lbl_80037804, &lbl_80037830, &lbl_8003785C, &lbl_8003787C, &lbl_8003789C,
    &lbl_800378B8, &lbl_800378D8, &lbl_80037904, &lbl_80037928, &lbl_80037950, &lbl_80037978, &lbl_800379A8,
    &lbl_800379D0, &lbl_800379FC, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80037A28, &lbl_80037A48, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80037A64, &lbl_80037A8C, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80037AB0,
    &lbl_80037AC0, &lbl_80037AD0, &lbl_80037B10, &lbl_80037B50, &lbl_80037B90, &lbl_80037BD0, &lbl_80037C14,
    &lbl_80037C58, &lbl_80037C68, &lbl_80037CAC, &lbl_80037CEC, &lbl_80037D2C, &lbl_80037D6C, &lbl_80037DAC,
    &lbl_80037DF0,
};
#else
void* jumptable_801719D8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171AD8[] = {
    &lbl_800370FC, &lbl_80037128, &lbl_80037154, &lbl_80037180, &lbl_800371AC, &lbl_800371D0, &lbl_800371F4,
    &lbl_80037210, &lbl_80037230, &lbl_8003725C, &lbl_80037280, &lbl_800372A8, &lbl_800372D0, &lbl_80037300,
    &lbl_80037328, &lbl_80037354, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80037380, &lbl_8003739C, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_800373B8, &lbl_800373E0, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80037404,
    &lbl_80037414, &lbl_80037424, &lbl_80037464, &lbl_800374A4, &lbl_800374E4, &lbl_80037524, &lbl_80037568,
    &lbl_800375AC, &lbl_800375BC, &lbl_80037600, &lbl_80037640, &lbl_80037680, &lbl_800376C0, &lbl_80037700,
    &lbl_80037744,
};
#else
void* jumptable_80171AD8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171BD8[] = {
    &lbl_80036E64, &lbl_80036EB0, &lbl_80036ED4, &lbl_80039BC8, &lbl_80036EF0, &lbl_80036F58, &lbl_80036F7C,
};
#else
void* jumptable_80171BD8[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171BF4[] = {
    &lbl_80036D80, &lbl_80036DBC, &lbl_80039BC8, &lbl_80039BC8, &lbl_80036DF0,
    &lbl_80036E10, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
};
#else
void* jumptable_80171BF4[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171C18[] = {
    &lbl_80036D5C, &lbl_80036ACC, &lbl_80036B20, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036B34, &lbl_80036D5C,
    &lbl_80036D5C, &lbl_80036BCC, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C,
    &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C,
    &lbl_80036D5C, &lbl_80036D5C, &lbl_80036D5C, &lbl_80036CF8,
};
#else
void* jumptable_80171C18[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171C7C[] = {
    &lbl_80036484, &lbl_800364B4, &lbl_800364E4, &lbl_80036530, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_80039BC8, &lbl_8003657C, &lbl_800365AC, &lbl_800365DC, &lbl_8003660C, &lbl_8003663C, &lbl_80039BC8,
    &lbl_8003666C, &lbl_80039BC8, &lbl_8003669C, &lbl_800366D8, &lbl_80036714, &lbl_80036768,
};
#else
void* jumptable_80171C7C[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171CCC[] = {
    &lbl_800359F8, &lbl_80039BC8, &lbl_80035A1C, &lbl_80035A40, &lbl_80035A64, &lbl_80039BC8, &lbl_80035A94,
    &lbl_80035AC4, &lbl_80035AF4, &lbl_80035B08, &lbl_80039BC8, &lbl_80039BC8, &lbl_80035B38, &lbl_80035B4C,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80035B60, &lbl_80035B7C, &lbl_80035B98, &lbl_80035BB4, &lbl_80035BD0,
    &lbl_80039BC8, &lbl_80035C08, &lbl_80035C40, &lbl_80035C78, &lbl_80035CCC, &lbl_80035D20, &lbl_80035D74,
    &lbl_80035DC8, &lbl_80035E38, &lbl_80035EA8, &lbl_80035EFC, &lbl_80035F50, &lbl_80035F7C, &lbl_80035FA8,
    &lbl_80035FD4, &lbl_80036000, &lbl_8003602C, &lbl_80036058, &lbl_80036084, &lbl_80039BC8, &lbl_80039BC8,
    &lbl_800360B0, &lbl_800360EC, &lbl_80036124, &lbl_80036160, &lbl_8003619C, &lbl_800361D8, &lbl_80036214,
    &lbl_80036248, &lbl_8003627C, &lbl_800362B0, &lbl_800362E4, &lbl_80039BC8, &lbl_80036318, &lbl_80039BC8,
    &lbl_8003634C, &lbl_80039BC8, &lbl_80036378, &lbl_800363A4, &lbl_800363D0, &lbl_80039BC8, &lbl_80036400,
    &lbl_80036430,
};
#else
void* jumptable_80171CCC[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171DCC[] = {
    &lbl_800359D4, &lbl_80036460, &lbl_800367BC, &lbl_80036818, &lbl_80036844, &lbl_800368D0, &lbl_8003690C,
    &lbl_8003693C, &lbl_8003696C, &lbl_80036994, &lbl_800369BC, &lbl_800369F4, &lbl_80036A28, &lbl_80036A4C,
    &lbl_80036A70, &lbl_80036A94, &lbl_80036AA8, &lbl_80036E30, &lbl_80039BC8, &lbl_80039BC8, &lbl_80038D98,
    &lbl_80038DF0, &lbl_80038E48, &lbl_80038E94, &lbl_80038EE0, &lbl_80038F18, &lbl_80038F78, &lbl_80039040,
    &lbl_80039BC8, &lbl_80039BC8, &lbl_80039BC8, &lbl_80038F50, &lbl_80039174, &lbl_800391D8, &lbl_80039238,
    &lbl_800392C8, &lbl_80039328, &lbl_80039388, &lbl_80039418, &lbl_80039108, &lbl_800394A8, &lbl_800394F8,
    &lbl_80039548, &lbl_800395B8, &lbl_80039608, &lbl_80039684, &lbl_800396FC, &lbl_80039BC8, &lbl_8003976C,
    &lbl_800397CC, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039870, &lbl_800398DC, &lbl_80039BC8, &lbl_80039944,
    &lbl_800399AC, &lbl_80039A14, &lbl_80039BC8, &lbl_80039BC8, &lbl_80039A90, &lbl_80039B0C, &lbl_80039BC8,
    &lbl_80039B6C,
};
#else
void* jumptable_80171DCC[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171ECC[] = {
    &lbl_8003A0D8, &lbl_8003A38C, &lbl_8003AB40, &lbl_8003A640, &lbl_8003A240, &lbl_8003A4F4,
    &lbl_8003AB40, &lbl_8003AB40, &lbl_8003A78C, &lbl_8003A8C8, &lbl_8003AB40, &lbl_8003AA04,
};
#else
void* jumptable_80171ECC[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171EFC[] = {
    &lbl_8003AEBC, &lbl_8003B5F8, &lbl_8003B5F8, &lbl_8003B5F8, &lbl_8003B100,
    &lbl_8003B5F8, &lbl_8003B300, &lbl_8003B5F8, &lbl_8003AFEC, &lbl_8003B5F8,
    &lbl_8003B5F8, &lbl_8003B5F8, &lbl_8003B200, &lbl_8003B5F8, &lbl_8003B47C,
};
#else
void* jumptable_80171EFC[] = {0};
#endif

_XL_OBJECTTYPE gClassCPU = {
    "CPU",
    sizeof(Cpu),
    NULL,
    (EventFunc)cpuEvent,
};

#ifndef NON_MATCHING
void* jumptable_80171F48[] = {
    &lbl_8003E0F0, &lbl_8003E338, &lbl_8003E338, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E338, &lbl_8003E0F0,
    &lbl_8003E0F0, &lbl_8003E338, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0,
    &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0,
    &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0F0, &lbl_8003E0C4,
};
#else
void* jumptable_80171F48[] = {0};
#endif

#ifndef NON_MATCHING
void* jumptable_80171FAC[] = {
    &lbl_8003DDB8, &lbl_8003DE78, &lbl_8003DE3C, &lbl_8003E330, &lbl_8003DF14, &lbl_8003E038, &lbl_8003E038,
    &lbl_8003E038, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330,
    &lbl_8003E330, &lbl_8003E330, &lbl_8003E0A0, &lbl_8003E180, &lbl_8003E330, &lbl_8003E330, &lbl_8003DF14,
    &lbl_8003E038, &lbl_8003E038, &lbl_8003E038, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330,
    &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330,
    &lbl_8003E220, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330, &lbl_8003E330,
    &lbl_8003E330, &lbl_8003E20C,
};
#else
void* jumptable_80171FAC[] = {0};
#endif

static s32 cpuCompile_DSLLV_function;
static s32 cpuCompile_DSRLV_function;
static s32 cpuCompile_DSRAV_function;
static s32 cpuCompile_DMULT_function;
static s32 cpuCompile_DMULTU_function;
static s32 cpuCompile_DDIV_function;
static s32 cpuCompile_DDIVU_function;
static s32 cpuCompile_DADD_function;
static s32 cpuCompile_DADDU_function;
static s32 cpuCompile_DSUB_function;
static s32 cpuCompile_DSUBU_function;
static s32 cpuCompile_S_SQRT_function;
static s32 cpuCompile_D_SQRT_function;
static s32 cpuCompile_W_CVT_SD_function;
static s32 cpuCompile_L_CVT_SD_function;
static s32 cpuCompile_CEIL_W_function;
static s32 cpuCompile_FLOOR_W_function;
static s32 cpuCompile_ROUND_W_function;
static s32 cpuCompile_TRUNC_W_function;
static s32 cpuCompile_LB_function;
static s32 cpuCompile_LH_function;
static s32 cpuCompile_LW_function;
static s32 cpuCompile_LBU_function;
static s32 cpuCompile_LHU_function;
static s32 cpuCompile_SB_function;
static s32 cpuCompile_SH_function;
static s32 cpuCompile_SW_function;
static s32 cpuCompile_LDC_function;
static s32 cpuCompile_SDC_function;
static s32 cpuCompile_LWL_function;
static s32 cpuCompile_LWR_function;
static VIRetraceCallback __cpuRetraceCallback;

static inline bool cpuCheckInterrupts(Cpu* pCPU) {
    System* pSystem;

    pSystem = gpSystem;
    if (pSystem->bException) {
        if (!systemCheckInterrupts(pSystem)) {
            return false;
        }
    } else {
        viForceRetrace(SYSTEM_VI(pSystem));
    }

    return true;
}

static inline bool cpuTLBRandom(Cpu* pCPU) {
    s32 iEntry;
    s32 nCount;

    nCount = 0;
    for (iEntry = 0; iEntry < ARRAY_COUNT(pCPU->aTLB); iEntry++) {
        if ((pCPU->aTLB[iEntry][2] & 2) == 0) {
            nCount++;
        }
    }

    return nCount;
}

static inline bool cpuExecuteCacheInstruction(Cpu* pCPU) {
    s32* pBuffer;

    if (!cpuGetAddressBuffer(pCPU, (void**)&pBuffer, pCPU->nPC)) {
        return false;
    }
    pBuffer[-1] = 0;
    pBuffer -= (pCPU->nPC - pCPU->nCallLast) >> 2;
    pBuffer[0] = 0x03E00008;
    pBuffer[1] = 0;

    return true;
}

static inline void treeCallerInit(CpuCallerID* block, s32 total) {
    s32 count;

    for (count = 0; count < total; count++) {
        block[count].N64address = 0;
        block[count].GCNaddress = 0;
    }
}

static inline bool treeCallerKill(Cpu* pCPU, CpuFunction* kill) {
    s32 left;
    s32 right;
    CpuTreeRoot* root;

    if (kill->pfCode != NULL) {
        root = pCPU->gTree;
        left = kill->nAddress0;
        right = kill->nAddress1;

        if (root->left != NULL) {
            treePrintNode(pCPU, root->left, 0x10, &left, &right);
        }
        if (root->right != NULL) {
            treePrintNode(pCPU, root->right, 0x10, &left, &right);
        }
    }

    pCPU->gTree->total_memory -= kill->memory_size + sizeof(CpuFunction);
    return true;
}

static inline bool treeForceCleanUp(Cpu* pCPU, CpuFunction* tree, s32 kill_limit) {
    CpuTreeRoot* root = pCPU->gTree;

    root->kill_limit = 0;
    root->restore = NULL;
    root->restore_side = 0;
    if (tree != NULL && tree->timeToLive > 0) {
        tree->timeToLive = pCPU->survivalTimer;
    }
    if (root->side == 0) {
        if (root->left != NULL) {
            treeForceCleanNodes(pCPU, root->left, kill_limit);
        }
    } else {
        if (root->right != NULL) {
            treeForceCleanNodes(pCPU, root->right, kill_limit);
        }
    }
    root->side ^= 1;
    return true;
}

#if VERSION < MK64_J 
static bool cpuHackHandler(Cpu* pCPU) {
    u32 nSize;
    u32* pnCode;
    s32 iCode;
    s32 iSave1;
    s32 iSave2;
    s32 iLoad;

    iSave1 = iSave2 = iLoad = 0;

    if (xlObjectTest(SYSTEM_RAM(gpSystem), &gClassRAM) &&
        ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pnCode, 0, NULL)) {
        if (!ramGetSize(SYSTEM_RAM(gpSystem), (s32*)&nSize)) {
            return false;
        }

        for (iCode = 0; iCode < (nSize >> 2) && (iSave1 != -1 || iSave2 != -1 || iLoad != -1); iCode++) {
            if (iSave1 != -1) {
                if (pnCode[iCode] == ganOpcodeSaveFP1[iSave1]) {
                    iSave1 += 1;
                    if (iSave1 == 5U) {
                        pnCode[iCode - 3] = 0;
                        iSave1 = -1;
                    }
                } else {
                    iSave1 = 0;
                }
            }

            if (iSave2 != -1) {
                if (pnCode[iCode] == ganOpcodeSaveFP2_0[iSave2]) {
                    iSave2 += 1;
                    if (iSave2 == 5U) {
                        pnCode[iCode - 3] = 0;
                        iSave2 = -1;
                    }
                } else if (pnCode[iCode] == ganOpcodeSaveFP2_1[iSave2]) {
                    iSave2 += 1;
                    if (iSave2 == 3U) {
                        pnCode[iCode - 2] = 0;
                        iSave2 = -1;
                    }
                } else {
                    iSave2 = 0;
                }
            }

            if (iLoad != -1) {
                if (pnCode[iCode] == ganOpcodeLoadFP[iLoad]) {
                    iLoad += 1;
                    if (iLoad == 5U) {
                        pnCode[iCode - 3] = 0;
                        iLoad = -1;
                    }
                } else {
                    iLoad = 0;
                }
            }
        }
    }

    return (iSave1 == -1 && iSave2 == -1 && iLoad == -1) ? true : false;
}
#else
static bool cpuHackHandler(Cpu* pCPU) {
    s32 iSave1;
    s32 iSave2;
    s32 iLoad;
    u32* pnCode;
    s32 nSize;
    s32 iCode;

    iSave1 = iSave2 = iLoad = 0;

    if (xlObjectTest(SYSTEM_RAM(gpSystem), &gClassRAM) &&
        ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&pnCode, 0, NULL)) {
        if (!ramGetSize(SYSTEM_RAM(gpSystem), &nSize)) {
            return false;
        }

        for (iCode = 0; iCode < (nSize >> 2) && (iSave1 != -1 || iSave2 != -1 || iLoad != -1); iCode++) {
            if (iSave1 != -1) {
                if (pnCode[iCode] == ganOpcodeSaveFP1[iSave1]) {
                    iSave1++;
                    if (iSave1 == 5U) {
                        pnCode[iCode - 3] = 0;
                        iSave1 = -1;
                    }
                } else {
                    iSave1 = 0;
                }
            }

            if (iSave2 != -1) {
                if (pnCode[iCode] == ganOpcodeSaveFP2_0[iSave2]) {
                    iSave2++;
                    if (iSave2 == 5U) {
                        pnCode[iCode - 3] = 0;
                        iSave2 = -1;
                    }
                } else if (pnCode[iCode] == ganOpcodeSaveFP2_1[iSave2]) {
                    iSave2++;
                    if (iSave2 == 3U) {
                        pnCode[iCode - 2] = 0;
                        iSave2 = -1;
                    }
                } else {
                    iSave2 = 0;
                }
            }

            if (iLoad != -1) {
                if (pnCode[iCode] == ganOpcodeLoadFP[iLoad]) {
                    iLoad++;
                    if (iLoad == 5U) {
                        pnCode[iCode - 3] = 0;
                        iLoad = -1;
                    }
                } else {
                    iLoad = 0;
                }
            }
        }
    }

    return (iSave1 == -1 && iSave2 == -1 && iLoad == -1) ? true : false;
}
#endif

static inline bool cpuMakeCachedAddress(Cpu* pCPU, s32 nAddressN64, s32 nAddressHost, CpuFunction* pFunction) {
    s32 iAddress;
    CpuAddress* aAddressCache;

    aAddressCache = pCPU->aAddressCache;
    if ((iAddress = pCPU->nCountAddress) == ARRAY_COUNT(pCPU->aAddressCache)) {
        iAddress--;
    } else {
        pCPU->nCountAddress++;
    }

    for (; iAddress > 0; iAddress--) {
        aAddressCache[iAddress] = aAddressCache[iAddress - 1];
    }

    aAddressCache[0].nN64 = nAddressN64;
    aAddressCache[0].nHost = nAddressHost;
    aAddressCache[0].pFunction = pFunction;
    return true;
}

static inline bool treeSearch(Cpu* pCPU, s32 target, CpuFunction** node) {
    CpuTreeRoot* root = pCPU->gTree;
    bool flag;

    if (target < root->root_address) {
        flag = treeSearchNode(root->left, target, node);
    } else {
        flag = treeSearchNode(root->right, target, node);
    }
    return flag;
}

bool cpuFreeCachedAddress(Cpu* pCPU, s32 nAddress0, s32 nAddress1) {
    s32 iAddress;
    s32 iAddressNext;
    s32 nAddressN64;
    CpuAddress* aAddressCache = pCPU->aAddressCache;

    iAddress = 0;
    while (iAddress < pCPU->nCountAddress) {
        nAddressN64 = aAddressCache[iAddress].nN64;
        if (nAddress0 <= nAddressN64 && nAddressN64 <= nAddress1) {
            for (iAddressNext = iAddress; iAddressNext < pCPU->nCountAddress - 1; iAddressNext++) {
                aAddressCache[iAddressNext] = aAddressCache[iAddressNext + 1];
            }
            pCPU->nCountAddress--;
        } else {
            iAddress++;
        }
    }

    return true;
}

static bool cpuFindCachedAddress(Cpu* pCPU, s32 nAddressN64, s32* pnAddressHost) {
    s32 iAddress;
    CpuFunction* pFunction;
    CpuAddress addressFound;
    CpuAddress* aAddressCache = pCPU->aAddressCache;

    for (iAddress = 0; iAddress < pCPU->nCountAddress; iAddress++) {
        if (nAddressN64 == aAddressCache[iAddress].nN64) {
            if (iAddress > ARRAY_COUNT(pCPU->aAddressCache) / 2) {
                addressFound = aAddressCache[iAddress];
                for (; iAddress > 0; iAddress--) {
                    aAddressCache[iAddress] = aAddressCache[iAddress - 1];
                }
                aAddressCache[iAddress] = addressFound;
            }

            pFunction = aAddressCache[iAddress].pFunction;
            if (pFunction->timeToLive > 0) {
                pFunction->timeToLive = pCPU->survivalTimer;
            }

            *pnAddressHost = aAddressCache[iAddress].nHost;
            return true;
        }
    }

    return false;
}

bool cpuTestInterrupt(Cpu* pCPU, s32 nMaskIP) {
    pCPU->anCP0[13] |= (nMaskIP & 0xFF) << 8;

    if ((pCPU->anCP0[12] & 6) != 0) {
        return false;
    }

    if ((pCPU->anCP0[12] & 1) == 0) {
        return false;
    }

    if ((((pCPU->anCP0[12] & 0xFF00) >> 8) & (nMaskIP & 0xFF)) == 0) {
        return false;
    }

    return true;
}

bool cpuException(Cpu* pCPU, CpuExceptionCode eCode, s32 nMaskIP) {
    s32 pad[2];

    if ((pCPU->anCP0[12] & 6) != 0) {
        return false;
    }
    nMaskIP &= 0xFF;
    if (eCode == CEC_NONE) {
        return false;
    }
    if ((eCode >= CEC_RESERVED_16 && eCode <= CEC_RESERVED_22) ||
        (eCode >= CEC_RESERVED_24 && eCode <= CEC_RESERVED_30)) {
        return false;
    }
    if (eCode == CEC_RESERVED) {
        return false;
    }

    if (eCode == CEC_INTERRUPT) {
        if (!cpuTestInterrupt(pCPU, nMaskIP)) {
            return false;
        }
    } else {
        pCPU->nPC -= 4;
        pCPU->nMode |= 4;
    }

    pCPU->nMode &= ~8;
    if (!(pCPU->nMode & 0x10)) {
        if (!cpuHackHandler(pCPU)) {}
        pCPU->nMode |= 0x10;
    }
    if (pCPU->nWaitPC != 0xFFFFFFFF) {
        pCPU->nWaitPC = -1;
        pCPU->anCP0[14] = pCPU->nPC - 4;
        pCPU->anCP0[13] |= 0x80000000;
    } else {
        pCPU->anCP0[14] = pCPU->nPC;
    }

    pCPU->nMode &= ~0x80;
    pCPU->anCP0[12] |= 2;
    pCPU->anCP0[13] = (pCPU->anCP0[13] & ~0x7C) | (eCode << 2);

    if (eCode - 1 <= 2U) {
        pCPU->nPC = 0x80000000;
    } else {
        pCPU->nPC = 0x80000180;
    }

    pCPU->nMode |= 4;
    pCPU->nMode |= 0x20;

    if (!libraryCall(SYSTEM_LIBRARY(gpSystem), pCPU, -1)) {
        return false;
    }

    return true;
}

/**
 * @brief Creates a new device and registers memory space for that device.
 *
 * @param pCPU The emulated VR4300.
 * @param piDevice A pointer to the index in the cpu->devices array which the device was created.
 * @param pObject The object which will handle reuqests for this device.
 * @param nOffset Starting address of the device's address space.
 * @param nAddress Starting physical address of the device's address space.
 * @param nSize Size of the device's memory space.
 * @param nType An argument which will be passed back to the device's event handler.
 * @return bool true on success, false otherwise.
 */
#if VERSION < MK64_J 
static bool cpuMakeDevice(Cpu* pCPU, s32* piDevice, void* pObject, s32 nOffset, u32 nAddress0, u32 nAddress1,
                          s32 nType) {
    CpuDevice* pDevice;
    s32 iDevice;
    s32 pad;

    iDevice = (nType & 0x100) ? (ARRAY_COUNT(pCPU->apDevice) / 2) : 0;
    for (; iDevice < ARRAY_COUNT(pCPU->apDevice); iDevice++) {
        if (pCPU->apDevice[iDevice] == NULL) {
            break;
        }
    }
    if (iDevice == ARRAY_COUNT(pCPU->apDevice)) {
        return false;
    }

    *piDevice = iDevice;
    if (!xlHeapTake((void**)&pDevice, sizeof(CpuDevice))) {
        return false;
    }

    pCPU->apDevice[iDevice] = pDevice;
    pDevice->nType = nType;
    pDevice->pObject = pObject;
    pDevice->nOffsetAddress = nOffset;
    pDevice->nAddressPhysical0 = nAddress0;
    pDevice->nAddressPhysical1 = nAddress1;

    if (!xlObjectEvent(pObject, 0x1002, pDevice)) {
        return false;
    }

    return true;
}
#else
static bool cpuMakeDevice(Cpu* pCPU, s32* piDevice, void* pObject, u32 nOffset, u32 nAddress, u32 nSize, s32 nType) {
    CpuDevice* pDevice;
    s32 iDevice;
    s32 j;
    s32 pad;

    for (iDevice = ((nType >> 8) & 1) ? 128 : 0; iDevice < 256; iDevice++) {
        if (pCPU->apDevice[iDevice] == NULL) {
            if (piDevice != NULL) {
                *piDevice = iDevice;
            }

            if (!xlHeapTake((void**)&pDevice, sizeof(CpuDevice))) {
                return false;
            }

            pCPU->apDevice[iDevice] = pDevice;
            pDevice->nType = nType;
            pDevice->pObject = pObject;
            pDevice->nOffsetAddress = nAddress - nOffset;

            if (nSize == 0) {
                pDevice->nAddressPhysical0 = 0;
                pDevice->nAddressVirtual0 = 0;
                pDevice->nAddressPhysical1 = 0xFFFFFFFF;
                pDevice->nAddressVirtual1 = 0xFFFFFFFF;

                for (j = 0; j < ARRAY_COUNT(pCPU->aiDevice); j++) {
                    pCPU->aiDevice[j] = iDevice;
                }
            } else {
                pDevice->nAddressVirtual0 = nOffset;
                pDevice->nAddressVirtual1 = nOffset + nSize - 1;
                pDevice->nAddressPhysical0 = nAddress;
                pDevice->nAddressPhysical1 = nAddress + nSize - 1;
                for (j = nSize; j > 0; nOffset += 0x10000, j -= 0x10000) {
                    pCPU->aiDevice[nOffset >> DEVICE_ADDRESS_OFFSET_BITS] = iDevice;
                }
            }

            if (!xlObjectEvent(pObject, 0x1002, (void*)pDevice)) {
                return false;
            }

            return true;
        }
    }

    return false;
}
#endif

static inline bool cpuFreeDevice(Cpu* pCPU, s32 iDevice) {
    s32 ret;
    if (!xlHeapFree((void**)&pCPU->apDevice[iDevice])) {
        return false;
    } else {
        s32 iAddress;

        pCPU->apDevice[iDevice] = NULL;
        for (iAddress = 0; iAddress < ARRAY_COUNT(pCPU->aiDevice); iAddress++) {
            if (pCPU->aiDevice[iAddress] == iDevice) {
                pCPU->aiDevice[iAddress] = pCPU->iDeviceDefault;
            }
        }
        return true;
    }
}

#if VERSION < MK64_J 
static bool cpuMapAddress(Cpu* pCPU, s32* piDevice, u32 nVirtual, u32 nPhysical, s32 nSize) {
    s32 iDeviceTarget;
    s32 iDeviceSource;
    u32 nAddressVirtual0;
    u32 nAddressVirtual1;

    for (iDeviceSource = 0; iDeviceSource < ARRAY_COUNT(pCPU->apDevice); iDeviceSource++) {
        if (iDeviceSource != pCPU->iDeviceDefault && pCPU->apDevice[iDeviceSource] != NULL &&
            pCPU->apDevice[iDeviceSource]->nAddressPhysical0 <= nPhysical &&
            nPhysical <= pCPU->apDevice[iDeviceSource]->nAddressPhysical1) {
            break;
        }
    }

    if (iDeviceSource == ARRAY_COUNT(pCPU->apDevice)) {
        iDeviceSource = pCPU->iDeviceDefault;
    }

    if (!cpuMakeDevice(pCPU, &iDeviceTarget, pCPU->apDevice[iDeviceSource]->pObject, nPhysical - nVirtual,
                       pCPU->apDevice[iDeviceSource]->nAddressPhysical0,
                       pCPU->apDevice[iDeviceSource]->nAddressPhysical1, pCPU->apDevice[iDeviceSource]->nType)) {
        return false;
    }

    nAddressVirtual0 = nVirtual;
    nAddressVirtual1 = nVirtual + nSize - 1;
    while (nAddressVirtual0 < nAddressVirtual1) {
        pCPU->aiDevice[nAddressVirtual0 >> 16] = iDeviceTarget;
        nAddressVirtual0 += 0x10000;
    }

    if (piDevice != NULL) {
        *piDevice = iDeviceTarget;
    }

    return true;
}

static bool cpuSetTLB(Cpu* pCPU, s32 iEntry) {
    s32 iDevice;
    u32 nMask;
    u32 nVirtual;
    u32 nPhysical;

    if ((pCPU->anCP0[10] & 0xFFFFE000) == 0x80000000) {
        pCPU->aTLB[iEntry][0] &= ~2;
        ;
        if ((iDevice = pCPU->aTLB[iEntry][4]) != -1) {
            if (!cpuFreeDevice(pCPU, iDevice)) {
                return false;
            }
            pCPU->aTLB[iEntry][4] = -1;
        }

        return true;
    }

    if ((iDevice = pCPU->aTLB[iEntry][4]) != -1) {
        if (!cpuFreeDevice(pCPU, iDevice)) {
            return false;
        }
    }

    pCPU->aTLB[iEntry][0] = pCPU->anCP0[2] | 2;
    pCPU->aTLB[iEntry][1] = pCPU->anCP0[3];
    pCPU->aTLB[iEntry][2] = pCPU->anCP0[10];
    pCPU->aTLB[iEntry][3] = pCPU->anCP0[5];

    nMask = pCPU->aTLB[iEntry][3] | 0x1FFF;
    nVirtual = pCPU->aTLB[iEntry][2] & 0xFFFFE000;
    nPhysical = ((s32)(pCPU->aTLB[iEntry][0] & 0xFFFFFFC0) << 6) + (nVirtual & nMask);

    if (nVirtual < 0x80000000 || 0xC0000000 <= nVirtual) {
        if (!cpuMapAddress(pCPU, &iDevice, nVirtual, nPhysical, nMask + 1)) {
            return false;
        }
        if (nVirtual == 0x70000000 && nPhysical == 0 && nMask == 0x007FFFFF) {
            if (!cpuMapAddress(pCPU, NULL, 0x7F000000, 0x10034B30, 0x01000000)) {
                return false;
            }
        }
    } else {
        iDevice = -1;
    }

    pCPU->aTLB[iEntry][4] = iDevice;
    return true;
}
#else
static bool cpuMapAddress(Cpu* pCPU, s32* piDevice, u32 nVirtual, u32 nPhysical, s32 nSize) {
    s32 iDeviceTarget;
    s32 iDeviceSource;
    u32 nAddressVirtual0;
    u32 nAddressVirtual1;

    for (iDeviceSource = 128; iDeviceSource < ARRAY_COUNT(pCPU->apDevice); iDeviceSource++) {
        if (iDeviceSource != pCPU->iDeviceDefault && pCPU->apDevice[iDeviceSource] != NULL &&
            pCPU->apDevice[iDeviceSource]->nAddressPhysical0 <= nPhysical &&
            nPhysical <= pCPU->apDevice[iDeviceSource]->nAddressPhysical1) {
            break;
        }
    }

    if (iDeviceSource == ARRAY_COUNT(pCPU->apDevice)) {
        iDeviceSource = pCPU->iDeviceDefault;
    }

    //! TODO: nAddress1 is set to nSize? bug?
    if (!cpuMakeDevice(pCPU, &iDeviceTarget, pCPU->apDevice[iDeviceSource]->pObject, nVirtual, nPhysical, nSize,
                       pCPU->apDevice[iDeviceSource]->nType)) {
        return false;
    }

    if (piDevice != NULL) {
        *piDevice = iDeviceTarget;
    }

    return true;
}

static bool cpuSetTLB(Cpu* pCPU, s32 iEntry) {
    s32 iDevice;
    s32 nPageSize;
    u32 nVirtual;
    u32 nPhysical;

    pCPU->aTLB[iEntry][3] = pCPU->anCP0[5] & TLB_PGSZ_MASK;
    pCPU->aTLB[iEntry][2] = pCPU->anCP0[10];
    pCPU->aTLB[iEntry][0] = pCPU->anCP0[2];
    pCPU->aTLB[iEntry][1] = pCPU->anCP0[3];

    if (pCPU->aTLB[iEntry][0] & 2 || pCPU->aTLB[iEntry][1] & 2) {
        switch (pCPU->aTLB[iEntry][3]) {
            case TLB_PGSZ_4K:
                nPageSize = 4 * 1024;
                break;
            case TLB_PGSZ_16K:
                nPageSize = 16 * 1024;
                break;
            case TLB_PGSZ_64K:
                nPageSize = 64 * 1024;
                break;
            case TLB_PGSZ_256K:
                nPageSize = 256 * 1024;
                break;
            case TLB_PGSZ_1M:
                nPageSize = 1 * 1024 * 1024;
                break;
            case TLB_PGSZ_4M:
                nPageSize = 4 * 1024 * 1024;
                break;
            case TLB_PGSZ_16M:
                nPageSize = 16 * 1024 * 1024;
                break;
            default:
                nPageSize = 0;
                break;
        }

        nVirtual = (u32)pCPU->aTLB[iEntry][2] & 0xFFFFE000;

        if (pCPU->aTLB[iEntry][0] & 2) {
            nPhysical = ((u32)pCPU->aTLB[iEntry][0] & 0x3FFFFC0) << 6;
            iDevice = pCPU->aTLB[iEntry][4] & 0xFF;

            if (iDevice != 0xFF) {
                if (!cpuFreeDevice(pCPU, iDevice)) {
                    return false;
                }
            }

            if (!cpuMapAddress(pCPU, &iDevice, nVirtual, nPhysical, nPageSize)) {
                return false;
            }

            pCPU->aTLB[iEntry][4] = (iDevice & 0xFF) | (pCPU->aTLB[iEntry][4] & ~0xFF);
        }

        if (pCPU->aTLB[iEntry][1] & 2) {
            nPhysical = ((u32)pCPU->aTLB[iEntry][1] & 0x3FFFFC0) << 6;
            iDevice = ((u32)pCPU->aTLB[iEntry][4] >> 16) & 0xFF;

            if (iDevice != 0xFF) {
                if (!cpuFreeDevice(pCPU, iDevice)) {
                    return false;
                }
            }

            if (!cpuMapAddress(pCPU, &iDevice, nVirtual + nPageSize, nPhysical, nPageSize)) {
                return false;
            }

            pCPU->aTLB[iEntry][4] = ((iDevice & 0xFF) << 16) | (pCPU->aTLB[iEntry][4] & ~0xFF0000);
        }
    } else {
        iDevice = pCPU->aTLB[iEntry][4] & 0xFF;
        if (iDevice != 0xFF) {
            if (!cpuFreeDevice(pCPU, iDevice)) {
                return false;
            }
        }

        iDevice = ((u32)pCPU->aTLB[iEntry][4] >> 16) & 0xFF;
        if (iDevice != 0xFF) {
            if (!cpuFreeDevice(pCPU, iDevice)) {
                return false;
            }
        }

        pCPU->aTLB[iEntry][4] = -1;
    }

    return true;
}
#endif

/**
 * @brief Gets the operating mode of the VR4300
 *
 * @param nStatus The status bits to determine the mode for.
 * @param peMode A pointer to the mode determined.
 * @return bool true on success, false otherwise.
 */
static bool cpuGetMode(u64 nStatus, CpuMode* peMode) NO_INLINE {
    if (nStatus & 2) {
        *peMode = CM_KERNEL;
        return true;
    }

    if (!(nStatus & 4)) {
        switch (nStatus & 0x18) {
            case 0x10:
                *peMode = CM_USER;
                break;
            case 8:
                *peMode = CM_SUPER;
                break;
            case 0:
                *peMode = CM_KERNEL;
                break;
            default:
                return false;
        }
        return true;
    }

    return false;
}

/**
 * @brief Determines the register size that the VR4300 is using.
 *
 * @param nStatus Status bits for determining the register size.
 * @param peSize A pointer to the size determined.
 * @param peMode A pointer to the mode determined.
 * @return bool
 */
static bool cpuGetSize(u64 nStatus, CpuSize* peSize, CpuMode* peMode) {
    CpuMode eMode;

    *peSize = CS_NONE;
    if (peMode != NULL) {
        *peMode = CM_NONE;
    }

    if (cpuGetMode(nStatus, &eMode)) {
        switch (eMode) {
            case CM_USER:
                *peSize = nStatus & 0x20 ? CS_64BIT : CS_32BIT;
                break;
            case CM_SUPER:
                *peSize = nStatus & 0x40 ? CS_64BIT : CS_32BIT;
                break;
            case CM_KERNEL:
                *peSize = nStatus & 0x80 ? CS_64BIT : CS_32BIT;
                break;
            default:
                return false;
        }

        if (peMode != NULL) {
            *peMode = eMode;
        }

        return true;
    }

    return false;
}

/**
 * @brief Sets the status bits of the VR4300
 *
 * @param pCPU The emulated VR4300
 * @param nStatus New status.
 * @param unknown Unused.
 * @return bool true on success, false otherwise.
 */
static bool cpuSetCP0_Status(Cpu* pCPU, u64 nStatus, u32 unknown) NO_INLINE {
    CpuMode eMode;
    CpuMode eModeLast;
    CpuSize eSize;
    CpuSize eSizeLast;

    if (!cpuGetSize(nStatus, &eSize, &eMode)) {
        return false;
    }
    if (!cpuGetSize(pCPU->anCP0[12], &eSizeLast, &eModeLast)) {
        return false;
    }

    pCPU->anCP0[12] = nStatus;

    return true;
}

#if VERSION < MK64_J 
bool cpuSetRegisterCP0(Cpu* pCPU, s32 iRegister, s64 nData) {
    s32 pad;
    s32 bFlag = false;

    switch (iRegister) {
        case 1:
        case 7:
        case 8:
            break;
        case 9:
            bFlag = true;
            break;
        case 11:
            bFlag = true;
            xlObjectEvent(gpSystem, 0x1001, (void*)3);
            if (pCPU->nMode & 1 || (nData & ganMaskSetCP0[iRegister]) == 0) {
                pCPU->nMode &= ~1;
            } else {
                pCPU->nMode |= 1;
            }
            break;
        case 12:
            cpuSetCP0_Status(pCPU, nData & ganMaskSetCP0[iRegister], 0);
            break;
        case 13:
            xlObjectEvent(gpSystem, (nData & 0x100) ? 0x1000 : 0x1001, (void*)0);
            xlObjectEvent(gpSystem, (nData & 0x200) ? 0x1000 : 0x1001, (void*)1);
            bFlag = true;
            break;
        case 14:
            bFlag = true;
            break;
        case 15:
            break;
        case 16:
            pCPU->anCP0[16] = (u32)(nData & ganMaskSetCP0[iRegister]);
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 27:
        case 31:
            break;
        default:
            bFlag = true;
            break;
    }

    if (bFlag) {
        pCPU->anCP0[iRegister] = nData & ganMaskSetCP0[iRegister];
    }

    return true;
}
#else
// matches but data doesn't
bool cpuSetRegisterCP0(Cpu* pCPU, s32 iRegister, s64 nData) {
    s32 pad;
    s32 bFlag = false;

    switch (iRegister) {
        case 0:
            pCPU->anCP0[0] = (pCPU->anCP0[0] & 0x80000000) | (nData & ganMaskSetCP0[0]);
            break;
        case 1:
        case 7:
        case 8:
            break;
        case 9:
            bFlag = true;
            break;
        case 11:
            bFlag = true;
            xlObjectEvent(gpSystem, 0x1001, (void*)3);
            if (pCPU->nMode & 1 || (nData & ganMaskSetCP0[11]) == 0) {
                pCPU->nMode &= ~1;
            } else {
                pCPU->nMode |= 1;
            }
            break;
        case 12:
            cpuSetCP0_Status(pCPU, nData & ganMaskSetCP0[12], 0);
            break;
        case 13:
            xlObjectEvent(gpSystem, (nData & 0x100) ? 0x1000 : 0x1001, (void*)0);
            xlObjectEvent(gpSystem, (nData & 0x200) ? 0x1000 : 0x1001, (void*)1);
            bFlag = true;
            break;
        case 14:
            bFlag = true;
            break;
        case 15:
            break;
        case 16:
            pCPU->anCP0[16] = (u32)(nData & ganMaskSetCP0[16]);
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 27:
        case 31:
            break;
        default:
            bFlag = true;
            break;
    }

    if (bFlag) {
        pCPU->anCP0[iRegister] = nData & ganMaskSetCP0[iRegister];
    }

    return true;
}
#endif

// matches but data doesn't
bool cpuGetRegisterCP0(Cpu* pCPU, s32 iRegister, s64* pnData) {
    bool bFlag = false;

    switch (iRegister) {
        case 1:
            *pnData = cpuTLBRandom(pCPU);
            break;
        case 9:
            bFlag = true;
            break;
        case 11:
            bFlag = true;
            break;
        case 14:
            bFlag = true;
            break;
        case 7:
            *pnData = 0;
            break;
        case 8:
            bFlag = true;
            break;
        case 21:
            *pnData = 0;
            break;
        case 22:
            *pnData = 0;
            break;
        case 23:
            *pnData = 0;
            break;
        case 24:
            *pnData = 0;
            break;
        case 25:
            *pnData = 0;
            break;
        case 31:
            *pnData = 0;
            break;
        default:
            bFlag = true;
            break;
    }

    if (bFlag) {
        *pnData = pCPU->anCP0[iRegister] & ganMaskGetCP0[iRegister];
    }

    return true;
}

/**
 * @brief Sets CP0 values for returnning from an exception.
 *
 * @param pCPU The emulated VR4300.
 * @return bool true on success, false otherwise
 */
bool __cpuERET(Cpu* pCPU) {
    if (pCPU->anCP0[12] & 4) {
        pCPU->nPC = pCPU->anCP0[30];
        pCPU->anCP0[12] &= ~4;
    } else {
        pCPU->nPC = pCPU->anCP0[14];
        pCPU->anCP0[12] &= ~2;
    }

    pCPU->nMode |= 4;
    pCPU->nMode |= 0x20;

    return true;
}

/**
 * @brief Sets flags for handling cpu breakpoints.
 *
 * @param pCPU The emulated VR4300.
 * @return bool true on success, false otherwise
 */
bool __cpuBreak(Cpu* pCPU) {
    pCPU->nMode |= 2;
    return true;
}

bool cpuFindBranchOffset(Cpu* pCPU, CpuFunction* pFunction, s32* pnOffset, s32 nAddress, s32* anCode) {
    s32 iJump;

    if (anCode == NULL) {
        *pnOffset = 0;
        return true;
    }

    for (iJump = 0; iJump < pFunction->nCountJump; iJump++) {
        if (nAddress == pFunction->aJump[iJump].nAddressN64) {
            *pnOffset = pFunction->aJump[iJump].nOffsetHost;
            return true;
        }
    }

    return false;
}

// matches but data doesn't
/**
 * @brief Checks the type of delay an instruction has.
 *
 * @param opcode The instruction to determine the delay type for.
 * @return s32 The type of delay the instruction has.
 */
static s32 cpuCheckDelaySlot(u32 opcode) {
    s32 flag = 0;

    if (opcode == 0) {
        return false;
    }

    switch (MIPS_OP(opcode)) {
        case 0x00: // special
            switch (MIPS_FUNCT(opcode)) {
                case 0x08: // jr
                    flag = 0xD05;
                    break;
                case 0x09: // jalr
                    flag = 0x8AE;
                    break;
            }
            break;
        case 0x01: // regimm
            switch (MIPS_RT(opcode)) {
                case 0x00: // bltz
                case 0x01: // bgez
                case 0x02: // bltzl
                case 0x03: // bgezl
                case 0x10: // bltzal
                case 0x11: // bgezal
                case 0x12: // bltzall
                case 0x13: // bgezall
                    flag = 0x457;
                    break;
            }
            break;
        case 0x10: // cop0
            switch (MIPS_FUNCT(opcode)) {
                case 0x01:
                case 0x02:
                case 0x05:
                case 0x08:
                case 0x18:
                    break;
                default:
                case 0x00:
                case 0x03:
                case 0x04:
                case 0x06:
                case 0x07:
                case 0x09:
                case 0x0A:
                case 0x0B:
                case 0x0C:
                case 0x0D:
                case 0x0E:
                case 0x0F:
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    switch (MIPS_RS(opcode)) {
                        case 0x08:
                            switch (MIPS_RT(opcode)) {
                                case 0x00:
                                case 0x01:
                                case 0x02:
                                case 0x03:
                                    flag = 0x457;
                                    break;
                            }
                            break;
                    }
                    break;
            }
            break;
        case 0x11: // cop1
            if (MIPS_RS(opcode) == 0x08) {
                switch (MIPS_RT(opcode)) {
                    case 0x00: // bc1f
                    case 0x01: // bc1t
                    case 0x02: // bc1fl
                    case 0x03: // bc1tl
                        flag = 0x457;
                        break;
                }
            }
            break;
        case 0x02: // j
            flag = 0xD05;
            break;
        case 0x03: // jal
            flag = 0x8AE;
            break;
        case 0x04: // beq
        case 0x05: // bne
        case 0x06: // blez
        case 0x07: // bgtz
        case 0x14: // beql
        case 0x15: // bnel
        case 0x16: // blezl
        case 0x17: // bgtzl
            flag = 0x457;
            break;
    }

    return flag;
}

/**
 * @brief Filles a code section of NOPs
 *
 * @param anCode Pointer to fill nops to.
 * @param iCode Position in @code to start filling.
 * @param number The amount of NOPs to fill.
 */
static void cpuCompileNOP(s32* anCode, s32* iCode, s32 number) {
    if (anCode == NULL) {
        *iCode += number;
        return;
    }

    while (number-- != 0) {
        anCode[(*iCode)++] = 0x60000000;
    }
}

#if VERSION >= MK64_J
#pragma optimization_level 1
#endif

#define EMIT_PPC(i, instruction)         \
    do {                                 \
        if (anCode != NULL) {            \
            anCode[(i)++] = instruction; \
        } else {                         \
            (i)++;                       \
        }                                \
    } while (0)

static bool fn_8000E734(Cpu* pCPU, s32 nOpcode, s32 nOpcodePrev, s32 nOpcodeNext, s32 nAddress, s32* anCode, s32* iCode,
                        s32* arg8) {
    if (gpSystem->eTypeROM == CLBJ || gpSystem->eTypeROM == CLBE || gpSystem->eTypeROM == CLBP) {
        // Mario Party
        if (nOpcode == 0x8C9F0004 && nOpcodePrev == 0x8C9D0000 && nOpcodeNext == 0x8C900008) {
            pCPU->nFlagCODE |= 2;
        }
    } else if (gpSystem->eTypeROM == NFXJ || gpSystem->eTypeROM == NFXE || gpSystem->eTypeROM == NFXP) {
        // Star Fox 64
        if (nOpcode == 0x8FBF003C && nOpcodePrev == 0x00000000 && nOpcodeNext == 0xAFB20040) {
            pCPU->nFlagCODE |= 2;
        }
    }

    return true;
}

static bool fn_8000E81C(Cpu* pCPU, s32 nOpcode, s32 nOpcodePrev, s32 nOpcodeNext, s32 nAddress, s32* anCode, s32* iCode,
                        s32* arg8) {
    s32 var_r4;

    if (gpSystem->eTypeROM == CLBJ || gpSystem->eTypeROM == CLBE || gpSystem->eTypeROM == CLBP) {
        // Mario Party
        if (nOpcode == 0xAC9F0004 && nOpcodePrev == 0xAC9D0000 && nOpcodeNext == 0xAC900008) {
            EMIT_PPC(*iCode, 0x80A30000 + OFFSETOF(pCPU, nReturnAddrLast));
            var_r4 = ganMapGPR[31];
            *arg8 = var_r4;
            if (var_r4 & 0x100) {
                EMIT_PPC(*iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
            } else {
                EMIT_PPC(*iCode, 0x7CA02B78 | (*arg8 << 16));
            }
            pCPU->nFlagCODE |= 2;
        }
    } else if (gpSystem->eTypeROM == NFXJ || gpSystem->eTypeROM == NFXE || gpSystem->eTypeROM == NFXP) {
        // Star Fox 64
        if (nOpcode == 0xAFBF003C && nOpcodePrev == 0x0080A025 && nOpcodeNext == 0xAFB00018) {
            EMIT_PPC(*iCode, 0x80A30000 + OFFSETOF(pCPU, nReturnAddrLast));
            var_r4 = ganMapGPR[31];
            *arg8 = var_r4;
            if (var_r4 & 0x100) {
                EMIT_PPC(*iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
            } else {
                EMIT_PPC(*iCode, 0x7CA02B78 | (*arg8 << 16));
            }
            pCPU->nFlagCODE |= 2;
        }
    }

    return true;
}

//! TODO: fake
#if VERSION < MK64_J 
#pragma optimization_level 1
#endif

/**
 * @brief The main MIPS->PPC Dynamic recompiler.
 * @param pCPU The emulated VR4300.
 * @param pnAddress The address to recompile.
 * @param pFunction The function that is being recompiled.
 * @param anCode Pointer to the recompiled code.
 * @param piCode Pointer to the current position in the recompiled code.
 * @param bSlot true if we are recompiling a delay slot.
 * @return bool true on success, false otherwise.
 */
static bool cpuGetPPC(Cpu* pCPU, s32* pnAddress, CpuFunction* pFunction, s32* anCode, s32* piCode, bool bSlot) {
    // pCPU: r25
    // pnAddress: r26
    // pFunction: r27
    // anCode: r28
    // piCode: r29
    // bSlot: r30
    s32 nSize; // r1+0x18
    s32 iHack; // r3
    bool bInterpret; // r23
    s32 iCode; // r1+0x14
    bool var_r17; // r23
    s32 iJump; // r18
    s32 nAddress; // r22
    s32 nReturnAddress;
    s32 nDeltaAddress; // r21
    bool bFlag; // r20
    s32 nAddressJump; // r19
    s32 nOffset; // r1+0x10
    u32 nOpcode; // r18
    u32 nOpcodePrev; // r17
    u32 nOpcodeNext;
    u32* pnOpcode; // r1+0xc
    s32 prev; // r15
    s32 iRegisterA; // r1+0x8
    s32 iRegisterB; // r15
    s32 iRegisterC; // r16
    s32 nTemp1; // r16
    s32 nTemp2; // r17
    s32 nTemp3;
    bool update; // r31
    s32 reg;
    s32 iUpdate; // r14
    s32 nTarget; // r15

    bFlag = true;
    prev = 0;
    update = false;

    if (cpuGetAddressBuffer(pCPU, (void**)&pnOpcode, *pnAddress)) {
        bInterpret = false;

        nOpcode = pnOpcode[0];
        nOpcodeNext = pnOpcode[1];
        nOpcodePrev = pnOpcode[-1];

        nAddress = *pnAddress;
        *pnAddress += 4;

        for (iHack = 0; iHack < pCPU->nCountCodeHack; iHack++) {
            if (pCPU->aCodeHack[iHack].nAddress == nAddress && pCPU->aCodeHack[iHack].nOpcodeOld == nOpcode) {
                if (pCPU->aCodeHack[iHack].nOpcodeNew == 0xFFFFFFFF) {
                    bInterpret = true;
                } else {
                    nOpcode = pCPU->aCodeHack[iHack].nOpcodeNew;
                }
            }
            if (pCPU->aCodeHack[iHack].nAddress == nAddress + 4 && pCPU->aCodeHack[iHack].nOpcodeOld == nOpcodeNext) {
                if (pCPU->aCodeHack[iHack].nOpcodeNew != 0xFFFFFFFF) {
                    nOpcodeNext = pCPU->aCodeHack[iHack].nOpcodeNew;
                }
            }

            if (pCPU->aCodeHack[iHack].nAddress == nAddress - 4 && pCPU->aCodeHack[iHack].nOpcodeOld == nOpcodePrev) {
                if (pCPU->aCodeHack[iHack].nOpcodeNew != 0xFFFFFFFF) {
                    nOpcodePrev = pCPU->aCodeHack[iHack].nOpcodeNew;
                }
            }
        }

        iCode = *piCode;
        nAddressJump = -1;
        pCPU->nOptimize.addr_check = 0;
        if (iCode == 0) {
            pCPU->nOptimize.addr_last = -1;
            if (!bInterpret && cpuCheckDelaySlot(nOpcode) == 0) {
                if (((s32)(pFunction->nAddress1 - pFunction->nAddress0) >> 2) + 1 > 25) {
                    bInterpret = true;
                } else {
                    update = true;
                }
            }
            if (anCode != NULL) {
                anCode[iCode++] = 0x3CA00000 | ((u32)pFunction >> 16);
                anCode[iCode++] = 0x60A50000 | ((u32)pFunction & 0xFFFF);
                anCode[iCode++] = 0x90A30000 + (OFFSETOF(pCPU, pFunctionLast) & 0xFFFF);
                anCode[iCode++] = 0x80C50000 + OFFSETOF(pFunction, timeToLive);
                anCode[iCode++] = 0x2C060000;
                anCode[iCode++] = 0x41820008;
                anCode[iCode++] = 0x90850000 + OFFSETOF(pFunction, timeToLive);
            } else {
                iCode += 7;
            }
            if (update) {
                if (anCode != NULL) {
                    anCode[iCode++] = 0x80A30000 + OFFSETOF(pCPU, nRetrace);
                    anCode[iCode++] = 0x80C30000 + OFFSETOF(pCPU, nRetraceUsed);
                    anCode[iCode++] = 0x7CA62850;
                    anCode[iCode++] = 0x2C050002;
                    anCode[iCode++] = 0x41800014;
                    anCode[iCode++] = 0x3CA00000 | ((u32)nAddress >> 16);
                    anCode[iCode++] = 0x60A50000 | ((u32)nAddress & 0xFFFF);
                    anCode[iCode++] = 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1;
                    iUpdate = iCode++;
                } else {
                    iCode += 9;
                }
            }
            *piCode = iCode;
        } else {
            prev = cpuCheckDelaySlot(nOpcodePrev);
        }

        var_r17 = nOpcode != 0 && (prev != 0 || bSlot) ? true : false;
        if (!bInterpret) {
            if (anCode == NULL || var_r17) {
                pCPU->nOptimize.destGPR_check = 0;
                pCPU->nOptimize.destFPR_check = 0;
                pCPU->nOptimize.addr_last = -1;
                pCPU->nOptimize.checkNext = 0;
            }

            switch (MIPS_OP(nOpcode)) {
                case 0x00: // special
                    switch (MIPS_FUNCT(nOpcode)) {
                        case 0x00: // sll
                            if (nOpcode != 0) {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                                if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                    iRegisterA = 5;
                                }
                                if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    iRegisterB = 6;
                                    if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                        EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                            (iRegisterB << 16) |
                                                            (pCPU->nOptimize.destGPR_mapping << 11));
                                    } else {
                                        EMIT_PPC(iCode,
                                                 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                    }
                                }
                                nTemp1 = MIPS_SA(nOpcode);
                                EMIT_PPC(iCode, 0x38E00000 | (nTemp1 & 0xFFFF));
                                EMIT_PPC(iCode, 0x7C000030 | (iRegisterB << 21) | (iRegisterA << 16) | (7 << 11));
                                if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                    pCPU->nOptimize.destGPR_check = 2;
                                    pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                    pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    EMIT_PPC(iCode,
                                             0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                                }
                            } else {
                                EMIT_PPC(iCode, 0x60000000);
                            }
                            break;
                        case 0x02: // srl
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            nTemp2 = MIPS_SA(nOpcode);
                            EMIT_PPC(iCode, 0x38E00000 | (nTemp2 & 0xFFFF));
                            EMIT_PPC(iCode, 0x7C000430 | (iRegisterB << 21) | (iRegisterA << 16) | (7 << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x03: // sra
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            nTemp1 = MIPS_SA(nOpcode);
                            EMIT_PPC(iCode, 0x7C000670 | (iRegisterB << 21) | (iRegisterA << 16) | (nTemp1 << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x04: // sllv
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x70000000 | (iRegisterC << 21) | (iRegisterC << 16) | 0x1F);
                            EMIT_PPC(iCode, 0x7C000030 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x06: // srlv
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x70000000 | (iRegisterC << 21) | (iRegisterC << 16) | 0x1F);
                            EMIT_PPC(iCode, 0x7C000430 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x07: // srav
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x70000000 | (iRegisterC << 21) | (iRegisterC << 16) | 0x1F);
                            EMIT_PPC(iCode, 0x7C000630 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x08: // jr
                            if (nOpcodeNext != 0) {
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                    return false;
                                }
                                *pnAddress -= 4;
                            }
                            if (MIPS_RS(nOpcode) != 31) {
                                pCPU->nOptimize.validCheck = 0;
                            }
                            if (MIPS_RS(nOpcode) == 31 && !(pCPU->nFlagCODE & 2)) {
                                if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                    iRegisterA = 5;
                                    EMIT_PPC(iCode,
                                             0x80A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                                EMIT_PPC(iCode, 0x7C0803A6 | ((iRegisterA & 0x1F) << 21));
                                EMIT_PPC(iCode, 0x4E800020);
                            } else {
                                if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                    EMIT_PPC(iCode,
                                             0x80A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                } else {
                                    EMIT_PPC(iCode, 0x7C050378 | (iRegisterA << 21) | (iRegisterA << 11));
                                }
                                EMIT_PPC(iCode,
                                         0x48000000 | (((u32)pCPU->pfJump - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            }
                            break;
                        case 0x09: // jalr
                            if (nOpcodeNext != 0) {
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                    return false;
                                }
                                *pnAddress -= 4;
                            }
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, pFunctionLast));
                            EMIT_PPC(iCode, 0x3CE08000);
                            EMIT_PPC(iCode, 0x80C50000 + OFFSETOF(pFunction, timeToLive));
                            EMIT_PPC(iCode, 0x7CC63B78);
                            EMIT_PPC(iCode, 0x3CE00000 | (((u32)nAddress + 8) >> 16));
                            EMIT_PPC(iCode, 0x90C50000 + OFFSETOF(pFunction, timeToLive));
                            EMIT_PPC(iCode, 0x60E70000 | (((u32)nAddress + 8) & 0xFFFF));
                            EMIT_PPC(iCode, 0x90E30000 + OFFSETOF(pCPU, nReturnAddrLast));
                            if ((iRegisterA = ganMapGPR[31]) & 0x100) {
                                nTemp1 = (u32)&anCode[iCode] + 20;
                                EMIT_PPC(iCode, 0x3CA00000 | ((u32)nTemp1 >> 16));
                                EMIT_PPC(iCode, 0x60A50000 | ((u32)nTemp1 & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
                            } else {
                                nTemp1 = (u32)&anCode[iCode] + 16;
                                EMIT_PPC(iCode, 0x3C000000 | ((u32)nTemp1 >> 16) | (iRegisterA << 21));
                                EMIT_PPC(iCode,
                                         0x60000000 | ((u32)nTemp1 & 0xFFFF) | (iRegisterA << 21) | (iRegisterA << 16));
                            }
                            if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, 0x80A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            } else {
                                EMIT_PPC(iCode, 0x7C050378 | (iRegisterA << 21) | (iRegisterA << 11));
                            }
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfJump - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            if (anCode != NULL) {
                                anCode[iCode++] = 0x3CA00000 | ((u32)pFunction >> 16);
                                anCode[iCode++] = 0x60A50000 | ((u32)pFunction & 0xFFFF);
                                anCode[iCode++] = 0x90A30000 + (OFFSETOF(pCPU, pFunctionLast) & 0xFFFF);
                                anCode[iCode++] = 0x3CE08000;
                                anCode[iCode++] = 0x80C50000 + OFFSETOF(pFunction, timeToLive);
                                anCode[iCode++] = 0x7CC63878;
                                anCode[iCode++] = 0x2C060000;
                                anCode[iCode++] = 0x41820008;
                                anCode[iCode++] = 0x90850000 + OFFSETOF(pFunction, timeToLive);
                            } else {
                                iCode += 9;
                            }
                            iJump = iCode++;
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (iCode - iJump) * 4;
                            }
                            EMIT_PPC(iJump, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            break;
                        case 0x0C: // syscall
                        case 0x0D: // break
                        case 0x0F: // sync
                            break;
                        case 0x10: // mfhi
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, 0x80A30000 | ((OFFSETOF(pCPU, nHi) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, nHi) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) & 0xFFFF));
                            } else {
                                EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, nHi) + 4));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, nHi) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) & 0xFFFF));
                            }
                            break;
                        case 0x11: // mthi
                            if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, 0x80A30000 | ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nHi) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, nHi) & 0xFFFF));
                            } else {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, nHi) + 4));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, nHi) & 0xFFFF));
                            }
                            break;
                        case 0x12: // mflo
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, 0x80A30000 | ((OFFSETOF(pCPU, nLo) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, nLo) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) & 0xFFFF));
                            } else {
                                EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, nLo) + 4));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, nLo) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) & 0xFFFF));
                            }
                            break;
                        case 0x13: // mtlo
                            if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, 0x80A30000 | ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nLo) + 4) & 0xFFFF));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, nLo) & 0xFFFF));
                            } else {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, nLo) + 4));
                                EMIT_PPC(iCode, 0x80A30000 | (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x90A30000 | (OFFSETOF(pCPU, nLo) & 0xFFFF));
                            }
                            break;
                        case 0x14: // dsllv
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            iRegisterC = MIPS_RS(nOpcode);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DSLLV_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x16: // dsrlv
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            iRegisterC = MIPS_RS(nOpcode);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DSRLV_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x17: // dsrav
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            iRegisterC = MIPS_RS(nOpcode);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DSRAV_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x18: // mult
                            iRegisterA = 5;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                            EMIT_PPC(iCode, 0x7C0001D6 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nLo) + 4) & 0xFFFF));
                            EMIT_PPC(iCode, 0x7C000096 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nHi) + 4) & 0xFFFF));
                            break;
                        case 0x19: // multu
                            iRegisterA = 5;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                            EMIT_PPC(iCode, 0x7C0001D6 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nLo) + 4) & 0xFFFF));
                            EMIT_PPC(iCode, 0x7C000016 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nHi) + 4) & 0xFFFF));
                            break;
                        case 0x1A: // div
                            iRegisterA = 5;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                            EMIT_PPC(iCode, 0x7C0003D6 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nLo) + 4) & 0xFFFF));
                            EMIT_PPC(iCode, 0x7C0001D6 | (iRegisterA << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x7C000050 | (iRegisterA << 21) | (iRegisterA << 16) | (iRegisterB << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nHi) + 4) & 0xFFFF));
                            break;
                        case 0x1B: // divu
                            iRegisterA = 5;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                            EMIT_PPC(iCode, 0x7C000396 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nLo) + 4) & 0xFFFF));
                            EMIT_PPC(iCode, 0x7C0001D6 | (iRegisterA << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x7C000050 | (iRegisterA << 21) | (iRegisterA << 16) | (iRegisterB << 11));
                            EMIT_PPC(iCode, 0x90A30000 | ((OFFSETOF(pCPU, nHi) + 4) & 0xFFFF));
                            break;
                        case 0x1C: // dmult
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFC0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x91210010);
                            EMIT_PPC(iCode, 0x91410018);
                            EMIT_PPC(iCode, 0x91610020);
                            EMIT_PPC(iCode, 0x91810028);
                            EMIT_PPC(iCode, 0x91C10030);
                            EMIT_PPC(iCode, 0x91E10038);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81030004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x81E30000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DMULT_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x81210010);
                            EMIT_PPC(iCode, 0x81410018);
                            EMIT_PPC(iCode, 0x81610020);
                            EMIT_PPC(iCode, 0x81810028);
                            EMIT_PPC(iCode, 0x81C10030);
                            EMIT_PPC(iCode, 0x81E10038);
                            EMIT_PPC(iCode, 0x38210040);
                            break;
                        case 0x1D: // dmultu
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFD0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x91210010);
                            EMIT_PPC(iCode, 0x91410018);
                            EMIT_PPC(iCode, 0x91610020);
                            EMIT_PPC(iCode, 0x91810028);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81030004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x48000000 |
                                                ((cpuCompile_DMULTU_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x81210010);
                            EMIT_PPC(iCode, 0x81410018);
                            EMIT_PPC(iCode, 0x81610020);
                            EMIT_PPC(iCode, 0x81810028);
                            EMIT_PPC(iCode, 0x38210030);
                            break;
                        case 0x1E: // ddiv
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFC0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x91210010);
                            EMIT_PPC(iCode, 0x91410018);
                            EMIT_PPC(iCode, 0x91610020);
                            EMIT_PPC(iCode, 0x91810028);
                            EMIT_PPC(iCode, 0x91C10030);
                            EMIT_PPC(iCode, 0x91E10038);
                            EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x81230004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x81430000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81630004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x81E30000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DDIV_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x81210010);
                            EMIT_PPC(iCode, 0x81410018);
                            EMIT_PPC(iCode, 0x81610020);
                            EMIT_PPC(iCode, 0x81810028);
                            EMIT_PPC(iCode, 0x81C10030);
                            EMIT_PPC(iCode, 0x81E10038);
                            EMIT_PPC(iCode, 0x38210040);
                            break;
                        case 0x1F: // ddivu
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFD0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x91210010);
                            EMIT_PPC(iCode, 0x91410018);
                            EMIT_PPC(iCode, 0x91610020);
                            EMIT_PPC(iCode, 0x91810028);
                            EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x81230004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x81430000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81630004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DDIVU_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x81210010);
                            EMIT_PPC(iCode, 0x81410018);
                            EMIT_PPC(iCode, 0x81610020);
                            EMIT_PPC(iCode, 0x81810028);
                            EMIT_PPC(iCode, 0x38210030);
                            break;
                        case 0x20: { // add
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }

                            nTemp2 = nTemp1 = false;
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                                nTemp2 = true;
                            }

                            if (MIPS_RS(nOpcode) == 0) {
                                if (MIPS_RT(nOpcode) == 31 && !(pCPU->nFlagCODE & 4)) {
                                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                                    EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                                    nTemp1 = true;
                                    pCPU->nFlagCODE |= 8;
                                } else if (!nTemp2) {
                                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                    } else {
                                        EMIT_PPC(iCode, 0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) |
                                                            (iRegisterB << 11));
                                    }
                                } else {
                                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                        pCPU->nOptimize.destGPR_check = 2;
                                        pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    } else {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                    }
                                }

                                if (MIPS_RD(nOpcode) == 31) {
                                    pCPU->nFlagCODE |= 2;
                                    if (pCPU->nFlagCODE & 8) {
                                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                            iRegisterB = 6;
                                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) &
                                                                          0xFFFF));
                                        }
                                        EMIT_PPC(iCode,
                                                 (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                                    }
                                }
                            } else if (MIPS_RT(nOpcode) == 0) {
                                if ((MIPS_RS(nOpcode) == 31) && !(pCPU->nFlagCODE & 4)) {
                                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                                    EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                                    nTemp1 = true;
                                    pCPU->nFlagCODE |= 8;
                                } else if (!nTemp2) {
                                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                    } else {
                                        EMIT_PPC(iCode, 0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) |
                                                            (iRegisterB << 11));
                                    }
                                } else {
                                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                        pCPU->nOptimize.destGPR_check = 2;
                                        pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    } else {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                    }
                                }

                                if (MIPS_RD(nOpcode) == 31) {
                                    pCPU->nFlagCODE |= 2;
                                    if (pCPU->nFlagCODE & 8) {
                                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                            iRegisterB = 6;
                                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) &
                                                                          0xFFFF));
                                        }
                                        EMIT_PPC(iCode,
                                                 (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                                    }
                                }
                            } else {
                                if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                    iRegisterB = 6;
                                    if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                        EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                            (iRegisterB << 16) |
                                                            (pCPU->nOptimize.destGPR_mapping << 11));
                                    } else {
                                        EMIT_PPC(iCode,
                                                 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                    }
                                }
                                if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    iRegisterC = 7;
                                    if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                        EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                            (iRegisterC << 16) |
                                                            (pCPU->nOptimize.destGPR_mapping << 11));
                                    } else {
                                        EMIT_PPC(iCode,
                                                 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                    }
                                }
                                EMIT_PPC(iCode,
                                         0x7C000214 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                                nTemp1 = true;
                            }
                            if (nTemp1 && (ganMapGPR[MIPS_RD(nOpcode)] & 0x100)) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        }
                        case 0x21: { // addu
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }

                            nTemp2 = nTemp1 = false;
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                                nTemp2 = true;
                            }

                            if (MIPS_RS(nOpcode) == 0) {
                                if (MIPS_RT(nOpcode) == 31 && !(pCPU->nFlagCODE & 4)) {
                                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                                    EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                                    nTemp1 = true;
                                    pCPU->nFlagCODE |= 8;
                                } else if (!nTemp2) {
                                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                    } else {
                                        EMIT_PPC(iCode, 0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) |
                                                            (iRegisterB << 11));
                                    }
                                } else {
                                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                        pCPU->nOptimize.destGPR_check = 2;
                                        pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    } else {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                    }
                                }

                                if (MIPS_RD(nOpcode) == 31) {
                                    pCPU->nFlagCODE |= 2;
                                    if (pCPU->nFlagCODE & 8) {
                                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                            iRegisterB = 6;
                                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) &
                                                                          0xFFFF));
                                        }
                                        EMIT_PPC(iCode,
                                                 (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                                    }
                                }
                            } else if (MIPS_RT(nOpcode) == 0) {
                                if ((MIPS_RS(nOpcode) == 31) && !(pCPU->nFlagCODE & 4)) {
                                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                                    EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                                    nTemp1 = true;
                                    pCPU->nFlagCODE |= 8;
                                } else if (!nTemp2) {
                                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                    } else {
                                        EMIT_PPC(iCode, 0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) |
                                                            (iRegisterB << 11));
                                    }
                                } else {
                                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                        pCPU->nOptimize.destGPR_check = 2;
                                        pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    } else {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                    }
                                }

                                if (MIPS_RD(nOpcode) == 31) {
                                    pCPU->nFlagCODE |= 2;
                                    if (pCPU->nFlagCODE & 8) {
                                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                            iRegisterB = 6;
                                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) &
                                                                          0xFFFF));
                                        }
                                        EMIT_PPC(iCode,
                                                 (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                                    }
                                }
                            } else {
                                if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                    iRegisterB = 6;
                                    if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                        EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                            (iRegisterB << 16) |
                                                            (pCPU->nOptimize.destGPR_mapping << 11));
                                    } else {
                                        EMIT_PPC(iCode,
                                                 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                    }
                                }
                                if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    iRegisterC = 7;
                                    if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                        EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                            (iRegisterC << 16) |
                                                            (pCPU->nOptimize.destGPR_mapping << 11));
                                    } else {
                                        EMIT_PPC(iCode,
                                                 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                    }
                                }
                                EMIT_PPC(iCode,
                                         0x7C000214 | (iRegisterA << 21) | (iRegisterB << 16) | (iRegisterC << 11));
                                nTemp1 = true;
                            }
                            if (nTemp1 && (ganMapGPR[MIPS_RD(nOpcode)] & 0x100)) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        }
                        case 0x22: // sub
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x7C000050 | (iRegisterA << 21) | (iRegisterC << 16) | (iRegisterB << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x23: // subu
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x7C000050 | (iRegisterA << 21) | (iRegisterC << 16) | (iRegisterB << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x24: // and
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x7C000038 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x25: { // or
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }

                            nTemp2 = nTemp1 = false;
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                                nTemp2 = true;
                            }

                            if (MIPS_RS(nOpcode) == 0) {
                                if (MIPS_RT(nOpcode) == 31 && !(pCPU->nFlagCODE & 4)) {
                                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                                    EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                                    nTemp1 = true;
                                    pCPU->nFlagCODE |= 8;
                                } else if (!nTemp2) {
                                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                    } else {
                                        EMIT_PPC(iCode, 0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) |
                                                            (iRegisterB << 11));
                                    }
                                } else {
                                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                        pCPU->nOptimize.destGPR_check = 2;
                                        pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    } else {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                    }
                                }

                                if (MIPS_RD(nOpcode) == 31) {
                                    pCPU->nFlagCODE |= 2;
                                    if (pCPU->nFlagCODE & 8) {
                                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                            iRegisterB = 6;
                                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) &
                                                                          0xFFFF));
                                        }
                                        EMIT_PPC(iCode,
                                                 (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                                    }
                                }
                            } else if (MIPS_RT(nOpcode) == 0) {
                                if ((MIPS_RS(nOpcode) == 31) && !(pCPU->nFlagCODE & 4)) {
                                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                                    EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                                    nTemp1 = true;
                                    pCPU->nFlagCODE |= 8;
                                } else if (!nTemp2) {
                                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                    } else {
                                        EMIT_PPC(iCode, 0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) |
                                                            (iRegisterB << 11));
                                    }
                                } else {
                                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                        EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                        pCPU->nOptimize.destGPR_check = 2;
                                        pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    } else {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                            (OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4));
                                    }
                                }

                                if (MIPS_RD(nOpcode) == 31) {
                                    pCPU->nFlagCODE |= 2;
                                    if (pCPU->nFlagCODE & 8) {
                                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                            iRegisterB = 6;
                                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) &
                                                                          0xFFFF));
                                        }
                                        EMIT_PPC(iCode,
                                                 (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                                    }
                                }
                            } else {
                                if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                    iRegisterB = 6;
                                    if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                        EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                            (iRegisterB << 16) |
                                                            (pCPU->nOptimize.destGPR_mapping << 11));
                                    } else {
                                        EMIT_PPC(iCode,
                                                 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                    }
                                }
                                if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    iRegisterC = 7;
                                    if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                        EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                            (iRegisterC << 16) |
                                                            (pCPU->nOptimize.destGPR_mapping << 11));
                                    } else {
                                        EMIT_PPC(iCode,
                                                 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                    }
                                }
                                EMIT_PPC(iCode,
                                         0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                                nTemp1 = true;
                            }
                            if (nTemp1 && (ganMapGPR[MIPS_RD(nOpcode)] & 0x100)) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        }
                        case 0x26: // xor
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x7C000278 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x27: // nor
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x7C0000F8 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterC << 11));
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x2A: // slt
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x7C000000 | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x4180000C);
                            EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21));
                            EMIT_PPC(iCode, 0x42800008);
                            EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | 1);
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x2B: // sltu
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if ((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100) {
                                iRegisterA = 5;
                            }
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            if ((iRegisterC = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                iRegisterC = 7;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterC << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x7C000040 | (iRegisterB << 16) | (iRegisterC << 11));
                            EMIT_PPC(iCode, 0x4180000C);
                            EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21));
                            EMIT_PPC(iCode, 0x42800008);
                            EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | 1);
                            if (ganMapGPR[MIPS_RD(nOpcode)] & 0x100) {
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RD(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RD(nOpcode)]) + 4) & 0xFFFF));
                            }
                            break;
                        case 0x2C: // dadd
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFF0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DADD_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x90A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x38210010);
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x2D: // daddu
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFF0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DADDU_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x90A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x38210010);
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x2E: // dsub
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFF0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DSUB_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x90A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x38210010);
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x2F: // dsubu
                            if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode)) || pCPU->nFlagRAM & (1 << MIPS_RT(nOpcode))) {
                                pCPU->nFlagRAM |= (1 << MIPS_RD(nOpcode));
                            } else {
                                pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            }
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            if (!((iRegisterC = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterC << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RS(nOpcode);
                            iRegisterC = MIPS_RT(nOpcode);
                            EMIT_PPC(iCode, 0x9421FFF0);
                            EMIT_PPC(iCode, 0x91010008);
                            EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80E30004 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aGPR[iRegisterC]));
                            EMIT_PPC(iCode,
                                     0x48000000 | ((cpuCompile_DSUBU_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x90A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x81010008);
                            EMIT_PPC(iCode, 0x38210010);
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                            }
                            break;
                        case 0x30: // tge
                        case 0x31: // tgeu
                        case 0x32: // tlt
                        case 0x33: // tltu
                        case 0x34: // teq
                        case 0x36: // tne
                            break;
                        case 0x38: // dsll
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            nTemp1 = MIPS_SA(nOpcode);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x54A50000 | (nTemp1 << 11) | ((31 - nTemp1) << 1));
                            EMIT_PPC(iCode, 0x50C50000 | (nTemp1 << 11) | ((32 - nTemp1) << 6) | (31 << 1));
                            EMIT_PPC(iCode, 0x54C60000 | (nTemp1 << 11) | ((31 - nTemp1) << 1));
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x3A: // dsrl
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            nTemp1 = MIPS_SA(nOpcode);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x54C60000 | ((32 - nTemp1) << 11) | (nTemp1 << 6) | (31 << 1));
                            EMIT_PPC(iCode, 0x50A60000 | ((32 - nTemp1) << 11) | ((nTemp1 - 1) << 1));
                            EMIT_PPC(iCode, 0x54A50000 | ((32 - nTemp1) << 11) | (nTemp1 << 6) | (31 << 1));
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x3B: // dsra
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            nTemp1 = MIPS_SA(nOpcode);
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x54C60000 | ((32 - nTemp1) << 11) | (nTemp1 << 6) | (31 << 1));
                            EMIT_PPC(iCode, 0x50A60000 | ((32 - nTemp1) << 11) | ((nTemp1 - 1) << 1));
                            EMIT_PPC(iCode, 0x7CA50670 | (nTemp1 << 11));
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x3C: // dsll32
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            nTemp1 = MIPS_SA(nOpcode) + 32;
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            while (nTemp1 > 0) {
                                if (nTemp1 > 31) {
                                    nTemp2 = 31;
                                    nTemp1 -= 31;
                                } else {
                                    nTemp2 = nTemp1;
                                    nTemp1 = 0;
                                }
                                EMIT_PPC(iCode, 0x54A50000 | (nTemp2 << 11) | ((31 - nTemp2) << 1));
                                EMIT_PPC(iCode, 0x50C50000 | (nTemp2 << 11) | ((32 - nTemp2) << 6) | (31 << 1));
                                EMIT_PPC(iCode, 0x54C60000 | (nTemp2 << 11) | ((31 - nTemp2) << 1));
                            }
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x3E: // dsrl32
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            nTemp1 = MIPS_SA(nOpcode) + 32;
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            while (nTemp1 > 0) {
                                if (nTemp1 > 31) {
                                    nTemp2 = 31;
                                    nTemp1 -= 31;
                                } else {
                                    nTemp2 = nTemp1;
                                    nTemp1 = 0;
                                }
                                EMIT_PPC(iCode, 0x54C60000 | ((32 - nTemp2) << 11) | (nTemp2 << 6) | (31 << 1));
                                EMIT_PPC(iCode, 0x50A60000 | ((32 - nTemp2) << 11) | ((nTemp2 - 1) << 1));
                                EMIT_PPC(iCode, 0x54A50000 | ((32 - nTemp2) << 11) | (nTemp2 << 6) | (31 << 1));
                            }
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        case 0x3F: // dsra32
                            pCPU->nFlagRAM &= ~(1 << MIPS_RD(nOpcode));
                            if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                            iRegisterA = MIPS_RD(nOpcode);
                            iRegisterB = MIPS_RT(nOpcode);
                            nTemp1 = MIPS_SA(nOpcode) + 32;
                            EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            EMIT_PPC(iCode, 0x80C30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                            while (nTemp1 > 0) {
                                if (nTemp1 > 31) {
                                    nTemp2 = 31;
                                    nTemp1 -= 31;
                                } else {
                                    nTemp2 = nTemp1;
                                    nTemp1 = 0;
                                }
                                EMIT_PPC(iCode, 0x54C60000 | ((32 - nTemp2) << 11) | (nTemp2 << 6) | (31 << 1));
                                EMIT_PPC(iCode, 0x50A60000 | ((32 - nTemp2) << 11) | ((nTemp2 - 1) << 1));
                                EMIT_PPC(iCode, 0x7CA50670 | (nTemp2 << 11));
                            }
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            EMIT_PPC(iCode, 0x90C30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                            if (!((iRegisterA = ganMapGPR[MIPS_RD(nOpcode)]) & 0x100)) {
                                EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            }
                            break;
                        default:
                            bFlag = false;
                            break;
                    }
                    break;
                case 0x01: // regimm
                    switch (MIPS_RT(nOpcode)) {
                        case 0x00: // bltz
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterA = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));

                            iJump = iCode++;
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            EMIT_PPC(iJump, 0x40800000 | (((iCode - iJump) & 0x3FFF) << 2));
                            break;
                        case 0x01: // bgez
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterA = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));

                            iJump = iCode++;
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            EMIT_PPC(iJump, 0x41800000 | (((iCode - iJump) & 0x3FFF) << 2));
                            break;
                        case 0x02: // bltzl
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterA = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));

                            iJump = iCode++;
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                                return false;
                            }
                            EMIT_PPC(iJump, 0x40800000 | (((iCode - iJump) & 0x3FFF) << 2));
                            break;
                        case 0x03: // bgezl
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterA = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));

                            iJump = iCode++;
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                                return false;
                            }
                            EMIT_PPC(iJump, 0x41800000 | (((iCode - iJump) & 0x3FFF) << 2));
                            break;
                        case 0x08: // tgei
                        case 0x09: // tgeiu
                        case 0x0A: // tlti
                        case 0x0B: // tltiu
                        case 0x0C: // teqi
                        case 0x0E: // tnei
                            break;
                        case 0x10: // bltzal
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterB << 16));

                            iJump = iCode++;
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)(nAddress + 8) >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)(nAddress + 8) & 0xFFFF));
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, nReturnAddrLast));
                            if (ganMapGPR[31] & 0x100) {
                                iCode += 3;
                            } else {
                                iCode += 2;
                            }
                            nTemp1 = (u32)&anCode[iCode];
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            nTemp2 = (u32)&anCode[iCode];
                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            EMIT_PPC(iJump, 0x40800000 | (((iCode - iJump) * 4) & 0xFFFC));

                            iJump += 3;
                            nTarget = (u32)&anCode[iCode] + (nTemp2 - nTemp1);
                            if ((iRegisterA = ganMapGPR[31]) & 0x100) {
                                EMIT_PPC(iJump, 0x3CA00000 | ((u32)nTarget >> 16));
                                EMIT_PPC(iJump, 0x60A50000 | ((u32)nTarget & 0xFFFF));
                                EMIT_PPC(iJump, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
                            } else {
                                EMIT_PPC(iJump, 0x3C000000 | ((u32)nTarget >> 16) | (iRegisterA << 21));
                                EMIT_PPC(iJump, 0x60000000 | ((u32)nTarget & 0xFFFF) | (iRegisterA << 21) |
                                                    (iRegisterA << 16));
                            }
                            break;
                        case 0x11: // bgezal
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterB << 16));

                            iJump = iCode++;
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)(nAddress + 8) >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)(nAddress + 8) & 0xFFFF));
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, nReturnAddrLast));
                            if (ganMapGPR[31] & 0x100) {
                                iCode += 3;
                            } else {
                                iCode += 2;
                            }
                            nTemp1 = (u32)&anCode[iCode];
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            nTemp2 = (u32)&anCode[iCode];
                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            EMIT_PPC(iJump, 0x41800000 | (((iCode - iJump) * 4) & 0xFFFC));

                            iJump += 3;
                            nTarget = (u32)&anCode[iCode] + (nTemp2 - nTemp1);
                            if ((iRegisterA = ganMapGPR[31]) & 0x100) {
                                EMIT_PPC(iJump, 0x3CA00000 | ((u32)nTarget >> 16));
                                EMIT_PPC(iJump, 0x60A50000 | ((u32)nTarget & 0xFFFF));
                                EMIT_PPC(iJump, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
                            } else {
                                EMIT_PPC(iJump, 0x3C000000 | ((u32)nTarget >> 16) | (iRegisterA << 21));
                                EMIT_PPC(iJump, 0x60000000 | ((u32)nTarget & 0xFFFF) | (iRegisterA << 21) |
                                                    (iRegisterA << 16));
                            }
                            break;
                        case 0x12: // bltzall
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterB << 16));

                            iJump = iCode++;
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)(nAddress + 8) >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)(nAddress + 8) & 0xFFFF));
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, nReturnAddrLast));
                            if (ganMapGPR[31] & 0x100) {
                                iCode += 3;
                            } else {
                                iCode += 2;
                            }
                            nTemp1 = (u32)&anCode[iCode];
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            nTemp2 = (u32)&anCode[iCode];
                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                                return false;
                            }
                            EMIT_PPC(iJump, 0x40800000 | (((iCode - iJump) * 4) & 0xFFFC));

                            iJump += 3;
                            nTarget = (u32)&anCode[iCode] + (nTemp2 - nTemp1);
                            if ((iRegisterA = ganMapGPR[31]) & 0x100) {
                                EMIT_PPC(iJump, 0x3CA00000 | ((u32)nTarget >> 16));
                                EMIT_PPC(iJump, 0x60A50000 | ((u32)nTarget & 0xFFFF));
                                EMIT_PPC(iJump, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
                            } else {
                                EMIT_PPC(iJump, 0x3C000000 | ((u32)nTarget >> 16) | (iRegisterA << 21));
                                EMIT_PPC(iJump, 0x60000000 | ((u32)nTarget & 0xFFFF) | (iRegisterA << 21) |
                                                    (iRegisterA << 16));
                            }
                            break;
                        case 0x13: // bgezall
                            nAddressJump = *pnAddress + MIPS_IMM_S16(nOpcode) * 4;
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                iRegisterB = 6;
                                if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                    EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                        (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                                } else {
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                            }
                            EMIT_PPC(iCode, 0x2C000000 | (iRegisterB << 16));

                            iJump = iCode++;
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)(nAddress + 8) >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)(nAddress + 8) & 0xFFFF));
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, nReturnAddrLast));
                            if (ganMapGPR[31] & 0x100) {
                                iCode += 3;
                            } else {
                                iCode += 2;
                            }
                            nTemp1 = (u32)&anCode[iCode];
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;

                            nTemp2 = (u32)&anCode[iCode];
                            if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                return false;
                            }
                            if (anCode != NULL) {
                                nDeltaAddress = (nOffset - iCode) * 4;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                                return false;
                            }
                            EMIT_PPC(iJump, 0x41800000 | (((iCode - iJump) * 4) & 0xFFFC));

                            iJump += 3;
                            nTarget = (u32)&anCode[iCode] + (nTemp2 - nTemp1);
                            if ((iRegisterA = ganMapGPR[31]) & 0x100) {
                                EMIT_PPC(iJump, 0x3CA00000 | ((u32)nTarget >> 16));
                                EMIT_PPC(iJump, 0x60A50000 | ((u32)nTarget & 0xFFFF));
                                EMIT_PPC(iJump, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
                            } else {
                                EMIT_PPC(iJump, 0x3C000000 | ((u32)nTarget >> 16) | (iRegisterA << 21));
                                EMIT_PPC(iJump, 0x60000000 | ((u32)nTarget & 0xFFFF) | (iRegisterA << 21) |
                                                    (iRegisterA << 16));
                            }
                            break;
                        default:
                            bFlag = false;
                            break;
                    }
                    break;
                case 0x02: // j
                    nAddressJump = (*pnAddress & 0xF0000000) | (MIPS_TARGET(nOpcode) << 2);
                    if (pFunction->nAddress0 <= nAddressJump && nAddressJump < pFunction->nAddress1) {
                        if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                            return false;
                        }
                        *pnAddress -= 4;
                        if (nAddressJump == nAddress) {
                            if (anCode != NULL) {
                                anCode[iCode++] = 0x3CA00000 | ((u32)nAddress >> 16);
                                anCode[iCode++] = 0x60A50000 | ((u32)nAddress & 0xFFFF);
                            } else {
                                iCode += 2;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfIdle - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                        if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                            return false;
                        }
                        if (anCode != NULL) {
                            nDeltaAddress = (nOffset - iCode) * 4;
                        }
                        EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    } else {
                        if (nOpcodeNext != 0) {
                            if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                return false;
                            }
                            *pnAddress -= 4;
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddressJump >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddressJump & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfJump - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        nAddressJump = -1;
                    }
                    break;
                case 0x03: // jal
                    nTarget = MIPS_TARGET(nOpcode) << 2;
                    if (nOpcodeNext != 0) {
                        if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                            return false;
                        }
                        *pnAddress -= 4;
                    }
                    EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, pFunctionLast));
                    EMIT_PPC(iCode, 0x3CE08000);
                    EMIT_PPC(iCode, 0x80C50000 + OFFSETOF(pFunction, timeToLive));
                    EMIT_PPC(iCode, 0x7CC63B78);
                    EMIT_PPC(iCode, 0x3CE00000 | ((u32)(nAddress + 8) >> 16));
                    EMIT_PPC(iCode, 0x90C50000 + OFFSETOF(pFunction, timeToLive));
                    EMIT_PPC(iCode, 0x60E70000 | ((u32)(nAddress + 8) & 0xFFFF));
                    EMIT_PPC(iCode, 0x90E30000 + OFFSETOF(pCPU, nReturnAddrLast));
                    nAddressJump = (*pnAddress & 0xF0000000) | nTarget;
                    if ((iRegisterA = ganMapGPR[31]) & 0x100) {
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddressJump >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddressJump & 0xFFFF));
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[31]) + 4) & 0xFFFF));
                    } else {
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddressJump >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddressJump & 0xFFFF));
                    }
                    EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfCall - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                    if (pFunction->callerID_flag == 0x16) {
                        pFunction->block[pFunction->callerID_total].N64address = nAddressJump;
                    }
                    if (pFunction->callerID_flag != 0x21) {
                        pFunction->callerID_total++;
                    }
                    if (anCode != NULL) {
                        anCode[iCode++] = 0x3CA00000 | ((u32)pFunction >> 16);
                        anCode[iCode++] = 0x60A50000 | ((u32)pFunction & 0xFFFF);
                        anCode[iCode++] = 0x90A30000 + (OFFSETOF(pCPU, pFunctionLast) & 0xFFFF);
                        anCode[iCode++] = 0x3CE08000;
                        anCode[iCode++] = 0x80C50000 + OFFSETOF(pFunction, timeToLive);
                        anCode[iCode++] = 0x7CC63878;
                        anCode[iCode++] = 0x2C060000;
                        anCode[iCode++] = 0x41820008;
                        anCode[iCode++] = 0x90850000 + OFFSETOF(pFunction, timeToLive);
                    } else {
                        iCode += 9;
                    }
                    iJump = iCode++;
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                        return false;
                    }
                    if (anCode != NULL) {
                        nDeltaAddress = (iCode - iJump) * 4;
                    }
                    if (anCode != NULL) {
                        anCode[iJump] = 0x48000000 | (nDeltaAddress & 0x03FFFFFC);
                    }
                    nAddressJump = -1;
                    break;
                case 0x04: // beq
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if (MIPS_RS(nOpcode) == MIPS_RT(nOpcode)) {
                        if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                            return false;
                        }
                        *pnAddress -= 4;
                        if (nAddressJump == nAddress) {
                            if (anCode != NULL) {
                                anCode[iCode++] = 0x3CA00000 | ((u32)nAddress >> 16);
                                anCode[iCode++] = 0x60A50000 | ((u32)nAddress & 0xFFFF);
                            } else {
                                iCode += 2;
                            }
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfIdle - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                        if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                            return false;
                        }
                        if (anCode != NULL) {
                            nDeltaAddress = (nOffset - iCode) * 4;
                        }
                        EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    } else {
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterB = 7;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x7C000000 | (iRegisterA << 16) | (iRegisterB << 11));
                        iJump = iCode++;
                        if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                            return false;
                        }
                        *pnAddress -= 4;

                        if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                            return false;
                        }
                        if (anCode != NULL) {
                            nDeltaAddress = (nOffset - iCode) * 4;
                        }
                        EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                        EMIT_PPC(iJump, 0x40820000 | (((iCode - iJump) & 0x3FFF) << 2));
                    }
                    break;
                case 0x05: // bne
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterA = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterA << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterB = 7;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterB << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x7C000000 | (iRegisterA << 16) | (iRegisterB << 11));
                    iJump = iCode++;
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                        return false;
                    }
                    *pnAddress -= 4;

                    if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                        return false;
                    }
                    if (anCode != NULL) {
                        nDeltaAddress = (nOffset - iCode) * 4;
                    }
                    EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    EMIT_PPC(iJump, 0x41820000 | (((iCode - iJump) & 0x3FFF) << 2));
                    break;
                case 0x06: // blez
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterA = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterA << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));
                    iJump = iCode++;
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                        return false;
                    }
                    *pnAddress -= 4;

                    if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                        return false;
                    }
                    if (anCode != NULL) {
                        nDeltaAddress = (nOffset - iCode) * 4;
                    }
                    EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    EMIT_PPC(iJump, 0x41810000 | (((iCode - iJump) & 0x3FFF) << 2));
                    break;
                case 0x07: // bgtz
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterA = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterA << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));
                    iJump = iCode++;
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                        return false;
                    }
                    *pnAddress -= 4;

                    if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                        return false;
                    }
                    if (anCode != NULL) {
                        nDeltaAddress = (nOffset - iCode) * 4;
                    }
                    EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    EMIT_PPC(iJump, 0x40810000 | (((iCode - iJump) & 0x3FFF) << 2));
                    break;
                case 0x08: { // addi
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        pCPU->nFlagRAM |= (1 << MIPS_RT(nOpcode));
                    } else {
                        pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    }

                    nTemp2 = nTemp1 = false;
                    if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterA = 5;
                        nTemp2 = true;
                    }
                    if (MIPS_IMM_S16(nOpcode) == 0) {
                        if ((MIPS_RS(nOpcode) == 31) && !(pCPU->nFlagCODE & 4)) {
                            EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                            EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            nTemp1 = true;
                            pCPU->nFlagCODE |= 8;
                        } else if (!nTemp2) {
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            } else {
                                EMIT_PPC(iCode,
                                         0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterB << 11));
                            }
                        } else {
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            } else {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                        }
                        if (MIPS_RT(nOpcode) == 31) {
                            pCPU->nFlagCODE |= 2;
                            if (pCPU->nFlagCODE & 8) {
                                if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                    iRegisterB = 6;
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                            }
                        }
                    } else if (MIPS_RS(nOpcode) == 0) {
                        EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                        nTemp1 = true;
                    } else {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        nTemp1 = true;
                    }
                    if (nTemp1 && (ganMapGPR[MIPS_RT(nOpcode)] & 0x100)) {
                        pCPU->nOptimize.destGPR_check = 2;
                        pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                    }
                    break;
                }
                case 0x09: { // addiu
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        pCPU->nFlagRAM |= (1 << MIPS_RT(nOpcode));
                    } else {
                        pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    }

                    nTemp2 = nTemp1 = false;
                    if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterA = 5;
                        nTemp2 = true;
                    }
                    if (MIPS_IMM_S16(nOpcode) == 0) {
                        if ((MIPS_RS(nOpcode) == 31) && !(pCPU->nFlagCODE & 4)) {
                            EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, nReturnAddrLast));
                            EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                            nTemp1 = true;
                            pCPU->nFlagCODE |= 8;
                        } else if (!nTemp2) {
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                            } else {
                                EMIT_PPC(iCode,
                                         0x7C000378 | (iRegisterB << 21) | (iRegisterA << 16) | (iRegisterB << 11));
                            }
                        } else {
                            if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                pCPU->nOptimize.destGPR_check = 2;
                                pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                                pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            } else {
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) +
                                                    (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                            }
                        }
                        if (MIPS_RT(nOpcode) == 31) {
                            pCPU->nFlagCODE |= 2;
                            if (pCPU->nFlagCODE & 8) {
                                if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                                    iRegisterB = 6;
                                    EMIT_PPC(iCode,
                                             0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                                }
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterB << 21)) + OFFSETOF(pCPU, nReturnAddrLast));
                            }
                        }
                    } else if (MIPS_RS(nOpcode) == 0) {
                        EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                        nTemp1 = true;
                    } else if (!cpuNextInstruction(pCPU, nAddress, nOpcode, anCode, &iCode)) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        nTemp1 = true;
                    }
                    if (nTemp1 && (ganMapGPR[MIPS_RT(nOpcode)] & 0x100)) {
                        pCPU->nOptimize.destGPR_check = 2;
                        pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                    }
                    break;
                }
                case 0x0A: // slti
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterA = 5;
                    }
                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterB = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterB << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x2C000000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                    EMIT_PPC(iCode, 0x4180000C);
                    EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21));
                    EMIT_PPC(iCode, 0x42800008);
                    EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | 1);
                    if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                        pCPU->nOptimize.destGPR_check = 2;
                        pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                    }
                    break;
                case 0x0B: // sltiu
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterA = 5;
                    }
                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterB = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterB << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    iRegisterC = 7;
                    EMIT_PPC(iCode, 0x38000000 | (iRegisterC << 21) | MIPS_IMM_U16(nOpcode));
                    EMIT_PPC(iCode, 0x7C000040 | (iRegisterB << 16) | (iRegisterC << 11));
                    EMIT_PPC(iCode, 0x4180000C);
                    EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21));
                    EMIT_PPC(iCode, 0x42800008);
                    EMIT_PPC(iCode, 0x38000000 | (iRegisterA << 21) | 1);
                    if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                        pCPU->nOptimize.destGPR_check = 2;
                        pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                    }
                    break;
                case 0x0C: // andi
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterA = 5;
                    }
                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterB = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterB << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x70000000 | (iRegisterB << 21) | (iRegisterA << 16) | MIPS_IMM_U16(nOpcode));
                    if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                        pCPU->nOptimize.destGPR_check = 2;
                        pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                    }
                    break;
                case 0x0D: // ori
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        pCPU->nFlagRAM |= (1 << MIPS_RT(nOpcode));
                    } else {
                        pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    }
                    if (!cpuNextInstruction(pCPU, nAddress, nOpcode, anCode, &iCode)) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 5;
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x60000000 | (iRegisterB << 21) | (iRegisterA << 16) | MIPS_IMM_U16(nOpcode));
                        if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                            pCPU->nOptimize.destGPR_check = 2;
                            pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                            pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    break;
                case 0x0E: // xori
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        pCPU->nFlagRAM |= (1 << MIPS_RT(nOpcode));
                    } else {
                        pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    }
                    if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterA = 5;
                    }
                    if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterB = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterB << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x68000000 | (iRegisterB << 21) | (iRegisterA << 16) | MIPS_IMM_U16(nOpcode));
                    if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                        pCPU->nOptimize.destGPR_check = 2;
                        pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                    }
                    break;
                case 0x0F: // lui
                    if (!ramGetSize(SYSTEM_RAM(gpSystem), &nSize)) {
                        return false;
                    }
                    if ((u32)MIPS_IMM_U16(nOpcode) >= 0x8000 &&
                        (u32)MIPS_IMM_U16(nOpcode) <= (0x8000 | (nSize >> 16))) {
                        pCPU->nFlagRAM |= (1 << MIPS_RT(nOpcode));
                    } else {
                        pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    }
                    if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterA = 5;
                    }
                    EMIT_PPC(iCode, 0x3C000000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                    if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                        pCPU->nOptimize.destGPR_check = 2;
                        pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                        pCPU->nOptimize.destGPR_mapping = iRegisterA;
                        EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        pCPU->nOptimize.checkType = 0x3E8;
                        pCPU->nOptimize.checkNext = nAddress;
                    }
                    break;
                case 0x10: // cop0
                    switch (MIPS_FUNCT(nOpcode)) {
                        case 0x01: // tlbr
                        case 0x02: // tlbwi
                        case 0x05: // tlbwr
                        case 0x08: // tlbp
                        case 0x18: // eret
                            break;
                        default:
                            switch (MIPS_RS(nOpcode)) {
                                case 0x00: // mfc0
                                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                                    break;
                                case 0x01: // dmfc0
                                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                                    break;
                                case 0x04: // mtc0
                                    break;
                                case 0x05: // dmtc0
                                    break;
                                case 0x08: // dmtc0
                                    reg = MIPS_RS(nOpcode);
                                    if (reg >= 4 || reg < 0) {
                                        bFlag = false;
                                    }
                                    break;
                                default:
                                    bFlag = false;
                                    break;
                            }
                            break;
                    }
                    break;
                case 0x11: // cop1
                    if ((nOpcode & 0x7FF) == 0 && MIPS_FMT(nOpcode) < 0x10) {
                        switch ((u8)MIPS_FMT(nOpcode)) {
                            case 0x00: // mfc1
                                pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                                if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    iRegisterA = 5;
                                }
                                iRegisterB = MIPS_FS(nOpcode);
                                if (iRegisterB % 2 == 1) {
                                    EMIT_PPC(iCode,
                                             (0x80030000 | (iRegisterA << 21)) + OFFSETOF(pCPU, aFPR[iRegisterB - 1]));
                                } else {
                                    EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                        (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                }
                                if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                                    pCPU->nOptimize.destGPR_check = 2;
                                    pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                                    pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    EMIT_PPC(iCode,
                                             0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                                break;
                            case 0x01: // dmfc1
                                pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                                iRegisterA = MIPS_RT(nOpcode);
                                iRegisterB = MIPS_FS(nOpcode);
                                EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                                EMIT_PPC(iCode, 0x80C30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                EMIT_PPC(iCode, 0x90C30000 + (OFFSETOF(pCPU, aGPR[iRegisterA]) + 4));
                                if (!((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                    EMIT_PPC(iCode, 0x7CC03378 | (iRegisterA << 16));
                                }
                                break;
                            case 0x02: // cfc1
                                pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                                if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    iRegisterA = 5;
                                }
                                iRegisterB = MIPS_FS(nOpcode);
                                EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) + OFFSETOF(pCPU, anFCR[iRegisterB]));
                                if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                                    pCPU->nOptimize.destGPR_check = 2;
                                    pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                                    pCPU->nOptimize.destGPR_mapping = iRegisterA;
                                    EMIT_PPC(iCode,
                                             0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                                }
                                break;
                            case 0x04: // mtc1
                                iRegisterB = MIPS_FS(nOpcode);
                                if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    EMIT_PPC(iCode, 0x80A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                    if (iRegisterB % 2 == 1) {
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aFPR[iRegisterB - 1]));
                                    } else {
                                        EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                    }
                                } else {
                                    if (iRegisterB % 2 == 1) {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            OFFSETOF(pCPU, aFPR[iRegisterB - 1]));
                                    } else {
                                        EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                    }
                                }
                                break;
                            case 0x05: // dmtc1
                                if (!((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                                    EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) +
                                                        (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                }
                                iRegisterA = MIPS_RT(nOpcode);
                                iRegisterB = MIPS_FS(nOpcode);
                                EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                                EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                EMIT_PPC(iCode, 0x80C30000 + (OFFSETOF(pCPU, aGPR[iRegisterA]) + 4));
                                EMIT_PPC(iCode, 0x90C30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                break;
                            case 0x06: // ctc1
                                if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                                    iRegisterA = 5;
                                }
                                iRegisterB = MIPS_FS(nOpcode);
                                if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                                    EMIT_PPC(iCode, (0x80030000 | (iRegisterA << 21)) +
                                                        (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                                }
                                EMIT_PPC(iCode, (0x90030000 | (iRegisterA << 21)) + OFFSETOF(pCPU, anFCR[iRegisterB]));
                                break;
                            default:
                                bFlag = false;
                                break;
                        }
                    } else if (MIPS_FMT(nOpcode) == 0x08) {
                        switch (MIPS_FT(nOpcode)) {
                            case 0x00: // bc1f
                                nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                                EMIT_PPC(iCode, 0x3CA00080);
                                EMIT_PPC(iCode, 0x80C30000 + (OFFSETOF(pCPU, anFCR[31]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x7CC62839);
                                iJump = iCode++;
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                    return false;
                                }
                                *pnAddress -= 4;
                                if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                    return false;
                                }
                                if (anCode != NULL) {
                                    nDeltaAddress = (nOffset - iCode) * 4;
                                }
                                EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                                EMIT_PPC(iJump, 0x40820000 | (((iCode - iJump) & 0x3FFF) << 2));
                                break;
                            case 0x01: // bc1t
                                nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                                EMIT_PPC(iCode, 0x3CA00080);
                                EMIT_PPC(iCode, 0x80C30000 + (OFFSETOF(pCPU, anFCR[31]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x7CC62839);
                                iJump = iCode++;
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                    return false;
                                }
                                *pnAddress -= 4;
                                if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                    return false;
                                }
                                if (anCode != NULL) {
                                    nDeltaAddress = (nOffset - iCode) * 4;
                                }
                                EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                                EMIT_PPC(iJump, 0x41820000 | (((iCode - iJump) & 0x3FFF) << 2));
                                break;
                            case 0x02: // bc1fl
                                nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                                EMIT_PPC(iCode, 0x3CA00080);
                                EMIT_PPC(iCode, 0x80C30000 + (OFFSETOF(pCPU, anFCR[31]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x7CC62839);
                                iJump = iCode++;
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                    return false;
                                }
                                *pnAddress -= 4;
                                if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                    return false;
                                }
                                if (anCode != NULL) {
                                    nDeltaAddress = (nOffset - iCode) * 4;
                                }
                                EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                                    return false;
                                }
                                EMIT_PPC(iJump, 0x40820000 | (((iCode - iJump) & 0x3FFF) << 2));
                                break;
                            case 0x03: // bc1tl
                                nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                                EMIT_PPC(iCode, 0x3CA00080);
                                EMIT_PPC(iCode, 0x80C30000 + (OFFSETOF(pCPU, anFCR[31]) & 0xFFFF));
                                EMIT_PPC(iCode, 0x7CC62839);
                                iJump = iCode++;
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                                    return false;
                                }
                                *pnAddress -= 4;
                                if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                                    return false;
                                }
                                if (anCode != NULL) {
                                    nDeltaAddress = (nOffset - iCode) * 4;
                                }
                                EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                                if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                                    return false;
                                }
                                EMIT_PPC(iJump, 0x41820000 | (((iCode - iJump) & 0x3FFF) << 2));
                                break;
                            default:
                                bFlag = false;
                                break;
                        }
                    } else {
                        switch ((u8)MIPS_FMT(nOpcode)) {
                            case 0x10: // s
                                switch (MIPS_FUNCT(nOpcode)) {
                                    case 0x00: // add.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        if (iRegisterB != iRegisterC) {
                                            if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                                EMIT_PPC(iCode, 0x60000000);
                                            } else {
                                                EMIT_PPC(iCode, (0xC0030000 | (iRegisterC << 21)) +
                                                                    ((OFFSETOF(pCPU, aFPR[iRegisterC]) + 4) & 0xFFFF));
                                            }
                                        }
                                        EMIT_PPC(iCode, 0xEC00002A | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 11));
                                        EMIT_PPC(iCode, (0xD0030000 | (iRegisterA << 21)) +
                                                            ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x01: // sub.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterC << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterC]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xEC000028 | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 11));
                                        EMIT_PPC(iCode, (0xD0030000 | (iRegisterA << 21)) +
                                                            ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x02: // mul.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        if (iRegisterB != iRegisterC) {
                                            if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                                EMIT_PPC(iCode, 0x60000000);
                                            } else {
                                                EMIT_PPC(iCode, (0xC0030000 | (iRegisterC << 21)) +
                                                                    ((OFFSETOF(pCPU, aFPR[iRegisterC]) + 4) & 0xFFFF));
                                            }
                                        }
                                        EMIT_PPC(iCode, 0xEC000032 | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 6));
                                        EMIT_PPC(iCode, (0xD0030000 | (iRegisterA << 21)) +
                                                            ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x03: // div.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterC << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterC]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xEC000024 | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 11));
                                        EMIT_PPC(iCode, (0xD0030000 | (iRegisterA << 21)) +
                                                            ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x04: // sqrt.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        }
                                        EMIT_PPC(iCode,
                                                 0x48000000 |
                                                     ((cpuCompile_S_SQRT_function - (u32)&anCode[iCode]) & 0x03FFFFFC) |
                                                     1);
                                        EMIT_PPC(iCode, 0xD0230000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        break;
                                    case 0x05: // abs.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000210 | (iRegisterA << 21) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, (0xD0030000 | (iRegisterA << 21)) +
                                                            ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x06: // mov.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        EMIT_PPC(iCode, 0xD0230000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        break;
                                    case 0x07: // neg.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000050 | (iRegisterA << 21) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, (0xD0030000 | (iRegisterA << 21)) +
                                                            ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x08: // round.l.s
                                        break;
                                    case 0x09: // trunc.l.s
                                        break;
                                    case 0x0A: // ceil.l.s
                                        break;
                                    case 0x0B: // floor.l.s
                                        break;
                                    case 0x0C: // round.w.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        }
                                        EMIT_PPC(iCode, 0x38A00000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_ROUND_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        break;
                                    case 0x0D: // trunc.w.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        }
                                        EMIT_PPC(iCode, 0x38A00000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_TRUNC_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        break;
                                    case 0x0E: // ceil.w.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        OSReport("CALLED: ceil_w single (%p)\n", nAddress);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        }
                                        EMIT_PPC(iCode,
                                                 0x48000000 |
                                                     ((cpuCompile_CEIL_W_function - (u32)&anCode[iCode]) & 0x03FFFFFC) |
                                                     1);
                                        EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        break;
                                    case 0x0F: // floor.w.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        OSReport("CALLED: floor_w single (%p)\n", nAddress);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        }
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_FLOOR_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        break;
                                    case 0x21: // cvt.d.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        EMIT_PPC(iCode, 0xD8230000 + OFFSETOF(pCPU, aFPR[iRegisterA]));
                                        break;
                                    case 0x24: // cvt.w.s
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC0230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        }
                                        EMIT_PPC(iCode, 0x38A00000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_TRUNC_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        break;
                                    case 0x25: // cvt.l.s
                                        break;
                                    case 0x30: // c.f.s
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x31: // c.un.s
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x32: // c.eq.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x33: // c.ueq.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x34: // c.olt.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x35: // c.ult.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x36: // c.ole.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x37: // c.ule.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x38: // c.sf.s
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x39: // c.ngle.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3A: // c.seq.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3B: // c.ngl.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3C: // c.lt.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3D: // c.nge.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3E: // c.le.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3F: // c.ngt.s
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterA << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterA]) + 4) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC0030000 | (iRegisterB << 21)) +
                                                                ((OFFSETOF(pCPU, aFPR[iRegisterB]) + 4) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    default:
                                        bFlag = false;
                                        break;
                                }
                                break;
                            case 0x11: // d
                                switch (MIPS_FUNCT(nOpcode)) {
                                    case 0x00: // add.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        if (iRegisterB != iRegisterC) {
                                            if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                                EMIT_PPC(iCode, 0x60000000);
                                            } else {
                                                EMIT_PPC(iCode, (0xC8030000 | (iRegisterC << 21)) +
                                                                    (OFFSETOF(pCPU, aFPR[iRegisterC]) & 0xFFFF));
                                            }
                                        }
                                        EMIT_PPC(iCode, 0xFC00002A | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 11));
                                        EMIT_PPC(iCode, (0xD8030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x01: // sub.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterC << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterC]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000028 | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 11));
                                        EMIT_PPC(iCode, (0xD8030000 | (iRegisterA << 21)) +
                                                            ((OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x02: // mul.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        if (iRegisterB != iRegisterC) {
                                            if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                                EMIT_PPC(iCode, 0x60000000);
                                            } else {
                                                EMIT_PPC(iCode, (0xC8030000 | (iRegisterC << 21)) +
                                                                    (OFFSETOF(pCPU, aFPR[iRegisterC]) & 0xFFFF));
                                            }
                                        }
                                        EMIT_PPC(iCode, 0xFC000032 | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 6));
                                        EMIT_PPC(iCode, (0xD8030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x03: // div.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        iRegisterC = MIPS_FT(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterC)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterC << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterC]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000024 | (iRegisterA << 21) | (iRegisterB << 16) |
                                                            (iRegisterC << 11));
                                        EMIT_PPC(iCode, (0xD8030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x04: // sqrt.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC8230000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        }
                                        EMIT_PPC(iCode,
                                                 0x48000000 |
                                                     ((cpuCompile_D_SQRT_function - (u32)&anCode[iCode]) & 0x03FFFFFC) |
                                                     1);
                                        EMIT_PPC(iCode, 0xD8230000 + OFFSETOF(pCPU, aFPR[iRegisterA]));
                                        break;
                                    case 0x05: // abs.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000210 | (iRegisterA << 21) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, (0xD8030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x06: // mov.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        EMIT_PPC(iCode, 0xC8230000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        EMIT_PPC(iCode, 0xD8230000 + OFFSETOF(pCPU, aFPR[iRegisterA]));
                                        break;
                                    case 0x07: // neg.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000050 | (iRegisterA << 21) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, (0xD8030000 | (iRegisterA << 21)) +
                                                            (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x08: // round.l.d
                                        break;
                                    case 0x09: // trunc.l.d
                                        break;
                                    case 0x0A: // ceil.l.d
                                        break;
                                    case 0x0B: // floor.l.d
                                        break;
                                    case 0x0C: // round.w.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC8230000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        }
                                        EMIT_PPC(iCode, 0x38A00000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_ROUND_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        break;
                                    case 0x0D: // trunc.w.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC8230000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        }
                                        EMIT_PPC(iCode, 0x38A00000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_TRUNC_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        break;
                                    case 0x0E: // ceil.w.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        OSReport("CALLED: ceil_w double (%p)\n", nAddress);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC8230000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        }
                                        EMIT_PPC(iCode,
                                                 0x48000000 |
                                                     ((cpuCompile_CEIL_W_function - (u32)&anCode[iCode]) & 0x03FFFFFC) |
                                                     1);
                                        EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        break;
                                    case 0x0F: // floor.w.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        OSReport("CALLED: floor_w double (%p)\n", nAddress);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC8230000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        }
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_FLOOR_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        break;
                                    case 0x21: // cvt.d.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: CVT_S_D\n");
                                        break;
                                    case 0x24: // cvt.w.d
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0xFC200090 | (iRegisterB << 11));
                                        } else {
                                            EMIT_PPC(iCode, 0xC8230000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        }
                                        EMIT_PPC(iCode, 0x38A00000 + (OFFSETOF(pCPU, aFPR[iRegisterA]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_TRUNC_W_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        break;
                                    case 0x25: // cvt.l.d
                                        break;
                                    case 0x30: // c.f.d
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x31: // c.un.d
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x32: // c.eq.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x33: // c.ueq.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x34: // c.olt.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x35: // c.ult.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x36: // c.ole.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x37: // c.ule.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x38: // c.sf.d
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x39: // c.ngle.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3A: // c.seq.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3B: // c.ngl.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000000 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3C: // c.lt.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3D: // c.nge.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4080000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3E: // c.le.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    case 0x3F: // c.ngt.d
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterA)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterA << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterA]) & 0xFFFF));
                                        }
                                        if (cpuCutStoreLoadF(pCPU, nAddress, iRegisterB)) {
                                            EMIT_PPC(iCode, 0x60000000);
                                        } else {
                                            EMIT_PPC(iCode, (0xC8030000 | (iRegisterB << 21)) +
                                                                (OFFSETOF(pCPU, aFPR[iRegisterB]) & 0xFFFF));
                                        }
                                        EMIT_PPC(iCode, 0xFC000040 | (iRegisterA << 16) | (iRegisterB << 11));
                                        EMIT_PPC(iCode, 0x4C401382);
                                        EMIT_PPC(iCode, 0x4082000C);
                                        EMIT_PPC(iCode, 0x64A50080);
                                        EMIT_PPC(iCode, 0x42800008);
                                        EMIT_PPC(iCode, 0x54A5024E);
                                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, anFCR[31]));
                                        break;
                                    default:
                                        bFlag = false;
                                        break;
                                }
                                break;
                            case 0x14: // w
                                switch (MIPS_FUNCT(nOpcode)) {
                                    case 0x00: // add.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: ADD_W\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x01: // sub.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: SUB_W\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x02: // mul.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: MUL_W\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x03: // div.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: DIV_W\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x04: // sqrt.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: SQRT_W\n");
                                        break;
                                    case 0x05: // abs.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: ABS_W\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x06: // mov.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: MOVE_W\n");
                                        break;
                                    case 0x07: // neg.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: NEG_W\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x08: // round.l.w
                                        break;
                                    case 0x09: // trunc.l.w
                                        break;
                                    case 0x0A: // ceil.l.w
                                        break;
                                    case 0x0B: // floor.l.w
                                        break;
                                    case 0x0C: // round.w.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: ROUND_W_W\n");
                                        break;
                                    case 0x0D: // trunc.w.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: TRUNC_W_W\n");
                                        break;
                                    case 0x0E: // ceil.w.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: CEIL_W_W\n");
                                        break;
                                    case 0x0F: // floor.w.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: FLOOR_W_W\n");
                                        break;
                                    case 0x21: // cvt.d.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_W_CVT_SD_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        EMIT_PPC(iCode, 0xD8230000 + OFFSETOF(pCPU, aFPR[iRegisterA]));
                                        break;
                                    case 0x24: // cvt.w.w
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: CVT_W_W\n");
                                        break;
                                    case 0x25: // cvt.l.w
                                        break;
                                    case 0x30: // c.f.w
                                        OSReport("ERROR: C.F_W\n");
                                        break;
                                    case 0x31: // c.un.w
                                        OSReport("ERROR: C.UN_W\n");
                                        break;
                                    case 0x32: // c.eq.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.EQ_W\n");
                                        break;
                                    case 0x33: // c.ueq.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.UEQ_W\n");
                                        break;
                                    case 0x34: // c.olt.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.OLT_W\n");
                                        break;
                                    case 0x35: // c.ult.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.ULT_W\n");
                                        break;
                                    case 0x36: // c.ole.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.OLE_W\n");
                                        break;
                                    case 0x37: // c.ule.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.ULE_W\n");
                                        break;
                                    case 0x38: // c.sf.w
                                        OSReport("ERROR: C.SF_W\n");
                                        break;
                                    case 0x39: // c.ngle.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGLE_W\n");
                                        break;
                                    case 0x3A: // c.seq.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.SEQ_W\n");
                                        break;
                                    case 0x3B: // c.ngl.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGL_W\n");
                                        break;
                                    case 0x3C: // c.lt.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.LT_W\n");
                                        break;
                                    case 0x3D: // c.nge.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGE_W\n");
                                        break;
                                    case 0x3E: // c.le.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.LE_W\n");
                                        break;
                                    case 0x3F: // c.ngt.w
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGT_W\n");
                                        break;
                                    default:
                                        bFlag = false;
                                        break;
                                }
                                break;
                            case 0x15: // l
                                switch (MIPS_FUNCT(nOpcode)) {
                                    case 0x00: // add.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: ADD_L\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x01: // sub.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: SUB_L\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x02: // mul.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: MUL_L\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x03: // div.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: DIV_L\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x04: // sqrt.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: SQRT_L\n");
                                        break;
                                    case 0x05: // abs.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: ABS_L\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x06: // mov.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: MOVE_L\n");
                                        break;
                                    case 0x07: // neg.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: NEG_L\n");
                                        pCPU->nOptimize.destFPR_check = 2;
                                        pCPU->nOptimize.destFPR = iRegisterA;
                                        break;
                                    case 0x08: // round.l.l
                                        break;
                                    case 0x09: // trunc.l.l
                                        break;
                                    case 0x0A: // ceil.l.l
                                        break;
                                    case 0x0B: // floor.l.l
                                        break;
                                    case 0x0C: // round.w.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: ROUND_W_L\n");
                                        break;
                                    case 0x0D: // trunc.w.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: TRUNC_W_L\n");
                                        break;
                                    case 0x0E: // ceil.w.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: CEIL_W_L\n");
                                        break;
                                    case 0x0F: // floor.w.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: FLOOR_W_L\n");
                                        break;
                                    case 0x21: // cvt.d.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        iRegisterB = MIPS_FS(nOpcode);
                                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                                        EMIT_PPC(iCode, 0x80C30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                                        EMIT_PPC(iCode, 0x48000000 |
                                                            ((cpuCompile_L_CVT_SD_function - (u32)&anCode[iCode]) &
                                                             0x03FFFFFC) |
                                                            1);
                                        EMIT_PPC(iCode, 0xD8230000 + OFFSETOF(pCPU, aFPR[iRegisterA]));
                                        break;
                                    case 0x24: // cvt.w.l
                                        iRegisterA = MIPS_FD(nOpcode);
                                        OSReport("ERROR: CVT_W_L\n");
                                        break;
                                    case 0x25: // cvt.l.l
                                        break;
                                    case 0x30: // c.f.l
                                        OSReport("ERROR: C.F_L\n");
                                        break;
                                    case 0x31: // c.un.l
                                        OSReport("ERROR: C.UN_L\n");
                                        break;
                                    case 0x32: // c.eq.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.EQ_L\n");
                                        break;
                                    case 0x33: // c.ueq.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.UEQ_L\n");
                                        break;
                                    case 0x34: // c.olt.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.OLT_L\n");
                                        break;
                                    case 0x35: // c.ult.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.ULT_L\n");
                                        break;
                                    case 0x36: // c.ole.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.OLE_L\n");
                                        break;
                                    case 0x37: // c.ule.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.ULE_L\n");
                                        break;
                                    case 0x38: // c.sf.l
                                        OSReport("ERROR: C.SF_L\n");
                                        break;
                                    case 0x39: // c.ngle.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGLE_L\n");
                                        break;
                                    case 0x3A: // c.seq.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.SEQ_L\n");
                                        break;
                                    case 0x3B: // c.ngl.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGL_L\n");
                                        break;
                                    case 0x3C: // c.lt.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.LT_L\n");
                                        break;
                                    case 0x3D: // c.nge.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGE_L\n");
                                        break;
                                    case 0x3E: // c.le.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.LE_L\n");
                                        break;
                                    case 0x3F: // c.ngt.l
                                        iRegisterA = MIPS_FS(nOpcode);
                                        iRegisterB = MIPS_FT(nOpcode);
                                        OSReport("ERROR: C.NGT_L\n");
                                        break;
                                    default:
                                        bFlag = false;
                                        break;
                                }
                                break;
                        }
                    }
                    break;
                case 0x12: // cop2
                    bFlag = false;
                    break;
                case 0x13: // cop1x
                    bFlag = false;
                    break;
                case 0x14: // beql
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if (MIPS_RS(nOpcode) == MIPS_RT(nOpcode)) {
                        if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                            return false;
                        }
                        *pnAddress -= 4;
                        if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                            return false;
                        }
                        if (anCode != NULL) {
                            nDeltaAddress = (nOffset - iCode) * 4;
                        }
                        EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    } else {
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterB = 7;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x7C000000 | (iRegisterA << 16) | (iRegisterB << 11));
                        iJump = iCode++;
                        if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                            return false;
                        }
                        *pnAddress -= 4;

                        if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                            return false;
                        }
                        if (anCode != NULL) {
                            nDeltaAddress = (nOffset - iCode) * 4;
                        }
                        EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                        if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                            return false;
                        }
                        EMIT_PPC(iJump, 0x40820000 | (((iCode - iJump) & 0x3FFF) << 2));
                    }
                    break;
                case 0x15: // bnel
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterA = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterA << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                        iRegisterB = 7;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterB << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x7C000000 | (iRegisterA << 16) | (iRegisterB << 11));
                    iJump = iCode++;
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                        return false;
                    }
                    *pnAddress -= 4;

                    if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                        return false;
                    }
                    if (anCode != NULL) {
                        nDeltaAddress = (nOffset - iCode) * 4;
                    }
                    EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                        return false;
                    }
                    EMIT_PPC(iJump, 0x41820000 | (((iCode - iJump) & 0x3FFF) << 2));
                    break;
                case 0x16: // blezl
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterA = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterA << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));
                    iJump = iCode++;
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                        return false;
                    }
                    *pnAddress -= 4;

                    if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                        return false;
                    }
                    if (anCode != NULL) {
                        nDeltaAddress = (nOffset - iCode) * 4;
                    }
                    EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                        return false;
                    }
                    EMIT_PPC(iJump, 0x41810000 | (((iCode - iJump) & 0x3FFF) << 2));
                    break;
                case 0x17: // bgtzl
                    nAddressJump = *pnAddress + (MIPS_IMM_S16(nOpcode) * 4);
                    if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                        iRegisterA = 6;
                        if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                            EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) | (iRegisterA << 16) |
                                                (pCPU->nOptimize.destGPR_mapping << 11));
                        } else {
                            EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                        }
                    }
                    EMIT_PPC(iCode, 0x2C000000 | (iRegisterA << 16));
                    iJump = iCode++;
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, true)) {
                        return false;
                    }
                    *pnAddress -= 4;

                    if (!cpuFindBranchOffset(pCPU, pFunction, &nOffset, nAddressJump, anCode)) {
                        return false;
                    }
                    if (anCode != NULL) {
                        nDeltaAddress = (nOffset - iCode) * 4;
                    }
                    EMIT_PPC(iCode, 0x48000000 | (nDeltaAddress & 0x03FFFFFC));
                    if (!cpuGetPPC(pCPU, pnAddress, pFunction, anCode, &iCode, false)) {
                        return false;
                    }
                    EMIT_PPC(iJump, 0x40810000 | (((iCode - iJump) & 0x3FFF) << 2));
                    break;
                case 0x18: // daddi
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        pCPU->nFlagRAM |= (1 << MIPS_RT(nOpcode));
                    } else {
                        pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    }
                    if (!((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                        EMIT_PPC(iCode,
                                 (0x90030000 | (iRegisterB << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                    }
                    iRegisterA = MIPS_RT(nOpcode);
                    iRegisterB = MIPS_RS(nOpcode);
                    EMIT_PPC(iCode, 0x9421FFF0);
                    EMIT_PPC(iCode, 0x91010008);
                    EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                    EMIT_PPC(iCode, 0x38E00000 | MIPS_IMM_U16(nOpcode));
                    EMIT_PPC(iCode, 0x39000000);
                    EMIT_PPC(iCode, 0x7CE70734);
                    EMIT_PPC(iCode, 0x2C070000);
                    EMIT_PPC(iCode, 0x4080000C);
                    EMIT_PPC(iCode, 0x3900FFFF);
                    EMIT_PPC(iCode, 0x7D080734);
                    EMIT_PPC(iCode, 0x7CA53814);
                    EMIT_PPC(iCode, 0x7CC64114);
                    EMIT_PPC(iCode, 0x90A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                    EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                    EMIT_PPC(iCode, 0x81010008);
                    EMIT_PPC(iCode, 0x38210010);
                    if (!((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                        EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                    }
                    break;
                case 0x19: // daddiu
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        pCPU->nFlagRAM |= (1 << MIPS_RT(nOpcode));
                    } else {
                        pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    }
                    if (!((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                        EMIT_PPC(iCode,
                                 (0x90030000 | (iRegisterB << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                    }
                    iRegisterA = MIPS_RT(nOpcode);
                    iRegisterB = MIPS_RS(nOpcode);
                    EMIT_PPC(iCode, 0x9421FFF0);
                    EMIT_PPC(iCode, 0x91010008);
                    EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                    EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                    EMIT_PPC(iCode, 0x38E00000 | MIPS_IMM_U16(nOpcode));
                    EMIT_PPC(iCode, 0x39000000);
                    EMIT_PPC(iCode, 0x7CE70734);
                    EMIT_PPC(iCode, 0x2C070000);
                    EMIT_PPC(iCode, 0x4080000C);
                    EMIT_PPC(iCode, 0x3900FFFF);
                    EMIT_PPC(iCode, 0x7D080734);
                    EMIT_PPC(iCode, 0x7CA53814);
                    EMIT_PPC(iCode, 0x7CC64114);
                    EMIT_PPC(iCode, 0x90A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                    EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                    EMIT_PPC(iCode, 0x81010008);
                    EMIT_PPC(iCode, 0x38210010);
                    if (!((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                        EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                    }
                    break;
                case 0x1F: // library call
                    if (libraryFunctionReplaced(SYSTEM_LIBRARY(gpSystem), MIPS_IMM_U16(nOpcode))) {
                        pCPU->nFlagCODE |= 1;
                        pFunction->nAddress1 = nAddress + 8;
                    }
                    break;
                case 0x1A: // ldl
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x1B: // ldr
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x27: // lwu
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x20: // lb
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 5;
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }

                        if (!cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0x88070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x7C000774 | (iRegisterA << 21) | (iRegisterA << 16));
                        if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                            pCPU->nOptimize.destGPR_check = 2;
                            pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                            pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800010);
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_LB_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                        } else {
                            EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x21: // lh
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 5;
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }

                        if (!cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0xA0070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x7C000734 | (iRegisterA << 21) | (iRegisterA << 16));
                        if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                            pCPU->nOptimize.destGPR_check = 2;
                            pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                            pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800010);
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_LH_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                        } else {
                            EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x22: // lwl
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        EMIT_PPC(iCode, 0x9421FFE8);
                        EMIT_PPC(iCode, 0x91210008);
                        EMIT_PPC(iCode, 0x91410010);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x80A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                        } else {
                            EMIT_PPC(iCode, 0x7C050378 | (iRegisterA << 21) | (iRegisterA << 11));
                        }
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x38E70000 | MIPS_IMM_U16(nOpcode));
                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x81230004 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7C090378 | (iRegisterB << 21) | (iRegisterB << 11));
                        }
                        EMIT_PPC(iCode,
                                 0x48000000 | ((cpuCompile_LWL_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x91230004 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7D204B78 | (iRegisterB << 16));
                        }
                        EMIT_PPC(iCode, 0x81210008);
                        EMIT_PPC(iCode, 0x81410010);
                        EMIT_PPC(iCode, 0x38210018);
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x23: // lw
                    if ((nOpcode & 0xFFFF0000) == 0x8FBF0000) { // lw $ra, 0x????($sp)
                        if ((pCPU->nFlagCODE & 4) && (pCPU->nFlagCODE >> 16) == MIPS_IMM_U16(nOpcode)) {
                            pCPU->nFlagCODE &= ~2;
                            if (prev != 0x457) {
                                pCPU->nFlagCODE &= ~4;
                                pCPU->nFlagCODE &= 0xFFFF;
                            }
                        }
                    }
                    fn_8000E734(pCPU, nOpcode, nOpcodePrev, nOpcodeNext, nAddress, anCode, &iCode, &iRegisterA);
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 5;
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }

                        if (!cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0x80070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                        if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                            pCPU->nOptimize.destGPR_check = 2;
                            pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                            pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800010);
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_LW_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                        } else {
                            EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x24: // lbu
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 5;
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }

                        if (!cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0x88070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                        if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                            pCPU->nOptimize.destGPR_check = 2;
                            pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                            pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800010);
                        EMIT_PPC(iCode,
                                 0x48000000 | ((cpuCompile_LBU_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                        } else {
                            EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x25: // lhu
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 5;
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }

                        if (!cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0xA0070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                        if (ganMapGPR[MIPS_RT(nOpcode)] & 0x100) {
                            pCPU->nOptimize.destGPR_check = 2;
                            pCPU->nOptimize.destGPR = MIPS_RT(nOpcode);
                            pCPU->nOptimize.destGPR_mapping = iRegisterA;
                            EMIT_PPC(iCode, 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                        }
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800010);
                        EMIT_PPC(iCode,
                                 0x48000000 | ((cpuCompile_LHU_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                        } else {
                            EMIT_PPC(iCode, 0x7CA02B78 | (iRegisterA << 16));
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x26: // lwr
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        EMIT_PPC(iCode, 0x9421FFE8);
                        EMIT_PPC(iCode, 0x91210008);
                        EMIT_PPC(iCode, 0x91410010);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x80A30000 + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                        } else {
                            EMIT_PPC(iCode, 0x7C050378 | (iRegisterA << 21) | (iRegisterA << 11));
                        }
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x38E70000 | MIPS_IMM_U16(nOpcode));
                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x81230004 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7C090378 | (iRegisterB << 21) | (iRegisterB << 11));
                        }
                        EMIT_PPC(iCode,
                                 0x48000000 | ((cpuCompile_LWR_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if ((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x91230004 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7D204B78 | (iRegisterB << 16));
                        }
                        EMIT_PPC(iCode, 0x81210008);
                        EMIT_PPC(iCode, 0x81410010);
                        EMIT_PPC(iCode, 0x38210018);
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x28: // sb
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        nTemp1 = true;
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 7;
                            nTemp1 = false;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        if (!nTemp1 || !cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), -1)) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0x98070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800020);
                        EMIT_PPC(iCode, 0x9421FFF0);
                        EMIT_PPC(iCode, 0x91010008);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x81030004 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7C080378 | (iRegisterA << 21) | (iRegisterA << 11));
                        }
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_SB_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        EMIT_PPC(iCode, 0x81010008);
                        EMIT_PPC(iCode, 0x38210010);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    break;
                case 0x29: // sh
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        nTemp1 = true;
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 7;
                            nTemp1 = false;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        if (!nTemp1 || !cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), -1)) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0xB0070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800020);
                        EMIT_PPC(iCode, 0x9421FFF0);
                        EMIT_PPC(iCode, 0x91010008);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x81030004 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7C080378 | (iRegisterA << 21) | (iRegisterA << 11));
                        }
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_SH_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        EMIT_PPC(iCode, 0x81010008);
                        EMIT_PPC(iCode, 0x38210010);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    break;
                case 0x2A: // swl
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                            EMIT_PPC(iCode,
                                     (0x90030000 | (iRegisterB << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                        }
                        iRegisterB = MIPS_RT(nOpcode);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7C050378 | (iRegisterA << 21) | (iRegisterA << 11));
                        }
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x38E70000 | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x38C00018);
                        EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                        EMIT_PPC(iCode, 0x7CA53430);
                        EMIT_PPC(iCode, 0x98A70000);
                        EMIT_PPC(iCode, 0x38C6FFF8);
                        EMIT_PPC(iCode, 0x54E507BF);
                        EMIT_PPC(iCode, 0x38E70001);
                        EMIT_PPC(iCode, 0x4082FFE8);
                    }
                    break;
                case 0x2B: // sw
                    if ((nOpcode & 0xFFFF0000) == 0xAFBF0000) { // sw $ra, ????($sp)
                        if (!(pCPU->nFlagCODE & 4)) {
                            pCPU->nFlagCODE |= 4;
                            pCPU->nFlagCODE |= MIPS_IMM_U16(nOpcode) << 16;
                        }
                    }
                    fn_8000E81C(pCPU, nOpcode, nOpcodePrev, nOpcodeNext, nAddress, anCode, &iCode, &iRegisterA);
                    if (nOpcode == 0xACBF011C) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RT(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        nTemp1 = true;
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 7;
                            nTemp1 = false;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        if (!nTemp1 || !cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), -1)) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0x90070000 | (iRegisterA << 21) | MIPS_IMM_U16(nOpcode));
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRam - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 3);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 2);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800020);
                        EMIT_PPC(iCode, 0x9421FFF0);
                        EMIT_PPC(iCode, 0x91010008);
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x81030004 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7C080378 | (iRegisterA << 21) | (iRegisterA << 11));
                        }
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_SW_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        EMIT_PPC(iCode, 0x81010008);
                        EMIT_PPC(iCode, 0x38210010);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    }
                    break;
                case 0x2C: // sdl
                case 0x2D: // sdr
                    break;
                case 0x2E: // swr
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if (!((iRegisterB = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100)) {
                            EMIT_PPC(iCode, ((0x90030000 | (iRegisterB << 21)) +
                                             (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4)));
                        }
                        iRegisterB = MIPS_RT(nOpcode);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]));
                        } else {
                            EMIT_PPC(iCode, 0x7C050378 | (iRegisterA << 21) | (iRegisterA << 11));
                        }
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x38E70000 | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x38C00000);
                        EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterB]));
                        EMIT_PPC(iCode, 0x7CA53430);
                        EMIT_PPC(iCode, 0x98A70000);
                        EMIT_PPC(iCode, 0x38C60008);
                        EMIT_PPC(iCode, 0x54E507BF);
                        EMIT_PPC(iCode, 0x38E7FFFF);
                        EMIT_PPC(iCode, 0x4082FFE8);
                    }
                    break;
                case 0x2F: // cache
                    EMIT_PPC(iCode, 0x60000000);
                    break;
                case 0x30: // sc
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x31: // lwc1
                    if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRamF - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 4);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 3);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        iRegisterB = MIPS_RT(nOpcode);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterA << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800010);
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_LW_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (iRegisterB % 2 == 1) {
                            EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aFPR[iRegisterB - 1]));
                        } else {
                            EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    } else {
                        if (!((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                            EMIT_PPC(iCode,
                                     (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                        }
                        iRegisterA = MIPS_RS(nOpcode);
                        iRegisterB = MIPS_RT(nOpcode);
                        if (!ramGetSize(SYSTEM_RAM(gpSystem), &nSize)) {
                            return false;
                        }
                        EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                        EMIT_PPC(iCode, 0x3CE08000);
                        EMIT_PPC(iCode, 0x7C072840);
                        EMIT_PPC(iCode, 0x41810014);
                        EMIT_PPC(iCode, 0x3CE08000 | ((nSize >> 16) - 1));
                        EMIT_PPC(iCode, 0x60E7FFFF);
                        EMIT_PPC(iCode, 0x7C072840);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x41810024);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x41810014);
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x42800028);
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB & 0x1F));
                        EMIT_PPC(iCode, 0x70A50001);
                        EMIT_PPC(iCode, 0x41820010);
                        EMIT_PPC(iCode, 0x80A70000 | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aFPR[iRegisterB - 1]));
                        EMIT_PPC(iCode, 0x4280000C);
                        EMIT_PPC(iCode, 0x80A70000 | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                    }
                    break;
                case 0x34: // lld
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x35: // ldc1
                    if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRamF - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 4);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 3);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        iRegisterB = MIPS_RT(nOpcode);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterA << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800014);
                        EMIT_PPC(iCode,
                                 0x48000000 | ((cpuCompile_LDC_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                        EMIT_PPC(iCode, 0x90C30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    } else {
                        if (!((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                            EMIT_PPC(iCode,
                                     (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                        }
                        iRegisterA = MIPS_RS(nOpcode);
                        iRegisterB = MIPS_RT(nOpcode);
                        if (!ramGetSize(SYSTEM_RAM(gpSystem), &nSize)) {
                            return false;
                        }
                        EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                        EMIT_PPC(iCode, 0x3CE08000);
                        EMIT_PPC(iCode, 0x7C072840);
                        EMIT_PPC(iCode, 0x41810014);
                        EMIT_PPC(iCode, 0x3CE08000 | ((nSize >> 16) - 1));
                        EMIT_PPC(iCode, 0x60E7FFFF);
                        EMIT_PPC(iCode, 0x7C072840);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x41810024);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x41810014);
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x42800018);
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x80A70000 | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                        EMIT_PPC(iCode, 0x80A70000 | (MIPS_IMM_U16(nOpcode) + 4));
                        EMIT_PPC(iCode, 0x90A30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                    }
                    break;
                case 0x37: // ld
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 5;
                        }
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        if (!cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), MIPS_RT(nOpcode))) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0x80A70000 | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x90A30000 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        EMIT_PPC(iCode, 0x80070000 | (iRegisterA << 21) | (MIPS_IMM_U16(nOpcode) + 4));
                        EMIT_PPC(iCode,
                                 (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]) + 4));
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRamF - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 4);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 3);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    }
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x38: // sc
                    break;
                case 0x39: // swc1
                    if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRamF - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 4);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 3);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        iRegisterB = MIPS_RT(nOpcode);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterA << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x41800020);
                        EMIT_PPC(iCode, 0x9421FFF0);
                        EMIT_PPC(iCode, 0x91010008);
                        if (iRegisterB % 2 == 1) {
                            EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aFPR[iRegisterB - 1]));
                        } else {
                            EMIT_PPC(iCode, 0x81030000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                        }
                        EMIT_PPC(iCode, 0x48000000 | ((cpuCompile_SW_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        EMIT_PPC(iCode, 0x81010008);
                        EMIT_PPC(iCode, 0x38210010);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    } else {
                        if (!((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                            EMIT_PPC(iCode,
                                     (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                        }
                        iRegisterA = MIPS_RS(nOpcode);
                        iRegisterB = MIPS_RT(nOpcode);
                        if (!ramGetSize(SYSTEM_RAM(gpSystem), &nSize)) {
                            return false;
                        }
                        EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                        EMIT_PPC(iCode, 0x3CE08000);
                        EMIT_PPC(iCode, 0x7C072840);
                        EMIT_PPC(iCode, 0x41810014);
                        EMIT_PPC(iCode, 0x3CE08000 | ((nSize >> 16) - 1));
                        EMIT_PPC(iCode, 0x60E7FFFF);
                        EMIT_PPC(iCode, 0x7C072840);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x41810024);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x41810014);
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x42800024);
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterB & 0x1F));
                        EMIT_PPC(iCode, 0x70A50001);
                        EMIT_PPC(iCode, 0x4182000C);
                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aFPR[iRegisterB - 1]));
                        EMIT_PPC(iCode, 0x42800008);
                        EMIT_PPC(iCode, 0x80A30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                        EMIT_PPC(iCode, 0x90A70000 | MIPS_IMM_U16(nOpcode));
                    }
                    break;
                case 0x3C: // scd
                    pCPU->nFlagRAM &= ~(1 << MIPS_RT(nOpcode));
                    break;
                case 0x3D: // sdc1
                    if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRamF - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 4);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 3);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    } else if (pCPU->nCompileFlag & 1) {
                        iRegisterB = MIPS_RT(nOpcode);
                        if ((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterA << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        EMIT_PPC(iCode, 0x38A00000 | (iRegisterA << 16) | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x54A6843E);
                        EMIT_PPC(iCode, 0x7CC648AE);
                        EMIT_PPC(iCode, 0x2C060080);
                        EMIT_PPC(iCode, 0x4180002C);
                        EMIT_PPC(iCode, 0x9421FFE8);
                        EMIT_PPC(iCode, 0x91010008);
                        EMIT_PPC(iCode, 0x91210010);
                        EMIT_PPC(iCode, 0x81030000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                        EMIT_PPC(iCode, 0x81230000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                        EMIT_PPC(iCode,
                                 0x48000000 | ((cpuCompile_SDC_function - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        EMIT_PPC(iCode, 0x81010008);
                        EMIT_PPC(iCode, 0x81210010);
                        EMIT_PPC(iCode, 0x38210018);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x42800020);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x42800010);
                            EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                            EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                            EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        }
                    } else {
                        if (!((iRegisterA = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100)) {
                            EMIT_PPC(iCode,
                                     (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4));
                        }
                        iRegisterA = MIPS_RS(nOpcode);
                        iRegisterB = MIPS_RT(nOpcode);
                        if (!ramGetSize(SYSTEM_RAM(gpSystem), &nSize)) {
                            return false;
                        }
                        EMIT_PPC(iCode, 0x80A30004 + OFFSETOF(pCPU, aGPR[iRegisterA]));
                        EMIT_PPC(iCode, 0x3CE08000);
                        EMIT_PPC(iCode, 0x7C072840);
                        EMIT_PPC(iCode, 0x41810014);
                        EMIT_PPC(iCode, 0x3CE08000 | ((nSize >> 16) - 1));
                        EMIT_PPC(iCode, 0x60E7FFFF);
                        EMIT_PPC(iCode, 0x7C072840);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x41810024);
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        } else {
                            EMIT_PPC(iCode, 0x41810014);
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x42800018);
                        EMIT_PPC(iCode, 0x7CE54214);
                        EMIT_PPC(iCode, 0x80A30000 + OFFSETOF(pCPU, aFPR[iRegisterB]));
                        EMIT_PPC(iCode, 0x90A70000 | MIPS_IMM_U16(nOpcode));
                        EMIT_PPC(iCode, 0x80A30000 + (OFFSETOF(pCPU, aFPR[iRegisterB]) + 4));
                        EMIT_PPC(iCode, 0x90A70000 | (MIPS_IMM_U16(nOpcode) + 4));
                    }
                    break;
                case 0x3F: // sd
                    if (pCPU->nFlagRAM & (1 << MIPS_RS(nOpcode))) {
                        nTemp1 = true;
                        if ((iRegisterB = ganMapGPR[MIPS_RS(nOpcode)]) & 0x100) {
                            iRegisterB = 7;
                            nTemp1 = false;
                            if (cpuCutStoreLoad(pCPU, nAddress, MIPS_RS(nOpcode))) {
                                EMIT_PPC(iCode, 0x7C000378 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                                    (iRegisterB << 16) | (pCPU->nOptimize.destGPR_mapping << 11));
                            } else {
                                EMIT_PPC(iCode, 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(nOpcode)]) + 4) & 0xFFFF));
                            }
                        }
                        if (!nTemp1 || !cpuStackOffset(pCPU, nAddress, anCode, MIPS_RS(nOpcode), -1)) {
                            EMIT_PPC(iCode, 0x7CE04214 | (iRegisterB << 16));
                        } else {
                            EMIT_PPC(iCode, 0x60000000);
                        }
                        EMIT_PPC(iCode, 0x80C30000 + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        EMIT_PPC(iCode, 0x90C70000 | MIPS_IMM_U16(nOpcode));
                        if ((iRegisterA = ganMapGPR[MIPS_RT(nOpcode)]) & 0x100) {
                            iRegisterA = 6;
                            EMIT_PPC(iCode, (0x80030004 | (iRegisterA << 21)) + OFFSETOF(pCPU, aGPR[MIPS_RT(nOpcode)]));
                        }
                        EMIT_PPC(iCode, 0x90070000 | (iRegisterA << 21) | (MIPS_IMM_U16(nOpcode) + 4));
                    } else if (pCPU->nCompileFlag & 0x10) {
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C00000);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                        EMIT_PPC(iCode, 0x3CA00000 | ((u32)nAddress >> 16));
                        EMIT_PPC(iCode, 0x60A50000 | ((u32)nAddress & 0xFFFF));
                        EMIT_PPC(iCode, 0x48000000 | (((u32)pCPU->pfRamF - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1);
                        if (pCPU->nCompileFlag & 0x100) {
                            cpuCompileNOP(anCode, &iCode, 4);
                        } else {
                            cpuCompileNOP(anCode, &iCode, 3);
                        }
                        if (var_r17) {
                            EMIT_PPC(iCode, 0x38C0FFFF);
                            EMIT_PPC(iCode, 0x90C30000 + OFFSETOF(pCPU, nWaitPC));
                        }
                    }
                    break;
                default:
                    bFlag = false;
                    break;
            }
        }

        if (!bFlag) {
            return false;
        }
        if (!bSlot && pFunction->pfCode == NULL) {
            if (nAddressJump != -1 && anCode == NULL) {
                pFunction->aJump[pFunction->nCountJump++].nAddressN64 = nAddressJump;
            }
            for (iJump = 0; iJump < pFunction->nCountJump; iJump++) {
                if (pFunction->aJump[iJump].nAddressN64 == nAddress) {
                    pFunction->aJump[iJump].nOffsetHost = *piCode;
                }
            }
        }
        if (iCode == *piCode) {
            if (anCode != NULL) {
                if (var_r17) {
                    anCode[iCode++] = 0x38C00000;
                    anCode[iCode++] = 0x90C30000 + OFFSETOF(pCPU, nWaitPC);
                }
                anCode[iCode++] = 0x3CA00000 | ((u32)nAddress >> 16);
                anCode[iCode++] = 0x60A50000 | ((u32)nAddress & 0xFFFF);
                anCode[iCode++] = 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[iCode]) & 0x03FFFFFC) | 1;
            } else {
                // TODO: wrong codegen
                iCode += (var_r17 ? 4 : 2) + 1;
            }
            if ((pCPU->nFlagCODE & 1) && anCode == NULL && pFunction->pfCode == NULL) {
                iCode += 12;
                *pnAddress = pFunction->nAddress1 + 4;
            }
            if (var_r17) {
                if (anCode != NULL) {
                    anCode[iCode++] = 0x38C0FFFF;
                    anCode[iCode++] = 0x90C30000 + OFFSETOF(pCPU, nWaitPC);
                } else {
                    iCode += 2;
                }
            }
        }

        if (update) {
            pCPU->nOptimize.addr_last = -1;
            if (anCode != NULL) {
                anCode[iUpdate] = 0x42800000 | (((iCode - iUpdate) * 4) & 0xFFFC);
            }
        }

        *piCode = iCode;
        if (anCode != NULL) {
            if (var_r17) {
                pCPU->nOptimize.destGPR_check = 0;
                pCPU->nOptimize.destFPR_check = 0;
                pCPU->nOptimize.checkNext = 0;
            } else if (pCPU->nOptimize.destGPR_check == 2) {
                pCPU->nOptimize.destGPR_check = 1;
                pCPU->nOptimize.destFPR_check = 0;
            } else if (pCPU->nOptimize.destFPR_check == 2) {
                pCPU->nOptimize.destFPR_check = 1;
                pCPU->nOptimize.destGPR_check = 0;
            } else {
                pCPU->nOptimize.destGPR_check = 0;
                pCPU->nOptimize.destFPR_check = 0;
            }
        }

        if (var_r17 || pCPU->nOptimize.addr_check == 0) {
            pCPU->nOptimize.addr_last = -1;
        }

        return true;
    } else {
        return false;
    }
}

#pragma optimization_level reset

//! TODO: remove NO_INLINE once this is matched
static bool fn_80031D4C(Cpu* pCPU, CpuFunction* pFunction, s32 unknown) NO_INLINE { return false; }

/**
 * @brief Creates a new recompiled function block.
 *
 * @param pCPU The emulated VR4300.
 * @param ppFunction A pointer to an already recompiled function, or one that has been created.
 * @param nAddressN64 The N64 address of the function to find or create.
 * @return bool true on success, false otherwise.
 */
static bool cpuMakeFunction(Cpu* pCPU, CpuFunction** ppFunction, s32 nAddressN64) {
    s32 iCode;
    s32 iCode0;
    s32 pad;
    s32 iJump;
    s32 iCheck;
    s32 firstTime;
    s32 kill_value;
    s32 memory_used;
    s32 codeMemory;
    s32 blockMemory;
    s32* chunkMemory;
    s32* anCode;
    s32 nAddress;
    CpuFunction* pFunction;
    CpuJump aJump[1024];

    firstTime = 1;
    if (!cpuFindFunction(pCPU, nAddressN64, &pFunction)) {
        return false;
    }

#if VERSION >= MK64_J
    if (fn_80031D4C(pCPU, pFunction, 1)) {
        if (ppFunction != NULL) {
            *ppFunction = pFunction;
        }

        return true;
    }
#endif

    if (pFunction->pfCode == NULL) {
        libraryTestFunction(SYSTEM_LIBRARY(gpSystem), pFunction);
        pFunction->nCountJump = 0;
        pFunction->aJump = aJump;
        pCPU->nFlagRAM = 0x20000000;
        pCPU->nFlagCODE = 0;
        pFunction->callerID_total = 0;
        pFunction->callerID_flag = 0xB;
        pCPU->nOptimize.validCheck = 1;
        pCPU->nOptimize.checkNext = 0;

        iCode = 0;
        nAddress = pFunction->nAddress0;
        while (nAddress <= pFunction->nAddress1) {
            if (!cpuGetPPC(pCPU, &nAddress, pFunction, NULL, &iCode, false)) {
                return false;
            }
        }

        iCode0 = iCode;
        codeMemory = iCode * sizeof(s32);
        memory_used = codeMemory;

        iCheck = pFunction->callerID_total;
        if (iCheck != 0) {
            blockMemory = iCheck * sizeof(CpuCallerID);
            memory_used += blockMemory;
        } else {
            blockMemory = 0;
        }

        if (pFunction->nCountJump > 0) {
            memory_used += pFunction->nCountJump * sizeof(CpuJump);
        }

        while (true) {
            if (cpuHeapTake(&chunkMemory, pCPU, pFunction, memory_used)) {
                break;
            }

            if (firstTime) {
                firstTime = 0;
                kill_value = pCPU->survivalTimer - 300;
            } else {
                kill_value += 95;
                if (kill_value > pCPU->survivalTimer - 10) {
                    kill_value = pCPU->survivalTimer - 10;
                }
            }

            treeForceCleanUp(pCPU, pFunction, kill_value);
        }

        anCode = chunkMemory;
        if (blockMemory != 0) {
            pFunction->block = (CpuCallerID*)((u8*)chunkMemory + codeMemory);
            treeCallerInit(pFunction->block, iCheck);
        }

        pCPU->nFlagRAM = 0x20000000;
        pCPU->nFlagCODE = 0;
        pFunction->callerID_total = 0;
        pFunction->callerID_flag = 0x16;
        pCPU->nOptimize.checkNext = 0;
        pCPU->nOptimize.destGPR_check = 0;
        pCPU->nOptimize.destFPR_check = 0;

        iCode = 0;
        nAddress = pFunction->nAddress0;
        while (nAddress <= pFunction->nAddress1) {
            if (!cpuGetPPC(pCPU, &nAddress, pFunction, anCode, &iCode, false)) {
                return false;
            }
        }

        if (iCode > iCode0) {
            return false;
        }

        // not `cpuCompileNOP`?
        while (iCode < iCode0) {
            anCode[iCode++] = 0x60000000;
        }

        pFunction->callerID_flag = 0x21;
        pFunction->pfCode = anCode;
        DCStoreRange(pFunction->pfCode, iCode * 4);
        ICInvalidateRange(pFunction->pfCode, iCode * 4);

        if (pFunction->nCountJump > 0) {
            if (pFunction->nCountJump >= 0x400) {
                return false;
            }

            pFunction->aJump = (CpuJump*)((u8*)chunkMemory + codeMemory + blockMemory);
            for (iJump = 0; iJump < pFunction->nCountJump; iJump++) {
                pFunction->aJump[iJump].nOffsetHost = aJump[iJump].nOffsetHost;
                pFunction->aJump[iJump].nAddressN64 = aJump[iJump].nAddressN64;
            }
        } else {
            pFunction->aJump = NULL;
        }

        pFunction->memory_size = memory_used;
        pCPU->gTree->total_memory += memory_used;

#if VERSION >= MK64_J
        if (fn_80031D4C(pCPU, pFunction, 0)) {}
#endif
    }

    if (ppFunction != NULL) {
        *ppFunction = pFunction;
    }

    return true;
}

/**
 * @brief Searches the recompiled block cache for an address, or creates a new block if one cannot be found.
 *
 * @param pCPU The emulated VR4300.
 * @param nAddressN64 N64 code address to search for.
 * @param pnAddressGCN A pointer to set the found PPC code to.
 * @return bool true on success, false otherwise.
 */
static bool cpuFindAddress(Cpu* pCPU, s32 nAddressN64, s32* pnAddressGCN) {
    s32 iJump;
    s32 iCode;
    s32 nAddress;
    CpuFunction* pFunction;
    s32 pad;

    if (pCPU->nMode & 0x20) {
        pCPU->nMode &= ~0x20;
    }

    if (cpuFindCachedAddress(pCPU, nAddressN64, pnAddressGCN)) {
        return true;
    }

    if ((pFunction = pCPU->pFunctionLast) == NULL || nAddressN64 < pFunction->nAddress0 ||
        pFunction->nAddress1 < nAddressN64) {
        if (!cpuMakeFunction(pCPU, &pFunction, nAddressN64)) {
            return false;
        }
    }

    for (iJump = 0; iJump < pFunction->nCountJump; iJump++) {
        if (pFunction->aJump[iJump].nAddressN64 == nAddressN64) {
            *pnAddressGCN = (s32)((s32*)pFunction->pfCode + pFunction->aJump[iJump].nOffsetHost);
            if (pFunction->timeToLive > 0) {
                pFunction->timeToLive = pCPU->survivalTimer;
            }
            cpuMakeCachedAddress(pCPU, nAddressN64, *pnAddressGCN, pFunction);
            return true;
        }
    }

    pCPU->nFlagRAM = 0x20000000;
    pCPU->nFlagCODE = 0;
    pFunction->callerID_flag = 0x21;
    iCode = 0;
    if (pFunction->nAddress0 != nAddressN64) {
        pFunction->timeToLive = 0;
    }

    nAddress = pFunction->nAddress0;
    while (nAddress <= pFunction->nAddress1) {
        if (nAddress == nAddressN64) {
            *pnAddressGCN = (s32)((s32*)pFunction->pfCode + iCode);
            if (pFunction->timeToLive > 0) {
                pFunction->timeToLive = pCPU->survivalTimer;
            }
            cpuMakeCachedAddress(pCPU, nAddressN64, *pnAddressGCN, pFunction);
            return true;
        }
        if (!cpuGetPPC(pCPU, &nAddress, pFunction, NULL, &iCode, false)) {
            return false;
        }
    }

    return false;
}

static inline bool cpuNoBranchTo(CpuFunction* pFunction, s32 addressN64) {
    s32 i;

    for (i = 0; i < pFunction->nCountJump; i++) {
        if (pFunction->aJump[i].nAddressN64 == addressN64) {
            return false;
        }
    }

    return true;
}

static bool cpuCutStoreLoad(Cpu* pCPU, s32 currentAddress, s32 source) {
    if (pCPU->nOptimize.validCheck == 0) {
        return false;
    }

    if (pCPU->nOptimize.destGPR_check == 0) {
        return false;
    }

    if (source != pCPU->nOptimize.destGPR) {
        return false;
    }

    if (!cpuNoBranchTo(pCPU->pFunctionLast, currentAddress)) {
        pCPU->nOptimize.destGPR_check = 0;
        return false;
    }

    pCPU->nOptimize.destGPR_check = 0;
    return true;
}

static bool cpuCutStoreLoadF(Cpu* pCPU, s32 currentAddress, s32 source) {
    if (pCPU->nOptimize.validCheck == 0) {
        return false;
    }

    if (pCPU->nOptimize.destFPR_check == 0) {
        return false;
    }

    if (source != pCPU->nOptimize.destFPR) {
        return false;
    }

    if (!cpuNoBranchTo(pCPU->pFunctionLast, currentAddress)) {
        pCPU->nOptimize.destFPR_check = 0;
        return false;
    }

    pCPU->nOptimize.destFPR_check = 0;
    return true;
}

static bool cpuStackOffset(Cpu* pCPU, s32 currentAddress, s32* anCode, s32 source, s32 target) {
    if (anCode == NULL) {
        return false;
    }

    if (pCPU->nOptimize.validCheck == 0) {
        return false;
    }

    if (!cpuNoBranchTo(pCPU->pFunctionLast, currentAddress)) {
        return false;
    }

    pCPU->nOptimize.addr_check = 1;

    if (source == target) {
        pCPU->nOptimize.addr_last = -1;
        return false;
    }

    if (pCPU->nOptimize.addr_last != source) {
        pCPU->nOptimize.addr_last = source;
        return false;
    }

    return true;
}

static bool cpuNextInstruction(Cpu* pCPU, s32 addressN64, s32 opcode, s32* anCode, int* iCode) {
    if (anCode == NULL) {
        return false;
    }
    if (pCPU->nOptimize.validCheck == 0) {
        return false;
    }
    if (pCPU->nOptimize.checkNext != addressN64 - 4) {
        pCPU->nOptimize.checkNext = 0;
        return false;
    }
    pCPU->nOptimize.checkNext = 0;

    if (!cpuNoBranchTo(pCPU->pFunctionLast, addressN64)) {
        return false;
    }

    switch (MIPS_OP(opcode)) {
        case 0x0D: // ori
            if (pCPU->nOptimize.destGPR == MIPS_RS(opcode) && MIPS_RS(opcode) == MIPS_RT(opcode)) {
                if (pCPU->nOptimize.checkType != 0x3E8) {
                    return false;
                }
                anCode[*iCode - 1] = 0x60000000;
                anCode[(*iCode)++] = 0x60000000 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                     (pCPU->nOptimize.destGPR_mapping << 16) | MIPS_IMM_U16(opcode);
                anCode[(*iCode)++] = 0x60000000;
                anCode[(*iCode)++] = (0x90030000 | (pCPU->nOptimize.destGPR_mapping << 21)) +
                                     (OFFSETOF(pCPU, aGPR[MIPS_RT(opcode)]) + 4);
                pCPU->nOptimize.destGPR_check = 2;
                return true;
            }
            return false;
        case 0x09: // addiu
            if (pCPU->nOptimize.destGPR == MIPS_RS(opcode) && MIPS_RS(opcode) == MIPS_RT(opcode)) {
                if (pCPU->nOptimize.checkType != 0x3E8) {
                    return false;
                }
                anCode[*iCode - 1] = 0x60000000;
                anCode[(*iCode)++] = 0x38000000 | (pCPU->nOptimize.destGPR_mapping << 21) |
                                     (pCPU->nOptimize.destGPR_mapping << 16) | MIPS_IMM_U16(opcode);
                anCode[(*iCode)++] = 0x60000000;
                anCode[(*iCode)++] = (0x90030000 | (pCPU->nOptimize.destGPR_mapping << 21)) +
                                     (OFFSETOF(pCPU, aGPR[MIPS_RT(opcode)]) + 4);
                pCPU->nOptimize.destGPR_check = 2;
                return true;
            }
            return false;
        default:
            xlExit();
            break;
    }

    return false;
}

static void cpuRetraceCallback(u32 nCount) {
    SYSTEM_CPU(gpSystem)->nRetrace = nCount;

    if (__cpuRetraceCallback != NULL) {
        __cpuRetraceCallback(nCount);
    }
}

static inline s32 treeMemory(Cpu* pCPU) {
    if (pCPU->gTree != NULL) {
        return pCPU->gTree->total_memory;
    } else {
        return 0;
    }
}

static inline bool treeKillReason(Cpu* pCPU, s32* value) {
    if (pCPU->survivalTimer < 300) {
        return false;
    }
    if (pCPU->survivalTimer == 300) {
        *value = 1;
        return true;
    }
    if (pCPU->survivalTimer % 400 == 0 && treeMemory(pCPU) > 3250000) {
        *value = pCPU->survivalTimer - 200;
        return true;
    }

    return false;
}

static bool cpuExecuteUpdate(Cpu* pCPU, s32* pnAddressGCN, u32 nCount) {
    RspUpdateMode eModeUpdate;
    s32 nDelta;
    u32 nCounter;
    u32 nCompare;

    u32 nCounterDelta;
    CpuTreeRoot* root;

    if (!romUpdate(SYSTEM_ROM(gpSystem))) {
        return false;
    }

    eModeUpdate = ((pCPU->nMode & 0x80) && !gpSystem->bException) ? RUM_IDLE : RUM_NONE;

    if (!rspUpdate(SYSTEM_RSP(gpSystem), eModeUpdate)) {
        return false;
    }

    root = pCPU->gTree;
    treeTimerCheck(pCPU);
    if (pCPU->nRetrace == pCPU->nRetraceUsed && root->kill_number < 12) {
        if (treeKillReason(pCPU, &root->kill_limit)) {
            pCPU->survivalTimer++;
        }
        if (root->kill_limit != 0) {
            treeCleanUp(pCPU, root);
        }
    }

    if (nCount > pCPU->nTickLast) {
        nCounterDelta = (f32)((nCount - pCPU->nTickLast) * 4);
    } else {
        nCounterDelta = (f32)(((-1 - pCPU->nTickLast) + nCount) * 4);
    }

    if ((pCPU->nMode & 0x40) && pCPU->nRetraceUsed != pCPU->nRetrace) {
        if (viForceRetrace(SYSTEM_VI(gpSystem))) {
            nDelta = pCPU->nRetrace - pCPU->nRetraceUsed;
            if (nDelta < 0) {
                nDelta = -nDelta;
            }

            if (nDelta < 4) {
                pCPU->nRetraceUsed++;
            } else {
                pCPU->nRetraceUsed = pCPU->nRetrace;
            }
        }
    }

    if (pCPU->nMode & 1) {
        nCounter = pCPU->anCP0[9];
        nCompare = pCPU->anCP0[11];
        if ((nCounter <= nCompare && nCounter + nCounterDelta >= nCompare) ||
            (nCounter >= nCompare && nCounter + nCounterDelta >= nCompare && nCounter + nCounterDelta < nCounter)) {
            pCPU->nMode &= ~1;
            xlObjectEvent(gpSystem, 0x1000, (void*)3);
        }
    }
    pCPU->anCP0[9] += nCounterDelta;

    if ((pCPU->nMode & 8) && !(pCPU->nMode & 4) && gpSystem->bException) {
        if (!systemCheckInterrupts(gpSystem)) {
            return false;
        }
    }

    if (pCPU->nMode & 4) {
        pCPU->nMode &= ~0x84;
        if (!cpuFindAddress(pCPU, pCPU->nPC, pnAddressGCN)) {
            return false;
        }
    }
    return true;
}

static bool cpuCompile_DSLLV(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 16;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x9421FFE8; // stwu    r1,-24(r1)
    compile[count++] = 0x70E7003F; // andi.   r7,r7,63
    compile[count++] = 0x91210010; // stw     r9,16(r1)
    compile[count++] = 0x21270020; // subfic  r9,r7,32
    compile[count++] = 0x91010008; // stw     r8,8(r1)
    compile[count++] = 0x7CA53830; // slw     r5,r5,r7
    compile[count++] = 0x7CC84C30; // srw     r8,r6,r9
    compile[count++] = 0x7CA54378; // or      r5,r5,r8
    compile[count++] = 0x3927FFE0; // addi    r9,r7,-32
    compile[count++] = 0x7CC84830; // slw     r8,r6,r9
    compile[count++] = 0x7CA54378; // or      r5,r5,r8
    compile[count++] = 0x7CC63830; // slw     r6,r6,r7
    compile[count++] = 0x81010008; // lwz     r8,8(r1)
    compile[count++] = 0x81210010; // lwz     r9,16(r1)
    compile[count++] = 0x38210018; // addi    r1,r1,24
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_DSRLV(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 16;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x9421FFE8; // stwu    r1,-24(r1)
    compile[count++] = 0x70E7003F; // andi.   r7,r7,63
    compile[count++] = 0x91210010; // stw     r9,16(r1)
    compile[count++] = 0x21270020; // subfic  r9,r7,32
    compile[count++] = 0x91010008; // stw     r8,8(r1)
    compile[count++] = 0x7CC63C30; // srw     r6,r6,r7
    compile[count++] = 0x7CA84830; // slw     r8,r5,r9
    compile[count++] = 0x7CC64378; // or      r6,r6,r8
    compile[count++] = 0x3927FFE0; // addi    r9,r7,-32
    compile[count++] = 0x7CA84C30; // srw     r8,r5,r9
    compile[count++] = 0x7CC64378; // or      r6,r6,r8
    compile[count++] = 0x7CA53C30; // srw     r5,r5,r7
    compile[count++] = 0x81010008; // lwz     r8,8(r1)
    compile[count++] = 0x81210010; // lwz     r9,16(r1)
    compile[count++] = 0x38210018; // addi    r1,r1,24
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_DSRAV(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 17;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x9421FFE8; // stwu    r1,-24(r1)
    compile[count++] = 0x70E7003F; // andi.   r7,r7,63
    compile[count++] = 0x91210010; // stw     r9,16(r1)
    compile[count++] = 0x21270020; // subfic  r9,r7,32
    compile[count++] = 0x91010008; // stw     r8,8(r1)
    compile[count++] = 0x7CC63C30; // srw     r6,r6,r7
    compile[count++] = 0x7CA84830; // slw     r8,r5,r9
    compile[count++] = 0x7CC64378; // or      r6,r6,r8
    compile[count++] = 0x3527FFE0; // addic.  r9,r7,-32
    compile[count++] = 0x7CA84E30; // sraw    r8,r5,r9
    compile[count++] = 0x40810008; // ble-    0x8
    compile[count++] = 0x61060000; // ori     r6,r8,0
    compile[count++] = 0x7CA53E30; // sraw    r5,r5,r7
    compile[count++] = 0x81010008; // lwz     r8,8(r1)
    compile[count++] = 0x81210010; // lwz     r9,16(r1)
    compile[count++] = 0x38210018; // addi    r1,r1,24
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_DMULT(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 53;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x39200000; // li      r9,0
    compile[count++] = 0x39400000; // li      r10,0
    compile[count++] = 0x39800040; // li      r12,64
    compile[count++] = 0x39600001; // li      r11,1
    compile[count++] = 0x2C050000; // cmpwi   r5,0
    compile[count++] = 0x40800014; // bge-    0x14
    compile[count++] = 0x7CC630F8; // not     r6,r6
    compile[count++] = 0x7CA528F8; // not     r5,r5
    compile[count++] = 0x7CC65814; // addc    r6,r6,r11
    compile[count++] = 0x7CA54914; // adde    r5,r5,r9
    compile[count++] = 0x2C070000; // cmpwi   r7,0
    compile[count++] = 0x40800014; // bge-    0x14
    compile[count++] = 0x7D0840F8; // not     r8,r8
    compile[count++] = 0x7CE738F8; // not     r7,r7
    compile[count++] = 0x7D085814; // addc    r8,r8,r11
    compile[count++] = 0x7CE74914; // adde    r7,r7,r9
    compile[count++] = 0x710B0001; // andi.   r11,r8,1
    compile[count++] = 0x41820018; // beq-    0x18
    compile[count++] = 0x39600000; // li      r11,0
    compile[count++] = 0x7D4A3014; // addc    r10,r10,r6
    compile[count++] = 0x7D292914; // adde    r9,r9,r5
    compile[count++] = 0x7D6B5914; // adde    r11,r11,r11
    compile[count++] = 0x42800008; // bc      20,lt,0x8
    compile[count++] = 0x39600000; // li      r11,0
    compile[count++] = 0x5508F87E; // srwi    r8,r8,1
    compile[count++] = 0x50E8F800; // rlwimi  r8,r7,31,0,0
    compile[count++] = 0x54E7F87E; // srwi    r7,r7,1
    compile[count++] = 0x5147F800; // rlwimi  r7,r10,31,0,0
    compile[count++] = 0x554AF87E; // srwi    r10,r10,1
    compile[count++] = 0x512AF800; // rlwimi  r10,r9,31,0,0
    compile[count++] = 0x5529F87E; // srwi    r9,r9,1
    compile[count++] = 0x5169F800; // rlwimi  r9,r11,31,0,0
    compile[count++] = 0x556BF87E; // srwi    r11,r11,1
    compile[count++] = 0x398CFFFF; // addi    r12,r12,-1
    compile[count++] = 0x2C0C0000; // cmpwi   r12,0
    compile[count++] = 0x4082FFB4; // bne+    0xFFFFFFB4
    compile[count++] = 0x39600001; // li      r11,1
    compile[count++] = 0x7DCE7A78; // xor     r14,r14,r15
    compile[count++] = 0x2C0E0000; // cmpwi   r14,0
    compile[count++] = 0x40800024; // bge-    0x24
    compile[count++] = 0x7D0840F8; // not     r8,r8
    compile[count++] = 0x7CE738F8; // not     r7,r7
    compile[count++] = 0x7D4A50F8; // not     r10,r10
    compile[count++] = 0x7D2948F8; // not     r9,r9
    compile[count++] = 0x7D085814; // addc    r8,r8,r11
    compile[count++] = 0x7CE76114; // adde    r7,r7,r12
    compile[count++] = 0x7D4A6114; // adde    r10,r10,r12
    compile[count++] = 0x7D296114; // adde    r9,r9,r12
    compile[count++] = 0x91030004 + OFFSETOF(pCPU, nLo); // stw     r8,4(r3)
    compile[count++] = 0x90E30000 + OFFSETOF(pCPU, nLo); // stw     r7,0(r3)
    compile[count++] = 0x91430004 + OFFSETOF(pCPU, nHi); // stw     r10,4(r3)
    compile[count++] = 0x91230000 + OFFSETOF(pCPU, nHi); // stw     r9,0(r3)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_DMULTU(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 28;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x39200000; // li      r9,0
    compile[count++] = 0x39400000; // li      r10,0
    compile[count++] = 0x39800040; // li      r12,64
    compile[count++] = 0x710B0001; // andi.   r11,r8,1
    compile[count++] = 0x41820018; // beq-    0x18
    compile[count++] = 0x39600000; // li      r11,0
    compile[count++] = 0x7D4A3014; // addc    r10,r10,r6
    compile[count++] = 0x7D292914; // adde    r9,r9,r5
    compile[count++] = 0x7D6B5914; // adde    r11,r11,r11
    compile[count++] = 0x42800008; // bc      20,lt,0x8
    compile[count++] = 0x39600000; // li      r11,0
    compile[count++] = 0x5508F87E; // srwi    r8,r8,1
    compile[count++] = 0x50E8F800; // rlwimi  r8,r7,31,0,0
    compile[count++] = 0x54E7F87E; // srwi    r7,r7,1
    compile[count++] = 0x5147F800; // rlwimi  r7,r10,31,0,0
    compile[count++] = 0x554AF87E; // srwi    r10,r10,1
    compile[count++] = 0x512AF800; // rlwimi  r10,r9,31,0,0
    compile[count++] = 0x5529F87E; // srwi    r9,r9,1
    compile[count++] = 0x5169F800; // rlwimi  r9,r11,31,0,0
    compile[count++] = 0x556BF87E; // srwi    r11,r11,1
    compile[count++] = 0x398CFFFF; // addi    r12,r12,-1
    compile[count++] = 0x2C0C0000; // cmpwi   r12,0
    compile[count++] = 0x4082FFB4; // bne+    0xFFFFFFB4
    compile[count++] = 0x91030004 + OFFSETOF(pCPU, nLo); // stw     r8,4(r3)
    compile[count++] = 0x90E30000 + OFFSETOF(pCPU, nLo); // stw     r7,0(r3)
    compile[count++] = 0x91430004 + OFFSETOF(pCPU, nHi); // stw     r10,4(r3)
    compile[count++] = 0x91230000 + OFFSETOF(pCPU, nHi); // stw     r9,0(r3)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_DDIV(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 64;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x38A00040; // li      r5,64
    compile[count++] = 0x38C00000; // li      r6,0
    compile[count++] = 0x38E00000; // li      r7,0
    compile[count++] = 0x39800001; // li      r12,1
    compile[count++] = 0x2C080000; // cmpwi   r8,0
    compile[count++] = 0x40800014; // bge-    0x14
    compile[count++] = 0x7D2948F8; // not     r9,r9
    compile[count++] = 0x7D0840F8; // not     r8,r8
    compile[count++] = 0x7D296014; // addc    r9,r9,r12
    compile[count++] = 0x7D083114; // adde    r8,r8,r6
    compile[count++] = 0x2C0A0000; // cmpwi   r10,0
    compile[count++] = 0x40800014; // bge-    0x14
    compile[count++] = 0x7D6B58F8; // not     r11,r11
    compile[count++] = 0x7D4A50F8; // not     r10,r10
    compile[count++] = 0x7D6B6014; // addc    r11,r11,r12
    compile[count++] = 0x7D4A3114; // adde    r10,r10,r6
    compile[count++] = 0x3D80FFFF; // lis     r12,-1
    compile[count++] = 0x618CFFFE; // ori     r12,r12,65534
    compile[count++] = 0x2C060000; // cmpwi   r6,0
    compile[count++] = 0x4180002C; // blt-    0x2C
    compile[count++] = 0x54C6083C; // slwi    r6,r6,1
    compile[count++] = 0x50E60FFE; // rlwimi  r6,r7,1,31,31
    compile[count++] = 0x54E7083C; // slwi    r7,r7,1
    compile[count++] = 0x51070FFE; // rlwimi  r7,r8,1,31,31
    compile[count++] = 0x5508083C; // slwi    r8,r8,1
    compile[count++] = 0x51280FFE; // rlwimi  r8,r9,1,31,31
    compile[count++] = 0x5529083C; // slwi    r9,r9,1
    compile[count++] = 0x7CEB3810; // subfc   r7,r11,r7
    compile[count++] = 0x7CCA3110; // subfe   r6,r10,r6
    compile[count++] = 0x42800028; // bc      20,lt,0x28
    compile[count++] = 0x54C6083C; // slwi    r6,r6,1
    compile[count++] = 0x50E60FFE; // rlwimi  r6,r7,1,31,31
    compile[count++] = 0x54E7083C; // slwi    r7,r7,1
    compile[count++] = 0x51070FFE; // rlwimi  r7,r8,1,31,31
    compile[count++] = 0x5508083C; // slwi    r8,r8,1
    compile[count++] = 0x51280FFE; // rlwimi  r8,r9,1,31,31
    compile[count++] = 0x5529083C; // slwi    r9,r9,1
    compile[count++] = 0x7CE75814; // addc    r7,r7,r11
    compile[count++] = 0x7CC65114; // adde    r6,r6,r10
    compile[count++] = 0x2C060000; // cmpwi   r6,0
    compile[count++] = 0x4180000C; // blt-    0xC
    compile[count++] = 0x61290001; // ori     r9,r9,1
    compile[count++] = 0x42800008; // bc      20,lt,0x8
    compile[count++] = 0x7D296038; // and     r9,r9,r12
    compile[count++] = 0x38A5FFFF; // addi    r5,r5,-1
    compile[count++] = 0x2C050000; // cmpwi   r5,0
    compile[count++] = 0x4082FF90; // bne+    0xFFFFFF90
    compile[count++] = 0x2C060000; // cmpwi   r6,0
    compile[count++] = 0x4080000C; // bge-    0xC
    compile[count++] = 0x7CE75814; // addc    r7,r7,r11
    compile[count++] = 0x7CC65114; // adde    r6,r6,r10
    compile[count++] = 0x39800001; // li      r12,1
    compile[count++] = 0x7DCE7A78; // xor     r14,r14,r15
    compile[count++] = 0x2C0E0000; // cmpwi   r14,0
    compile[count++] = 0x40800014; // bge-    0x14
    compile[count++] = 0x7D2948F8; // not     r9,r9
    compile[count++] = 0x7D0840F8; // not     r8,r8
    compile[count++] = 0x7D296014; // addc    r9,r9,r12
    compile[count++] = 0x7D082914; // adde    r8,r8,r5
    compile[count++] = 0x91030000 + OFFSETOF(pCPU, nLo); // stw     r8,0(r3)
    compile[count++] = 0x91230004 + OFFSETOF(pCPU, nLo); // stw     r9,4(r3)
    compile[count++] = 0x90C30000 + OFFSETOF(pCPU, nHi); // stw     r6,0(r3)
    compile[count++] = 0x90E30004 + OFFSETOF(pCPU, nHi); // stw     r7,4(r3)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_DDIVU(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 43;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x38A00040; // li      r5,64
    compile[count++] = 0x38C00000; // li      r6,0
    compile[count++] = 0x38E00000; // li      r7,0
    compile[count++] = 0x3D80FFFF; // lis     r12,-1
    compile[count++] = 0x618CFFFE; // ori     r12,r12,65534
    compile[count++] = 0x2C060000; // cmpwi   r6,0
    compile[count++] = 0x4180002C; // blt-    0x2C
    compile[count++] = 0x54C6083C; // slwi    r6,r6,1
    compile[count++] = 0x50E60FFE; // rlwimi  r6,r7,1,31,31
    compile[count++] = 0x54E7083C; // slwi    r7,r7,1
    compile[count++] = 0x51070FFE; // rlwimi  r7,r8,1,31,31
    compile[count++] = 0x5508083C; // slwi    r8,r8,1
    compile[count++] = 0x51280FFE; // rlwimi  r8,r9,1,31,31
    compile[count++] = 0x5529083C; // slwi    r9,r9,1
    compile[count++] = 0x7CEB3810; // subfc   r7,r11,r7
    compile[count++] = 0x7CCA3110; // subfe   r6,r10,r6
    compile[count++] = 0x42800028; // bc      20,lt,0x28
    compile[count++] = 0x54C6083C; // slwi    r6,r6,1
    compile[count++] = 0x50E60FFE; // rlwimi  r6,r7,1,31,31
    compile[count++] = 0x54E7083C; // slwi    r7,r7,1
    compile[count++] = 0x51070FFE; // rlwimi  r7,r8,1,31,31
    compile[count++] = 0x5508083C; // slwi    r8,r8,1
    compile[count++] = 0x51280FFE; // rlwimi  r8,r9,1,31,31
    compile[count++] = 0x5529083C; // slwi    r9,r9,1
    compile[count++] = 0x7CE75814; // addc    r7,r7,r11
    compile[count++] = 0x7CC65114; // adde    r6,r6,r10
    compile[count++] = 0x2C060000; // cmpwi   r6,0
    compile[count++] = 0x4180000C; // blt-    0xC
    compile[count++] = 0x61290001; // ori     r9,r9,1
    compile[count++] = 0x42800008; // bc      20,lt,0x8
    compile[count++] = 0x7D296038; // and     r9,r9,r12
    compile[count++] = 0x38A5FFFF; // addi    r5,r5,-1
    compile[count++] = 0x2C050000; // cmpwi   r5,0
    compile[count++] = 0x4082FF90; // bne+    0xFFFFFF90
    compile[count++] = 0x2C060000; // cmpwi   r6,0
    compile[count++] = 0x4080000C; // bge-    0xC
    compile[count++] = 0x7CE75814; // addc    r7,r7,r11
    compile[count++] = 0x7CC65114; // adde    r6,r6,r10
    compile[count++] = 0x91030000 + OFFSETOF(pCPU, nLo); // stw     r8,0(r3)
    compile[count++] = 0x91230004 + OFFSETOF(pCPU, nLo); // stw     r9,4(r3)
    compile[count++] = 0x90C30000 + OFFSETOF(pCPU, nHi); // stw     r6,0(r3)
    compile[count++] = 0x90E30004 + OFFSETOF(pCPU, nHi); // stw     r7,4(r3)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static inline bool cpuCompile_DADD(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 3;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x7CA53814; // addc    r5,r5,r7
    compile[count++] = 0x7CC64114; // adde    r6,r6,r8
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static inline bool cpuCompile_DADDU(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 3;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x7CA53814; // addc    r5,r5,r7
    compile[count++] = 0x7CC64114; // adde    r6,r6,r8
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static inline bool cpuCompile_DSUB(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 3;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x7CA72914; // adde    r5,r7,r5
    compile[count++] = 0x7CC83014; // addc    r6,r8,r6
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static inline bool cpuCompile_DSUBU(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 3;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x7CA72914; // adde    r5,r7,r5
    compile[count++] = 0x7CC83014; // addc    r6,r8,r6
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_S_SQRT(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 36;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0xC0030000 + (OFFSETOF(pCPU, aGPR) + 4); // lfs     f0,0(r3)
    compile[count++] = 0xFC010040; // fcmpo   cr0,f1,f0
    compile[count++] = 0x40810078; // ble-    0x78
    compile[count++] = 0xFC400834; // frsqrte f2,f1
    compile[count++] = 0x3CA03FE0; // lis     r5,16352
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0xC8830000 + OFFSETOF(pCPU, aGPR); // lfd     f4,0(r3)
    compile[count++] = 0x3CA04008; // lis     r5,16392
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0xC8630000 + OFFSETOF(pCPU, aGPR); // lfd     f3,0(r3)
    compile[count++] = 0xFCA400B2; // fmul    f5,f4,f2
    compile[count++] = 0xFCC200B2; // fmul    f6,f2,f2
    compile[count++] = 0xFCC101B2; // fmul    f6,f1,f6
    compile[count++] = 0xFCC33028; // fsub    f6,f3,f6
    compile[count++] = 0xFC4501B2; // fmul    f2,f5,f6
    compile[count++] = 0xFCA400B2; // fmul    f5,f4,f2
    compile[count++] = 0xFCC200B2; // fmul    f6,f2,f2
    compile[count++] = 0xFCC101B2; // fmul    f6,f1,f6
    compile[count++] = 0xFCC33028; // fsub    f6,f3,f6
    compile[count++] = 0xFC4501B2; // fmul    f2,f5,f6
    compile[count++] = 0xFCA400B2; // fmul    f5,f4,f2
    compile[count++] = 0xFCC200B2; // fmul    f6,f2,f2
    compile[count++] = 0xFCC101B2; // fmul    f6,f1,f6
    compile[count++] = 0xFCC33028; // fsub    f6,f3,f6
    compile[count++] = 0xFC4501B2; // fmul    f2,f5,f6
    compile[count++] = 0xFCA400B2; // fmul    f5,f4,f2
    compile[count++] = 0xFCC200B2; // fmul    f6,f2,f2
    compile[count++] = 0xFCC101B2; // fmul    f6,f1,f6
    compile[count++] = 0xFCC33028; // fsub    f6,f3,f6
    compile[count++] = 0xFC4501B2; // fmul    f2,f5,f6
    compile[count++] = 0xFCC100B2; // fmul    f6,f1,f2
    compile[count++] = 0xFC203018; // frsp    f1,f6
    compile[count++] = 0x38A00000; // li      r5,0
    compile[count++] = 0x90A30000 + (OFFSETOF(pCPU, aGPR) + 4); // stw     r5,0(r3)
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_D_SQRT(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 48;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0xC8030000 + OFFSETOF(pCPU, aGPR); // lfd     f0,0(r3)
    compile[count++] = 0xFC010040; // fcmpo   cr0,f1,f0
    compile[count++] = 0x40810068; // ble-    0x68
    compile[count++] = 0xFC400834; // frsqrte f2,f1
    compile[count++] = 0x3CA03FE0; // lis     r5,16352
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0xC8830000 + OFFSETOF(pCPU, aGPR); // lfd     f4,0(r3)
    compile[count++] = 0x3CA04008; // lis     r5,16392
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0xC8630000 + OFFSETOF(pCPU, aGPR); // lfd     f3,0(r3)
    compile[count++] = 0xFC0200B2; // fmul    f0,f2,f2
    compile[count++] = 0xFC4400B2; // fmul    f2,f4,f2
    compile[count++] = 0xFC01183C; // fnmsub  f0,f1,f0,f3
    compile[count++] = 0xFC420032; // fmul    f2,f2,f0
    compile[count++] = 0xFC0200B2; // fmul    f0,f2,f2
    compile[count++] = 0xFC4400B2; // fmul    f2,f4,f2
    compile[count++] = 0xFC01183C; // fnmsub  f0,f1,f0,f3
    compile[count++] = 0xFC420032; // fmul    f2,f2,f0
    compile[count++] = 0xFC0200B2; // fmul    f0,f2,f2
    compile[count++] = 0xFC4400B2; // fmul    f2,f4,f2
    compile[count++] = 0xFC01183C; // fnmsub  f0,f1,f0,f3
    compile[count++] = 0xFC420032; // fmul    f2,f2,f0
    compile[count++] = 0xFC0200B2; // fmul    f0,f2,f2
    compile[count++] = 0xFC4400B2; // fmul    f2,f4,f2
    compile[count++] = 0xFC01183C; // fnmsub  f0,f1,f0,f3
    compile[count++] = 0xFC020032; // fmul    f0,f2,f0
    compile[count++] = 0xFC210032; // fmul    f1,f1,f0
    compile[count++] = 0x42800044; // bc      20,lt,0x44
    compile[count++] = 0xFC010000; // fcmpu   cr0,f1,f0
    compile[count++] = 0x4082000C; // bne-    0xC
    compile[count++] = 0xFC200090; // fmr     f1,f0
    compile[count++] = 0x42800034; // bc      20,lt,0x34
    compile[count++] = 0xFC010000; // fcmpu   cr0,f1,f0
    compile[count++] = 0x41820020; // beq-    0x20
    compile[count++] = 0x3CA07FFF; // lis     r5,32767
    compile[count++] = 0x60A5FFFF; // ori     r5,r5,65535
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0x3CA0E000; // lis     r5,-8192
    compile[count++] = 0x90A30004 + OFFSETOF(pCPU, aGPR); // stw     r5,4(r3)
    compile[count++] = 0xC0230000 + OFFSETOF(pCPU, aGPR); // lfs     f1,0(r3)
    compile[count++] = 0x42800010; // bc      20,lt,0x10
    compile[count++] = 0x3CA07FF0; // lis     r5,32752
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0xC0230000 + OFFSETOF(pCPU, aGPR); // lfs     f1,0(r3)
    compile[count++] = 0x38A00000; // li      r5,0
    compile[count++] = 0x90A30004 + OFFSETOF(pCPU, aGPR); // stw     r5,4(r3)
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_W_CVT_SD(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 14;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x6CA58000; // xoris   r5,r5,32768
    compile[count++] = 0x3CC04330; // lis     r6,17200
    compile[count++] = 0x90A30004 + OFFSETOF(pCPU, aGPR); // stw     r5,4(r3)
    compile[count++] = 0x90C30000 + OFFSETOF(pCPU, aGPR); // stw     r6,0(r3)
    compile[count++] = 0xC8030000 + OFFSETOF(pCPU, aGPR); // lfd     f0,0(r3)
    compile[count++] = 0x90C30000 + OFFSETOF(pCPU, aGPR); // stw     r6,0(r3)
    compile[count++] = 0x3CA08000; // lis     r5,-32768
    compile[count++] = 0x90A30004 + OFFSETOF(pCPU, aGPR); // stw     r5,4(r3)
    compile[count++] = 0xC8230000 + OFFSETOF(pCPU, aGPR); // lfd     f1,0(r3)
    compile[count++] = 0xFC200828; // fsub    f1,f0,f1
    compile[count++] = 0x38A00000; // li      r5,0
    compile[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR); // stw     r5,0(r3)
    compile[count++] = 0x90A30004 + OFFSETOF(pCPU, aGPR); // stw     r5,4(r3)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_L_CVT_SD(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 56;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x9421FFD0; // stwu    r1,-48(r1)
    compile[count++] = 0x91010008; // stw     r8,8(r1)
    compile[count++] = 0x91210010; // stw     r9,16(r1)
    compile[count++] = 0x91410018; // stw     r10,24(r1)
    compile[count++] = 0x91610020; // stw     r11,32(r1)
    compile[count++] = 0x91810028; // stw     r12,40(r1)
    compile[count++] = 0x9421FFF0; // stwu    r1,-16(r1)
    compile[count++] = 0x54A70001; // clrrwi. r7,r5,31
    compile[count++] = 0x4182000C; // beq-    0xC
    compile[count++] = 0x20C60000; // subfic  r6,r6,0
    compile[count++] = 0x7CA50190; // subfze  r5,r5
    compile[count++] = 0x7CA93379; // or.     r9,r5,r6
    compile[count++] = 0x39000000; // li      r8,0
    compile[count++] = 0x41820080; // beq-    0x80
    compile[count++] = 0x7CA90034; // cntlzw  r9,r5
    compile[count++] = 0x7CCA0034; // cntlzw  r10,r6
    compile[count++] = 0x552BD008; // rlwinm  r11,r9,26,0,4
    compile[count++] = 0x7D6BFE70; // srawi   r11,r11,31
    compile[count++] = 0x7D6B5038; // and     r11,r11,r10
    compile[count++] = 0x7D295A14; // add     r9,r9,r11
    compile[count++] = 0x21490020; // subfic  r10,r9,32
    compile[count++] = 0x3169FFE0; // addic   r11,r9,-32
    compile[count++] = 0x7CA54830; // slw     r5,r5,r9
    compile[count++] = 0x7CCC5430; // srw     r12,r6,r10
    compile[count++] = 0x7CA56378; // or      r5,r5,r12
    compile[count++] = 0x7CCC5830; // slw     r12,r6,r11
    compile[count++] = 0x7CA56378; // or      r5,r5,r12
    compile[count++] = 0x7CC64830; // slw     r6,r6,r9
    compile[count++] = 0x7D094050; // subf    r8,r9,r8
    compile[count++] = 0x54C9057E; // clrlwi  r9,r6,21
    compile[count++] = 0x2C090400; // cmpwi   r9,1024
    compile[count++] = 0x3908043E; // addi    r8,r8,1086
    compile[count++] = 0x4180001C; // blt-    0x1C
    compile[count++] = 0x4181000C; // bgt-    0xC
    compile[count++] = 0x54C90529; // rlwinm. r9,r6,0,20,20
    compile[count++] = 0x41820010; // beq-    0x10
    compile[count++] = 0x30C60800; // addic   r6,r6,2048
    compile[count++] = 0x7CA50194; // addze   r5,r5
    compile[count++] = 0x7D080194; // addze   r8,r8
    compile[count++] = 0x54C6A83E; // rotlwi  r6,r6,21
    compile[count++] = 0x50A6A814; // rlwimi  r6,r5,21,0,10
    compile[count++] = 0x54A5AB3E; // rlwinm  r5,r5,21,12,31
    compile[count++] = 0x5508A016; // slwi    r8,r8,20
    compile[count++] = 0x7D052B78; // or      r5,r8,r5
    compile[count++] = 0x7CE52B78; // or      r5,r7,r5
    compile[count++] = 0x90A10008; // stw     r5,8(r1)
    compile[count++] = 0x90C1000C; // stw     r6,12(r1)
    compile[count++] = 0xC8210008; // lfd     f1,8(r1)
    compile[count++] = 0x38210010; // addi    r1,r1,16
    compile[count++] = 0x81010008; // lwz     r8,8(r1)
    compile[count++] = 0x81210010; // lwz     r9,16(r1)
    compile[count++] = 0x81410018; // lwz     r10,24(r1)
    compile[count++] = 0x81610020; // lwz     r11,32(r1)
    compile[count++] = 0x81810028; // lwz     r12,40(r1)
    compile[count++] = 0x38210030; // addi    r1,r1,48
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_CEIL_W(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 13;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x9421FFE0; // stwu    r1,-32(r1)
    compile[count++] = 0xC8030000 + (OFFSETOF(pCPU, aFPR) & 0xFFFF); // lfd     f0,0(r3)
    compile[count++] = 0xFC010040; // fcmpo   cr0,f1,f0
    compile[count++] = 0x4081000C; // ble-    0xC
    compile[count++] = 0x38C00001; // li      r6,1
    compile[count++] = 0x42800008; // bc      20,lt,0x8
    compile[count++] = 0x38C00000; // li      r6,0
    compile[count++] = 0xFC20081E; // fctiwz  f1,f1
    compile[count++] = 0xD8210010; // stfd    f1,16(r1)
    compile[count++] = 0x80A10014; // lwz     r5,20(r1)
    compile[count++] = 0x7CA62A14; // add     r5,r6,r5
    compile[count++] = 0x38210020; // addi    r1,r1,32
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_FLOOR_W(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 13;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x9421FFE0; // stwu    r1,-32(r1)
    compile[count++] = 0xC8030000 + (OFFSETOF(pCPU, aFPR) & 0xFFFF); // lfd     f0,0(r3)
    compile[count++] = 0xFC010040; // fcmpo   cr0,f1,f0
    compile[count++] = 0x4180000C; // blt-    0xC
    compile[count++] = 0x38C00000; // li      r6,0
    compile[count++] = 0x42800008; // bc      20,lt,0x8
    compile[count++] = 0x38C00001; // li      r6,1
    compile[count++] = 0xFC20081E; // fctiwz  f1,f1
    compile[count++] = 0xD8210010; // stfd    f1,16(r1)
    compile[count++] = 0x80A10014; // lwz     r5,20(r1)
    compile[count++] = 0x7CA62850; // subf    r5,r6,r5
    compile[count++] = 0x38210020; // addi    r1,r1,32
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static inline bool cpuCompile_ROUND_W(s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 3;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0xFC00081C; // fctiw   f0,f1
    compile[count++] = 0x7C051FAE; // stfiwx  f0,r5,r3
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static inline bool cpuCompile_TRUNC_W(s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 3;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0xFC00081E; // fctiwz  f0,f1
    compile[count++] = 0x7C051FAE; // stfiwx  f0,r5,r3
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LB(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 11;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7CA538AE; // lbzx    r5,r5,r7
    compile[count++] = 0x7CA50774; // extsb   r5,r5
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LH(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 11;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7CA53A2E; // lhzx    r5,r5,r7
    compile[count++] = 0x7CA50734; // extsh   r5,r5
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LW(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 10;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7CA5382E; // lwzx    r5,r5,r7
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LBU(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 10;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7CA538AE; // lbzx    r5,r5,r7
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LHU(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 10;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7CA53A2E; // lhzx    r5,r5,r7
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_SB(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 10;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7D0539AE; // stbx    r8,r5,r7
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_SH(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 10;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7D053B2E; // sthx    r8,r5,r7
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_SW(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 10;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7D05392E; // stwx    r8,r5,r7
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LDC(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 12;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7CE53A14; // add     r7,r5,r7
    compile[count++] = 0x80A70000; // lwz     r5,0(r7)
    compile[count++] = 0x80C70004; // lwz     r6,4(r7)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_SDC(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 12;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x54C6103A; // slwi    r6,r6,2
    compile[count++] = 0x38E30000 + OFFSETOF(pCPU, apDevice); // addi    r7,r3,0
    compile[count++] = 0x7CC6382E; // lwzx    r6,r6,r7
    compile[count++] = 0x80E60008; // lwz     r7,8(r6)
    compile[count++] = 0x7CA53A14; // add     r5,r5,r7
    compile[count++] = 0x80E60004; // lwz     r7,4(r6)
    compile[count++] = 0x80E70004; // lwz     r7,4(r7)
    compile[count++] = 0x7CE53A14; // add     r7,r5,r7
    compile[count++] = 0x91070000; // stw     r8,0(r7)
    compile[count++] = 0x91270004; // stw     r9,4(r7)
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LWL(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 12;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x38C00018; // li      r6,24
    compile[count++] = 0x88A70000; // lbz     r5,0(r7)
    compile[count++] = 0x7CA53030; // slw     r5,r5,r6
    compile[count++] = 0x394000FF; // li      r10,255
    compile[count++] = 0x7D4A3030; // slw     r10,r10,r6
    compile[count++] = 0x7D295078; // andc    r9,r9,r10
    compile[count++] = 0x7D292B78; // or      r9,r9,r5
    compile[count++] = 0x38C6FFF8; // addi    r6,r6,-8
    compile[count++] = 0x54E507BF; // clrlwi. r5,r7,30
    compile[count++] = 0x38E70001; // addi    r7,r7,1
    compile[count++] = 0x4082FFDC; // bne+    0xFFFFFFDC
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static bool cpuCompile_LWR(Cpu* pCPU, s32* addressGCN) {
    s32* compile;
    s32 count = 0;
    s32 nSize = 12;

    if (!xlHeapTake((void**)&compile, (nSize * sizeof(s32)) | 0x30000000)) {
        return false;
    }
    *addressGCN = (s32)compile;

    compile[count++] = 0x38C00000; // li      r6,0
    compile[count++] = 0x88A70000; // lbz     r5,0(r7)
    compile[count++] = 0x7CA53030; // slw     r5,r5,r6
    compile[count++] = 0x394000FF; // li      r10,255
    compile[count++] = 0x7D4A3030; // slw     r10,r10,r6
    compile[count++] = 0x7D295078; // andc    r9,r9,r10
    compile[count++] = 0x7D292B78; // or      r9,r9,r5
    compile[count++] = 0x38C60008; // addi    r6,r6,8
    compile[count++] = 0x54E507BF; // clrlwi. r5,r7,30
    compile[count++] = 0x38E7FFFF; // addi    r7,r7,-1
    compile[count++] = 0x4082FFDC; // bne+    0xFFFFFFDC
    compile[count++] = 0x4E800020; // blr

    nSize = count;
    DCStoreRange(compile, nSize * sizeof(s32));
    ICInvalidateRange(compile, nSize * sizeof(s32));

    return true;
}

static inline cpuUnknownMarioKartFrameSet(SystemRomType eTypeROM, void* pFrame, s32 nAddressN64) {
    if (eTypeROM == NKTJ) {
        if (nAddressN64 == 0x802A4118) {
            *((s32*)pFrame + 0x11) = 0;
        }
        if (nAddressN64 == 0x800729D4) {
            *((s32*)pFrame + 0x11) = 1;
        }
    } else if (eTypeROM == NKTP) {
        if (nAddressN64 == 0x802A4160) {
            *((s32*)pFrame + 0x11) = 0;
        }
        if (nAddressN64 == 0x80072E34) {
            *((s32*)pFrame + 0x11) = 1;
        }
    } else if (eTypeROM == NKTE) {
        if (nAddressN64 == 0x802A4160) {
            *((s32*)pFrame + 0x11) = 0;
        }
        if (nAddressN64 == 0x80072E54) {
            *((s32*)pFrame + 0x11) = 1;
        }
    }
}

static s32 cpuExecuteOpcode(Cpu* pCPU, s32 nCount0, s32 nAddressN64, s32 nAddressGCN) {
    s32 pad1[2];
    u64 save;
    s32 restore;
    u32 nOpcode;
    u32* opcode;
    s32 pad2;
    CpuDevice** apDevice;
    u8* aiDevice;
    s32 iEntry;
    s32 nCount;
    s8 nData8;
    s16 nData16;
    s32 nData32;
    s64 nData64;
    s32 nAddress;
    CpuFunction* pFunction;
    s32 nTick;
    s32 pad3[3];

    restore = 0;
    nTick = OSGetTick();
    if (pCPU->nWaitPC != 0) {
        pCPU->nMode |= 8;
    } else {
        pCPU->nMode &= ~8;
    }

#if VERSION >= MK64_J
    cpuUnknownMarioKartFrameSet(gpSystem->eTypeROM, SYSTEM_FRAME(gpSystem), nAddressN64);
#endif

    aiDevice = pCPU->aiDevice;
    apDevice = pCPU->apDevice;

#if VERSION < MK64_J 
    ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&opcode, nAddressN64, NULL);
#else
    if (!cpuGetAddressBuffer(pCPU, (void**)&opcode, nAddressN64)) {
        return false;
    }
#endif

    nOpcode = *opcode;
    pCPU->nPC = nAddressN64 + 4;
    if (nOpcode == 0xACBF011C) { // sw $ra,0x11C($a1)
        save = pCPU->aGPR[31].u64;
        restore = 1;
        pCPU->aGPR[31].s32 = pCPU->nReturnAddrLast;
    }

    switch (MIPS_OP(nOpcode)) {
        case 0x00: // special
            switch (MIPS_FUNCT(nOpcode)) {
                case 0x00: // sll
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 = pCPU->aGPR[MIPS_RT(nOpcode)].s32 << MIPS_SA(nOpcode);
                    break;
                case 0x02: // srl
                    pCPU->aGPR[MIPS_RD(nOpcode)].u32 = pCPU->aGPR[MIPS_RT(nOpcode)].u32 >> MIPS_SA(nOpcode);
                    break;
                case 0x03: // sra
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 = pCPU->aGPR[MIPS_RT(nOpcode)].s32 >> MIPS_SA(nOpcode);
                    break;
                case 0x04: // sllv
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 = pCPU->aGPR[MIPS_RT(nOpcode)].s32
                                                       << (pCPU->aGPR[MIPS_RS(nOpcode)].s32 & 0x1F);
                    break;
                case 0x06: // srlv
                    pCPU->aGPR[MIPS_RD(nOpcode)].u32 =
                        pCPU->aGPR[MIPS_RT(nOpcode)].u32 >> (pCPU->aGPR[MIPS_RS(nOpcode)].s32 & 0x1F);
                    break;
                case 0x07: // srav
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        pCPU->aGPR[MIPS_RT(nOpcode)].s32 >> (pCPU->aGPR[MIPS_RS(nOpcode)].s32 & 0x1F);
                    break;
                case 0x08: // jr
                    pCPU->nWaitPC = pCPU->aGPR[MIPS_RS(nOpcode)].u32;
                    break;
                case 0x09: // jalr
                    pCPU->nWaitPC = pCPU->aGPR[MIPS_RS(nOpcode)].u32;
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->nPC + 4;
                    break;
                case 0x0C: // syscall
                    cpuException(pCPU, CEC_SYSCALL, 0);
                    break;
                case 0x0D: // break
                    cpuException(pCPU, CEC_BREAK, 0);
                    break;
                case 0x10: // mfhi
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->nHi;
                    break;
                case 0x11: // mthi
                    pCPU->nHi = pCPU->aGPR[MIPS_RS(nOpcode)].s64;
                    break;
                case 0x12: // mflo
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->nLo;
                    break;
                case 0x13: // mtlo
                    pCPU->nLo = pCPU->aGPR[MIPS_RS(nOpcode)].s64;
                    break;
                case 0x14: // dsllv
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64
                                                       << (pCPU->aGPR[MIPS_RS(nOpcode)].s64 & 0x3F);
                    break;
                case 0x16: // dsrlv
                    pCPU->aGPR[MIPS_RD(nOpcode)].u64 =
                        pCPU->aGPR[MIPS_RT(nOpcode)].u64 >> (pCPU->aGPR[MIPS_RS(nOpcode)].s64 & 0x3F);
                    break;
                case 0x17: // dsrav
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 =
                        pCPU->aGPR[MIPS_RT(nOpcode)].s64 >> (pCPU->aGPR[MIPS_RS(nOpcode)].s64 & 0x3F);
                    break;
                case 0x18: // mult
                    nData64 = (s64)pCPU->aGPR[MIPS_RS(nOpcode)].s32 * (s64)pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                    pCPU->nLo = (s32)(nData64 & 0xFFFFFFFF);
                    pCPU->nHi = (s32)(nData64 >> 32);
                    break;
                case 0x19: // multu
                    nData64 = (u64)pCPU->aGPR[MIPS_RS(nOpcode)].u32 * (u64)pCPU->aGPR[MIPS_RT(nOpcode)].u32;
                    pCPU->nLo = (s32)(nData64 & 0xFFFFFFFF);
                    pCPU->nHi = (s32)(nData64 >> 32);
                    break;
                case 0x1A: // div
                    if (pCPU->aGPR[MIPS_RT(nOpcode)].s32 != 0) {
                        pCPU->nLo = pCPU->aGPR[MIPS_RS(nOpcode)].s32 / pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                        pCPU->nHi = pCPU->aGPR[MIPS_RS(nOpcode)].s32 % pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                    }
                    break;
                case 0x1B: // divu
                    if (pCPU->aGPR[MIPS_RT(nOpcode)].u32 != 0) {
                        pCPU->nLo = (s32)(pCPU->aGPR[MIPS_RS(nOpcode)].u32 / pCPU->aGPR[MIPS_RT(nOpcode)].u32);
                        pCPU->nHi = (s32)(pCPU->aGPR[MIPS_RS(nOpcode)].u32 % pCPU->aGPR[MIPS_RT(nOpcode)].u32);
                    }
                    break;
                case 0x1C: // dmult
                    pCPU->nLo = pCPU->aGPR[MIPS_RS(nOpcode)].s64 * pCPU->aGPR[MIPS_RT(nOpcode)].s64;
                    pCPU->nHi = (pCPU->nLo < 0) ? -1 : 0;
                    break;
                case 0x1D: // dmultu
                    pCPU->nLo = pCPU->aGPR[MIPS_RS(nOpcode)].u64 * pCPU->aGPR[MIPS_RT(nOpcode)].u64;
                    pCPU->nHi = (pCPU->nLo < 0) ? -1 : 0;
                    break;
                case 0x1E: // ddiv
                    if (pCPU->aGPR[MIPS_RT(nOpcode)].s64 != 0) {
                        pCPU->nLo = pCPU->aGPR[MIPS_RS(nOpcode)].s64 / pCPU->aGPR[MIPS_RT(nOpcode)].s64;
                        pCPU->nHi = pCPU->aGPR[MIPS_RS(nOpcode)].s64 % pCPU->aGPR[MIPS_RT(nOpcode)].s64;
                    }
                    break;
                case 0x1F: // ddivu
                    if (pCPU->aGPR[MIPS_RT(nOpcode)].u64 != 0) {
                        pCPU->nLo = pCPU->aGPR[MIPS_RS(nOpcode)].u64 / pCPU->aGPR[MIPS_RT(nOpcode)].u64;
                        pCPU->nHi = pCPU->aGPR[MIPS_RS(nOpcode)].u64 % pCPU->aGPR[MIPS_RT(nOpcode)].u64;
                    }
                    break;
                case 0x20: // add
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].s32 + pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                    break;
                case 0x21: // addu
                    pCPU->aGPR[MIPS_RD(nOpcode)].u32 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].u32 + pCPU->aGPR[MIPS_RT(nOpcode)].u32;
                    break;
                case 0x22: // sub
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].s32 - pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                    break;
                case 0x23: // subu
                    pCPU->aGPR[MIPS_RD(nOpcode)].u32 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].u32 - pCPU->aGPR[MIPS_RT(nOpcode)].u32;
                    break;
                case 0x24: // and
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].s32 & pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                    break;
                case 0x25: // or
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].s32 | pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                    break;
                case 0x26: // xor
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].s32 ^ pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                    break;
                case 0x27: // nor
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        ~(pCPU->aGPR[MIPS_RS(nOpcode)].s32 | pCPU->aGPR[MIPS_RT(nOpcode)].s32);
                    break;
                case 0x2A: // slt
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < pCPU->aGPR[MIPS_RT(nOpcode)].s32) ? 1 : 0;
                    break;
                case 0x2B: // sltu
                    pCPU->aGPR[MIPS_RD(nOpcode)].s32 =
                        (pCPU->aGPR[MIPS_RS(nOpcode)].u32 < pCPU->aGPR[MIPS_RT(nOpcode)].u32) ? 1 : 0;
                    break;
                case 0x2C: // dadd
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].s64 + pCPU->aGPR[MIPS_RT(nOpcode)].s64;
                    break;
                case 0x2D: // daddu
                    pCPU->aGPR[MIPS_RD(nOpcode)].u64 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].u64 + pCPU->aGPR[MIPS_RT(nOpcode)].u64;
                    break;
                case 0x2E: // dsub
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].s64 - pCPU->aGPR[MIPS_RT(nOpcode)].s64;
                    break;
                case 0x2F: // dsubu
                    pCPU->aGPR[MIPS_RD(nOpcode)].u64 =
                        pCPU->aGPR[MIPS_RS(nOpcode)].u64 - pCPU->aGPR[MIPS_RT(nOpcode)].u64;
                    break;
                case 0x30: // tge
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 >= pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x31: // tgeu
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].u32 >= pCPU->aGPR[MIPS_RT(nOpcode)].u32) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x32: // tlt
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x33: // tltu
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].u32 < pCPU->aGPR[MIPS_RT(nOpcode)].u32) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x34: // teq
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 == pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x36: // tne
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 != pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x38: // dsll
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64 << MIPS_SA(nOpcode);
                    break;
                case 0x3A: // dsrl
                    pCPU->aGPR[MIPS_RD(nOpcode)].u64 = pCPU->aGPR[MIPS_RT(nOpcode)].u64 >> MIPS_SA(nOpcode);
                    break;
                case 0x3B: // dsra
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64 >> MIPS_SA(nOpcode);
                    break;
                case 0x3C: // dsll32
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64 << (MIPS_SA(nOpcode) + 32);
                    break;
                case 0x3E: // dsrl32
                    pCPU->aGPR[MIPS_RD(nOpcode)].u64 = pCPU->aGPR[MIPS_RT(nOpcode)].u64 >> (MIPS_SA(nOpcode) + 32);
                    break;
                case 0x3F: // dsra32
                    pCPU->aGPR[MIPS_RD(nOpcode)].s64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64 >> (MIPS_SA(nOpcode) + 32);
                    break;
            }
            break;
        case 0x01: // regimm
            switch (MIPS_RT(nOpcode)) {
                case 0x00: // bltz
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < 0) {
                        pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    }
                    break;
                case 0x01: // bgez
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 >= 0) {
                        pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    }
                    break;
                case 0x02: // bltzl
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < 0) {
                        pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    } else {
                        pCPU->nMode |= 4;
                        pCPU->nPC += 4;
                    }
                    break;
                case 0x03: // bgezl
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 >= 0) {
                        pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    } else {
                        pCPU->nMode |= 4;
                        pCPU->nPC += 4;
                    }
                    break;
                case 0x08: // tgei
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 >= MIPS_IMM_S16(nOpcode)) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x09: // tgeiu
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].u32 >= MIPS_IMM_S16(nOpcode)) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x0A: // tlti
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < MIPS_IMM_S16(nOpcode)) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x0B: // tltiu
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].u32 < MIPS_IMM_S16(nOpcode)) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x0C: // teqi
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 == MIPS_IMM_S16(nOpcode)) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x0E: // tnei
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 != MIPS_IMM_S16(nOpcode)) {
                        cpuException(pCPU, CEC_TRAP, 0);
                    }
                    break;
                case 0x10: // bltzal
                    pCPU->aGPR[31].s32 = pCPU->nPC + 4;
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < 0) {
                        pCPU->nWaitPC = pCPU->nCallLast = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    }
                    break;
                case 0x11: // bgezal
                    pCPU->aGPR[31].s32 = pCPU->nPC + 4;
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 >= 0) {
                        pCPU->nWaitPC = pCPU->nCallLast = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    }
                    break;
                case 0x12: // bltzall
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < 0) {
                        pCPU->aGPR[31].s32 = pCPU->nPC + 4;
                        pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    } else {
                        pCPU->nMode |= 4;
                        pCPU->nPC = pCPU->nPC + 4;
                    }
                    break;
                case 0x13: // bgezall
                    if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 >= 0) {
                        pCPU->aGPR[31].s32 = pCPU->nPC + 4;
                        pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                    } else {
                        pCPU->nMode |= 4;
                        pCPU->nPC = pCPU->nPC + 4;
                    }
                    break;
            }
            break;
        case 0x02: // j
            pCPU->nWaitPC = (pCPU->nPC & 0xF0000000) | (MIPS_TARGET(nOpcode) << 2);
            if (pCPU->nWaitPC == pCPU->nPC - 4) {
                if (!cpuCheckInterrupts(pCPU)) {
                    return false;
                }
            }
            break;
        case 0x03: // jal
            pCPU->aGPR[31].s32 = pCPU->nPC + 4;
            pCPU->nWaitPC = pCPU->nCallLast = (pCPU->nPC & 0xF0000000) | (MIPS_TARGET(nOpcode) << 2);
            cpuFindFunction(pCPU, pCPU->nWaitPC, &pFunction);
            break;
        case 0x04: // beq
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 == (s32)pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            }
            if (pCPU->nWaitPC == pCPU->nPC - 4) {
                if (!cpuCheckInterrupts(pCPU)) {
                    return false;
                }
                break;
            }
            break;
        case 0x05: // bne
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 != (s32)pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            }
            break;
        case 0x06: // blez
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 <= 0) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            }
            break;
        case 0x07: // bgtz
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 > 0) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            }
            break;
        case 0x08: // addi
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            break;
        case 0x09: // addiu
            pCPU->aGPR[MIPS_RT(nOpcode)].u32 = pCPU->aGPR[MIPS_RS(nOpcode)].u32 + MIPS_IMM_S16(nOpcode);
            break;
        case 0x0A: // slti
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = (pCPU->aGPR[MIPS_RS(nOpcode)].s32 < MIPS_IMM_S16(nOpcode)) ? 1 : 0;
            break;
        case 0x0B: // sltiu
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = (pCPU->aGPR[MIPS_RS(nOpcode)].u32 < MIPS_IMM_S16(nOpcode)) ? 1 : 0;
            break;
        case 0x0C: // andi
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = pCPU->aGPR[MIPS_RS(nOpcode)].s32 & MIPS_IMM_U16(nOpcode);
            break;
        case 0x0D: // ori
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = pCPU->aGPR[MIPS_RS(nOpcode)].s32 | MIPS_IMM_U16(nOpcode);
            break;
        case 0x0E: // xori
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = pCPU->aGPR[MIPS_RS(nOpcode)].s32 ^ MIPS_IMM_U16(nOpcode);
            break;
        case 0x0F: // lui
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = MIPS_IMM_S16(nOpcode) << 16;
            break;
        case 0x10: // cop0
            switch (MIPS_FUNCT(nOpcode)) {
                case 0x01: // tlbr
                    iEntry = pCPU->anCP0[0] & 0x3F;
                    pCPU->anCP0[2] = pCPU->aTLB[iEntry][0];
                    pCPU->anCP0[3] = pCPU->aTLB[iEntry][1];
                    pCPU->anCP0[10] = pCPU->aTLB[iEntry][2];
                    pCPU->anCP0[5] = pCPU->aTLB[iEntry][3];
                    break;
                case 0x02: // tlbwi
                    iEntry = pCPU->anCP0[0] & 0x3F;
                    cpuSetTLB(pCPU, iEntry);
                    break;
                case 0x05: // tlbwr
                    iEntry = cpuTLBRandom(pCPU);
                    pCPU->anCP0[1] = iEntry;
                    cpuSetTLB(pCPU, iEntry);
                    break;
                case 0x08: // tlbp
                    pCPU->anCP0[0] |= 0x80000000;
                    for (iEntry = 0; iEntry < 48; iEntry++) {
                        if ((pCPU->aTLB[iEntry][0] & 2) && pCPU->aTLB[iEntry][2] == pCPU->anCP0[10]) {
                            pCPU->anCP0[0] = iEntry;
                            break;
                        }
                    }
                    break;
                case 0x18: // eret
                    if (pCPU->anCP0[12] & 4) {
                        pCPU->nPC = pCPU->anCP0[30];
                        pCPU->anCP0[12] &= ~4;
                    } else {
                        pCPU->nPC = pCPU->anCP0[14];
                        pCPU->anCP0[12] &= ~2;
                    }
                    pCPU->nMode |= 4;
                    pCPU->nMode |= 0x20;
                    break;
                default:
                    switch (MIPS_RS(nOpcode)) {
                        case 0x00: // mfc0
                            if (cpuGetRegisterCP0(pCPU, MIPS_RD(nOpcode), &nData64)) {
                                pCPU->aGPR[MIPS_RT(nOpcode)].s64 = nData64 & 0xFFFFFFFF;
                            }
                            break;
                        case 0x01: // dmfc0
                            if (cpuGetRegisterCP0(pCPU, MIPS_RD(nOpcode), &nData64)) {
                                pCPU->aGPR[MIPS_RT(nOpcode)].s64 = nData64;
                            }
                            break;
                        case 0x02:
                            break;
                        case 0x04: // mtc0
                            cpuSetRegisterCP0(pCPU, MIPS_RD(nOpcode), pCPU->aGPR[MIPS_RT(nOpcode)].u32);
                            break;
                        case 0x05: // dmtc0
                            cpuSetRegisterCP0(pCPU, MIPS_RD(nOpcode), pCPU->aGPR[MIPS_RT(nOpcode)].u64);
                            break;
                        case 0x08:
                            break;
                    }
                    break;
            }
            break;
        case 0x11: // cop1
            if ((nOpcode & 0x7FF) == 0 && MIPS_FMT(nOpcode) < 0x10) {
                switch ((u8)MIPS_FMT(nOpcode)) {
                    case 0x00: // mfc1
                        if (MIPS_FS(nOpcode) & 1) {
                            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = pCPU->aFPR[(u32)(MIPS_FS(nOpcode) - 1)].u64 >> 32;
                        } else {
                            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                        }
                        break;
                    case 0x01: // dmfc1
                        pCPU->aGPR[MIPS_RT(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                        break;
                    case 0x02: // cfc1
                        pCPU->aGPR[MIPS_RT(nOpcode)].s32 = pCPU->anFCR[MIPS_FS(nOpcode)];
                        break;
                    case 0x04: // mtc1
                        if (MIPS_FS(nOpcode) & 1) {
                            pCPU->aFPR[(u32)(MIPS_FS(nOpcode) - 1)].s64 &= 0xFFFFFFFF;
                            pCPU->aFPR[(u32)(MIPS_FS(nOpcode) - 1)].s64 |= (u64)pCPU->aGPR[MIPS_RT(nOpcode)].u32 << 32;
                        } else {
                            pCPU->aFPR[MIPS_FS(nOpcode)].s32 = pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                        }
                        break;
                    case 0x05: // dmtc1
                        pCPU->aFPR[MIPS_FS(nOpcode)].s64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64;
                        break;
                    case 0x06: // ctc1
                        pCPU->anFCR[MIPS_FS(nOpcode)] = pCPU->aGPR[MIPS_RT(nOpcode)].s32;
                        break;
                }
            } else if (MIPS_FMT(nOpcode) == 0x08) {
                switch (MIPS_FT(nOpcode)) {
                    case 0x00: // bc1f
                        if (!(pCPU->anFCR[31] & 0x800000)) {
                            pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                        }
                        break;
                    case 0x01: // bc1t
                        if (pCPU->anFCR[31] & 0x800000) {
                            pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                        }
                        break;
                    case 0x02: // bc1fl
                        if (!(pCPU->anFCR[31] & 0x800000)) {
                            pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                        } else {
                            pCPU->nMode |= 4;
                            pCPU->nPC += 4;
                        }
                        break;
                    case 0x03: // bc1tl
                        if (pCPU->anFCR[31] & 0x800000) {
                            pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
                        } else {
                            pCPU->nMode |= 4;
                            pCPU->nPC += 4;
                        }
                        break;
                }
            } else {
                switch ((u8)MIPS_FMT(nOpcode)) {
                    case 0x10: // s
                        switch (MIPS_FUNCT(nOpcode)) {
                            case 0x00: // add.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f32 + pCPU->aFPR[MIPS_FT(nOpcode)].f32;
                                break;
                            case 0x01: // sub.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f32 - pCPU->aFPR[MIPS_FT(nOpcode)].f32;
                                break;
                            case 0x02: // mul.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f32 * pCPU->aFPR[MIPS_FT(nOpcode)].f32;
                                break;
                            case 0x03: // div.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f32 / pCPU->aFPR[MIPS_FT(nOpcode)].f32;
                                break;
                            case 0x04: // sqrt.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = sqrt(pCPU->aFPR[MIPS_FS(nOpcode)].f32);
                                break;
                            case 0x05: // abs.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = fabs(pCPU->aFPR[MIPS_FS(nOpcode)].f32);
                                break;
                            case 0x06: // mov.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x07: // neg.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = -pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x08: // round.l.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].f32 + 0.5f;
                                break;
                            case 0x09: // trunc.l.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x0A: // ceil.l.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].f32);
                                break;
                            case 0x0B: // floor.l.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].f32);
                                break;
                            case 0x0C: // round.w.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].f32 + 0.5f;
                                break;
                            case 0x0D: // trunc.w.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x0E: // ceil.w.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].f32);
                                break;
                            case 0x0F: // floor.w.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].f32);
                                break;
                            case 0x20: // cvt.s.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x21: // cvt.d.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x24: // cvt.w.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x25: // cvt.l.s
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].f32;
                                break;
                            case 0x30: // c.f.s
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x31: // c.un.s
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x32: // c.eq.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 == pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x33: // c.ueq.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 == pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x34: // c.olt.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 < pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x35: // c.ult.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 < pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x36: // c.ole.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 <= pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x37: // c.ule.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 <= pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x38: // c.sf.s
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x39: // c.ngle.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 <= pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3A: // c.seq.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 == pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3B: // c.ngl.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 == pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3C: // c.lt.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 < pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3D: // c.nge.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 < pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3E: // c.le.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 <= pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3F: // c.ngt.s
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f32 <= pCPU->aFPR[MIPS_FT(nOpcode)].f32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                        }
                        break;
                    case 0x11: // d
                        switch (MIPS_FUNCT(nOpcode)) {
                            case 0x00: // add.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f64 + pCPU->aFPR[MIPS_FT(nOpcode)].f64;
                                break;
                            case 0x01: // sub.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f64 - pCPU->aFPR[MIPS_FT(nOpcode)].f64;
                                break;
                            case 0x02: // mul.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f64 * pCPU->aFPR[MIPS_FT(nOpcode)].f64;
                                break;
                            case 0x03: // div.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].f64 / pCPU->aFPR[MIPS_FT(nOpcode)].f64;
                                break;
                            case 0x04: // sqrt.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = sqrt(pCPU->aFPR[MIPS_FS(nOpcode)].f64);
                                break;
                            case 0x05: // abs.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = fabs(pCPU->aFPR[MIPS_FS(nOpcode)].f64);
                                break;
                            case 0x06: // mov.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x07: // neg.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = -pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x08: // round.l.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].f64 + 0.5f;
                                break;
                            case 0x09: // trunc.l.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x0A: // ceil.l.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].f64);
                                break;
                            case 0x0B: // floor.l.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].f64);
                                break;
                            case 0x0C: // round.w.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].f64 + 0.5f;
                                break;
                            case 0x0D: // trunc.w.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x0E: // ceil.w.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].f64);
                                break;
                            case 0x0F: // floor.w.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].f64);
                                break;
                            case 0x20: // cvt.s.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x21: // cvt.d.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x24: // cvt.w.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x25: // cvt.l.d
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].f64;
                                break;
                            case 0x30: // c.f.d
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x31: // c.un.d
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x32: // c.eq.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 == pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x33: // c.ueq.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 == pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x34: // c.olt.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 < pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x35: // c.ult.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 < pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x36: // c.ole.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 <= pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x37: // c.ule.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 <= pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x38: // c.sf.d
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x39: // c.ngle.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 <= pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3A: // c.seq.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 == pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3B: // c.ngl.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 == pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3C: // c.lt.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 < pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3D: // c.nge.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 < pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3E: // c.le.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 <= pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3F: // c.ngt.d
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].f64 <= pCPU->aFPR[MIPS_FT(nOpcode)].f64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                        }
                        break;
                    case 0x14: // w
                        switch (MIPS_FUNCT(nOpcode)) {
                            case 0x00: // add.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s32 + pCPU->aFPR[MIPS_FT(nOpcode)].s32;
                                break;
                            case 0x01: // sub.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s32 - pCPU->aFPR[MIPS_FT(nOpcode)].s32;
                                break;
                            case 0x02: // mul.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s32 * pCPU->aFPR[MIPS_FT(nOpcode)].s32;
                                break;
                            case 0x03: // div.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s32 / pCPU->aFPR[MIPS_FT(nOpcode)].s32;
                                break;
                            case 0x04: // sqrt.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = sqrt(pCPU->aFPR[MIPS_FS(nOpcode)].s32);
                                break;
                            case 0x05: // abs.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = fabs(pCPU->aFPR[MIPS_FS(nOpcode)].s32);
                                break;
                            case 0x06: // mov.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x07: // neg.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = -pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x08: // round.l.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x09: // trunc.l.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x0A: // ceil.l.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].s32);
                                break;
                            case 0x0B: // floor.l.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].s32);
                                break;
                            case 0x0C: // round.w.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x0D: // trunc.w.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x0E: // ceil.w.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].s32);
                                break;
                            case 0x0F: // floor.w.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].s32);
                                break;
                            case 0x20: // cvt.s.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x21: // cvt.d.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x24: // cvt.w.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x25: // cvt.l.w
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s32;
                                break;
                            case 0x30: // c.f.w
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x31: // c.un.w
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x32: // c.eq.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 == pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x33: // c.ueq.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 == pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x34: // c.olt.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 < pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x35: // c.ult.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 < pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x36: // c.ole.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 <= pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x37: // c.ule.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 <= pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x38: // c.sf.w
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x39: // c.ngle.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 <= pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3A: // c.seq.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 == pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3B: // c.ngl.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 == pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3C: // c.lt.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 < pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3D: // c.nge.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 < pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3E: // c.le.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 <= pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3F: // c.ngt.w
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s32 <= pCPU->aFPR[MIPS_FT(nOpcode)].s32) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                        }
                        break;
                    case 0x15: // l
                        switch (MIPS_FUNCT(nOpcode)) {
                            case 0x00: // add.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s64 + pCPU->aFPR[MIPS_FT(nOpcode)].s64;
                                break;
                            case 0x01: // sub.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s64 - pCPU->aFPR[MIPS_FT(nOpcode)].s64;
                                break;
                            case 0x02: // mul.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s64 * pCPU->aFPR[MIPS_FT(nOpcode)].s64;
                                break;
                            case 0x03: // div.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 =
                                    pCPU->aFPR[MIPS_FS(nOpcode)].s64 / pCPU->aFPR[MIPS_FT(nOpcode)].s64;
                                break;
                            case 0x04: // sqrt.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = sqrt(pCPU->aFPR[MIPS_FS(nOpcode)].s64);
                                break;
                            case 0x05: // abs.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = fabs(pCPU->aFPR[MIPS_FS(nOpcode)].s64);
                                break;
                            case 0x06: // mov.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x07: // neg.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = -pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x08: // round.l.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x09: // trunc.l.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x0A: // ceil.l.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].s64);
                                break;
                            case 0x0B: // floor.l.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].s64);
                                break;
                            case 0x0C: // round.w.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x0D: // trunc.w.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x0E: // ceil.w.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = ceil(pCPU->aFPR[MIPS_FS(nOpcode)].s64);
                                break;
                            case 0x0F: // floor.w.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = floor(pCPU->aFPR[MIPS_FS(nOpcode)].s64);
                                break;
                            case 0x20: // cvt.s.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].f32 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x21: // cvt.d.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].f64 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x24: // cvt.w.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s32 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x25: // cvt.l.l
                                pCPU->aFPR[MIPS_FD(nOpcode)].s64 = pCPU->aFPR[MIPS_FS(nOpcode)].s64;
                                break;
                            case 0x30: // c.f.l
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x31: // c.un.l
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x32: // c.eq.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 == pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x33: // c.ueq.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 == pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x34: // c.olt.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 < pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x35: // c.ult.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 < pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x36: // c.ole.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 <= pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x37: // c.ule.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 <= pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x38: // c.sf.l
                                pCPU->anFCR[31] &= ~0x800000;
                                break;
                            case 0x39: // c.ngle.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 <= pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3A: // c.seq.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 == pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3B: // c.ngl.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 == pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3C: // c.lt.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 < pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3D: // c.nge.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 < pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3E: // c.le.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 <= pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                            case 0x3F: // c.ngt.l
                                if (pCPU->aFPR[MIPS_FS(nOpcode)].s64 <= pCPU->aFPR[MIPS_FT(nOpcode)].s64) {
                                    pCPU->anFCR[31] |= 0x800000;
                                } else {
                                    pCPU->anFCR[31] &= ~0x800000;
                                }
                                break;
                        }
                        break;
                }
            }
            break;
        case 0x14: // beq
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 == (s32)pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            } else {
                pCPU->nMode |= 4;
                pCPU->nPC += 4;
            }
            break;
        case 0x15: // bne
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 != (s32)pCPU->aGPR[MIPS_RT(nOpcode)].s32) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            } else {
                pCPU->nMode |= 4;
                pCPU->nPC += 4;
            }
            break;
        case 0x16: // blez
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 <= 0) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            } else {
                pCPU->nMode |= 4;
                pCPU->nPC += 4;
            }
            break;
        case 0x17: // bgtz
            if (pCPU->aGPR[MIPS_RS(nOpcode)].s32 > 0) {
                pCPU->nWaitPC = pCPU->nPC + MIPS_IMM_S16(nOpcode) * 4;
            } else {
                pCPU->nMode |= 4;
                pCPU->nPC += 4;
            }
            break;
        case 0x18: // daddi
            pCPU->aGPR[MIPS_RT(nOpcode)].s64 = pCPU->aGPR[MIPS_RS(nOpcode)].s64 + MIPS_IMM_S16(nOpcode);
            break;
        case 0x19: // daddiu
            pCPU->aGPR[MIPS_RT(nOpcode)].u64 = pCPU->aGPR[MIPS_RS(nOpcode)].u64 + MIPS_IMM_S16(nOpcode);
            break;
        case 0x1F: // library call
            if (!libraryCall(SYSTEM_LIBRARY(gpSystem), pCPU, MIPS_IMM_S16(nOpcode))) {
                return false;
            }
            break;
        case 0x1A: // ldl
            nCount = 0x38;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s64 + MIPS_IMM_S16(nOpcode);
            do {
                if (CPU_DEVICE_GET8(apDevice, aiDevice, nAddress, &nData8)) {
                    nData64 = ((s64)nData8 & 0xFF) << nCount;
                    pCPU->aGPR[MIPS_RT(nOpcode)].s64 =
                        nData64 | (pCPU->aGPR[MIPS_RT(nOpcode)].s64 & ~((s64)0xFF << nCount));
                }
                nCount -= 8;
            } while ((nAddress++ & 7) != 0);
            break;
        case 0x1B: // ldr
            nCount = 0;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s64 + MIPS_IMM_S16(nOpcode);
            do {
                if (CPU_DEVICE_GET8(apDevice, aiDevice, nAddress, &nData8)) {
                    nData64 = ((s64)nData8 & 0xFF) << nCount;
                    pCPU->aGPR[MIPS_RT(nOpcode)].s64 =
                        nData64 | (pCPU->aGPR[MIPS_RT(nOpcode)].s64 & ~((s64)0xFF << nCount));
                }
                nCount += 8;
            } while ((nAddress-- & 7) != 0);
            break;
        case 0x27: // lwu
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s64 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET32(apDevice, aiDevice, nAddress, &nData32)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].u64 = (u32)nData32;
            }
            break;
        case 0x20: // lb
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET8(apDevice, aiDevice, nAddress, &nData8)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].s32 = nData8;
            }
            break;
        case 0x21: // lh
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET16(apDevice, aiDevice, nAddress, &nData16)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].s32 = nData16;
            }
            break;
        case 0x22: // lwl
            nCount = 0x18;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            do {
                if (CPU_DEVICE_GET8(apDevice, aiDevice, nAddress, &nData8)) {
                    nData32 = ((u32)nData8 & 0xFF) << nCount;
                    pCPU->aGPR[MIPS_RT(nOpcode)].s32 =
                        nData32 | (pCPU->aGPR[MIPS_RT(nOpcode)].s32 & ~(s32)(0xFF << nCount));
                }
                nCount -= 8;
            } while ((nAddress++ & 3) != 0);
            break;
        case 0x23: // lw
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET32(apDevice, aiDevice, nAddress, &nData32)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].s32 = nData32;
            }
            break;
        case 0x24: // lbu
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET8(apDevice, aiDevice, nAddress, &nData8)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].u32 = (u8)nData8;
            }
            break;
        case 0x25: // lhu
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (frameGetDepth(SYSTEM_FRAME(gpSystem), (u16*)&nData16, nAddress)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].u32 = (u16)nData16;
            } else {
                if (CPU_DEVICE_GET16(apDevice, aiDevice, nAddress, &nData16)) {
                    pCPU->aGPR[MIPS_RT(nOpcode)].u32 = (u16)nData16;
                }
            }
            break;
        case 0x26: // lwr
            nCount = 0;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            do {
                if (CPU_DEVICE_GET8(apDevice, aiDevice, nAddress, &nData8)) {
                    nData32 = ((u32)nData8 & 0xFF) << nCount;
                    pCPU->aGPR[MIPS_RT(nOpcode)].s32 =
                        nData32 | (pCPU->aGPR[MIPS_RT(nOpcode)].s32 & ~(s32)(0xFF << nCount));
                }
                nCount += 8;
            } while ((nAddress-- & 3) != 0);
            break;
        case 0x28: // sb
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            CPU_DEVICE_PUT8(apDevice, aiDevice, nAddress, &pCPU->aGPR[MIPS_RT(nOpcode)].s8);
            break;
        case 0x29: // sh
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            CPU_DEVICE_PUT16(apDevice, aiDevice, nAddress, &pCPU->aGPR[MIPS_RT(nOpcode)].s16);
            break;
        case 0x2A: // swl
            nCount = 0x18;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            do {
                nData8 = (pCPU->aGPR[MIPS_RT(nOpcode)].u32 >> nCount) & 0xFF;
                CPU_DEVICE_PUT8(apDevice, aiDevice, nAddress, &nData8);
                nCount -= 8;
            } while ((nAddress++ & 3) != 0);
            break;
        case 0x2B: // sw
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            CPU_DEVICE_PUT32(apDevice, aiDevice, nAddress, &pCPU->aGPR[MIPS_RT(nOpcode)].s32);
            break;
        case 0x2C: // sdl
            nCount = 0x38;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s64 + MIPS_IMM_S16(nOpcode);
            do {
                nData8 = (pCPU->aGPR[MIPS_RT(nOpcode)].u64 >> nCount) & 0xFF;
                CPU_DEVICE_PUT8(apDevice, aiDevice, nAddress, &nData8);
                nCount -= 8;
            } while ((nAddress++ & 7) != 0);
            break;
        case 0x2D: // sdr
            nCount = 0;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            do {
                nData8 = (pCPU->aGPR[MIPS_RT(nOpcode)].u64 >> nCount) & 0xFF;
                CPU_DEVICE_PUT8(apDevice, aiDevice, nAddress, &nData8);
                nCount += 8;
            } while ((nAddress-- & 7) != 0);
            break;
        case 0x2E: // swr
            nCount = 0;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            do {
                nData8 = (pCPU->aGPR[MIPS_RT(nOpcode)].u32 >> nCount) & 0xFF;
                CPU_DEVICE_PUT8(apDevice, aiDevice, nAddress, &nData8);
                nCount += 8;
            } while ((nAddress-- & 3) != 0);
            break;
        case 0x30: // ll
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET32(apDevice, aiDevice, nAddress, &nData32)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].s32 = nData32;
            }
            break;
        case 0x31: // lwc1
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET32(apDevice, aiDevice, nAddress, &nData32)) {
                if (MIPS_RT(nOpcode) & 1) {
                    pCPU->aFPR[(u32)(MIPS_RT(nOpcode) - 1)].u64 &= 0xFFFFFFFF;
                    pCPU->aFPR[(u32)(MIPS_RT(nOpcode) - 1)].u64 |= (s64)nData32 << 32;
                } else {
                    pCPU->aFPR[MIPS_RT(nOpcode)].s32 = nData32;
                }
            }
            break;
        case 0x34: // lld
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s64 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET64(apDevice, aiDevice, nAddress, &nData64)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].s64 = nData64;
            }
            break;
        case 0x35: // ldc1
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET64(apDevice, aiDevice, nAddress, &nData64)) {
                pCPU->aFPR[MIPS_RT(nOpcode)].s64 = nData64;
            }
            break;
        case 0x37: // ld
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (CPU_DEVICE_GET64(apDevice, aiDevice, nAddress, &nData64)) {
                pCPU->aGPR[MIPS_RT(nOpcode)].s64 = nData64;
            }
            break;
        case 0x38: // sc
            nData32 = pCPU->aGPR[MIPS_RT(nOpcode)].s32;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            pCPU->aGPR[MIPS_RT(nOpcode)].s32 = (CPU_DEVICE_PUT32(apDevice, aiDevice, nAddress, &nData32)) ? 1 : 0;
            break;
        case 0x39: // swc1
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            if (MIPS_RT(nOpcode) & 1) {
                nData32 = pCPU->aFPR[(u32)(MIPS_RT(nOpcode) - 1)].u64 >> 32;
            } else {
                nData32 = pCPU->aFPR[MIPS_RT(nOpcode)].s32;
            }
            CPU_DEVICE_PUT32(apDevice, aiDevice, nAddress, &nData32);
            break;
        case 0x3C: // scd
            nData64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s64 + MIPS_IMM_S16(nOpcode);
            pCPU->aGPR[MIPS_RT(nOpcode)].s64 = (CPU_DEVICE_PUT64(apDevice, aiDevice, nAddress, &nData64)) ? 1 : 0;
            break;
        case 0x3D: // sdc1
            nData64 = pCPU->aFPR[MIPS_RT(nOpcode)].s64;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            CPU_DEVICE_PUT64(apDevice, aiDevice, nAddress, &nData64);
            break;
        case 0x3F: // sd
            nData64 = pCPU->aGPR[MIPS_RT(nOpcode)].s64;
            nAddress = pCPU->aGPR[MIPS_RS(nOpcode)].s32 + MIPS_IMM_S16(nOpcode);
            CPU_DEVICE_PUT64(apDevice, aiDevice, nAddress, &nData64);
            break;
    }

    if (!cpuExecuteUpdate(pCPU, &nAddressGCN, nTick + 1)) {
        return false;
    }
    if (restore) {
        pCPU->aGPR[31].u64 = save;
    }
    pCPU->nWaitPC = -1;
    pCPU->nTickLast = OSGetTick();

    PAD_STACK();
    PAD_STACK();
    return nAddressGCN;
}

static s32 cpuExecuteIdle(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN) {
    Rom* pROM;

    pROM = SYSTEM_ROM(gpSystem);

    nCount = OSGetTick();
    if (pCPU->nWaitPC != 0) {
        pCPU->nMode |= 8;
    } else {
        pCPU->nMode &= ~8;
    }

    pCPU->nMode |= 0x80;
    pCPU->nPC = nAddressN64;
    if (!(pCPU->nMode & 0x40) && pROM->copy.nSize == 0) {
        viForceRetrace(SYSTEM_VI(gpSystem));
    }

    if (!cpuExecuteUpdate(pCPU, &nAddressGCN, nCount)) {
        return false;
    }

    pCPU->nTickLast = OSGetTick();
    return nAddressGCN;
}

static s32 cpuExecuteJump(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN) {
    nCount = OSGetTick();

    if (pCPU->nWaitPC != 0) {
        pCPU->nMode |= 8;
    } else {
        pCPU->nMode &= ~8;
    }

    pCPU->nMode |= 4;
    pCPU->nPC = nAddressN64;

    if (!cpuExecuteUpdate(pCPU, &nAddressGCN, nCount)) {
        return false;
    }

    pCPU->nTickLast = OSGetTick();
    return nAddressGCN;
}

/**
 * @brief Executes a call from the dynamic recompiler environment
 *
 * @param pCPU The emulated VR4300.
 * @param nCount Latest tick count
 * @param nAddressN64 The N64 address of the call.
 * @param nAddressGCN The GameCube address after the call has completed.
 * @return s32 The address of the recompiled called function.
 */
static s32 cpuExecuteCall(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN) {
    s32 pad;
    s32 nReg;
    s32 count;
    s32* anCode;
    s32 saveGCN;
    CpuFunction* node;
    CpuCallerID* block;
    s32 nDeltaAddress;

    nCount = OSGetTick();
    if (pCPU->nWaitPC != 0) {
        pCPU->nMode |= 8;
    } else {
        pCPU->nMode &= ~8;
    }

    pCPU->nMode |= 4;
    pCPU->nPC = nAddressN64;

    pCPU->aGPR[31].s32 = nAddressGCN;
    saveGCN = nAddressGCN - 4;

    pCPU->survivalTimer++;

    cpuFindFunction(pCPU, pCPU->nReturnAddrLast - 8, &node);

    block = node->block;
    for (count = 0; count < node->callerID_total; count++) {
        if (block[count].N64address == nAddressN64 && block[count].GCNaddress == 0) {
            block[count].GCNaddress = saveGCN;
            break;
        }
    }

    saveGCN = (ganMapGPR[31] & 0x100) ? true : false;
    anCode = (s32*)nAddressGCN - (saveGCN ? 4 : 3);
    if (saveGCN) {
        anCode[0] = 0x3CA00000 | ((u32)nAddressGCN >> 16); // lis r5,nAddressGCN@h
        anCode[1] = 0x60A50000 | ((u32)nAddressGCN & 0xFFFF); // ori r5,r5,nAddressGCN@l
        DCStoreRange(anCode, 8);
        ICInvalidateRange(anCode, 8);
    } else {
        nReg = ganMapGPR[31];
        anCode[0] = 0x3C000000 | ((u32)nAddressGCN >> 16) | (nReg << 21); // lis ri,nAddressGCN@h
        anCode[1] = 0x60000000 | ((u32)nAddressGCN & 0xFFFF) | (nReg << 21) | (nReg << 16); // ori ri,ri,nAddressGCN@l
        DCStoreRange(anCode, 8);
        ICInvalidateRange(anCode, 8);
    }

    //! @bug: If cpuExecuteUpdate decides to delete the function we're trying to
    //! call here, our lis/ori will be reverted by treeCallerCheck since we've
    //! already marked this call site in the callerID for-loop above. The
    //! reverted lis/ori will store the return N64 address instead of a GCN
    //! address, so the next time this recompiled call is executed, the CPU will
    //! jump to that N64 return address in GCN address space and bad things
    //! happen (usually an invalid instruction or invalid load/store). This is
    //! known as a "VC Crash".
    //!
    //! For more details, see https://pastebin.com/V6ANmXt8
    if (!cpuExecuteUpdate(pCPU, &nAddressGCN, nCount)) {
        return false;
    }

    nDeltaAddress = (u8*)nAddressGCN - (u8*)&anCode[3];
    if (saveGCN) {
        anCode[3] = 0x48000000 | (nDeltaAddress & 0x03FFFFFC); // b nDeltaAddress
        DCStoreRange(anCode, 16);
        ICInvalidateRange(anCode, 16);
    } else {
        anCode[2] = 0x48000000 | (nDeltaAddress & 0x03FFFFFC); // b nDeltaAddress
        DCStoreRange(anCode, 12);
        ICInvalidateRange(anCode, 12);
    }

    pCPU->nTickLast = OSGetTick();

    return nAddressGCN;
}

/**
 * @brief Recompiles a VR4300 load/store instruction
 *
 * @param pCPU The emulated VR4300.
 * @param nCount Unused.
 * @param nAddressN64 The address of the Load/Store instruction.
 * @param nAddressGCN A pointer to the location where recompiled code should be stored.
 * @return s32 The address of the recompiled called function.
 */
static s32 cpuExecuteLoadStore(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN) {
    u32* opcode;
    s32 iRegisterA;
    s32 iRegisterB;
    u8 device;
    s32 address;
    s32 total;
    s32 count;
    s32 save;
    s32 interpret;
    s32* before;
    s32* after;
    s32 check2;
    s32* anCode;
    s32 pad;

    count = 0;
    save = 0;
    interpret = 0;
    check2 = 0x90C30000 + OFFSETOF(pCPU, nWaitPC);

#if VERSION < MK64_J 
    ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&opcode, nAddressN64, NULL);
#else
    if (!cpuGetAddressBuffer(pCPU, (void**)&opcode, nAddressN64)) {
        return false;
    }
#endif

    address = pCPU->aGPR[MIPS_RS(*opcode)].s32 + MIPS_IMM_S16(*opcode);
    device = pCPU->aiDevice[(u32)(address) >> DEVICE_ADDRESS_OFFSET_BITS];

    if (pCPU->nCompileFlag & 0x100) {
        anCode = (s32*)nAddressGCN - 3;
        before = anCode - 2;
        after = (s32*)nAddressGCN + 3;
    } else {
        anCode = (s32*)nAddressGCN - 3;
        before = anCode - 2;
        after = (s32*)nAddressGCN + 2;
    }

    if (((u32)address >> 28) < 0x08) {
        interpret = 1;
    }

    if (!interpret && device >= 0x80) {
        switch (MIPS_OP(*opcode)) {
            case 0x20: // lb
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 5;
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }

                anCode[count++] = 0x7C000000 | (7 << 21) | (iRegisterB << 16) | (8 << 11) | 0x214;
                anCode[count++] = 0x88070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                anCode[count++] = 0x7C000774 | (iRegisterA << 21) | (iRegisterA << 16);
                if (ganMapGPR[MIPS_RT(*opcode)] & 0x100) {
                    anCode[count++] = 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                break;
            case 0x24: // lbu
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 5;
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }

                anCode[count++] = 0x7C000214 | (7 << 21) | (iRegisterB << 16) | (8 << 11);
                anCode[count++] = 0x88070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                if (ganMapGPR[MIPS_RT(*opcode)] & 0x100) {
                    anCode[count++] = 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                break;
            case 0x21: // lh
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 5;
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }

                anCode[count++] = 0x7C000214 | (7 << 21) | (iRegisterB << 16) | (8 << 11);
                anCode[count++] = 0xA0070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                anCode[count++] = 0x7C000734 | (iRegisterA << 21) | (iRegisterA << 16);
                if (ganMapGPR[MIPS_RT(*opcode)] & 0x100) {
                    anCode[count++] = 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                break;
            case 0x25: // lhu
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 5;
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }

                anCode[count++] = 0x7C000214 | (7 << 21) | (iRegisterB << 16) | (8 << 11);
                anCode[count++] = 0xA0070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                if (ganMapGPR[MIPS_RT(*opcode)] & 0x100) {
                    anCode[count++] = 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                break;
            case 0x23: // lw
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 5;
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }

                anCode[count++] = 0x7C000214 | (7 << 21) | (iRegisterB << 16) | (8 << 11);
                anCode[count++] = 0x80070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                if (ganMapGPR[MIPS_RT(*opcode)] & 0x100) {
                    anCode[count++] = 0x90A30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                break;
            case 0x28: // sb
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 7;
                    anCode[count++] = 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }
                anCode[count++] = 0x7CE00000 | (iRegisterB << 16) | 0x4214;
                anCode[count++] = 0x98070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                break;
            case 0x29: // sh
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 7;
                    anCode[count++] = 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }
                anCode[count++] = 0x7CE00000 | (iRegisterB << 16) | 0x4214;
                anCode[count++] = 0xB0070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                break;
            case 0x2B: // sw
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4) & 0xFFFF);
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 7;
                    anCode[count++] = 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }
                anCode[count++] = 0x7CE00000 | (iRegisterB << 16) | 0x4214;
                anCode[count++] = 0x90070000 | (iRegisterA << 21) | MIPS_IMM_U16(*opcode);
                break;
            default:
                xlExit();
                break;
        }
    } else {
        interpret = 1;
        anCode[count++] = 0x3CA00000 | ((u32)nAddressN64 >> 16);
        anCode[count++] = 0x60A50000 | ((u32)nAddressN64 & 0xFFFF);
        anCode[count++] = 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[count]) & 0x03FFFFFC) | 1;
    }

    if (pCPU->nCompileFlag & 0x100) {
        if (6 - count >= 2) {
            save = count;
            anCode[count++] = 0x48000000 | (((u32)&anCode[6] - (u32)&anCode[count]) & 0xFFFF);
        }
        while (count <= 5) {
            anCode[count++] = 0x60000000;
        }
        total = 6;
    } else {
        if (5 - count >= 2) {
            save = count;
            anCode[count++] = 0x48000000 | (((u32)&anCode[5] - (u32)&anCode[count]) & 0xFFFF);
        }
        while (count <= 4) {
            anCode[count++] = 0x60000000;
        }
        total = 5;
    }

    if (!interpret && before[0] == 0x38C00000 && before[1] == check2) {
        before[0] = 0x48000000 | (((u32)&before[2] - (u32)&before[0]) & 0xFFFF);
        before[1] = 0x60000000;
        DCStoreRange(before, 8);
        ICInvalidateRange(before, 8);

        if (save != 0) {
            anCode[save] = 0x48000000 | (((u32)&after[2] - (u32)&anCode[save]) & 0xFFFF);
        }
        after[0] = 0x60000000;
        after[1] = 0x60000000;

        total += 2;
        pCPU->nWaitPC = -1;
    }

    DCStoreRange(anCode, total * 4);
    ICInvalidateRange(anCode, total * 4);
    return (s32)anCode;
}

/**
 * @brief Recompiles a VR4300 load/store instruction on COP1 or doubleword load/store.
 *
 * @param pCPU The emulated VR4300.
 * @param nCount Unused.
 * @param nAddressN64 The address of the Load/Store instruction.
 * @param nAddressGCN A pointer to the location where recompiled code should be stored.
 * @return s32 The address of the recompiled called function.
 */
static s32 cpuExecuteLoadStoreF(Cpu* pCPU, s32 nCount, s32 nAddressN64, s32 nAddressGCN) {
    u32* opcode;
    s32 iRegisterA;
    s32 iRegisterB;
    u8 device;
    s32 address;
    s32 total;
    s32 count;
    s32 save;
    s32 interpret;
    s32* before;
    s32* after;
    s32 check2;
    s32* anCode;
    s32 rt;
    s32 pad;

    count = 0;
    save = 0;
    interpret = 0;
    check2 = 0x90C30000 + OFFSETOF(pCPU, nWaitPC);

#if VERSION < MK64_J 
    ramGetBuffer(SYSTEM_RAM(gpSystem), (void**)&opcode, nAddressN64, NULL);
#else
    if (!cpuGetAddressBuffer(pCPU, (void**)&opcode, nAddressN64)) {
        return false;
    }
#endif

    address = pCPU->aGPR[MIPS_RS(*opcode)].s32 + MIPS_IMM_S16(*opcode);
    device = pCPU->aiDevice[(u32)(address) >> DEVICE_ADDRESS_OFFSET_BITS];

    if (pCPU->nCompileFlag & 0x100) {
        anCode = (s32*)nAddressGCN - 3;
        before = anCode - 2;
        after = (s32*)nAddressGCN + 4;
    } else {
        anCode = (s32*)nAddressGCN - 3;
        before = anCode - 2;
        after = (s32*)nAddressGCN + 3;
    }

    if (((u32)address >> 28) < 0x08) {
        interpret = 1;
    }

    if (!interpret && device >= 0x80) {
        rt = MIPS_RT(*opcode);
        switch (MIPS_OP(*opcode)) {
            case 0x31: // lwc1
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if ((pCPU->nCompileFlag & 0x100) && ((u32)address >> 28) >= 10) {
                    anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                }

                anCode[count++] = 0x7C000000 | (7 << 21) | (iRegisterB << 16) | (8 << 11) | 0x214;

                if (rt % 2 == 1) {
                    anCode[count++] = 0x80A70000 | MIPS_IMM_U16(*opcode);
                    anCode[count++] = 0x90A30000 + OFFSETOF(pCPU, aFPR[rt - 1]);
                } else {
                    anCode[count++] = 0x80A70000 | MIPS_IMM_U16(*opcode);
                    anCode[count++] = 0x90A30000 + (OFFSETOF(pCPU, aFPR[rt]) + 4);
                }
                break;
            case 0x39: // swc1
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if ((pCPU->nCompileFlag & 0x100) && ((u32)address >> 28) >= 10) {
                    anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                }

                anCode[count++] = 0x7C000000 | (7 << 21) | (iRegisterB << 16) | (8 << 11) | 0x214;
                if (rt % 2 == 1) {
                    anCode[count++] = 0x80A30000 + OFFSETOF(pCPU, aFPR[rt - 1]);
                } else {
                    anCode[count++] = 0x80A30000 + OFFSETOF(pCPU, aFPR[rt]) + 4;
                }
                anCode[count++] = 0x90A70000 | MIPS_IMM_U16(*opcode);
                break;
            case 0x35: // ldc1
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if ((pCPU->nCompileFlag & 0x100) && ((u32)address >> 28) >= 10) {
                    anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                }

                anCode[count++] = 0x7C000000 | (7 << 21) | (iRegisterB << 16) | (8 << 11) | 0x214;
                anCode[count++] = 0x80A70000 | MIPS_IMM_U16(*opcode);
                anCode[count++] = 0x90A30000 + OFFSETOF(pCPU, aFPR[rt]);
                anCode[count++] = 0x80A70000 | (MIPS_IMM_U16(*opcode) + 4);
                anCode[count++] = 0x90A30000 + (OFFSETOF(pCPU, aFPR[rt]) + 4);
                break;
            case 0x3D: // sdc1
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if ((pCPU->nCompileFlag & 0x100) && ((u32)address >> 28) >= 10) {
                    anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                }

                anCode[count++] = 0x7C000000 | (7 << 21) | (iRegisterB << 16) | (8 << 11) | 0x214;
                anCode[count++] = 0x80A30000 + OFFSETOF(pCPU, aFPR[rt]);
                anCode[count++] = 0x90A70000 | MIPS_IMM_U16(*opcode);
                anCode[count++] = 0x80A30000 + (OFFSETOF(pCPU, aFPR[rt]) + 4);
                anCode[count++] = 0x90A70000 | (MIPS_IMM_U16(*opcode) + 4);
                break;
            case 0x37: // ld
                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 5;
                }
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 6;
                    anCode[count++] = 0x80C30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }

                anCode[count++] = 0x7C000000 | (7 << 21) | (iRegisterB << 16) | (8 << 11) | 0x214;
                anCode[count++] = 0x80A70000 | MIPS_IMM_U16(*opcode);
                anCode[count++] = 0x90A30000 + OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]);
                anCode[count++] = 0x80070000 | (iRegisterA << 21) | (MIPS_IMM_U16(*opcode) + 4);
                anCode[count++] = (0x90030000 | (iRegisterA << 21)) + (OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4);
                break;
            case 0x3F: // sd
                if ((iRegisterB = ganMapGPR[MIPS_RS(*opcode)]) & 0x100) {
                    iRegisterB = 7;
                    anCode[count++] = 0x80E30000 + ((OFFSETOF(pCPU, aGPR[MIPS_RS(*opcode)]) + 4) & 0xFFFF);
                }

                if (pCPU->nCompileFlag & 0x100) {
                    if (pCPU->nCompileFlag & 0x1000) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    } else if (((u32)address >> 28) >= 10) {
                        anCode[count++] = 0x7C000038 | (iRegisterB << 21) | (iRegisterB << 16) | (9 << 11);
                    }
                }

                anCode[count++] = 0x7C000000 | (7 << 21) | (iRegisterB << 16) | (8 << 11) | 0x214;
                anCode[count++] = 0x80C30000 + OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]);
                anCode[count++] = 0x90C70000 | MIPS_IMM_U16(*opcode);

                if ((iRegisterA = ganMapGPR[MIPS_RT(*opcode)]) & 0x100) {
                    iRegisterA = 6;
                    anCode[count++] = 0x80C30000 + (OFFSETOF(pCPU, aGPR[MIPS_RT(*opcode)]) + 4);
                }
                anCode[count++] = 0x90070000 | (iRegisterA << 21) | (MIPS_IMM_U16(*opcode) + 4);
                break;
            default:
                xlExit();
                break;
        }
    } else {
        interpret = 1;
        anCode[count++] = 0x3CA00000 | ((u32)nAddressN64 >> 16);
        anCode[count++] = 0x60A50000 | ((u32)nAddressN64 & 0xFFFF);
        anCode[count++] = 0x48000000 | (((u32)pCPU->pfStep - (u32)&anCode[count]) & 0x03FFFFFC) | 1;
    }

    if (pCPU->nCompileFlag & 0x100) {
        if (7 - count >= 2) {
            save = count;
            anCode[count++] = 0x48000000 | (((u32)&anCode[7] - (u32)&anCode[count]) & 0xFFFF);
        }
        while (count <= 6) {
            anCode[count++] = 0x60000000;
        }
        total = 7;
    } else {
        if (6 - count >= 2) {
            save = count;
            anCode[count++] = 0x48000000 | (((u32)&anCode[6] - (u32)&anCode[count]) & 0xFFFF);
        }
        while (count <= 5) {
            anCode[count++] = 0x60000000;
        }
        total = 6;
    }

    if (!interpret && before[0] == 0x38C00000 && before[1] == check2) {
        before[0] = 0x48000000 | (((u32)&before[2] - (u32)&before[0]) & 0xFFFF);
        before[1] = 0x60000000;
        DCStoreRange(before, 8);
        ICInvalidateRange(before, 8);

        if (save != 0) {
            anCode[save] = 0x48000000 | (((u32)&after[2] - (u32)&anCode[save]) & 0xFFFF);
        }
        after[0] = 0x60000000;
        after[1] = 0x60000000;

        total += 2;
        pCPU->nWaitPC = -1;
    }

    DCStoreRange(anCode, total * 4);
    ICInvalidateRange(anCode, total * 4);
    return (s32)anCode;
}

/**
 * @brief Generates a call to a virtual-console function from within the dynarec envrionment
 * Dedicated PPC registers are saved to the cpu object, and restored once the virtual-console function has finished.
 * Jump to the return value of the virtual-console function
 * @param pCPU The emulated VR4300.
 * @param ppfLink A pointer to store the generated PPC code.
 * @param pfFunction The virtual-console function to call.
 * @return bool true on success, false otherwise.
 */
static bool cpuMakeLink(Cpu* pCPU, CpuExecuteFunc* ppfLink, CpuExecuteFunc pfFunction) {
    s32 iGPR;
    s32* pnCode;
    s32 nData;
    s32 pad;

    if (!xlHeapTake((void**)&pnCode, 0x200 | 0x30000000)) {
        return false;
    }
    *ppfLink = (CpuExecuteFunc)pnCode;

    *pnCode++ = 0x7CC802A6;

    for (iGPR = 1; iGPR < 32; iGPR++) {
        if (!(ganMapGPR[iGPR] & 0x100)) {
            nData = OFFSETOF(pCPU, aGPR[iGPR]) + 4;
            *pnCode++ = 0x90030000 | (ganMapGPR[iGPR] << 21) | nData; // lwz ri,(aGPR[i] + 4)(r3)
        }
    }

    *pnCode++ = 0x48000000 | (((u8*)pfFunction - (u8*)pnCode) & 0x03FFFFFC) | 1; // bl pfFunction
    *pnCode++ = 0x7C6803A6; // mtlr r3
    *pnCode++ = 0x3C600000 | ((u32)pCPU >> 16); // lis r3,pCPU@h
    *pnCode++ = 0x60630000 | ((u32)pCPU & 0xFFFF); // ori r3,r3,pCPU@l
    *pnCode++ = 0x80830000 + OFFSETOF(pCPU, survivalTimer); // lwz r4,survivalTimer(r3)

    nData = (u32)(SYSTEM_RAM(gpSystem)->pBuffer) - 0x80000000;
    *pnCode++ = 0x3D000000 | ((u32)nData >> 16); // lis r8,ramOffset@h
    if (pCPU->nCompileFlag & 0x100) {
        *pnCode++ = 0x3D20DFFF; // lis r9,0xDFFF
        *pnCode++ = 0x61080000 | ((u32)nData & 0xFFFF); // ori r8,r8,ramOffset@l
        *pnCode++ = 0x6129FFFF; // ori r9,r9,0xFFFF
    } else if (pCPU->nCompileFlag & 1) {
        *pnCode++ = 0x39230000 + OFFSETOF(pCPU, aiDevice); // addi r9,r3,aiDevice
        *pnCode++ = 0x61080000 | ((u32)nData & 0xFFFF); // ori r8,r8,ramOffset@l
    }

    *pnCode++ = 0x38000000 | (ganMapGPR[0] << 21); // li r0,0
    for (iGPR = 1; iGPR < 32; iGPR++) {
        if (!(ganMapGPR[iGPR] & 0x100)) {
            nData = OFFSETOF(pCPU, aGPR[iGPR]) + 4;
            *pnCode++ = 0x80030000 | (ganMapGPR[iGPR] << 21) | nData; // stw ri,(aGPR[i] + 4)(r3)
        }
    }

    *pnCode++ = 0x4E800020; // blr

    DCStoreRange(*ppfLink, 0x200);
    ICInvalidateRange(*ppfLink, 0x200);
    return true;
}

static inline bool cpuFreeLink(Cpu* pCPU, CpuExecuteFunc* ppfLink) {
    if (!xlHeapFree((void**)&ppfLink)) {
        return false;
    } else {
        *ppfLink = NULL;
        return true;
    }
}

/**
 * @brief Begins execution of the emulated VR4300
 *
 * @param pCPU The emulated VR4300.
 * @param nCount Unused.
 * @param nAddressBreak Unused.
 * @return bool true on success, false otherwise.
 */
bool cpuExecute(Cpu* pCPU, s32 nCount, u64 nAddressBreak) {
    s32 pad1;
    s32 iGPR;
    s32* pnCode;
    s32 nData;
    s32 pad2;
    CpuFunction* pFunction;
    void (*pfCode)(void);

    if (pCPU->nCompileFlag & 0x1000) {
        pCPU->nCompileFlag |= 0x100;
    }

    if (!cpuMakeLink(pCPU, &pCPU->pfStep, &cpuExecuteOpcode)) {
        return false;
    }
    if (!cpuMakeLink(pCPU, &pCPU->pfJump, &cpuExecuteJump)) {
        return false;
    }
    if (!cpuMakeLink(pCPU, &pCPU->pfCall, &cpuExecuteCall)) {
        return false;
    }
    if (!cpuMakeLink(pCPU, &pCPU->pfIdle, &cpuExecuteIdle)) {
        return false;
    }
    if (!cpuMakeLink(pCPU, &pCPU->pfRam, &cpuExecuteLoadStore)) {
        return false;
    }
    if (!cpuMakeLink(pCPU, &pCPU->pfRamF, &cpuExecuteLoadStoreF)) {
        return false;
    }

    cpuCompile_DSLLV(pCPU, &cpuCompile_DSLLV_function);
    cpuCompile_DSRLV(pCPU, &cpuCompile_DSRLV_function);
    cpuCompile_DSRAV(pCPU, &cpuCompile_DSRAV_function);
    cpuCompile_DMULT(pCPU, &cpuCompile_DMULT_function);
    cpuCompile_DMULTU(pCPU, &cpuCompile_DMULTU_function);
    cpuCompile_DDIV(pCPU, &cpuCompile_DDIV_function);
    cpuCompile_DDIVU(pCPU, &cpuCompile_DDIVU_function);
    cpuCompile_DADD(pCPU, &cpuCompile_DADD_function);
    cpuCompile_DADDU(pCPU, &cpuCompile_DADDU_function);
    cpuCompile_DSUB(pCPU, &cpuCompile_DSUB_function);
    cpuCompile_DSUBU(pCPU, &cpuCompile_DSUBU_function);
    cpuCompile_S_SQRT(pCPU, &cpuCompile_S_SQRT_function);
    cpuCompile_D_SQRT(pCPU, &cpuCompile_D_SQRT_function);
    cpuCompile_W_CVT_SD(pCPU, &cpuCompile_W_CVT_SD_function);
    cpuCompile_L_CVT_SD(pCPU, &cpuCompile_L_CVT_SD_function);
    cpuCompile_CEIL_W(pCPU, &cpuCompile_CEIL_W_function);
    cpuCompile_FLOOR_W(pCPU, &cpuCompile_FLOOR_W_function);
    cpuCompile_ROUND_W(&cpuCompile_ROUND_W_function);
    cpuCompile_TRUNC_W(&cpuCompile_TRUNC_W_function);
    cpuCompile_LB(pCPU, &cpuCompile_LB_function);
    cpuCompile_LH(pCPU, &cpuCompile_LH_function);
    cpuCompile_LW(pCPU, &cpuCompile_LW_function);
    cpuCompile_LBU(pCPU, &cpuCompile_LBU_function);
    cpuCompile_LHU(pCPU, &cpuCompile_LHU_function);
    cpuCompile_SB(pCPU, &cpuCompile_SB_function);
    cpuCompile_SH(pCPU, &cpuCompile_SH_function);
    cpuCompile_SW(pCPU, &cpuCompile_SW_function);
    cpuCompile_LDC(pCPU, &cpuCompile_LDC_function);
    cpuCompile_SDC(pCPU, &cpuCompile_SDC_function);
    cpuCompile_LWL(pCPU, &cpuCompile_LWL_function);
    cpuCompile_LWR(pCPU, &cpuCompile_LWR_function);

    if (cpuMakeFunction(pCPU, &pFunction, pCPU->nPC)) {
        if (!xlHeapTake((void**)&pnCode, 0x100 | 0x30000000)) {
            return false;
        }

        pfCode = (void (*)(void))pnCode;

        *pnCode++ = 0x3C600000 | ((u32)pCPU >> 0x10); // lis r3,pCPU@h
        *pnCode++ = 0x60630000 | ((u32)pCPU & 0xFFFF); // ori r3,r3,pCPU@l

        *pnCode++ = 0x80830000 + OFFSETOF(pCPU, survivalTimer); // lwz r4,survivalTimer(r3)

        nData = (u32)(SYSTEM_RAM(gpSystem)->pBuffer) - 0x80000000;
        *pnCode++ = 0x3D000000 | ((u32)nData >> 16); // lis r8,ramOffset@h
        *pnCode++ = 0x61080000 | ((u32)nData & 0xFFFF); // ori r8,r8,ramOffset@l

        if (pCPU->nCompileFlag & 0x100) {
            *pnCode++ = 0x3D20DFFF; // lis r9,0xDFFF
            *pnCode++ = 0x6129FFFF; // ori r9,r9,0xFFFF
        } else if (pCPU->nCompileFlag & 1) {
            *pnCode++ = 0x39230000 + OFFSETOF(pCPU, aiDevice); // addi r9,r3,aiDevice
        }

        for (iGPR = 0; iGPR < ARRAY_COUNT(ganMapGPR); iGPR++) {
            if (!(ganMapGPR[iGPR] & 0x100)) {
                nData = OFFSETOF(pCPU, aGPR[iGPR]) + 4;
                *pnCode++ = 0x80030000 | (ganMapGPR[iGPR] << 21) | nData; // lwz ri,(aGPR[i] + 4)(r3)
            }
        }

        *pnCode++ = 0x48000000 | (((u32)pFunction->pfCode - (u32)pnCode) & 0x03FFFFFC); // b pFunction->pfCode

        DCStoreRange(pfCode, 0x100);
        ICInvalidateRange(pfCode, 0x100);

        pCPU->nRetrace = pCPU->nRetraceUsed = 0;

        VIWaitForRetrace();
        __cpuRetraceCallback = VISetPostRetraceCallback(&cpuRetraceCallback);

        pfCode();

        if (!xlHeapFree((void**)&pfCode)) {
            return false;
        }

        if (!cpuFreeLink(pCPU, &pCPU->pfIdle)) {
            return false;
        }
        if (!cpuFreeLink(pCPU, &pCPU->pfCall)) {
            return false;
        }
        if (!cpuFreeLink(pCPU, &pCPU->pfJump)) {
            return false;
        }
        if (!cpuFreeLink(pCPU, &pCPU->pfStep)) {
            return false;
        }
        if (!cpuFreeLink(pCPU, &pCPU->pfRam)) {
            return false;
        }
        if (!cpuFreeLink(pCPU, &pCPU->pfRamF)) {
            return false;
        }

        if (!xlHeapFree((void**)&cpuCompile_DSLLV_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DSRLV_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DSRAV_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DMULT_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DMULTU_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DDIV_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DDIVU_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DADD_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DADDU_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DSUB_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_DSUBU_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_S_SQRT_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_D_SQRT_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_W_CVT_SD_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_L_CVT_SD_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_CEIL_W_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_FLOOR_W_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_TRUNC_W_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_ROUND_W_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LB_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LH_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LW_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LBU_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LHU_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_SB_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_SH_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_SW_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LDC_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_SDC_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LWL_function)) {
            return false;
        }
        if (!xlHeapFree((void**)&cpuCompile_LWR_function)) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Maps an object to a cpu device.
 *
 * @param pCPU The emulated VR4300.
 * @param pObject The device that will handle requests for this memory space.
 * @param nAddress0 The start of the memory space for which the device will be responsible.
 * @param nAddress1 The end of the memory space for which the device will be responsible.
 * @param nType An argument which will be passed back to the device on creation.
 * @return bool true on success, false otherwise.
 */
#if VERSION < MK64_J 
bool cpuMapObject(Cpu* pCPU, void* pObject, u32 nAddress0, u32 nAddress1, s32 nType) {
    s32 iDevice;
    s32 iAddress;
    u32 nAddressVirtual0;
    u32 nAddressVirtual1;

    if (nAddress0 == 0 && nAddress1 == 0xFFFFFFFF) {
        if (!cpuMakeDevice(pCPU, &iDevice, pObject, 0, nAddress0, nAddress1, nType)) {
            return false;
        }

        pCPU->iDeviceDefault = iDevice;
        for (iAddress = 0; iAddress < ARRAY_COUNT(pCPU->aiDevice); iAddress++) {
            pCPU->aiDevice[iAddress] = iDevice;
        }
    } else {
        //! @bug: nAddress0 should not be added to nOffset (0x80000000) here. The start address
        //! is computed as nAddress0 + nOffset, so essentially the address gets added twice.
        if (!cpuMakeDevice(pCPU, &iDevice, pObject, nAddress0 + 0x80000000, nAddress0, nAddress1, nType)) {
            return false;
        }

        nAddressVirtual0 = nAddress0 | 0x80000000;
        nAddressVirtual1 = nAddress1 | 0x80000000;
        iAddress = nAddressVirtual0 >> 16;
        while (nAddressVirtual0 < nAddressVirtual1) {
            pCPU->aiDevice[iAddress] = iDevice;
            nAddressVirtual0 += 0x10000;
            iAddress++;
        }

        //! @bug: nAddress0 should not be added to nOffset (0x60000000) here. The start address
        //! is computed as nAddress0 + nOffset, so essentially the address gets added twice.
        if (!cpuMakeDevice(pCPU, &iDevice, pObject, nAddress0 + 0x60000000, nAddress0, nAddress1, nType)) {
            return false;
        }

        nAddressVirtual0 = nAddress0 | 0xA0000000;
        nAddressVirtual1 = nAddress1 | 0xA0000000;
        iAddress = nAddressVirtual0 >> 16;
        while (nAddressVirtual0 < nAddressVirtual1) {
            pCPU->aiDevice[iAddress] = iDevice;
            nAddressVirtual0 += 0x10000;
            iAddress++;
        }
    }

    return true;
}
#else
bool cpuMapObject(Cpu* pCPU, void* pObject, u32 nAddress0, u32 nAddress1, s32 nType) {
    u32 nSize;
    s32 iDevice;

    nSize = nAddress1 - nAddress0 + 1;

    if (nAddress0 == 0 && nAddress1 == 0xFFFFFFFF) {
        if (!cpuMakeDevice(pCPU, &iDevice, pObject, 0, 0, nSize, nType)) {
            return false;
        }

        pCPU->iDeviceDefault = iDevice;
    } else {
        if (!cpuMakeDevice(pCPU, &iDevice, pObject, nAddress0 | 0x80000000, nAddress0, nSize, nType)) {
            return false;
        }

        if (!cpuMakeDevice(pCPU, &iDevice, pObject, nAddress0 | 0xA0000000, nAddress0, nSize, nType)) {
            return false;
        }
    }

    return true;
}
#endif

bool cpuGetBlock(Cpu* pCPU, CpuBlock* pBlock) {
    u32 nAddress;
    CpuDevice* pDevice;
    s32 iDevice;

    nAddress = pBlock->nAddress0;

    if (nAddress < 0x04000000) {
        nAddress = pBlock->nAddress1;
    }

    for (iDevice = 1; pCPU->apDevice[iDevice] != NULL; iDevice++) {
        pDevice = pCPU->apDevice[iDevice];

        if (pDevice->nAddressPhysical0 <= nAddress && nAddress <= pDevice->nAddressPhysical1) {
            if (pDevice->pfGetBlock != NULL) {
                return pDevice->pfGetBlock(pDevice->pObject, pBlock);
            }

            return false;
        }
    }

#if VERSION >= MK64_J
    pDevice = pCPU->apDevice[pCPU->iDeviceDefault];

    if (pDevice != NULL && pDevice->pfGetBlock != NULL) {
        return pDevice->pfGetBlock(pDevice->pObject, pBlock);
    }
#endif

    return false;
}

bool cpuSetGetBlock(Cpu* pCPU, CpuDevice* pDevice, GetBlockFunc pfGetBlock) {
    pDevice->pfGetBlock = pfGetBlock;
    return true;
}

/**
 * @brief Sets load handlers for a device.
 *
 * @param pCPU The emulated VR4300.
 * @param pDevice The device which handles the load operations.
 * @param pfGet8 byte handler.
 * @param pfGet16 halfword handler.
 * @param pfGet32 word handler.
 * @param pfGet64 doubleword handler.
 * @return bool true on success, false otherwise.
 */
bool cpuSetDeviceGet(Cpu* pCPU, CpuDevice* pDevice, Get8Func pfGet8, Get16Func pfGet16, Get32Func pfGet32,
                     Get64Func pfGet64) {
    pDevice->pfGet8 = pfGet8;
    pDevice->pfGet16 = pfGet16;
    pDevice->pfGet32 = pfGet32;
    pDevice->pfGet64 = pfGet64;
    return true;
}

/**
 * @brief Sets store handlers for a device.
 *
 * @param pCPU The emulated VR4300.
 * @param pDevice The device which handles the store operations.
 * @param pfPut8 byte handler.
 * @param pfPut16 halfword handler.
 * @param pfPut32 word handler.
 * @param pfPut64 doubleword handler.
 * @return bool true on success, false otherwise.
 */
bool cpuSetDevicePut(Cpu* pCPU, CpuDevice* pDevice, Put8Func pfPut8, Put16Func pfPut16, Put32Func pfPut32,
                     Put64Func pfPut64) {
    pDevice->pfPut8 = pfPut8;
    pDevice->pfPut16 = pfPut16;
    pDevice->pfPut32 = pfPut32;
    pDevice->pfPut64 = pfPut64;
    return true;
}

bool cpuSetCodeHack(Cpu* pCPU, s32 nAddress, s32 nOpcodeOld, s32 nOpcodeNew) {
    s32 iHack;

    for (iHack = 0; iHack < pCPU->nCountCodeHack; iHack++) {
        if (pCPU->aCodeHack[iHack].nAddress == nAddress) {
            return false;
        }
    }

    pCPU->aCodeHack[iHack].nAddress = nAddress;
    pCPU->aCodeHack[iHack].nOpcodeOld = nOpcodeOld;
    pCPU->aCodeHack[iHack].nOpcodeNew = nOpcodeNew;
    pCPU->nCountCodeHack++;
    return true;
}

static inline bool cpuHeapReset(u32* array, s32 count) {
    s32 i;

    for (i = 0; i < count; i++) {
        array[i] = 0;
    }

    return true;
}

bool cpuReset(Cpu* pCPU) {
    s32 iRegister;
    s32 iTLB;

    pCPU->nTick = 0;
    pCPU->nCountCodeHack = 0;
    pCPU->nMode = 0x40;
    pCPU->pfStep = NULL;

    for (iTLB = 0; iTLB < ARRAY_COUNT(pCPU->aTLB); iTLB++) {
        for (iRegister = 0; iRegister < 5; iRegister++) {
            pCPU->aTLB[iTLB][iRegister] = 0;
        }
        pCPU->aTLB[iTLB][4] = -1;
    }

    pCPU->nLo = 0;
    pCPU->nHi = 0;
    pCPU->nPC = 0x80000400;
    pCPU->nWaitPC = -1;

    for (iRegister = 0; iRegister < ARRAY_COUNT(pCPU->aGPR); iRegister++) {
        pCPU->aGPR[iRegister].u64 = 0;
    }

    for (iRegister = 0; iRegister < ARRAY_COUNT(pCPU->aFPR); iRegister++) {
        pCPU->aFPR[iRegister].f64 = 0.0;
    }

    for (iRegister = 0; iRegister < ARRAY_COUNT(pCPU->anFCR); iRegister++) {
        pCPU->anFCR[iRegister] = 0;
    }

    pCPU->aGPR[20].u64 = 1;
    pCPU->aGPR[22].u64 = 0x3F;
    pCPU->aGPR[29].u64 = 0xA4001FF0;

    for (iRegister = 0; iRegister < ARRAY_COUNT(pCPU->anCP0); iRegister++) {
        pCPU->anCP0[iRegister] = 0;
    }

    pCPU->anCP0[15] = 0xB00;
    pCPU->anCP0[9] = 0x10000000;
    cpuSetCP0_Status(pCPU, 0x2000FF01, 1);
    pCPU->anCP0[16] = 0x6E463;

    pCPU->nCountAddress = 0;
    if (cpuHackHandler(pCPU)) {
        pCPU->nMode |= 0x10;
    }

    if (!cpuHeapReset(pCPU->aHeap1Flag, ARRAY_COUNT(pCPU->aHeap1Flag))) {
        return false;
    }
    if (pCPU->gHeap1 == NULL && !xlHeapTake(&pCPU->gHeap1, 0x300000 | 0x30000000)) {
        return false;
    }

    if (!cpuHeapReset(pCPU->aHeap2Flag, ARRAY_COUNT(pCPU->aHeap2Flag))) {
        return false;
    }
    if (pCPU->gHeap2 == NULL && !xlHeapTake(&pCPU->gHeap2, 0x104000 | 0x30000000)) {
        return false;
    }

    if (!cpuHeapReset(pCPU->aHeapTreeFlag, ARRAY_COUNT(pCPU->aHeapTreeFlag))) {
        return false;
    }

    if (pCPU->gTree != NULL) {
        treeKill(pCPU);
    }

    pCPU->nCompileFlag = 1;

#if VERSION >= MK64_J
    pCPU->unk_12228[0] = 0;
    pCPU->unk_12228[1] = 0;
    pCPU->unk_12228[2] = 0;
    pCPU->unk_12228[3] = 0;
    pCPU->unk_12228[4] = 0;
    pCPU->unk_12228[5] = 0;
    pCPU->unk_12228[6] = 0;
    pCPU->unk_12228[7] = 0;
    pCPU->unk_12228[8] = 0;
    pCPU->unk_12228[9] = 0;
    pCPU->unk_12228[10] = 0;
    pCPU->unk_12228[11] = 0;
    pCPU->unk_12228[12] = 0;
    pCPU->unk_12228[13] = 0;
    pCPU->unk_12228[14] = 0;
    pCPU->unk_12228[15] = 0;
    pCPU->unk_12228[16] = 0;
    pCPU->unk_12228[17] = 0;
#endif

    return true;
}

bool cpuGetXPC(Cpu* pCPU, s64* pnPC, s64* pnLo, s64* pnHi) {
    if (!xlObjectTest(pCPU, &gClassCPU)) {
        return false;
    }

    if (pnPC != NULL) {
        *pnPC = pCPU->nPC;
    }

    if (pnLo != NULL) {
        *pnLo = pCPU->nLo;
    }

    if (pnHi != NULL) {
        *pnHi = pCPU->nHi;
    }

    return true;
}

bool cpuSetXPC(Cpu* pCPU, s64 nPC, s64 nLo, s64 nHi) {
    if (!xlObjectTest(pCPU, &gClassCPU)) {
        return false;
    }

    pCPU->nMode |= 4;
    pCPU->nPC = nPC;
    pCPU->nLo = nLo;
    pCPU->nHi = nHi;

    return true;
}

static inline bool cpuInitAllDevices(Cpu* pCPU) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(pCPU->apDevice); i++) {
        pCPU->apDevice[i] = NULL;
    }

    return true;
}

static inline bool cpuFreeAllDevices(Cpu* pCPU) {
    s32 i;

    for (i = 0; i < ARRAY_COUNT(pCPU->apDevice); i++) {
        if (pCPU->apDevice[i] != NULL) {
            if (!cpuFreeDevice(pCPU, i)) {
                return false;
            }
        } else {
            pCPU->apDevice[i] = NULL;
        }
    }

    return true;
}

bool cpuEvent(Cpu* pCPU, s32 nEvent, void* pArgument) {
    s32 i;

    switch (nEvent) {
        case 1:
            if (!cpuReset(pCPU)) {
                return false;
            }
            break;
        case 2:
            cpuInitAllDevices(pCPU);

            if (!cpuReset(pCPU)) {
                return false;
            }

            if (!xlHeapTake((void**)&pCPU->gHeapTree, 0x46500 | 0x30000000)) {
                return false;
            }
            break;
        case 3:
#if VERSION < MK64_J 
            if (!cpuFreeAllDevices(pCPU)) {
                return false;
            }
#else
            // using `cpuFreeAllDevices` doesn't work
            for (i = 0; i < ARRAY_COUNT(pCPU->apDevice); i++) {
                if (pCPU->apDevice[i] != NULL) {
                    if (!cpuFreeDevice(pCPU, i)) {
                        return false;
                    }
                }
            }
#endif
            break;
        case 0:
        case 0x1003:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return false;
    }

    return true;
}

bool cpuGetAddressOffset(Cpu* pCPU, s32* pnOffset, u32 nAddress) {
    s32 iDevice;

    if (0x80000000 <= nAddress && nAddress < 0xC0000000) {
        *pnOffset = nAddress & 0x7FFFFF;
    } else {
        iDevice = pCPU->aiDevice[nAddress >> DEVICE_ADDRESS_OFFSET_BITS];

        if (pCPU->apDevice[iDevice]->nType & 0x100) {
            *pnOffset = (nAddress + pCPU->apDevice[iDevice]->nOffsetAddress) & 0x7FFFFF;
        } else {
            return false;
        }
    }

    return true;
}

bool cpuGetAddressBuffer(Cpu* pCPU, void** ppBuffer, u32 nAddress) {
    CpuDevice* pDevice = pCPU->apDevice[pCPU->aiDevice[nAddress >> DEVICE_ADDRESS_OFFSET_BITS]];

#if VERSION < MK64_J 
    if ((Ram*)pDevice->pObject == SYSTEM_RAM(gpSystem)) {
        if (!ramGetBuffer(SYSTEM_RAM(gpSystem), ppBuffer, nAddress + pDevice->nOffsetAddress, NULL)) {
            return false;
        }

        return true;
    } else if (SYSTEM_ROM(gpSystem) == (Rom*)pDevice->pObject) {
        if (!romGetBuffer(pDevice->pObject, ppBuffer, nAddress + pDevice->nOffsetAddress, NULL)) {
            return false;
        }

        return true;
    }

    return false;
#else
    if ((Ram*)pDevice->pObject == SYSTEM_RAM(gpSystem)) {
        if (!ramGetBuffer(pDevice->pObject, ppBuffer, nAddress + pDevice->nOffsetAddress, NULL)) {
            return false;
        }
    } else if ((Rom*)pDevice->pObject == SYSTEM_ROM(gpSystem)) {
        if (!romGetBuffer(pDevice->pObject, ppBuffer, nAddress + pDevice->nOffsetAddress, NULL)) {
            return false;
        }
    } else if ((Rsp*)pDevice->pObject == SYSTEM_RSP(gpSystem)) {
        if (!rspGetBuffer(pDevice->pObject, ppBuffer, nAddress + pDevice->nOffsetAddress, NULL)) {
            return false;
        }
    } else {
        return false;
    }

    return true;
#endif
}

bool cpuGetOffsetAddress(Cpu* pCPU, u32* anAddress, s32* pnCount, u32 nOffset, u32 nSize) {
    s32 iEntry;
    s32 iAddress = 0;
    u32 nAddress;
    u32 nMask;
    u32 nSizeMapped;

    anAddress[iAddress++] = nOffset | 0x80000000;
    anAddress[iAddress++] = nOffset | 0xA0000000;

    for (iEntry = 0; iEntry < ARRAY_COUNT(pCPU->aTLB); iEntry++) {
        if (pCPU->aTLB[iEntry][0] & 2) {
            nMask = pCPU->aTLB[iEntry][3] | 0x1FFF;

            switch (nMask) {
                case 0x1FFF:
                    nSizeMapped = 4 * 1024;
                    break;
                case 0x7FFF:
                    nSizeMapped = 16 * 1024;
                    break;
                case 0x1FFFF:
                    nSizeMapped = 64 * 1024;
                    break;
                case 0x7FFFF:
                    nSizeMapped = 256 * 1024;
                    break;
                case 0x1FFFFF:
                    nSizeMapped = 1 * 1024 * 1024;
                    break;
                case 0x7FFFFF:
                    nSizeMapped = 4 * 1024 * 1024;
                    break;
                case 0x1FFFFFF:
                    nSizeMapped = 16 * 1024 * 1024;
                    break;
                default:
                    return false;
            }

            nAddress = ((u32)(pCPU->aTLB[iEntry][0] & ~0x3F) << 6) + (nOffset & nMask);
            if (nAddress < (nOffset + nSize - 1) && (nAddress + nSizeMapped - 1) >= nOffset) {
                nAddress = pCPU->aTLB[iEntry][2] & 0xFFFFE000;
                anAddress[iAddress++] = ((nAddress) & ~nMask) | (nOffset & nMask);
            }
        }
    }

    *pnCount = iAddress;
    return true;
}

bool cpuInvalidateCache(Cpu* pCPU, s32 nAddress0, s32 nAddress1) {
    if ((nAddress0 & 0xF0000000) == 0xA0000000) {
        return true;
    }

    if (!cpuFreeCachedAddress(pCPU, nAddress0, nAddress1)) {
        return false;
    }

    cpuDMAUpdateFunction(pCPU, nAddress0, nAddress1);
    return true;
}

#if VERSION < MK64_J 
bool cpuGetFunctionChecksum(Cpu* pCPU, u32* pnChecksum, CpuFunction* pFunction) {
    s32 nSize;
    u32* pnBuffer;
    u32 nChecksum;
    u32 nData;
    u32 pad;

    if (pFunction->nChecksum != 0) {
        *pnChecksum = pFunction->nChecksum;
        return true;
    }

    if (!cpuGetAddressBuffer(pCPU, (void**)&pnBuffer, pFunction->nAddress0)) {
        return false;
    }

    nChecksum = 0;
    nSize = ((pFunction->nAddress1 - pFunction->nAddress0) >> 2) + 1;

    while (nSize > 0) {
        nSize--;
        nData = *pnBuffer;
        nData = nData >> 0x1A;
        nData = nData << ((nSize % 5) * 6);
        nChecksum += nData;
        pnBuffer++;
    }

    *pnChecksum = nChecksum;
    pFunction->nChecksum = nChecksum;

    return true;
}
#else
bool cpuGetFunctionChecksum(Cpu* pCPU, u32* pnChecksum, CpuFunction* pFunction) {
    s32 nSize;
    u32* pnBuffer;
    u32 nChecksum;
    u32 nData;
    u32 pad;

    if (pFunction->nChecksum != 0) {
        *pnChecksum = pFunction->nChecksum;
        return true;
    }

    if (cpuGetAddressBuffer(pCPU, (void**)&pnBuffer, pFunction->nAddress0)) {
        nChecksum = 0;
        nSize = ((pFunction->nAddress1 - pFunction->nAddress0) >> 2) + 1;

        while (nSize > 0) {
            nSize--;
            nData = *pnBuffer;
            nData = nData >> 0x1A;
            nData = nData << ((nSize % 5) * 6);
            nChecksum += nData;
            pnBuffer++;
        }

        *pnChecksum = nChecksum;
        pFunction->nChecksum = nChecksum;

        return true;
    }

    return false;
}
#endif

bool cpuHeapTake(void* heap, Cpu* pCPU, CpuFunction* pFunction, int memory_size) {
    s32 done;
    s32 second;
    u32* anPack;
    s32 nPackCount;
    int nBlockCount;
    s32 nOffset;
    s32 nCount;
    s32 iPack;
    u32 nPack;
    u32 nMask;
    u32 nMask0;

    done = 0;
    second = 0;
    for (;;) {
        if (pFunction->heapID == -1) {
            if (memory_size > 0x3200) {
                pFunction->heapID = 2;
            } else {
                pFunction->heapID = 1;
            }
        } else if (pFunction->heapID == 1) {
            pFunction->heapID = 2;
            second = 1;
        } else if (pFunction->heapID == 2) {
            pFunction->heapID = 1;
            second = 1;
        }

        if (pFunction->heapID == 1) {
            pFunction->heapID = 1;
            nBlockCount = (memory_size + 0x1FF) / 0x200;
            nPackCount = ARRAY_COUNT(pCPU->aHeap1Flag);
            anPack = pCPU->aHeap1Flag;

            if (second && nBlockCount >= 32) {
                pFunction->heapID = 3;
                pFunction->heapWhere = -1;
                if (!xlHeapTake(heap, memory_size)) {
                    return false;
                }
                return true;
            }
        } else if (pFunction->heapID == 2) {
            pFunction->heapID = 2;
            nBlockCount = (memory_size + 0x9FF) / 0xA00;
            nPackCount = ARRAY_COUNT(pCPU->aHeap2Flag);
            anPack = pCPU->aHeap2Flag;
        }

        if (nBlockCount >= 32) {
            pFunction->heapID = 3;
            pFunction->heapWhere = -1;
            if (!xlHeapTake(heap, memory_size)) {
                return false;
            }
            return true;
        }

        nCount = 33 - nBlockCount;
        nMask0 = (1 << nBlockCount) - 1;
        for (iPack = 0; iPack < nPackCount; iPack++) {
            if ((nPack = anPack[iPack]) != -1) {
                nMask = nMask0;
                nOffset = nCount;
                do {
                    if (!(nPack & nMask)) {
                        anPack[iPack] |= nMask;
                        pFunction->heapWhere = (nBlockCount << 16) | ((iPack << 5) + (nCount - nOffset));
                        done = 1;
                        break;
                    }
                    nMask = nMask << 1;
                    nOffset--;
                } while (nOffset != 0);
            }

            if (done) {
                break;
            }
        }

        if (done) {
            break;
        }

        if (second) {
            pFunction->heapID = -1;
            pFunction->heapWhere = -1;
            return false;
        }
    }

    if (pFunction->heapID == 1) {
        *((s32*)heap) = (s32)pCPU->gHeap1 + (pFunction->heapWhere & 0xFFFF) * 0x200;
    } else {
        *((s32*)heap) = (s32)pCPU->gHeap2 + (pFunction->heapWhere & 0xFFFF) * 0xA00;
    }

    return true;
}

static bool cpuHeapFree(Cpu* pCPU, CpuFunction* pFunction) {
    u32* anPack;
    s32 iPack;
    u32 nMask;

    if (pFunction->heapID == 1) {
        anPack = pCPU->aHeap1Flag;
    } else if (pFunction->heapID == 2) {
        anPack = pCPU->aHeap2Flag;
    } else {
        if (pFunction->pnBase != NULL) {
            if (!xlHeapFree(&pFunction->pnBase)) {
                return false;
            }
        } else {
            if (!xlHeapFree(&pFunction->pfCode)) {
                return false;
            }
        }

        return true;
    }

    if (pFunction->heapWhere == -1) {
        return false;
    }

    nMask = ((1 << (pFunction->heapWhere >> 16)) - 1) << (pFunction->heapWhere & 0x1F);
    iPack = ((pFunction->heapWhere & 0xFFFF) >> 5);

    if ((anPack[iPack] & nMask) == nMask) {
        anPack[iPack] &= ~nMask;
        pFunction->heapID = -1;
        pFunction->heapWhere = -1;
        return true;
    }

    return false;
}

static bool cpuTreeTake(void* heap, s32* where, s32 size) {
    bool done;
    s32 nOffset;
    s32 nCount;
    s32 iPack;
    u32 nPack;
    u32 nMask;
    u32 nMask0;
    u32* paHeapTreeFlag = SYSTEM_CPU(gpSystem)->aHeapTreeFlag;

    done = false;
    for (iPack = 0; iPack < 125; iPack++) {
        if ((nPack = paHeapTreeFlag[iPack]) != -1) {
            nMask = 1;
            nOffset = 32;
            do {
                if (!(nPack & nMask)) {
                    paHeapTreeFlag[iPack] |= nMask;
                    *where = (1 << 16) | ((iPack << 5) + (32 - nOffset));
                    done = true;
                    break;
                }
                nMask = nMask << 1;
                nOffset--;
            } while (nOffset != 0);
        }

        if (done) {
            break;
        }
    }

    if (!done) {
        *where = -1;
        return false;
    }

    *((s32*)heap) = (s32)SYSTEM_CPU(gpSystem)->gHeapTree + ((*where & 0xFFFF) * sizeof(CpuFunction));

    return true;
}

bool cpuFindFunction(Cpu* pCPU, s32 theAddress, CpuFunction** tree_node) {
    CpuDevice** apDevice;
    u8* aiDevice;
    u32 opcode;
    u8 follow;
    u8 valid;
    u8 check;
    u8 end_flag;
    u8 save_restore;
    u8 alert;
    s32 beginAddress;
    s32 cheat_address;
    s32 current_address;
    s32 temp_address;
    s32 branch;
    int anAddr[3];

    save_restore = false;
    alert = false;
    cheat_address = 0;
    if (pCPU->gTree == NULL) {
        check = 0;
        pCPU->survivalTimer = 1;
        if (!xlHeapTake((void**)&pCPU->gTree, sizeof(CpuTreeRoot))) {
            return false;
        }
        treeInit(pCPU, 0x80150002);
    } else {
        check = 1;
        if (treeSearch(pCPU, theAddress, tree_node)) {
            pCPU->pFunctionLast = *tree_node;
            return true;
        }
    }

    anAddr[0] = 0;
    anAddr[1] = 0;
    anAddr[2] = 0;
    aiDevice = pCPU->aiDevice;
    apDevice = pCPU->apDevice;
    beginAddress = branch = theAddress;
    current_address = theAddress;

    do {
        CPU_DEVICE_GET32(apDevice, aiDevice, current_address, &opcode);
        follow = true;

        if (check == 0) {
            if (opcode != 0 && anAddr[0] == 0) {
                anAddr[0] = current_address;
            }
        } else {
            if (anAddr[0] == 0) {
                anAddr[0] = current_address;
            }
        }

        valid = true;
        end_flag = 0;

        switch ((u8)MIPS_OP(opcode)) {
            case 0x00: { // special
                switch ((u8)MIPS_FUNCT(opcode)) {
                    case 0x08: // jr
                        if (!save_restore && (anAddr[1] == 0 || current_address > anAddr[1]) &&
                            (anAddr[2] == 0 || current_address >= anAddr[2])) {
                            end_flag = 111;
                        }
                        break;
                    case 0x0D: // break
                        if ((anAddr[1] == 0 || current_address > anAddr[1]) &&
                            (anAddr[2] == 0 || current_address >= anAddr[2])) {
                            end_flag = 111;
                            save_restore = false;
                        }
                        break;
                    default:
                        valid = SpecialOpcode[MIPS_FUNCT(opcode)];
                        break;
                }
                break;
            }
            case 0x02: // j
                if ((branch = (MIPS_TARGET(opcode) << 2) | (current_address & 0xF0000000)) >= current_address &&
                    branch - current_address <= 0x1000) {
                    if (anAddr[2] == 0) {
                        anAddr[2] = branch;
                    } else if (branch > anAddr[2]) {
                        anAddr[2] = branch;
                    }
                }
                break;
            case 0x01: // regimm
                switch ((u8)MIPS_RT(opcode)) {
                    case 0x00: // bltz
                    case 0x01: // bgez
                    case 0x02: // bltzl
                    case 0x03: // bgezl
                    case 0x10: // bltzal
                    case 0x11: // bgezal
                    case 0x12: // bltzall
                    case 0x13: // bgezall
                        branch = MIPS_IMM_S16(opcode) * 4;
                        if (branch < 0) {
                            if (check == 1 && current_address + branch + 4 < beginAddress) {
                                anAddr[0] = 0;
                                anAddr[1] = 0;
                                anAddr[2] = 0;
                                current_address = beginAddress = current_address + branch + 4;
                                alert = true;
                                continue;
                            }
                        } else {
                            if (anAddr[1] == 0) {
                                anAddr[1] = current_address + branch;
                            } else if (current_address + branch > anAddr[1]) {
                                anAddr[1] = current_address + branch;
                            }
                        }
                        break;
                    default:
                        valid = RegimmOpcode[MIPS_RT(opcode)];
                        break;
                }
                break;
            case 0x04: // beq
            case 0x14: // beql
                branch = MIPS_IMM_S16(opcode) * 4;
                if (branch < 0) {
                    if (check == 1 && current_address + branch + 4 < beginAddress) {
                        anAddr[0] = 0;
                        anAddr[1] = 0;
                        anAddr[2] = 0;
                        current_address = beginAddress = current_address + branch + 4;
                        alert = true;
                        continue;
                    }

                    temp_address = current_address + 8;
                    CPU_DEVICE_GET32(apDevice, aiDevice, temp_address, &opcode);
                    if (opcode == 0) {
                        do {
                            temp_address += 4;
                            CPU_DEVICE_GET32(apDevice, aiDevice, temp_address, &opcode);
                        } while (opcode == 0);

                        if (MIPS_OP(opcode) != 0x23) { // lw
                            current_address = temp_address - 8;
                            if ((anAddr[1] == 0 || current_address > anAddr[1]) &&
                                (anAddr[2] == 0 || current_address >= anAddr[2])) {
                                end_flag = 111;
                                save_restore = false;
                            }
                        } else {
                            current_address = temp_address - 4;
                        }
                    }
                } else {
                    if (anAddr[1] == 0) {
                        anAddr[1] = current_address + branch;
                    } else if (current_address + branch > anAddr[1]) {
                        anAddr[1] = current_address + branch;
                    }
                }
                break;
            case 0x05: // bne
            case 0x06: // blez
            case 0x07: // bgtz
            case 0x15: // bnel
            case 0x16: // blezl
            case 0x17: // bgtzl
                branch = MIPS_IMM_S16(opcode) * 4;
                if (branch < 0) {
                    if (check == 1 && current_address + branch + 4 < beginAddress) {
                        anAddr[0] = 0;
                        anAddr[1] = 0;
                        anAddr[2] = 0;
                        current_address = beginAddress = current_address + branch + 4;
                        alert = true;
                        continue;
                    }
                } else {
                    if (anAddr[1] == 0) {
                        anAddr[1] = current_address + branch;
                    } else if (current_address + branch > anAddr[1]) {
                        anAddr[1] = current_address + branch;
                    }
                }
                break;
            case 0x10: // cop0
                switch ((u8)MIPS_FUNCT(opcode)) {
                    case 0x01: // tlbr
                    case 0x02: // tlbwi
                    case 0x05: // tlbwr
                    case 0x08: // tlbp
                        break;
                    case 0x18: // eret
                        if ((anAddr[1] == 0 || current_address > anAddr[1]) &&
                            (anAddr[2] == 0 || current_address >= anAddr[2])) {
                            end_flag = 222;
                            save_restore = false;
                        }
                        break;
                    default:
                        switch ((u8)MIPS_FMT(opcode)) {
                            case 0x08:
                                switch (MIPS_FT(opcode)) {
                                    case 0x00:
                                    case 0x01:
                                    case 0x02:
                                    case 0x03:
                                        branch = MIPS_IMM_S16(opcode) * 4;
                                        if (branch < 0) {
                                            if (check == 1 && current_address + branch + 4 < beginAddress) {
                                                anAddr[0] = 0;
                                                anAddr[1] = 0;
                                                anAddr[2] = 0;
                                                current_address = beginAddress = current_address + branch + 4;
                                                alert = true;
                                                continue;
                                            }
                                        } else {
                                            if (anAddr[1] == 0) {
                                                anAddr[1] = current_address + branch;
                                            } else if (current_address + branch > anAddr[1]) {
                                                anAddr[1] = current_address + branch;
                                            }
                                        }
                                        break;
                                }
                                break;
                        }
                        break;
                }
                break;
            case 0x11: // cop1
                if (MIPS_FMT(opcode) == 0x08) {
                    switch ((u8)MIPS_FT(opcode)) {
                        case 0x00:
                        case 0x01:
                        case 0x02:
                        case 0x03:
                            branch = MIPS_IMM_S16(opcode) * 4;
                            if (branch < 0) {
                                if (check == 1 && current_address + branch + 4 < beginAddress) {
                                    anAddr[0] = 0;
                                    anAddr[1] = 0;
                                    anAddr[2] = 0;
                                    current_address = beginAddress = current_address + branch + 4;
                                    alert = true;
                                    continue;
                                }
                            } else {
                                if (anAddr[1] == 0) {
                                    anAddr[1] = current_address + branch;
                                } else if (current_address + branch > anAddr[1]) {
                                    anAddr[1] = current_address + branch;
                                }
                            }
                            break;
                    }
                }
                break;
            case 0x2B: // sw
                if (MIPS_RT(opcode) == 31) {
                    save_restore = true;
                }
                break;
            case 0x23: // lw
                if (MIPS_RT(opcode) == 31) {
                    save_restore = false;
                    if (check == 1 && alert) {
                        while (true) {
                            CPU_DEVICE_GET32(apDevice, aiDevice, beginAddress, &opcode);
                            if (MIPS_OP(opcode) == 0x2B && MIPS_RT(opcode) == 31) { // sw ra, ...
                                break;
                            }
                            beginAddress -= 4;
                        }

                        do {
                            beginAddress -= 4;
                            CPU_DEVICE_GET32(apDevice, aiDevice, beginAddress, &opcode);
                            if (opcode != 0 && treeSearch(pCPU, beginAddress - 4, tree_node)) {
                                break;
                            }
                        } while (opcode != 0);

                        anAddr[0] = 0;
                        anAddr[1] = 0;
                        anAddr[2] = 0;
                        current_address = beginAddress = beginAddress + 4;
                        alert = false;
                        continue;
                    }
                }
                break;
            default:
                valid = Opcode[MIPS_OP(opcode)];
                break;
        }

        if (end_flag != 0) {
            if (end_flag == 111) {
                anAddr[2] = current_address + 4;
                current_address += 8;
            } else {
                anAddr[2] = current_address;
                current_address += 4;
            }

#if VERSION >= MK64_J
            if (check == 1) {
                if (gpSystem->eTypeROM == NM8E) {
                    if (anAddr[2] == 0x802F1FF0) {
                        anAddr[0] = 0x802F1F50;
                    } else if (anAddr[2] == 0x80038308) {
                        anAddr[0] = 0x800382F0;
                    }
                } else if (gpSystem->eTypeROM == NMFE) {
                    if (anAddr[2] == 0x8009E420) {
                        anAddr[0] = 0x8009E380;
                    }
                } else if (gpSystem->eTypeROM == NMQE || gpSystem->eTypeROM == NMQJ || gpSystem->eTypeROM == NMQP) {
                    if (anAddr[0] == 0x802C88FC) {
                        anAddr[2] = 0x802C8974;
                    } else if (anAddr[0] == 0x802C8978) {
                        anAddr[2] = 0x802C8A5C;
                    } else if (anAddr[0] == 0x802C8A60) {
                        anAddr[2] = 0x802C8C60;
                    }
                }
            }
#endif

            if (!treeInsert(pCPU, anAddr[0], anAddr[2])) {
                return false;
            }

            if (cheat_address != 0) {
                treeSearch(pCPU, cheat_address, tree_node);
                (*tree_node)->timeToLive = 0;
                cheat_address = 0;
            }

            if (check == 1) {
                if (treeSearch(pCPU, theAddress, tree_node)) {
                    pCPU->pFunctionLast = *tree_node;
                    return true;
                } else {
                    return false;
                }
            }

            anAddr[0] = 0;
            anAddr[1] = 0;
            anAddr[2] = 0;
            follow = false;
            if (check == 0 && pCPU->gTree->total > 3970) {
                valid = false;
            }
        }

        if (follow) {
            current_address += 4;
        }
    } while (valid);

    if (check == 0) {
        treeInsert(pCPU, 0x80000180, 0x8000018C);
        treeSearch(pCPU, 0x80000180, tree_node);
        (*tree_node)->timeToLive = 0;

        if (treeSearch(pCPU, theAddress, tree_node)) {
            pCPU->pFunctionLast = *tree_node;
            return true;
        } else {
            return false;
        }
    }

    return false;
}

static bool cpuDMAUpdateFunction(Cpu* pCPU, s32 start, s32 end) {
    CpuTreeRoot* root = pCPU->gTree;
    s32 count;
    bool cancel;

    if (root == NULL) {
        return true;
    }

    if ((start < root->root_address) && (end > root->root_address)) {
        treeAdjustRoot(pCPU, start, end);
    }

    if (root->kill_limit != 0) {
        if (root->restore != NULL) {
            cancel = false;
            if (start <= root->restore->nAddress0) {
                if ((end >= root->restore->nAddress1) || (end >= root->restore->nAddress0)) {
                    cancel = true;
                }
            } else {
                if ((end >= root->restore->nAddress1) &&
                    ((start <= root->restore->nAddress0) || (start <= root->restore->nAddress1))) {
                    cancel = true;
                }
            }
            if (cancel) {
                root->restore = NULL;
                root->restore_side = 0;
            }
        }
    }

    if (start < root->root_address) {
        do {
            count = treeKillRange(pCPU, root->left, start, end);
            root->total = root->total - count;
        } while (count != 0);
    } else {
        do {
            count = treeKillRange(pCPU, root->right, start, end);
            root->total = root->total - count;
        } while (count != 0);
    }

    return true;
}

static bool treeCallerCheck(Cpu* pCPU, CpuFunction* tree, bool flag, s32 nAddress0, s32 nAddress1) {
    s32 count;
    s32 saveGCN;
    s32 saveN64;
    s32* addr_function;
    s32* addr_call;
    s32 pad;

    if (tree->callerID_total == 0) {
        return false;
    }

    if (tree->block != NULL) {
        CpuCallerID* block = tree->block;

        for (count = 0; count < tree->callerID_total; count++) {
            saveN64 = block[count].N64address;
            saveGCN = block[count].GCNaddress;
            if (saveN64 >= nAddress0 && saveN64 <= nAddress1 && saveGCN != 0) {
                addr_function = (s32*)saveGCN;
                addr_call = addr_function - (flag ? 3 : 2);

                addr_call[0] = 0x3CA00000 | ((u32)saveN64 >> 16);
                addr_call[1] = 0x60A50000 | ((u32)saveN64 & 0xFFFF);
                addr_function[0] = 0x48000000 | (((u32)pCPU->pfCall - saveGCN) & 0x03FFFFFC) | 1;

                block[count].GCNaddress = 0;
                DCStoreRange(addr_call, 16);
                ICInvalidateRange(addr_call, 16);
            }
        }
    }

    return true;
}

static bool treeInit(Cpu* pCPU, s32 root_address) {
    CpuTreeRoot* root = pCPU->gTree;

    if (root == NULL) {
        return false;
    }

    root->total = 0;
    root->total_memory = 0;
    root->root_address = root_address;
    root->start_range = 0;
    root->end_range = 0x80000000;
    root->left = NULL;
    root->right = NULL;
    root->kill_limit = 0;
    root->kill_number = 0;
    root->side = 0;
    root->restore = NULL;
    root->restore_side = 0;
    return true;
}

static bool treeInitNode(CpuFunction** tree, CpuFunction* prev, s32 start, s32 end) {
    CpuFunction* node;
    s32 where;

    if (!cpuTreeTake(&node, &where, sizeof(CpuFunction))) {
        return false;
    }

    node->nAddress0 = start;
    node->nAddress1 = end;
    node->block = NULL;
    node->callerID_total = 0;
    node->callerID_flag = 0x21;
    node->pnBase = NULL;
    node->pfCode = NULL;
    node->nCountJump = 0;
    node->aJump = NULL;
    node->nChecksum = 0;
    node->timeToLive = 1;
    node->memory_size = 0;
    node->heapID = -1;
    node->heapWhere = -1;
    node->treeheapWhere = where;
    node->prev = prev;
    node->left = NULL;
    node->right = NULL;

    *tree = node;
    return true;
}

static inline bool cpuTreeFree(CpuFunction* pFunction) {
    u32* anPack;
    s32 iPack;
    u32 nMask;

    if (pFunction->treeheapWhere == -1) {
        return false;
    }

    anPack = SYSTEM_CPU(gpSystem)->aHeapTreeFlag;
    nMask = ((1 << (pFunction->treeheapWhere >> 16)) - 1) << (pFunction->treeheapWhere & 0x1F);
    iPack = (pFunction->treeheapWhere & 0xFFFF) >> 5;
    if ((anPack[iPack] & nMask) == nMask) {
        anPack[iPack] &= ~nMask;
        return true;
    }

    return false;
}

static bool treeKill(Cpu* pCPU) {
    CpuTreeRoot* root;
    s32 count;

    count = 0;
    root = pCPU->gTree;
    if (root->left != NULL) {
        count += treeKillNodes(pCPU, root->left);
        treeCallerKill(pCPU, root->left);
        if (root->left->pfCode != NULL) {
            cpuHeapFree(pCPU, root->left);
        }
        if (!cpuTreeFree(root->left)) {
            return false;
        }
        PAD_STACK();
        PAD_STACK();

        count++;
    }

    if (root->right != NULL) {
        count += treeKillNodes(pCPU, root->right);
        treeCallerKill(pCPU, root->right);
        if (root->right->pfCode != NULL) {
            cpuHeapFree(pCPU, root->right);
        }
        if (!cpuTreeFree(root->right)) {
            return false;
        }
        PAD_STACK();
        PAD_STACK();

        count++;
    }

    root->total -= count;
    if (!xlHeapFree((void**)&pCPU->gTree)) {
        return false;
    }

    pCPU->gTree = NULL;
    return true;
}

static bool treeKillNodes(Cpu* pCPU, CpuFunction* tree) {
    CpuFunction* current;
    CpuFunction* kill;
    s32 count;

    count = 0;
    if (tree == NULL) {
        return false;
    }
    current = tree;

    do {
        while (current->left != NULL) {
            current = current->left;
        }

        do {
            if (current->right != NULL) {
                current = current->right;
                break;
            }

            if (current == tree) {
                return count;
            }

            while (current != current->prev->left) {
                kill = current;
                current = current->prev;

#if VERSION < MK64_J 
                treeCallerKill(pCPU, kill);
                if (kill->pfCode != NULL) {
                    cpuHeapFree(pCPU, kill);
                }
#else
                if (!fn_80031D4C(pCPU, kill, 2)) {
                    treeCallerKill(pCPU, kill);
                    if (kill->pfCode != NULL) {
                        cpuHeapFree(pCPU, kill);
                    }
                }
#endif

                // TODO: regalloc hacks
                (void)kill->treeheapWhere;
                if (!cpuTreeFree(kill)) {
                    return false;
                }
                PAD_STACK();
                PAD_STACK();

                count++;
                if (current == tree) {
                    return count;
                }
            }

            kill = current;
            current = current->prev;

#if VERSION < MK64_J 
            treeCallerKill(pCPU, kill);
            if (kill->pfCode != NULL) {
                cpuHeapFree(pCPU, kill);
            }
#else
            if (!fn_80031D4C(pCPU, kill, 2)) {
                treeCallerKill(pCPU, kill);
                if (kill->pfCode != NULL) {
                    cpuHeapFree(pCPU, kill);
                }
            }
#endif

            // TODO: regalloc hacks
            (void)kill->treeheapWhere;
            if (!cpuTreeFree(kill)) {
                return false;
            }
            PAD_STACK();
            PAD_STACK();

            count++;
        } while (current != NULL);
    } while (current != NULL);

    return count;
}

static bool treeDeleteNode(Cpu* pCPU, CpuFunction** top, CpuFunction* kill) {
    CpuTreeRoot* root;
    CpuFunction* save1;
    CpuFunction* save2;
    CpuFunction* connect;

    root = pCPU->gTree;
    if (kill == NULL) {
        return false;
    }

    root->total--;
    connect = kill->prev;
    save1 = kill->left;
    save2 = kill->right;

    if (connect != NULL) {
        if (save1 != NULL) {
            if (connect->left == kill) {
                connect->left = save1;
            } else {
                connect->right = save1;
            }
            save1->prev = connect;
            if (save2 != NULL) {
                while (save1->right != NULL) {
                    save1 = save1->right;
                }
                save1->right = save2;
                save2->prev = save1;
            }
        } else if (save2 != NULL) {
            if (connect->left == kill) {
                connect->left = save2;
            } else {
                connect->right = save2;
            }
            save2->prev = connect;
        } else if (connect->left == kill) {
            connect->left = NULL;
        } else {
            connect->right = NULL;
        }
    } else if (save1 != NULL) {
        *top = save1;
        if (root->left == kill) {
            root->left = save1;
        } else {
            root->right = save1;
        }
        save1->prev = NULL;
        if (save2 != NULL) {
            while (save1->right != NULL) {
                save1 = save1->right;
            }
            save1->right = save2;
            save2->prev = save1;
        }
    } else if (save2 != NULL) {
        *top = save2;
        if (root->left == kill) {
            root->left = save2;
        } else {
            root->right = save2;
        }
        save2->prev = NULL;
    } else {
        *top = NULL;
        if (root->left == kill) {
            root->left = NULL;
        } else {
            root->right = NULL;
        }
    }

    if (root->start_range == kill->nAddress0) {
        if (save2 != NULL) {
            while (save2->left != NULL) {
                save2 = save2->left;
            }
            root->start_range = save2->nAddress0;
        } else if (connect != NULL) {
            root->start_range = connect->nAddress0;
        } else {
            root->start_range = root->root_address;
        }
    }

    if (root->end_range == kill->nAddress1) {
        if (save1 != NULL) {
            while (save1->right != NULL) {
                save1 = save1->right;
            }
            root->end_range = save1->nAddress1;
        } else if (connect != NULL) {
            root->end_range = connect->nAddress1;
        } else {
            root->end_range = root->root_address;
        }
    }

#if VERSION < MK64_J 
    treeCallerKill(pCPU, kill);
    if (kill->pfCode != NULL) {
        cpuHeapFree(pCPU, kill);
    }
#else
    if (!fn_80031D4C(pCPU, kill, 2)) {
        treeCallerKill(pCPU, kill);
        if (kill->pfCode != NULL) {
            cpuHeapFree(pCPU, kill);
        }
    }
#endif

    // TODO: regalloc hacks
    (void)kill->treeheapWhere;
    if (!cpuTreeFree(kill)) {
        return false;
    }
    PAD_STACK();
    PAD_STACK();

    return true;
}

bool fn_8003F330(Cpu* pCPU, CpuFunction* pFunction) {
    CpuFunction* top;

    if (cpuFreeCachedAddress(pCPU, pFunction->nAddress0, pFunction->nAddress1) == 0) {
        return false;
    }

    if (!treeDeleteNode(pCPU, &top, pFunction)) {
        return false;
    }

    return true;
}

static bool treeInsert(Cpu* pCPU, s32 start, s32 end) {
    CpuTreeRoot* root;
    CpuFunction* current;
    s32 flag;

    root = pCPU->gTree;
    if (root == NULL) {
        return false;
    }
    if (start < root->root_address && end > root->root_address) {
        treeAdjustRoot(pCPU, start, end);
    }
    root->total++;
    root->total_memory += sizeof(CpuFunction);
    if (start != 0x80000180) {
        if (start < root->start_range) {
            root->start_range = start;
        }
        if (end > root->end_range) {
            root->end_range = end;
        }
    }
    if (start < root->root_address) {
        flag = treeInsertNode(&root->left, start, end, &current);
    } else if (start > root->root_address) {
        flag = treeInsertNode(&root->right, start, end, &current);
    } else {
        return false;
    }

    if (flag != 0) {
        return treeBalance(root);
    }
    return false;
}

static bool treeInsertNode(CpuFunction** tree, s32 start, s32 end, CpuFunction** ppFunction) {
    CpuFunction** current;
    CpuFunction* prev;

    current = tree;
    if (*tree == NULL) {
        if (treeInitNode(current, NULL, start, end)) {
            *ppFunction = *current;
            return true;
        }
        return false;
    }

    do {
        if (start < (*current)->nAddress0) {
            prev = *current;
            current = &(*current)->left;
        } else if (start > (*current)->nAddress0) {
            prev = *current;
            current = &(*current)->right;
        } else {
            return false;
        }
    } while (*current != NULL);

    if (treeInitNode(current, prev, start, end)) {
        *ppFunction = *current;
        return true;
    }
    return false;
}

static bool treeBalance(CpuTreeRoot* root) {
    CpuFunction* tree;
    CpuFunction* current;
    CpuFunction* save;
    s32 total;
    s32 count;

    for (total = 0; total < 2; total++) {
        if (total == 0) {
            tree = root->left;
        } else {
            tree = root->right;
        }

        if (tree != NULL) {
            current = tree;
            count = 0;

            while (current->right != NULL) {
                current = current->right;
                count++;
            }

            if (count >= 12) {
                current = tree;
                save = tree->right;
                count = count / 2;

                while (count-- != 0) {
                    current = current->right;
                }

                current->prev->right = NULL;
                tree->right = current;
                current->prev = tree;

                while (current->left != NULL) {
                    current = current->left;
                }

                current->left = save;
                save->prev = current;
            }

            current = tree;
            count = 0;

            while (current->left != NULL) {
                current = current->left;
                count++;
            }

            if (count >= 12) {
                current = tree;
                save = tree->left;
                count = count / 2;

                while (count-- != 0) {
                    current = current->left;
                }

                current->prev->left = NULL;
                tree->left = current;
                current->prev = tree;

                while (current->right != NULL) {
                    current = current->right;
                }

                current->right = save;
                save->prev = current;
            }
        }
    }

    return true;
}

static bool treeAdjustRoot(Cpu* pCPU, s32 new_start, s32 new_end) {
    s32 old_root;
    s32 new_root = new_end + 2;
    s32 kill_start = 0;
    s32 check1 = 0;
    s32 check2 = 0;
    u16 total;
    s32 total_memory;
    s32 address;
    CpuTreeRoot* root = pCPU->gTree;
    CpuFunction* node = NULL;
    CpuFunction* change = NULL;

    old_root = root->root_address;
    total = root->total;
    total_memory = root->total_memory;
    address = old_root + 2;

    do {
        node = NULL;
        treeSearchNode(root->right, address, &node);
        if (node != NULL) {
            if (kill_start == 0) {
                kill_start = address;
            }

            root->root_address = new_root;
            if (!treeInsert(pCPU, node->nAddress0, node->nAddress1)) {
                return false;
            }
            if (!treeSearchNode(root->left, address, &change)) {
                return false;
            }

            change->timeToLive = node->timeToLive;
            change->memory_size = node->memory_size;
            if (node->pfCode != NULL) {
                change->pfCode = node->pfCode;
                node->pfCode = NULL;
            }
            change->nCountJump = node->nCountJump;
            if (node->aJump != NULL) {
                change->aJump = node->aJump;
                node->aJump = NULL;
            }
            change->nChecksum = node->nChecksum;
            change->callerID_flag = node->callerID_flag;
            change->callerID_total = node->callerID_total;
            if (node->callerID_total != 0) {
                change->block = node->block;
                node->block = NULL;
            }

            address = node->nAddress1;
            root->root_address = old_root;
            check2 += treeKillRange(pCPU, root->right, node->nAddress0, node->nAddress1 - 4);
        }

        address += 4;
    } while (address <= new_end);

    root->root_address = new_root;
    root->total = total;
    root->total_memory = total_memory;
    return true;
}

static bool treeSearchNode(CpuFunction* tree, s32 target, CpuFunction** node) {
    CpuFunction* current;

    current = tree;
    if (current == NULL) {
        return false;
    }

    do {
        if (target >= current->nAddress0 && target < current->nAddress1) {
            *node = current;
            return true;
        }
        if (target < current->nAddress0) {
            current = current->left;
        } else if (target > current->nAddress0) {
            current = current->right;
        } else {
            current = NULL;
        }
    } while (current != NULL);

    return false;
}

static bool treeKillRange(Cpu* pCPU, CpuFunction* tree, s32 start, s32 end) {
    CpuTreeRoot* root = pCPU->gTree;
    CpuFunction* node1 = NULL;
    CpuFunction* node2 = NULL;
    CpuFunction* save1;
    CpuFunction* save2;
    CpuFunction* connect;
    bool update = false;
    s32 count = 0;

    if (start < root->start_range && end < root->start_range) {
        return false;
    }
    if (start > root->end_range && end > root->end_range) {
        return false;
    }

    do {
        treeSearchNode(tree, start, &node1);
        if (node1 != NULL) {
            break;
        }
        start += 4;
    } while (start < end);

    if (node1 != NULL) {
        connect = node1->prev;
        node1->prev = NULL;
        save1 = node1->left;
        node1->left = NULL;
        save2 = node1->right;

        while (save2 != NULL) {
            if (save2->nAddress0 < end) {
                if (save2->nAddress1 == root->end_range) {
                    update = true;
                }
                save2 = save2->right;
            } else if (save2 == NULL) {
                break;
            } else {
                save2->prev->right = NULL;
                break;
            }
        }

        if (connect != NULL) {
            if (save1 != NULL) {
                if (connect->left == node1) {
                    connect->left = save1;
                } else {
                    connect->right = save1;
                }
                save1->prev = connect;
                if (save2 != NULL) {
                    while (save1->right != NULL) {
                        save1 = save1->right;
                    }
                    save1->right = save2;
                    save2->prev = save1;
                }
            } else if (save2 != NULL) {
                if (connect->left == node1) {
                    connect->left = save2;
                } else {
                    connect->right = save2;
                }
                save2->prev = connect;
            } else if (connect->left == node1) {
                connect->left = NULL;
            } else {
                connect->right = NULL;
            }
        } else if (save1 != NULL) {
            tree = save1;
            if (root->left == node1) {
                root->left = save1;
            } else {
                root->right = save1;
            }
            save1->prev = NULL;
            if (save2 != NULL) {
                while (save1->right != NULL) {
                    save1 = save1->right;
                }
                save1->right = save2;
                save2->prev = save1;
            }
        } else if (save2 != NULL) {
            tree = save2;
            if (root->left == node1) {
                root->left = save2;
            } else {
                root->right = save2;
            }
            save2->prev = NULL;
        } else {
            tree = NULL;
            if (root->left == node1) {
                root->left = NULL;
            } else {
                root->right = NULL;
            }
        }
        if (root->start_range == node1->nAddress0) {
            if (save2 != NULL) {
                while (save2->left != NULL) {
                    save2 = save2->left;
                }
                root->start_range = save2->nAddress0;
            } else if (connect != NULL) {
                root->start_range = connect->nAddress0;
            } else {
                root->start_range = root->root_address;
            }
        }

        if (update) {
            if (save1 != NULL) {
                while (save1->right != NULL) {
                    save1 = save1->right;
                }
                root->end_range = save1->nAddress1;
            } else if (connect != NULL) {
                root->end_range = connect->nAddress1;
            } else {
                root->end_range = root->root_address;
            }
        }

        count += treeKillNodes(pCPU, node1);

#if VERSION < MK64_J 
        treeCallerKill(pCPU, node1);
        if (node1->pfCode != NULL) {
            cpuHeapFree(pCPU, node1);
        }
#else
        if (!fn_80031D4C(pCPU, node1, 2)) {
            treeCallerKill(pCPU, node1);
            if (node1->pfCode != NULL) {
                cpuHeapFree(pCPU, node1);
            }
        }
#endif

        if (!cpuTreeFree(node1)) {
            return false;
        }
        PAD_STACK();
        PAD_STACK();

        count++;
    }

    do {
        treeSearchNode(tree, end, &node2);
        if (node2 != NULL) {
            break;
        }
        end -= 4;
    } while (start < end);

    if (node2 != NULL) {
        connect = node2->prev;
        node2->prev = NULL;
        save1 = node2->left;
        save2 = node2->right;
        node2->right = NULL;

        while (save1 != NULL) {
            if (save1->nAddress0 > start) {
                save1 = save1->left;
            } else if (save1 != NULL) {
                save1->prev->left = NULL;
                break;
            } else {
                break;
            }
        }

        if (connect != NULL) {
            if (save2 != NULL) {
                if (connect->left == node2) {
                    connect->left = save2;
                } else {
                    connect->right = save2;
                }
                save2->prev = connect;
                if (save1 != NULL) {
                    while (save2->left != NULL) {
                        save2 = save2->left;
                    }
                    save2->left = save1;
                    save1->prev = save2;
                }
            } else if (save1 != NULL) {
                if (connect->left == node2) {
                    connect->left = save1;
                } else {
                    connect->right = save1;
                }
                save1->prev = connect;
            } else if (connect->left == node2) {
                connect->left = NULL;
            } else {
                connect->right = NULL;
            }
        } else if (save2 != NULL) {
            if (root->left == node2) {
                root->left = save2;
            } else {
                root->right = save2;
            }
            save2->prev = NULL;
            if (save1 != NULL) {
                while (save2->left != NULL) {
                    save2 = save2->left;
                }
                save2->left = save1;
                save1->prev = save2;
            }
        } else if (save1 != NULL) {
            if (root->left == node2) {
                root->left = save1;
            } else {
                root->right = save1;
            }
            save1->prev = NULL;
        } else if (root->left == node2) {
            root->left = NULL;
        } else {
            root->right = NULL;
        }

        if (root->end_range == node2->nAddress1) {
            if (save1 != NULL) {
                while (save1->right != NULL) {
                    save1 = save1->right;
                }
                root->end_range = save1->nAddress1;
            } else if (connect != NULL) {
                root->end_range = connect->nAddress1;
            } else {
                root->end_range = root->root_address;
            }
        }

        count += treeKillNodes(pCPU, node2);

#if VERSION < MK64_J 
        treeCallerKill(pCPU, node2);
        if (node2->pfCode != NULL) {
            cpuHeapFree(pCPU, node2);
        }
#else
        if (!fn_80031D4C(pCPU, node2, 2)) {
            treeCallerKill(pCPU, node2);
            if (node2->pfCode != NULL) {
                cpuHeapFree(pCPU, node2);
            }
        }
#endif

        if (!cpuTreeFree(node2)) {
            return false;
        }
        PAD_STACK();
        PAD_STACK();

        count++;
    }

    return count;
}

static bool treeTimerCheck(Cpu* pCPU) {
    CpuTreeRoot* root;
    s32 begin;
    s32 end;

    if (pCPU->survivalTimer > 0x7FFFF000) {
        root = pCPU->gTree;
        if (root->kill_limit != 0) {
            return false;
        }
        begin = 0;
        end = 0x7FFFF000;
        if (root->left != NULL) {
            treePrintNode(pCPU, root->left, 0x100, &begin, &end);
        }
        if (root->right != NULL) {
            treePrintNode(pCPU, root->right, 0x100, &begin, &end);
        }
        begin = end - 3;
        if (root->left != NULL) {
            treePrintNode(pCPU, root->left, 0x1000, &begin, &end);
        }
        if (root->right != NULL) {
            treePrintNode(pCPU, root->right, 0x1000, &begin, &end);
        }
        pCPU->survivalTimer -= begin;
        return true;
    }
    return false;
}

bool treeCleanUpCheck(Cpu* pCPU, CpuFunction* node) {
    s32 kill_limit = pCPU->survivalTimer - 200;

    if (!treeForceCleanUp(pCPU, pCPU->pFunctionLast, kill_limit)) {
        return false;
    }

    if (!treeForceCleanUp(pCPU, pCPU->pFunctionLast, kill_limit)) {
        return false;
    }

    return true;
}

static bool treeCleanUp(Cpu* pCPU, CpuTreeRoot* root) {
    bool done = false;
    bool complete = false;
    s32 pad;

    if (root->side == 0) {
        done = treeCleanNodes(pCPU, root->left);
    }
    if ((root->side != 0 || done) && treeCleanNodes(pCPU, root->right)) {
        complete = true;
    }
    if (!complete) {
        return false;
    }

    if (treeMemory(pCPU) > 0x400000) {
        root->kill_limit = pCPU->survivalTimer - 10;
    } else if (treeMemory(pCPU) > 3250000) {
        root->kill_limit += 95;
        if (root->kill_limit > pCPU->survivalTimer - 10) {
            root->kill_limit = pCPU->survivalTimer - 10;
        }
    } else {
        root->kill_limit = 0;
        root->restore = NULL;
        root->restore_side = 0;
    }

    return true;
}

static bool treeCleanNodes(Cpu* pCPU, CpuFunction* top) {
    CpuFunction** current;
    CpuFunction* kill = NULL;
    CpuTreeRoot* root = pCPU->gTree;
    s32 kill_limit = root->kill_limit;
    CpuFunction* temp;

    if (top == NULL) {
        root->side ^= 1;
        return true;
    }

    current = &root->restore;
    if (root->restore == NULL) {
        *current = top;
    }

    while (*current != NULL) {
        if (pCPU->nRetrace != pCPU->nRetraceUsed || root->kill_number >= 12) {
            break;
        }

        if (root->restore_side == 0) {
            while ((*current)->left != NULL) {
                *current = (*current)->left;
            }
            root->restore_side = 1;
        }

        while (*current != NULL) {
            if (pCPU->nRetrace != pCPU->nRetraceUsed || root->kill_number >= 12) {
                break;
            }

            if (kill != NULL) {
                if (!cpuFreeCachedAddress(pCPU, kill->nAddress0, kill->nAddress1)) {
                    return false;
                }
                if (!treeDeleteNode(pCPU, &top, kill)) {
                    return false;
                }
                kill = NULL;
                root->kill_number++;
            }

            temp = *current;
            if (temp->timeToLive > 0 && temp->timeToLive <= kill_limit) {
                kill = *current;
            }

            if ((*current)->right != NULL) {
                *current = (*current)->right;
                root->restore_side = 0;
                break;
            }

            if (*current == top) {
                if (kill != NULL) {
                    if (!cpuFreeCachedAddress(pCPU, kill->nAddress0, kill->nAddress1)) {
                        return false;
                    }
                    if (!treeDeleteNode(pCPU, &top, kill)) {
                        return false;
                    }
                }

                root->side ^= 1;
                *current = NULL;
                root->restore_side = 0;
                return true;
            }

            while (*current != (*current)->prev->left) {
                *current = (*current)->prev;
                if (*current == top) {
                    if (kill != NULL) {
                        if (!cpuFreeCachedAddress(pCPU, kill->nAddress0, kill->nAddress1)) {
                            return false;
                        }
                        if (!treeDeleteNode(pCPU, &top, kill)) {
                            return false;
                        }
                    }

                    root->side ^= 1;
                    *current = NULL;
                    root->restore_side = 0;
                    return true;
                }
            }

            *current = (*current)->prev;
            root->restore_side = 1;
        }
    }

    if (kill != NULL) {
        if (!cpuFreeCachedAddress(pCPU, kill->nAddress0, kill->nAddress1)) {
            return false;
        }
        if (!treeDeleteNode(pCPU, &top, kill)) {
            return false;
        }
    }
    return false;
}

static bool treeForceCleanNodes(Cpu* pCPU, CpuFunction* tree, s32 kill_limit) {
    CpuFunction* current;
    CpuFunction* kill = NULL;

    if (tree == NULL) {
        return false;
    }
    current = tree;

    do {
        while (current->left != NULL) {
            current = current->left;
        }

        do {
            if (kill != NULL) {
                if (!cpuFreeCachedAddress(pCPU, kill->nAddress0, kill->nAddress1)) {
                    return false;
                }
                if (!treeDeleteNode(pCPU, &tree, kill)) {
                    return false;
                }
                kill = NULL;
            }

            if (current->timeToLive > 0 && current->timeToLive <= kill_limit) {
                kill = current;
            }

            if (current->right != NULL) {
                current = current->right;
                break;
            }

            if (current == tree) {
                if (kill != NULL) {
                    if (!cpuFreeCachedAddress(pCPU, kill->nAddress0, kill->nAddress1)) {
                        return false;
                    }
                    if (!treeDeleteNode(pCPU, &tree, kill)) {
                        return false;
                    }
                }
                return true;
            }

            while (current != current->prev->left) {
                current = current->prev;
                if (current == tree) {
                    if (kill != NULL) {
                        if (!cpuFreeCachedAddress(pCPU, kill->nAddress0, kill->nAddress1)) {
                            return false;
                        }
                        if (!treeDeleteNode(pCPU, &tree, kill)) {
                            return false;
                        }
                    }
                    return true;
                }
            }

            current = current->prev;
        } while (current != NULL);
    } while (current != NULL);

    return false;
}

static bool treePrintNode(Cpu* pCPU, CpuFunction* tree, s32 print_flag, s32* left, s32* right) {
    CpuFunction* current;
    bool flag;
    s32 level;

    level = 0;
    if (tree == NULL) {
        return false;
    }

    flag = ganMapGPR[31] & 0x100 ? true : false;
    current = tree;

    while (true) {
        while (current->left != NULL) {
            current = current->left;
            level++;
            if (print_flag & 1) {
                if (level > *left) {
                    (*left)++;
                }
            }
        }

        do {
            if (print_flag & 0x10) {
                treeCallerCheck(pCPU, current, flag, *left, *right);
            } else if (print_flag & 0x100) {
                if (current->timeToLive > 0) {
                    if (current->timeToLive > *left) {
                        *left = current->timeToLive;
                    }
                    if (current->timeToLive < *right) {
                        *right = current->timeToLive;
                    }
                }
            } else if (print_flag & 0x1000) {
                if (current->timeToLive > 0) {
                    current->timeToLive -= *left;
                }
            }

            if (current->right != NULL) {
                current = current->right;
                level++;
                if (print_flag & 1) {
                    if (level > *right) {
                        (*right)++;
                    }
                }
                break;
            }

            if (current == tree) {
                return true;
            }

            while (current != current->prev->left) {
                current = current->prev;
                level--;
                if (current == tree) {
                    return true;
                }
            }

            current = current->prev;
        } while (current != NULL);

        if (current == NULL) {
            return false;
        }
    }

    return false;
}
