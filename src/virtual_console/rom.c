#include "ram.h"
#include "rom.h"
#include "system.h"
#include "xlFile.h"
#include "xlHeap.h"

s32 func_80042E68(rom_class_t*);
s32 func_800428E8(rom_class_t*);
s32 func_80042A50(rom_class_t*);

s32 func_800070C4(s32, s32, u32, u32);
s32 func_800070D4(); // simulatorDVDRead

s32 func_800426E0(rom_class_t*);

s32 func_800A84F8(s32*);

void func_800427C4(void);

void func_80042064(void); // wrong prototype

s32 romGetPC(rom_class_t* rom, u64* pc) {
    u32 crc;
    s32 i;
    u32 drom[0x1000 / sizeof(u32)];
    u32 off;

    if (romCopy(rom, (void*)drom, 0, sizeof(drom), NULL)) {
        crc = 0;
        for (i = 0; i < (0x1000 - 0x40) / 4; i++) {
            crc += drom[i + 0x40 / 4];
        }

        switch (crc) {
            case 0x49F60E96:
            case 0xFB631223:
            case 0x2ADFE50A:
            case 0x57C85244:
                off = 0;
                break;
            case 0x027FDF31:
                off = -0x80;
                break;
            case 0x497E414B:
            case 0xE6DECB4B:
            case 0x27C4ED44:
                off = 0x100000;
                break;
            case 0xD5BE5580:
                off = 0x200000;
                break;
            default:
                return 0;
        }
        *pc = drom[2] - off;
        return 1;
    } else {
        return 0;
    }
}

s32 romGetCode(rom_class_t* rom, rom_id_t* rom_id) {
    *rom_id = (rom->unk_19AAF << 0x18) | (rom->unk_19AB0 << 0x10) | (rom->unk_19AB1 << 0x8) | rom->unk_19AB2;
    return 1;
}

s32 romPut8(void* obj, u32 addr, s8* src) {
    return 1;
}

s32 romPut16(void* obj, u32 addr, s16* src) {
    return 1;
}

s32 romPut32(void* obj, u32 addr, s32* src) {
    return 1;
}

s32 romPut64(void* obj, u32 addr, s64* src) {
    return 1;
}

s32 romGet8(void* obj, u32 addr, s8* dst) {
    rom_class_t* rom = (rom_class_t*)obj;
    s8 buf[1];

    if ((addr & 0x7FFFFFF) < rom->unk_214 && romCopy(rom, buf, addr & 0x7FFFFFF, sizeof(s8), NULL)) {
        *dst = buf[0];
        return 1;
    }
    return 1;
}

s32 romGet16(void* obj, u32 addr, s16* dst) {
    rom_class_t* rom = (rom_class_t*)obj;
    s16 buf[1];

    if ((addr & 0x7FFFFFF) < rom->unk_214 && romCopy(rom, buf, addr & 0x7FFFFFF, sizeof(s16), NULL)) {
        *dst = buf[0];
        return 1;
    }
    return 1;
}

s32 romGet32(void* obj, u32 addr, s32* dst) {
    rom_class_t* rom = (rom_class_t*)obj;
    s32 buf[1];

    if ((addr & 0x7FFFFFF) < rom->unk_214 && romCopy(rom, buf, addr & 0x7FFFFFF, sizeof(s32), NULL)) {
        *dst = buf[0];
        return 1;
    }
    return 1;
}

s32 romGet64(void* obj, u32 addr, s64* dst) {
    rom_class_t* rom = (rom_class_t*)obj;
    s64 buf[1];

    if ((addr & 0x7FFFFFF) < rom->unk_214 && romCopy(rom, buf, addr & 0x7FFFFFF, sizeof(s64), NULL)) {
        *dst = buf[0];
        return 1;
    }
    return 1;
}

s32 romGetBlock(rom_class_t* rom, cpu_blk_req_t* req) {
    char* buf;

    if (req->dst_phys_ram < 0x4000000) {
        if (!ramGetBuffer(gSystem->ram, (void**)&buf, req->dst_phys_ram, &req->len)) {
            return 0;
        }
        if (!romCopy(rom, buf, req->dev_addr, req->len, NULL)) {
            return 0;
        }
    }
    if (!(*req->handler)(req, 1)) {
        return 0;
    }
    return 1;
}

s32 romPutDebug8(void* obj, u32 addr, s8* src) {
    return 1;
}

s32 romPutDebug16(void* obj, u32 addr, s16* src) {
    return 1;
}

s32 romPutDebug32(void* obj, u32 addr, s32* src) {
    return 1;
}

s32 romPutDebug64(void* obj, u32 addr, s64* src) {
    return 1;
}

s32 romGetDebug8(void* obj, u32 addr, s8* dst) {
    *dst = 0;
    return 1;
}

s32 romGetDebug16(void* obj, u32 addr, s16* dst) {
    *dst = 0;
    return 1;
}

s32 romGetDebug32(void* obj, u32 addr, s32* dst) {
    *dst = 0;
    return 1;
}

s32 romGetDebug64(void* obj, u32 addr, s64* dst) {
    *dst = 0;
    return 1;
}

inline s32 romCopyLoop(rom_class_t* rom, void* dst, u32 addr, s32 len, unk_rom_callback callback) {
    s32 i;
    rom->unk_19A28 = 0;
    rom->unk_19A34 = len;
    rom->unk_19A30 = dst;
    rom->unk_19A38 = addr;
    rom->unk_19A2C = callback;

    for (i = 0; i < rom->unk_19AB8; i += 2) {
        if (rom->unk_19AB4[i] <= addr && addr <= rom->unk_19AB4[i + 1]) {
            rom->unk_19A5C = rom->unk_19AB4[i];
            rom->unk_19A60 = rom->unk_19AB4[i + 1];
            return 1;
        }
    }
    return 0;
}

inline s32 romCopyUnkInline(rom_class_t* rom) {
    if (!func_80042E68(rom)) {
        return 0;
    }
    rom->unk_8 = 0;
    return 1;
}

s32 romCopy(rom_class_t* rom, void* dst, u32 addr, s32 len, unk_rom_callback callback) {
    file_class_t* file;

    addr &= 0x7FFFFFF;
    if (rom->unk_19A70 == 0) {
        if (!xlFileOpen(&file, 1, rom->rom_fn)) {
            return 0;
        } else if (!func_8008039C(file, addr + rom->unk_19AFC)) {
            return 0;
        } else if (!xlFileRead(file, dst, len)) {
            return 0;
        } else if (!xlFileClose(&file)) {
            return 0;
        } else if (callback != NULL && !callback()) {
            return 0;
        } else {
            return 1;
        }
    } else if (rom->unk_8 != 0 && !romCopyUnkInline(rom)) {
        return 0;
    } else if (addr + len > rom->unk_214 && (len = rom->unk_214 - addr, len < 0)) {
        return 1;
    } else if (rom->unk_21C == 0) {
        if (romCopyLoop(rom, dst, addr, len, callback) && !func_800428E8(rom)) {
            return 0;
        } else if (!func_80042A50(rom)) {
            return 0;
        } else {
            return 1;
        }
    } else if (rom->unk_21C == 1) {
        if (!xlHeapCopy(dst, (void*)((u32)rom->common.ref_list + addr), len)) {
            return 0;
        } else if (callback != NULL && !callback()) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

s32 romUpdate(rom_class_t* rom) {
    s32 ret;
    s32 iVar2;
    s32 fret;

    if (rom->unk_19A28 != 0 || rom->unk_19A3C != 0) {
        if (rom->unk_19A40 != 0 && rom->unk_19A44 == rom->unk_19A68) {
            rom->unk_19A40 = 0;
            if (!func_800426E0(rom)) {
                return 0;
            }
        }
        ret = func_800A84F8(&rom->unk_19AC0);
        if (ret != 1) {
            if (func_800070C4(ret, rom->unk_19A48, rom->unk_19A68, rom->unk_19AFC + rom->unk_19A58) == 0) {
                return 0;
            }
            if ((ret == 0xb || ret == -1) && !func_800070D4(&rom->unk_19AC0, rom->unk_19A48, rom->unk_19A68,
                                                            rom->unk_19AFC + rom->unk_19A58, func_800427C4)) {
                return 0;
            }
        }
    }
    if (!func_800428E8(rom)) {
        fret = 0;
    } else {
        ret = func_80042A50(rom);
        fret = ((u8)((-ret >> 0x18) | (ret >> 0x18)) >> 7);
    }
    return fret;
}

s32 romSetImage(rom_class_t* rom, char* rom_fn) {
    s32 fret;
    s32 i;
    s32 size;
    file_class_t* file;
    s32 file_size;
    u32 local_410[0x400 / sizeof(u32)];

    for (i = 0; rom_fn[i] != '\0' && i < 0x200; i++) {
        rom->rom_fn[i] = rom_fn[i];
    }
    rom->rom_fn[i] = '\0';

    if (func_80082074(&file_size, rom->rom_fn)) {
        rom->unk_214 = file_size - rom->unk_19AFC;
        size = (file_size + 0x1FFF) & ~0x1FFF;
        rom->unk_19A6C = size / 0x2000;
        rom->unk_19A70 = size;
        if (!xlHeapTake((void**)&rom->common.ref_list, size | 0x70000000) &&
            !func_8005F5F4(gSystem->help_menu, rom, size | 0x70000000, func_80042064)) {
            fret = 0;
        } else {
            fret = 1;
            rom->unk_18224 = rom->common.ref_list;
        }
        if (!fret) {
            return 0;
        }
    } else {
        return 0;
    }
    if (!xlFileOpen(&file, 1, rom_fn)) {
        return 0;
    } else if (!func_8008039C(file, rom->unk_19AFC)) {
        return 0;
    } else if (!xlFileRead(file, (char*)&rom->unk_19A74, 0x40)) {
        return 0;
    } else if (!func_8008039C(file, rom->unk_19AFC + 0x1000)) {
        return 0;
    } else if (!xlFileRead(file, (char*)&local_410, 0x400)) {
        return 0;
    } else if (!xlFileClose(&file)) {
        return 0;
    } else {
        rom->unk_19ABC = 0;
        for (i = 0; i < (0x400) / 4; i++) {
            rom->unk_19ABC += local_410[i];
        }
        rom->common.unk_4 = (rom->unk_19A74 == 0x37 && rom->unk_19A75 == 0x80);
        return 1;
    }
}

s32 romGetImage(rom_class_t* rom, char* buf) {
    if (rom->rom_fn[0] == '\0') {
        return 0;
    }
    if (buf != NULL) {
        s32 iVar1;
        for (iVar1 = 0; rom->rom_fn[iVar1] != '\0'; iVar1++) {
            buf[iVar1] = rom->rom_fn[iVar1];
        }
    }
    return 1;
}

s32 romGetBuffer(rom_class_t* rom, void** param_2, u32 addr, s32* param_4) {
    if (rom->unk_21C == 1) {
        addr &= 0x7ffffff;

        if (param_4 != NULL) {
            u32 uVar1 = rom->unk_214;
            if (addr >= uVar1) {
                return 0;
            }
            if (addr + *param_4 > uVar1) {
                *param_4 -= (uVar1 - addr);
            }
        }
        *param_2 = (void*)((u32)rom->common.ref_list + addr);
        return 1;
    }
    return 0;
}

s32 romEvent(void* obj, event_t event, void* arg) {
    rom_class_t* rom = (rom_class_t*)obj;
    cpu_dev_t* cpu_dev = (cpu_dev_t*)arg;

    switch (event) {
        case 2:
            rom->unk_214 = 0;
            rom->unk_18220 = 0;
            rom->unk_8 = 1;
            rom->common.unk_4 = 0;
            rom->rom_fn[0] = '\0';
            rom->unk_21C = 0xffffffff;
            rom->unk_C = 1;
            rom->common.ref_list = NULL;
            rom->unk_19AFC = 0;
            rom->unk_19AB4 = NULL;
            rom->unk_19AB8 = 0;
            rom->unk_19A34 = 0;
            rom->unk_19A28 = 0;
            rom->unk_19A3C = 0;
            rom->unk_19A60 = 0;
            rom->unk_19A5C = 0;
            rom->unk_19A40 = 0;
            rom->unk_19A70 = 0;
            rom->unk_19A6C = 0;
            rom->unk_18224 = 0;
            break;
        case 3:
            if (rom->common.ref_list != NULL) {
                OSSetArena2Lo(rom->common.ref_list);
            }
            break;
        case 0x1002:
            switch (cpu_dev->unk_0) {
                case 0:
                    if (!cpuSetGetBlock(gSystem->cpu, cpu_dev, romGetBlock)) {
                        return 0;
                    }
                    if (!cpuSetDevicePut(gSystem->cpu, cpu_dev, romPut8, romPut16, romPut32, romPut64)) {
                        return 0;
                    }
                    if (!cpuSetDeviceGet(gSystem->cpu, cpu_dev, romGet8, romGet16, romGet32, romGet64)) {
                        return 0;
                    }
                    break;
                case 1:
                    if (!cpuSetDevicePut(gSystem->cpu, cpu_dev, romPutDebug8, romPutDebug16, romPutDebug32,
                                         romPutDebug64)) {
                        return 0;
                    }
                    if (!cpuSetDeviceGet(gSystem->cpu, cpu_dev, romGetDebug8, romGetDebug16, romGetDebug32,
                                         romGetDebug64)) {
                        return 0;
                    }
                    break;
            }
            break;
        case 0:
        case 1:
        case 0x1003:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return 0;
    }
    return 1;
}
