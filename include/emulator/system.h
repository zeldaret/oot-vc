#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "emulator/cpu.h"
#include "emulator/mi.h"
#include "emulator/xlObject.h"
#include "macros.h"
#include "revolution/pad.h"
#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// `C` if `eTypeROM` is `CZLJ`
#define GET_GAME_MEDIUM(eTypeROM) ((eTypeROM >> 24) & 0xFF)

// `Z` if `eTypeROM` is `CZLJ`
#define GET_GAME_ID_1(eTypeROM) ((eTypeROM >> 16) & 0xFF)

// `L` if `eTypeROM` is `CZLJ`
#define GET_GAME_ID_2(eTypeROM) ((eTypeROM >> 8) & 0xFF)

// `J` if `eTypeROM` is `CZLJ`
#define GET_GAME_REGION(eTypeROM) (eTypeROM & 0xFF)

#define N64_BTN_A (1 << 31)
#define N64_BTN_B (1 << 30)
#define N64_BTN_Z (1 << 29)
#define N64_BTN_START (1 << 28)
#define N64_BTN_DUP (1 << 27)
#define N64_BTN_DDOWN (1 << 26)
#define N64_BTN_DLEFT (1 << 25)
#define N64_BTN_DRIGHT (1 << 24)
#define N64_BTN_L (1 << 21)
#define N64_BTN_R (1 << 20)
#define N64_BTN_CUP (1 << 19)
#define N64_BTN_CDOWN (1 << 18)
#define N64_BTN_CLEFT (1 << 17)
#define N64_BTN_CRIGHT (1 << 16)
#define N64_BTN_UNSET 0

// note: each stick direction counts as an input
typedef enum GcnButton {
    GCN_BTN_A = 0,
    GCN_BTN_B = 1,
    GCN_BTN_X = 2,
    GCN_BTN_Y = 3,
    GCN_BTN_L = 4,
    GCN_BTN_R = 5,
    GCN_BTN_Z = 6,
    GCN_BTN_START = 7,
    GCN_BTN_UNK8 = 8,
    GCN_BTN_UNK9 = 9,
    GCN_BTN_UNK10 = 10,
    GCN_BTN_UNK11 = 11,
    GCN_BTN_DPAD_UP = 12,
    GCN_BTN_DPAD_DOWN = 13,
    GCN_BTN_DPAD_LEFT = 14,
    GCN_BTN_DPAD_RIGHT = 15,
    GCN_BTN_CSTICK_UP = 16,
    GCN_BTN_CSTICK_DOWN = 17,
    GCN_BTN_CSTICK_LEFT = 18,
    GCN_BTN_CSTICK_RIGHT = 19,
    GCN_BTN_COUNT = 20,
} GcnButton;

typedef enum SystemMode {
    SM_NONE = -1,
    SM_RUNNING = 0,
    SM_STOPPED = 1,
} SystemMode;

typedef enum SystemRomType {
    NONE = 0,
    NTEJ = 'NTEJ', // 1080° Snowboarding (JP)
    NTEA = 'NTEA', // 1080° Snowboarding (US)
    NTEP = 'NTEP', // 1080° Snowboarding (EU)
    NBYJ = 'NBYJ', // A Bug's Life (JP)
    NBYE = 'NBYE', // A Bug's Life (US)
    NBYP = 'NBYP', // A Bug's Life (EU)
    NABJ = 'NABJ', // Airboarder 64 (JP)
    NABE = 'NABE', // Airboarder 64 (US)
    NABP = 'NABP', // Airboarder 64 (EU)
    NAFJ = 'NAFJ', // Animal Forest (JP)
    NAFE = 'NAFE', // Animal Forest (US)
    NAFP = 'NAFP', // Animal Forest (EU)
    NBNJ = 'NBNJ', // Bangai-O (JP)
    NBNE = 'NBNE', // Bangai-O (US)
    NBNP = 'NBNP', // Bangai-O (EU)
    NBCJ = 'NBCJ', // Blast Corps (JP)
    NBCE = 'NBCE', // Blast Corps (US)
    NBCP = 'NBCP', // Blast Corps (EU)
    NTSJ = 'NTSJ', // Center Court Tennis (JP)
    NFUJ = 'NFUJ', // Conker's Bad Fur Day (JP)
    NFUE = 'NFUE', // Conker's Bad Fur Day (US)
    NFUP = 'NFUP', // Conker's Bad Fur Day (EU)
    NCUJ = 'NCUJ', // Cruis'n USA (JP)
    NCUE = 'NCUE', // Cruis'n USA (US)
    NCUP = 'NCUP', // Cruis'n USA (EU)
    NDYJ = 'NDYJ', // Diddy Kong Racing (JP)
    NDYE = 'NDYE', // Diddy Kong Racing (US)
    NDYP = 'NDYP', // Diddy Kong Racing (EU)
    NDOJ = 'NDOJ', // Donkey Kong 64 (JP)
    NDOE = 'NDOE', // Donkey Kong 64 (US)
    NDOP = 'NDOP', // Donkey Kong 64 (EU)
    NN6J = 'NN6J', // Dr. Mario 64 (JP)
    NN6E = 'NN6E', // Dr. Mario 64 (US)
    NN6P = 'NN6P', // Dr. Mario 64 (EU)
    NFZJ = 'NFZJ', // F-Zero X (JP)
    CFZE = 'CFZE', // F-Zero X (US)
    NFZP = 'NFZP', // F-Zero X (EU)
    NSIJ = 'NSIJ', // Fushigi no Dungeon Furai no Siren 2 Oni Shuurai! Siren Shiro! (JP)
    NK4J = 'NK4J', // Kirby 64: The Crystal Shards (JP)
    NK4E = 'NK4E', // Kirby 64: The Crystal Shards (US)
    NK4P = 'NK4P', // Kirby 64: The Crystal Shards (EU)
    NLRJ = 'NLRJ', // Lode Runner 3-D (JP)
    NLRE = 'NLRE', // Lode Runner 3-D (US)
    NLRP = 'NLRP', // Lode Runner 3-D (EU)
    NMFJ = 'NMFJ', // Mario Golf (JP)
    NMFE = 'NMFE', // Mario Golf (US)
    NMFP = 'NMFP', // Mario Golf (EU)
    NKTJ = 'NKTJ', // Mario Kart 64 (JP)
    NKTE = 'NKTE', // Mario Kart 64 (US)
    NKTP = 'NKTP', // Mario Kart 64 (EU)
    CLBJ = 'CLBJ', // Mario Party (JP)
    CLBE = 'CLBE', // Mario Party (US)
    CLBP = 'CLBP', // Mario Party (EU)
    NMWJ = 'NMWJ', // Mario Party 2 (JP)
    NMWE = 'NMWE', // Mario Party 2 (US)
    NMWP = 'NMWP', // Mario Party 2 (EU)
    NMVJ = 'NMVJ', // Mario Party 3 (JP)
    NMVE = 'NMVE', // Mario Party 3 (US)
    NMVP = 'NMVP', // Mario Party 3 (EU)
    NM8J = 'NM8J', // Mario Tennis (JP)
    NM8E = 'NM8E', // Mario Tennis (US)
    NM8P = 'NM8P', // Mario Tennis (EU)
    NRBJ = 'NRBJ', // Mini Racers (JP)
    NRBE = 'NRBE', // Mini Racers (US)
    NRBP = 'NRBP', // Mini Racers (EU)
    NMQJ = 'NMQJ', // Paper Mario (JP)
    NMQE = 'NMQE', // Paper Mario (US)
    NMQP = 'NMQP', // Paper Mario (EU)
    NOBJ = 'NOBJ', // Ogre Battle 64: Person of Lordly Caliber (JP)
    NOBE = 'NOBE', // Ogre Battle 64: Person of Lordly Caliber (US)
    NOBP = 'NOBP', // Ogre Battle 64: Person of Lordly Caliber (EU)
    NYLJ = 'NYLJ', // Panel de Pon 64 (JP)
    NYLE = 'NYLE', // Panel de Pon 64 (US)
    NYLP = 'NYLP', // Panel de Pon 64 (EU)
    NPOJ = 'NPOJ', // Pokémon Stadium (JP)
    NPOE = 'NPOE', // Pokémon Stadium (US)
    NPOP = 'NPOP', // Pokémon Stadium (EU)
    NPWJ = 'NPWJ', // Pilotwings 64 (JP)
    NPWE = 'NPWE', // Pilotwings 64 (US)
    NPWP = 'NPWP', // Pilotwings 64 (EU)
    NQKJ = 'NQKJ', // Quake (JP)
    NQKE = 'NQKE', // Quake (US)
    NQKP = 'NQKP', // Quake (EU)
    NRXJ = 'NRXJ', // Robotron 64 (JP)
    NRXE = 'NRXE', // Robotron 64 (US)
    NRXP = 'NRXP', // Robotron 64 (EU)
    NGUJ = 'NGUJ', // Sin and Punishment: Hoshi no Keishosha (JP)
    NGUE = 'NGUE', // Sin and Punishment: Hoshi no Keishosha (US)
    NGUP = 'NGUP', // Sin and Punishment: Hoshi no Keishosha (EU)
    NFXJ = 'NFXJ', // Star Fox 64 (JP)
    NFXE = 'NFXE', // Star Fox 64 (US)
    NFXP = 'NFXP', // Star Fox 64 (EU)
    NSQJ = 'NSQJ', // Starcraft 64 (JP)
    NSQE = 'NSQE', // Starcraft 64 (US)
    NSQP = 'NSQP', // Starcraft 64 (EU)
    NSMJ = 'NSMJ', // Super Mario 64 (JP)
    NSME = 'NSME', // Super Mario 64 (US)
    NSMP = 'NSMP', // Super Mario 64 (EU)
    NALJ = 'NALJ', // Super Smash Bros. (JP)
    NALE = 'NALE', // Super Smash Bros. (US)
    NALP = 'NALP', // Super Smash Bros. (EU)
    NZSJ = 'NZSJ', // The Legend of Zelda: Majora's Mask (JP)
    NZSE = 'NZSE', // The Legend of Zelda: Majora's Mask (US)
    NZSP = 'NZSP', // The Legend of Zelda: Majora's Mask (EU)
    CZLJ = 'CZLJ', // The Legend of Zelda: Ocarina of Time (JP)
    CZLE = 'CZLE', // The Legend of Zelda: Ocarina of Time (US)
    NZLP = 'NZLP', // The Legend of Zelda: Ocarina of Time (EU)
    NRIJ = 'NRIJ', // The New Tetris (JP)
    NRIE = 'NRIE', // The New Tetris (US)
    NRIP = 'NRIP', // The New Tetris (EU)
    NTUJ = 'NTUJ', // Turok: Dinosaur Hunter (JP)
    NTUE = 'NTUE', // Turok: Dinosaur Hunter (US)
    NTUP = 'NTUP', // Turok: Dinosaur Hunter (EU)
    NWRJ = 'NWRJ', // Wave Race 64 (JP)
    NWRE = 'NWRE', // Wave Race 64 (US)
    NWRP = 'NWRP', // Wave Race 64 (EU)
    NYSJ = 'NYSJ', // Yoshi's Story (JP)
    NYSE = 'NYSE', // Yoshi's Story (US)
    NYSP = 'NYSP', // Yoshi's Story (EU)
} SystemRomType;

typedef enum SystemObjectType {
    SOT_NONE = -1,
    SOT_CPU = 0,
    SOT_PIF = 1,
    SOT_RAM = 2,
    SOT_ROM = 3,
    SOT_RSP = 4,
    SOT_RDP = 5,
    SOT_MI = 6,
    SOT_DISK = 7,
    SOT_AI = 8,
    SOT_VI = 9,
    SOT_SI = 10,
    SOT_PI = 11,
    SOT_RDB = 12,
    SOT_PAK = 13,
    SOT_SRAM = 14,
    SOT_FLASH = 15,
    SOT_CODE = 16,
    SOT_HELP = 17,
    SOT_LIBRARY = 18,
    SOT_FRAME = 19,
    SOT_AUDIO = 20,
    SOT_VIDEO = 21,
    SOT_CONTROLLER = 22,
    SOT_COUNT = 23,
} SystemObjectType;

typedef enum SystemInterruptType {
    SIT_NONE = -1,
    SIT_SW0 = 0,
    SIT_SW1 = 1,
    SIT_CART = 2,
    SIT_COUNTER = 3,
    SIT_RDB = 4,
    SIT_SP = 5,
    SIT_SI = 6,
    SIT_AI = 7,
    SIT_VI = 8,
    SIT_PI = 9,
    SIT_DP = 10,
    SIT_CPU_BREAK = 11,
    SIT_SP_BREAK = 12,
    SIT_FAULT = 13,
    SIT_THREADSTATUS = 14,
    SIT_PRENMI = 15,
    SIT_COUNT = 16,
} SystemInterruptType;

typedef struct SystemDeviceInfo {
    /* 0x00 */ s32 nType;
    /* 0x04 */ u32 nAddress0;
    /* 0x08 */ u32 nAddress1;
} SystemDeviceInfo; // size = 0xC

typedef struct SystemDevice {
    /* 0x00 */ SystemObjectType storageDevice;
    /* 0x04 */ _XL_OBJECTTYPE* pClass;
    /* 0x08 */ s32 nSlotUsed;
    /* 0x0C */ SystemDeviceInfo aDeviceSlot[3];
} SystemDevice; // size = 0x30

typedef struct SystemException {
    /* 0x00 */ char* szType;
    /* 0x04 */ u32 nMask;
    /* 0x08 */ CpuExceptionCode eCode;
    /* 0x0C */ MIInterruptType eTypeMips;
    /* 0x10 */ SystemInterruptType eType;
} SystemException; // size = 0x14

typedef struct System {
    /* 0x00 */ bool bException;
    /* 0x04 */ SystemMode eMode;
    /* 0x08 */ SystemObjectType storageDevice;
    /* 0x0C */ SystemRomType eTypeROM;
    /* 0x10 */ void* apObject[SOT_COUNT];
    /* 0x6C */ struct Store* unk_6C;
    /* 0x70 */ u64 nAddressBreak;
    /* 0x78 */ CpuBlock aBlock[4];
    /* 0xC8 */ u8 anException[16];
} System; // size = 0xD8

typedef struct SystemRomConfig {
    /* 0x0000 */ s32 controllerConfiguration[PAD_MAX_CONTROLLERS][GCN_BTN_COUNT];
    /* 0x0140 */ s32 rumbleConfiguration;
    /* 0x0144 */ SystemObjectType storageDevice;
    /* 0x0148 */ s32 normalControllerConfig;
    /* 0x014C */ s32 currentControllerConfig;
} SystemRomConfig; // size = 0x150

#define SYSTEM_CPU(pSystem) ((Cpu*)(((System*)(pSystem))->apObject[SOT_CPU]))
#define SYSTEM_PIF(pSystem) ((Pif*)(((System*)(pSystem))->apObject[SOT_PIF]))
#define SYSTEM_RAM(pSystem) ((Ram*)(((System*)(pSystem))->apObject[SOT_RAM]))
#define SYSTEM_ROM(pSystem) ((Rom*)(((System*)(pSystem))->apObject[SOT_ROM]))
#define SYSTEM_RSP(pSystem) ((Rsp*)(((System*)(pSystem))->apObject[SOT_RSP]))
#define SYSTEM_RDP(pSystem) ((Rdp*)(((System*)(pSystem))->apObject[SOT_RDP]))
#define SYSTEM_MI(pSystem) ((MI*)(((System*)(pSystem))->apObject[SOT_MI]))
#define SYSTEM_DISK(pSystem) ((Disk*)(((System*)(pSystem))->apObject[SOT_DISK]))
#define SYSTEM_AI(pSystem) ((AI*)(((System*)(pSystem))->apObject[SOT_AI]))
#define SYSTEM_VI(pSystem) ((VI*)(((System*)(pSystem))->apObject[SOT_VI]))
#define SYSTEM_SI(pSystem) ((SI*)(((System*)(pSystem))->apObject[SOT_SI]))
#define SYSTEM_PI(pSystem) ((PI*)(((System*)(pSystem))->apObject[SOT_PI]))
#define SYSTEM_RDB(pSystem) ((Rdb*)(((System*)(pSystem))->apObject[SOT_RDB]))
#define SYSTEM_PAK(pSystem) ((Pak*)(((System*)(pSystem))->apObject[SOT_PAK]))
#define SYSTEM_SRAM(pSystem) ((Sram*)(((System*)(pSystem))->apObject[SOT_SRAM]))
#define SYSTEM_FLASH(pSystem) ((Flash*)(((System*)(pSystem))->apObject[SOT_FLASH]))
#define SYSTEM_CODE(pSystem) ((Code*)(((System*)(pSystem))->apObject[SOT_CODE]))
#define SYSTEM_HELP(pSystem) ((HelpMenu*)(((System*)(pSystem))->apObject[SOT_HELP]))
#define SYSTEM_LIBRARY(pSystem) ((Library*)(((System*)(pSystem))->apObject[SOT_LIBRARY]))
#define SYSTEM_FRAME(pSystem) ((Frame*)(((System*)(pSystem))->apObject[SOT_FRAME]))
#define SYSTEM_SOUND(pSystem) ((Sound*)(((System*)(pSystem))->apObject[SOT_AUDIO])) // not `Audio*`?
#define SYSTEM_VIDEO(pSystem) ((Video*)(((System*)(pSystem))->apObject[SOT_VIDEO]))
#define SYSTEM_CONTROLLER(pSystem) ((Controller*)(((System*)(pSystem))->apObject[SOT_CONTROLLER]))

bool systemSetStorageDevice(System* pSystem, SystemObjectType eStorageDevice, void* pArgument) NO_INLINE;
bool systemCreateStorageDevice(System* pSystem, void* pArgument);
bool fn_8000A6A4(System* pSystem, CpuBlock* pBlock);
bool systemSetMode(System* pSystem, SystemMode eMode);
bool systemGetMode(System* pSystem, SystemMode* peMode);
bool fn_8000A830(System* pSystem, s32 nEvent, void* pArgument);
bool fn_8000A8A8(System* pSystem);
bool systemReset(System* pSystem);
bool systemExecute(System* pSystem, s32 nCount);
bool systemCheckInterrupts(System* pSystem);
bool systemExceptionPending(System* pSystem, SystemInterruptType nException);
bool systemEvent(System* pSystem, s32 nEvent, void* pArgument);

extern _XL_OBJECTTYPE gClassSystem;

#ifdef __cplusplus
}
#endif

#endif
