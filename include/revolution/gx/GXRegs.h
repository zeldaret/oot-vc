#ifndef GXREGS_H
#define GXREGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "macros.h"
#include "revolution/base/PPCWGPipe.h"

extern volatile PPCWGPipe gxfifo AT_ADDRESS(0xCC008000);
extern volatile void* __piReg;
extern volatile void* __cpReg;
extern volatile void* __peReg;
extern volatile void* __memReg;

#define MEM_PE_REQCOUNTH_IDX 0x27
#define MEM_PE_REQCOUNTL_IDX 0x28

#define SET_FLAG(regOrg, newFlag, shift, size) \
    (regOrg) = (u32)__rlwimi((int)(regOrg), (int)(newFlag), (shift), (32 - (shift) - (size)), (31 - (shift)));

/* GX fifo write helpers */
#define GX_WRITE_U8(ub) gxfifo.u8 = (u8)(ub)

#define GX_WRITE_U16(us) gxfifo.u16 = (u16)(us)

#define GX_WRITE_U32(ui) gxfifo.u32 = (u32)(ui)

#define GX_WRITE_F32(f) gxfifo.f32 = (f32)(f);

#define GX_WRITE_REG(reg) \
    GX_WRITE_U8((0x61));  \
    GX_WRITE_U32((reg))

#define CP_OPCODE(index, cmd) ((((unsigned long)(index)) << 0) | (((unsigned long)(cmd)) << 3))

#define CP_STREAM_REG(index, addr) ((((unsigned long)(index)) << 0) | (((unsigned long)(addr)) << 4))

#define CP_XF_LOADREGS(addr, cnt) ((((unsigned long)(addr)) << 0) | (((unsigned long)(cnt)) << 16))

#define GX_PI_REG_WRITE_U32(a, d) *(vu32*)((vu8*)__piReg + (a)) = (u32)(d)

#define GX_PI_REG_READ_U32(a) *(vu32*)((vu8*)__piReg + (a))

#define GX_CP_REG_WRITE_U16(a, d) *(vu16*)((vu16*)__cpReg + (a)) = (u16)(d)

#define GX_CP_REG_READ_U16(a) *(vu16*)((vu16*)__cpReg + (a))

#define GX_CP_REG_WRITE_U32(a, d) *(vu32*)((vu16*)__cpReg + (a)) = (u32)(d)

#define GX_CP_REG_READ_U32(a) *(vu32*)((vu16*)__cpReg + (a))

#define GX_MEM_REG_WRITE_U16(a, d) *(vu16*)((vu16*)__memReg + (a)) = (u16)(d)

#define GX_MEM_REG_READ_U16(a) *(vu16*)((vu16*)__memReg + (a))

#define GX_PE_REG_WRITE_U16(a, d) *(vu16*)((vu16*)__peReg + (a)) = (u16)(d)

#define GX_PE_REG_READ_U16(a) *(vu16*)((vu16*)__peReg + (a))

#define GX_CP_IDLE_REG_READ_U16(a) GX_CP_REG_READ_U16(a)

#define GX_WRITE_CP_STRM_REG(addr, vtxfmt, data)      \
    {                                                 \
        GX_WRITE_U8(CP_OPCODE(0, 1));                 \
        GX_WRITE_U8(CP_STREAM_REG((vtxfmt), (addr))); \
        GX_WRITE_U32((data));                         \
    }

#define GX_WRITE_RAS_REG(value) \
    do {                        \
        GX_WRITE_U8(0x61);      \
        GX_WRITE_U32(value);    \
    } while (0)

#define GX_WRITE_XF_REG(addr, data, cnt)             \
    {                                                \
        GX_WRITE_U8(CP_OPCODE(0, 0x2));              \
        GX_WRITE_U32(CP_XF_LOADREGS((addr), (cnt))); \
        GX_WRITE_U32((data));                        \
    }

#define GX_WRITE_XF_REG2(addr, value)  \
    do {                               \
        GX_WRITE_U8(0x10);             \
        GX_WRITE_U32(0x1000 + (addr)); \
        GX_WRITE_U32(value);           \
    } while (0)

#define GX_WRITE_XF_MEM_U32(addr, data) GX_WRITE_U32(data)
#define GX_WRITE_XF_MEM_F32(addr, data) GX_WRITE_F32(data)

#define GX_WRITE_RA_REG(reg)            \
    {                                   \
        GX_WRITE_U8(CP_OPCODE(1, 0xC)); \
        GX_WRITE_U32((reg));            \
    }

#define GX_DEFINE_GX_READ_COUNTER(unit)                                 \
    inline u32 __GXRead##unit##CounterU32(u32 regAddrL, u32 regAddrH) { \
        u32 ctrH0, ctrH1, ctrL;                                         \
        ctrH0 = GX_##unit##_REG_READ_U16(regAddrH);                     \
        do {                                                            \
            ctrH1 = ctrH0;                                              \
            ctrL = GX_##unit##_REG_READ_U16(regAddrL);                  \
            ctrH0 = GX_##unit##_REG_READ_U16(regAddrH);                 \
        } while (ctrH0 != ctrH1);                                       \
        return ((ctrH0 << 16) | ctrL);                                  \
    }

GX_DEFINE_GX_READ_COUNTER(CP)
GX_DEFINE_GX_READ_COUNTER(PE)
GX_DEFINE_GX_READ_COUNTER(MEM)

#define GX_CP_COUNTER_READ_U32(name) __GXReadCPCounterU32(name##L, name##H)

#define GX_PE_COUNTER_READ_U32(name) __GXReadPECounterU32(name##L_ID, name##H_ID)

#define GX_MEM_COUNTER_READ_U32(name) __GXReadMEMCounterU32(name##L_IDX, name##H_IDX)

#define FAST_FLAG_SET(regOrg, newFlag, shift, size)                                                                \
    do {                                                                                                           \
        (regOrg) = (u32)__rlwimi((int)(regOrg), (int)(newFlag), (shift), (32 - (shift) - (size)), (31 - (shift))); \
    } while (0);

#define GX_CP_ADDR 0x0C000000
#define GX_PE_ADDR 0x0C001000
#define GX_PI_ADDR 0x0C003000
#define GX_MEM_ADDR 0x0C004000

#define GX_GET_MEM_REG(offset) (*(vu16*)((vu16*)(__memReg) + (offset)))
#define GX_GET_CP_REG(offset) (*(vu16*)((vu16*)(__cpReg) + (offset)))
#define GX_GET_PE_REG(offset) (*(vu16*)((vu16*)(__peReg) + (offset)))
#define GX_GET_PI_REG(offset) (*(vu32*)((vu32*)(__piReg) + (offset)))

#define GX_SET_MEM_REG(offset, val) (*(vu16*)((vu16*)(__memReg) + (offset)) = val)
#define GX_SET_CP_REG(offset, val) (*(vu16*)((vu16*)(__cpReg) + (offset)) = val)
#define GX_SET_PE_REG(offset, val) (*(vu16*)((vu16*)(__peReg) + (offset)) = val)
#define GX_SET_PI_REG(offset, val) (*(vu32*)((vu32*)(__piReg) + (offset)) = val)

#define GX_CHECK_FLUSH() (!(*(u32*)(&gx->vNumNot)))
#define GX_COLOR_AS_U32(color) (*((u32*)&(color)))

// Useful reading register inlines
static inline u32 GXReadMEMReg(u32 addrLo, u32 addrHi) {
    u32 hiStart, hiNew, lo;
    hiStart = GX_GET_MEM_REG(addrHi);
    do {
        hiNew = hiStart;
        lo = GX_GET_MEM_REG(addrLo);
        hiStart = GX_GET_MEM_REG(addrHi);
    } while (hiStart != hiNew);

    return ((hiStart << 16) | lo);
}

static inline u32 GXReadCPReg(u32 addrLo, u32 addrHi) {
    u32 hiStart, hiNew, lo;
    hiStart = GX_GET_CP_REG(addrHi);
    do {
        hiNew = hiStart;
        lo = GX_GET_CP_REG(addrLo);
        hiStart = GX_GET_CP_REG(addrHi);
    } while (hiStart != hiNew);

    return ((hiStart << 16) | lo);
}

static inline u32 GXReadPEReg(u32 addrLo, u32 addrHi) {
    u32 hiStart, hiNew, lo;
    hiStart = GX_GET_PE_REG(addrHi);
    do {
        hiNew = hiStart;
        lo = GX_GET_PE_REG(addrLo);
        hiStart = GX_GET_PE_REG(addrHi);
    } while (hiStart != hiNew);

    return ((hiStart << 16) | lo);
}

static inline u32 GXReadPIReg(u32 addrLo, u32 addrHi) {
    u32 hiStart, hiNew, lo;
    hiStart = GX_GET_PI_REG(addrHi);
    do {
        hiNew = hiStart;
        lo = GX_GET_PI_REG(addrLo);
        hiStart = GX_GET_PI_REG(addrHi);
    } while (hiStart != hiNew);

    return ((hiStart << 16) | lo);
}

#define GX_WRITE_SOME_REG4(a, b, c, addr) \
    do {                                  \
        long regAddr;                     \
        GX_WRITE_U8(a);                   \
        GX_WRITE_U8(b);                   \
        GX_WRITE_U32(c);                  \
        regAddr = addr;                   \
    } while (0)

#define SET_REG_FIELD(reg, size, shift, val)                                                      \
    do {                                                                                          \
        (reg) = ((u32)__rlwimi((u32)(reg), (val), (shift), 32 - (shift) - (size), 31 - (shift))); \
    } while (0)

#define GX_BITFIELD(field, pos, size, value) \
    (__rlwimi((field), (value), 31 - (pos) - (size) + 1, (pos), (pos) + (size) - 1))
#define GX_BITFIELD_SET(field, pos, size, value) ((field) = GX_BITFIELD(field, pos, size, value))
#define GX_BITFIELD_TRUNC(field, pos, size, value) (__rlwimi((field), (value), 0, (pos), (pos) + (size) - 1))
#define GX_BITGET(field, pos, size) ((field) >> (31 - (pos) - (size) + 1) & ((1 << (size)) - 1))

#define GX_SET_REG(reg, x, st, end) GX_BITFIELD_SET((reg), (st), ((end) - (st) + 1), (x))
#define GX_SET_REG2(reg, x, st, end) GX_SET_REG(reg, x, st, end)

#define GX_BITGET(field, pos, size) ((field) >> (31 - (pos) - (size) + 1) & ((1 << (size)) - 1))
#define GX_GET_REG(reg, st, end) GX_BITGET((reg), (st), ((end) - (st) + 1))
#define GET_REG_FIELD(reg, size, shift) ((int)((reg) >> (shift)) & ((1 << (size)) - 1))

#ifdef __cplusplus
}
#endif

#endif // GXREGS_H
