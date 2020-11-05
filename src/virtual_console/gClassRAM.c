#include "types.h"
#include "xlHeap.h"
#include "CPUClass.h"
#include "RAMClass.h"
#include "SystemClass.h"

/* RDRAM Interface Registers */
#define RI_MODE 0
#define RI_CONFIG 4
#define RI_CURRENT_LOAD 8
#define RI_SELECT 0xC
#define RI_REFRESH 0x10
#define RI_LATENCY 0x14
#define RI_RERROR 0x18
#define RI_WERROR 0x1C

/* RDRAM Control Registers */
#define RDRAM_CONFIG 0
#define RDRAM_DEVICE_ID 4
#define RDRAM_DELAY 8
#define RDRAM_MODE 0xC
#define RDRAM_REF_INTERVAL 0x10
#define RDRAM_REF_NOW 0x14
#define RDRAM_RAS_INTERVAL 0x18
#define RDRAM_MIN_INTERVAL 0x1C
#define RDRAM_ADDR_SELECT 0x20
#define RDRAM_DEVICE_MANUF 0x24

s32 ramPutControl8(void *obj, u32 addr, s8 *src) {
    return 0;
}

s32 ramPutControl16(void *obj, u32 addr, s16 *src) {
    return 0;
}

s32 ramPutControl32(void *obj, u32 addr, s32 *src) {
    ram_class_t *ram = (ram_class_t*)obj;
    switch(addr & 0x3F) {
        case RDRAM_CONFIG:
            ram->RDRAM_CONFIG_REG = *src;
            break;
        case RDRAM_DEVICE_ID:
            ram->RDRAM_DEVICE_ID_REG = *src;
            break;
        case RDRAM_DELAY:
            ram->RDRAM_DELAY_REG = *src;
            break;
        case RDRAM_MODE:
            ram->RDRAM_MODE_REG = *src;
            break;
        case RDRAM_REF_INTERVAL:
            ram->RDRAM_REF_INTERVAL_REG = *src;
            break;
        case RDRAM_REF_NOW:
            ram->RDRAM_REF_ROW_REG = *src;
            break;
        case RDRAM_RAS_INTERVAL:
            ram->RDRAM_RAS_INTERVAL_REG = *src;
            break;
        case RDRAM_MIN_INTERVAL:
            ram->RDRAM_MIN_INTERVAL_REG = *src;
            break;
        case RDRAM_ADDR_SELECT:
            ram->RDRAM_ADDR_SELECT_REG = *src;
            break;
        case RDRAM_DEVICE_MANUF:
            ram->RDRAM_DEVICE_MANUF_REG = *src;
            break;
        default:
            return 0;
    }
    return 1;
}

s32 ramPutControl64(void *obj, u32 addr, s64 *src) {
    return 0;
}

s32 ramGetControl8(void *obj, u32 addr, s8 *dst) {
    return 0;
}

s32 ramGetControl16(void *obj, u32 addr, s16 *dst) {
    return 0;
}

s32 ramGetControl32(void *obj, u32 addr, s32 *dst) {
    ram_class_t *ram = (ram_class_t*)obj;
    *dst = 0;
    switch(addr & 0x3F) {
        case RDRAM_CONFIG:
            *dst = ram->RDRAM_CONFIG_REG;
            break;
        case RDRAM_DEVICE_ID:
            *dst = ram->RDRAM_DEVICE_ID_REG;
            break;
        case RDRAM_DELAY:
            *dst = ram->RDRAM_DELAY_REG;
            break;
        case RDRAM_MODE:
            *dst = ram->RDRAM_MODE_REG;
            break;
        case RDRAM_REF_INTERVAL:
            *dst = ram->RDRAM_REF_INTERVAL_REG;
            break;
        case RDRAM_REF_NOW:
            *dst = ram->RDRAM_REF_ROW_REG;
            break;
        case RDRAM_RAS_INTERVAL:
            *dst = ram->RDRAM_RAS_INTERVAL_REG;
            break;
        case RDRAM_MIN_INTERVAL:
            *dst = ram->RDRAM_MIN_INTERVAL_REG;
            break;
        case RDRAM_ADDR_SELECT:
            *dst = ram->RDRAM_ADDR_SELECT_REG;
            break;
        case RDRAM_DEVICE_MANUF:
            *dst = ram->RDRAM_DEVICE_MANUF_REG;
            break;
        default:
            return 0;
    }
    return 1;
}

s32 ramGetControl64(void *obj, u32 addr, s64 *dst) {
    return 0;
}

s32 ramPutRI8(void *obj, u32 addr, s8 *src) {
    return 0;
}

s32 ramPutRI16(void *obj, u32 addr, s16 *src) {
    return 0;
}

s32 ramPutRI32(void *obj, u32 addr, s32 *src) {
    ram_class_t *ram = (ram_class_t*)obj;
    switch(addr & 0x1F) {
        case RI_MODE:
            ram->RI_MODE_REG = *src & 0xF;
            break;
        case RI_CONFIG:
            ram->RI_CONFIG_REG = *src & 0x7F;
        case RI_CURRENT_LOAD:
        case RI_RERROR:
        case RI_WERROR:
            break;
        case RI_SELECT:
            ram->RI_SELECT_REG = *src & 0x7;
            break;
        case RI_REFRESH:
            ram->RI_REFRESH_REG = *src;
            break;
        case RI_LATENCY:
            ram->RI_LATENCY_REG = *src & 0xF;
            break;
        default:
            return 0;
    }
    return 1;
}

s32 ramPutRI64(void *obj, u32 addr, s64 *src) {
    return 0;
}

s32 ramGetRI8(void *obj, u32 addr, s8 *dst) {
    return 0;
}

s32 ramGetRI16(void *obj, u32 addr, s16 *dst) {
    return 0;
}

s32 ramGetRI32(void *obj, u32 addr, s32 *dst) {
    ram_class_t *ram = (ram_class_t*)obj;
    switch(addr & 0x1F) {
        case RI_MODE:
            *dst = ram->RI_MODE_REG & 0xF;
            break;
        case RI_CONFIG:
            *dst = ram->RI_CONFIG_REG & 0x7F;
            break;
        case RI_CURRENT_LOAD:
        case RI_WERROR:
            break;
        case RI_SELECT:
            *dst = ram->RI_SELECT_REG & 7;
            break;
        case RI_REFRESH:
            *dst = ram->RI_REFRESH_REG;
            break;
        case RI_LATENCY:
            *dst = ram->RI_LATENCY_REG & 0xF;
            break;
        case RI_RERROR:
            *dst = 0;
            break;
        default:
            return 0;
    }
    return 1;
}

s32 ramGetRI64(void *obj, u32 addr, s64 *dst) {
    return 0;
}

s32 ramPut8(void *obj, u32 addr, s8 *src) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        ram->dram[addr] = *src;
    }
    return 1;
}

s32 ramPut16(void *obj, u32 addr, s16 *src) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        *(u16*)&ram->dram[addr & ~1] = *src;
    }
    return 1;
}

s32 ramPut32(void *obj, u32 addr, s32 *src) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        *(u32*)&ram->dram[addr & ~3] = *src;
    }
    return 1;
}

s32 ramPut64(void *obj, u32 addr, s64 *src) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        *(s64*)&ram->dram[addr & ~7] = *src;
    }
    return 1;
}

s32 ramGet8(void *obj, u32 addr, s8 *dst) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        *dst = ram->dram[addr];
    } else {
        *dst = 0;
    }
    return 1;
}

s32 ramGet16(void *obj, u32 addr, s16 *dst) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        *dst = *(s16*)&ram->dram[addr & ~1];
    } else {
        *dst = 0;
    }
    return 1;
}

s32 ramGet32(void *obj, u32 addr, s32 *dst) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        *dst = *(s32*)&ram->dram[addr & ~3];
    } else {
        *dst = 0;
    }
    return 1;
}

s32 ramGet64(void *obj, u32 addr, s64 *dst) {
    ram_class_t *ram = (ram_class_t*)obj;
    addr = addr & 0x3FFFFFF;
    if(addr < ram->dram_size) {
        *dst = *(s64*)&ram->dram[addr & ~7];
    } else {
        *dst = 0;
    }
    return 1;
}

typedef struct unk_func_80041C90_s unk_func_80041C90_t;
struct unk_func_80041C90_s{
    char unk_0x00[0x8];
    s32 (*unk_0x08)(unk_func_80041C90_t*, s32);
};

s32 func_80041C90(ram_class_t *ram, unk_func_80041C90_t *arg1) {
    if(arg1->unk_0x08 != NULL) {
        if(!arg1->unk_0x08(arg1, 1)) {
            return 0;
        }
    }
    
    return 1;
}

s32 ramGetBuffer(ram_class_t *ram, void **buffer, s32 addr, s32 *len) {
    u32 dram_size = ram->dram_size;
    addr = addr & 0x3FFFFFF;
    if(dram_size == 0) {
        return 0;
    }

    if((len != NULL) && (addr + *len >= dram_size)) {
        *len = dram_size - addr;
        if(*len < 0) {
            *len = 0;
        }
    }

    *buffer = ram->dram + addr;
    return 1;
}

s32 ramWipe(ram_class_t *ram){
    if(ram->dram_size != 0 && !xlHeapFill32((u32*)ram->dram, ram->dram_size, 0x0)) {
        return 0;
    }

    return 1;
}

s32 ramSetSize(ram_class_t *ram, u32 size) {
    if(ram->dram != NULL) {
        return 0;
    }

    size = (size + (0x400 - 1)) & ~(0x400 - 1);
    if(!xlHeapTake((void**)&ram->dram, size | 0x30000000)) {
        return 0;
    }

    ram->dram_size = size;
    return 1;
}

s32 ramGetSize(ram_class_t *ram, s32 *size) {
    if(size != NULL) {
        *size = ram->dram_size;
    }

    return 1;
}

s32 ramEvent(void *obj, s32 event, void *arg) {
    ram_class_t *ram = (ram_class_t*)obj;
    switch(event) {
        case 0x2:
            ram->dram_size = 0;
            ram->dram = NULL;
            break;
        case 0x1002: 
            switch(*(u32*)arg & 0xFF) {
                case 0:
                    if(!cpuSetGetBlock(gSystem->cpu, arg, func_80041C90)) {
                        return 0;
                    }
                    if(!cpuSetDevicePut(gSystem->cpu, arg, ramPut8, ramPut16, ramPut32, ramPut64)) {
                        return 0;
                    }
                    if(!cpuSetDeviceGet(gSystem->cpu, arg, ramGet8, ramGet16, ramGet32, ramGet64)) {
                        return 0;
                    }
                    break;
                case 1:
                    if(!cpuSetDevicePut(gSystem->cpu, arg, ramPutRI8, ramPutRI16, ramPutRI32, ramPutRI64)) {
                        return 0;
                    }
                    if(!cpuSetDeviceGet(gSystem->cpu, arg, ramGetRI8, ramGetRI16, ramGetRI32, ramGetRI64)) {
                        return 0;
                    }
                    break;
                case 2:
                    if(!cpuSetDevicePut(gSystem->cpu, arg, ramPutControl8, ramPutControl16, ramPutControl32, ramPutControl64)) {
                        return 0;
                    }
                    if(!cpuSetDeviceGet(gSystem->cpu, arg, ramGetControl8, ramGetControl16, ramGetControl32, ramGetControl64)) {
                        return 0;
                    }
                    break;
            }
            break;
        case 0:
        case 1:
        case 3:
        case 0x1003:
        case 0x1004:
        case 0x1007:
            break;
        default:
            return 0;
    }
    return 1;
}
