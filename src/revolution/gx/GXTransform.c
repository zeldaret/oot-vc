#include "revolution/gx.h"

// TODO: Fake inline
static inline void LoadProjPS(register f32* dst) {
    register f32 ps_0, ps_1, ps_2;
    register GXData* src;

#ifdef __MWERKS__ // clang-format off
    asm volatile {
        lwz src, gx
        psq_l  ps_0,  0  + GXData.projMtx(src), 0, 0
        psq_l  ps_1,  8  + GXData.projMtx(src), 0, 0
        psq_l  ps_2,  16 + GXData.projMtx(src), 0, 0
        psq_st ps_0,  0(dst),                0, 0
        psq_st ps_1,  8(dst),                0, 0
        psq_st ps_2, 16(dst),                0, 0
    }
#endif // clang-format on
}

static inline void WriteProjPS(register volatile void* dst, register const f32* src) {
    register f32 ps_0, ps_1, ps_2;

#ifdef __MWERKS__ // clang-format off
    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_st ps_0,  0(dst), 0, 0
        psq_st ps_1,  0(dst), 0, 0
        psq_st ps_2,  0(dst), 0, 0
    }
#endif // clang-format on
}

static inline void Copy6Floats(register f32* dst, register const f32* src) {
    register f32 ps_0, ps_1, ps_2;

#ifdef __MWERKS__ // clang-format off
    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_st ps_0,  0(dst), 0, 0
        psq_st ps_1,  8(dst), 0, 0
        psq_st ps_2, 16(dst), 0, 0
    }
#endif // clang-format on
}

void __GXSetProjection(void) {
    // Temp required to match
    volatile void* wgpipe = &WGPIPE;

    GX_XF_LOAD_REGS(ARRAY_COUNT(gx->projMtx), GX_XF_REG_PROJECTIONA);
    WriteProjPS(wgpipe, gx->projMtx);
    WGPIPE.i = gx->projType;
}

void GXSetProjection(const Mtx44 projMtx, GXProjectionType type) {
    gx->projType = type;

    gx->projMtx[0] = projMtx[0][0];
    gx->projMtx[2] = projMtx[1][1];
    gx->projMtx[4] = projMtx[2][2];
    gx->projMtx[5] = projMtx[2][3];

    if (type == GX_ORTHOGRAPHIC) {
        gx->projMtx[1] = projMtx[0][3];
        gx->projMtx[3] = projMtx[1][3];
    } else {
        gx->projMtx[1] = projMtx[0][2];
        gx->projMtx[3] = projMtx[1][2];
    }

    gx->dirtyState |= GX_DIRTY_PROJECTION;
}

void GXSetProjectionv(const f32 projMtx[7]) {
    gx->projType = projMtx[0] == 0.0f ? GX_PERSPECTIVE : GX_ORTHOGRAPHIC;
    Copy6Floats(gx->projMtx, projMtx + 1);
    gx->dirtyState |= GX_DIRTY_PROJECTION;
}

inline void WriteMTXPS4x3(register volatile void* dst, register const Mtx src) {
    register f32 ps_0, ps_1, ps_2, ps_3, ps_4, ps_5;

#ifdef __MWERKS__ // clang-format off
    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_l  ps_3, 24(src), 0, 0
        psq_l  ps_4, 32(src), 0, 0
        psq_l  ps_5, 40(src), 0, 0

        psq_st ps_0, 0(dst),  0, 0
        psq_st ps_1, 0(dst),  0, 0
        psq_st ps_2, 0(dst),  0, 0
        psq_st ps_3, 0(dst),  0, 0
        psq_st ps_4, 0(dst),  0, 0
        psq_st ps_5, 0(dst),  0, 0
    }
#endif // clang-format on
}

inline void WriteMTXPS3x3(register volatile void* dst, register const Mtx src) {
    register f32 ps_0, ps_1, ps_2, ps_3, ps_4, ps_5;

#ifdef __MWERKS__ // clang-format off
    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        lfs    ps_1,  8(src)
        psq_l  ps_2, 16(src), 0, 0
        lfs    ps_3, 24(src)
        psq_l  ps_4, 32(src), 0, 0
        lfs    ps_5, 40(src)

        psq_st ps_0, 0(dst),  0, 0
        stfs   ps_1, 0(dst)
        psq_st ps_2, 0(dst),  0, 0
        stfs   ps_3, 0(dst)
        psq_st ps_4, 0(dst),  0, 0
        stfs   ps_5, 0(dst)
    }
#endif // clang-format on
}

inline void WriteMTXPS4x2(register volatile void* dst, register const Mtx src) {
    register f32 ps_0, ps_1, ps_2, ps_3;

#ifdef __MWERKS__ // clang-format off
    asm volatile {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_l  ps_3, 24(src), 0, 0

        psq_st ps_0, 0(dst),  0, 0
        psq_st ps_1, 0(dst),  0, 0
        psq_st ps_2, 0(dst),  0, 0
        psq_st ps_3, 0(dst),  0, 0
    }
#endif // clang-format on
}

void GXLoadPosMtxImm(const Mtx mtx, u32 id) {
    // Position matrices are 4x3
    GX_XF_LOAD_REGS(4 * 3 - 1, id * 4 + GX_XF_MEM_POSMTX);
    WriteMTXPS4x3(&WGPIPE, mtx);
}

void GXLoadNrmMtxImm(const Mtx mtx, u32 id) {
    // Normal matrices are 3x3
    GX_XF_LOAD_REGS(3 * 3 - 1, id * 3 + GX_XF_MEM_NRMMTX);
    WriteMTXPS3x3(&WGPIPE, mtx);
}

void GXSetCurrentMtx(u32 id) {
    GX_XF_SET_MATRIXINDEX0_GEOM(gx->matIdxA, id);
    gx->dirtyState |= GX_DIRTY_MTX_IDX;
}

void GXLoadTexMtxImm(const Mtx mtx, u32 id, GXTexMtxType type) {
    u32 addr;
    u32 num;
    u32 reg;

    // Matrix address in XF memory
    addr = id >= GX_PTTEXMTX0 ? (id - GX_PTTEXMTX0) * 4 + GX_XF_MEM_DUALTEXMTX : id * 4 + (u64)GX_XF_MEM_POSMTX;

    // Number of elements in matrix
    num = type == GX_MTX2x4 ? (u64)(2 * 4) : 3 * 4;

    reg = addr;
    reg |= (num - 1) << 16;

    GX_XF_LOAD_REG_HDR(reg);

    if (type == GX_MTX3x4) {
        WriteMTXPS4x3(&WGPIPE, mtx);
    } else {
        WriteMTXPS4x2(&WGPIPE, mtx);
    }
}

void __GXSetViewport(void) {
    f32 a, b, c, d, e, f;
    f32 near, far;

    a = gx->vpWd / 2.0f;
    b = -gx->vpHt / 2.0f;
    d = gx->vpLeft + (gx->vpWd / 2.0f) + 342.0f;
    e = gx->vpTop + (gx->vpHt / 2.0f) + 342.0f;

    near = gx->vpNearz * gx->zScale;
    far = gx->vpFarz * gx->zScale;

    c = far - near;
    f = far + gx->zOffset;

    GX_XF_LOAD_REGS(6 - 1, GX_XF_REG_SCALEX);
    WGPIPE.f = a;
    WGPIPE.f = b;
    WGPIPE.f = c;
    WGPIPE.f = d;
    WGPIPE.f = e;
    WGPIPE.f = f;
}

void GXSetViewportJitter(f32 ox, f32 oy, f32 sx, f32 sy, f32 near, f32 far, u32 nextField) {
    // "Field" as in VI field
    // TODO: Is this an enum? I don't know anything about the return value other
    // than that it is a u32 (NW4R signature)
    if (nextField == 0) {
        oy -= 0.5f;
    }

    gx->vpLeft = ox;
    gx->vpTop = oy;
    gx->vpWd = sx;
    gx->vpHt = sy;
    gx->vpNearz = near;
    gx->vpFarz = far;
    gx->dirtyState |= GX_DIRTY_VIEWPORT;
}

void GXSetViewport(f32 ox, f32 oy, f32 sx, f32 sy, f32 near, f32 far) {
    gx->vpLeft = ox;
    gx->vpTop = oy;
    gx->vpWd = sx;
    gx->vpHt = sy;
    gx->vpNearz = near;
    gx->vpFarz = far;
    gx->dirtyState |= GX_DIRTY_VIEWPORT;
}

void GXGetViewportv(f32 view[6]) { Copy6Floats(view, gx->view); }

void GXSetScissor(u32 x, u32 y, u32 w, u32 h) {
    u32 x1, y1, x2, y2;
    u32 reg;

    x1 = x + 342;
    y1 = y + 342;
    x2 = x1 + w - 1;
    y2 = y1 + h - 1;

    reg = gx->suScis0;
    GX_BP_SET_SCISSORTL_TOP(reg, y1);
    GX_BP_SET_SCISSORTL_LEFT(reg, x1);
    gx->suScis0 = reg;

    reg = gx->suScis1;
    GX_BP_SET_SCISSORBR_BOT(reg, y2);
    GX_BP_SET_SCISSORBR_RIGHT(reg, x2);
    gx->suScis1 = reg;

    GX_BP_LOAD_REG(gx->suScis0);
    GX_BP_LOAD_REG(gx->suScis1);
    gx->bpSentNot = false;
}

void GXGetScissor(u32* x, u32* y, u32* w, u32* h) {
    u32 y2, y1;
    u32 x2, x1;

    x1 = GX_BP_GET_SCISSORTL_LEFT(gx->suScis0);
    y1 = GX_BP_GET_SCISSORTL_TOP(gx->suScis0);
    x2 = GX_BP_GET_SCISSORBR_RIGHT(gx->suScis1);
    y2 = GX_BP_GET_SCISSORBR_BOT(gx->suScis1);

    *x = x1 - 342;
    *y = y1 - 342;
    *w = x2 - x1 + 1;
    *h = y2 - y1 + 1;
}

void GXSetScissorBoxOffset(u32 ox, u32 oy) {
    u32 cmd = 0;
    GX_BP_SET_SCISSOROFFSET_OX(cmd, (ox + 342) / 2);
    GX_BP_SET_SCISSOROFFSET_OY(cmd, (oy + 342) / 2);
    GX_BP_SET_OPCODE(cmd, GX_BP_REG_SCISSOROFFSET);

    GX_BP_LOAD_REG(cmd);
    gx->bpSentNot = false;
}

void GXSetClipMode(GXClipMode mode) {
    GX_XF_LOAD_REG(GX_XF_REG_CLIPDISABLE, mode);
    gx->bpSentNot = true;
}

void __GXSetMatrixIndex(GXAttr index) {
    // Tex4 and after is stored in XF MatrixIndex1
    if (index < GX_VA_TEX4MTXIDX) {
        GX_CP_LOAD_REG(GX_CP_REG_MATRIXINDEXA, gx->matIdxA);
        GX_XF_LOAD_REG(GX_XF_REG_MATRIXINDEX0, gx->matIdxA);
    } else {
        GX_CP_LOAD_REG(GX_CP_REG_MATRIXINDEXB, gx->matIdxB);
        GX_XF_LOAD_REG(GX_XF_REG_MATRIXINDEX1, gx->matIdxB);
    }

    gx->bpSentNot = true;
}
