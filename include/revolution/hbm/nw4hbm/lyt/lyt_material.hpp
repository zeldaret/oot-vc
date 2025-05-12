#ifndef RVL_SDK_HBM_NW4HBM_LYT_MATERIAL_HPP
#define RVL_SDK_HBM_NW4HBM_LYT_MATERIAL_HPP

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/gx/GXTypes.h"
#include "revolution/hbm/nw4hbm/lyt/lyt_animation.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_types.hpp"
#include "revolution/hbm/nw4hbm/math/math_types.hpp" // math::VEC2
#include "revolution/hbm/nw4hbm/ut/ut_Color.hpp"
#include "revolution/mtx/mtx.h"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

namespace nw4hbm {
namespace lyt {
// forward declarations
class Material;

namespace detail {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47655f
struct BitGXNums {
    u32 texMap : 4; // offset be0  / le31
    u32 texSRT : 4; // offset be4  / le27
    u32 texCoordGen : 4; // offset be8  / le23
    u32 indSRT : 2; // offset be12 / le19
    u32 indStage : 3; // offset be14 / le17
    u32 tevSwap : 1; // offset be17 / le14
    u32 tevStage : 5; // offset be18 / le13
    u32 chanCtrl : 1; // offset be23 / le8
    u32 matCol : 1; // offset be24 / le7
    u32 alpComp : 1; // offset be25 / le6
    u32 blendMode : 1; // offset be26 / le5
    /* 5 bits padding */
}; // size 0x04

Size GetTextureSize(Material* pMaterial, u8 texMapIdx);
} // namespace detail

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48eef8
struct TexSRT {
    math::VEC2 translate; // offset 0x00, size 0x08
    f32 rotate; // offset 0x08, size 0x04
    math::VEC2 scale; // offset 0x0c, size 0x08
}; // size 0x14

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f1b2
class TexCoordGen {
    // methods
  public:
    // cdtors
    TexCoordGen() : reserve(0) {
        // TODO: What does 60 mean? GX_TEXMTX_IDENT?
        Set(GX_TG_MTX2x4, GX_TG_TEX0, 60);
    }

    // methods
    u32 GetTexMtx() const { return texMtx; }
    GXTexGenType GetTexGenType() const { return static_cast<GXTexGenType>(texGenType); }
    GXTexGenSrc GetTexGenSrc() const { return static_cast<GXTexGenSrc>(texGenSrc); }

    void Set(GXTexGenType aTexGenType, GXTexGenSrc aTexGenSrc, u32 aTexMtx) {
        texGenType = aTexGenType;
        texGenSrc = aTexGenSrc;
        texMtx = aTexMtx;
    }

    // members
  private:
    u8 texGenType; // offset 0x00, size 0x01
    u8 texGenSrc; // offset 0x01, size 0x01
    u8 texMtx; // offset 0x02, size 0x01
    u8 reserve ATTRIBUTE_UNUSED; // offset 0x03, size 0x01
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f336
class IndirectStage {
    // methods
  public:
    // cdtors
    IndirectStage() { Set(GX_TEXCOORD0, GX_TEXMAP0, GX_ITS_1, GX_ITS_1); }

    // methods
    GXTexCoordID GetTexCoordGen() const { return static_cast<GXTexCoordID>(texCoordGen); }
    GXTexMapID GetTexMap() const { return static_cast<GXTexMapID>(texMap); }
    GXIndTexScale GetScaleS() const { return static_cast<GXIndTexScale>(scaleS); }
    GXIndTexScale GetScaleT() const { return static_cast<GXIndTexScale>(scaleT); }

    void Set(GXTexCoordID aTexCoordGen, GXTexMapID aTexMap, GXIndTexScale aScaleS, GXIndTexScale aScaleT) {
        texCoordGen = aTexCoordGen;

        // possible ERRATA? GX_TEX_DISABLE is out of range of a u8
        texMap = aTexMap;
        scaleS = aScaleS;
        scaleT = aScaleT;
    }

    // members
  private:
    u8 texCoordGen; // offset 0x00, size 0x01
    u8 texMap; // offset 0x01, size 0x01
    u8 scaleS; // offset 0x02, size 0x01
    u8 scaleT; // offset 0x03, size 0x01
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f2fc
class TevSwapMode {
    // methods
  public:
    // methods
    GXTevColorChan GetR() const { return static_cast<GXTevColorChan>(swap & 0x03); }
    GXTevColorChan GetG() const { return static_cast<GXTevColorChan>((swap >> 2) & 0x03); }
    GXTevColorChan GetB() const { return static_cast<GXTevColorChan>((swap >> 4) & 0x03); }
    GXTevColorChan GetA() const { return static_cast<GXTevColorChan>((swap >> 6) & 0x03); }

    void Set(GXTevColorChan r, GXTevColorChan g, GXTevColorChan b, GXTevColorChan a) {
        // no masking
        swap = r | g << 2 | b << 4 | a << 6;
    }

    // members
  private:
    /*	struct
            {
                    GXTevColorChan	a	: 2;
                    GXTevColorChan	b	: 2;
                    GXTevColorChan	g	: 2;
                    GXTevColorChan	r	: 2;
            };
    */
    u8 swap; // offset 0x00, size 0x01
}; // size 0x01

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f468
class TevStageInOp {
    // methods
  public:
    // methods
    u8 GetA() const { return ab & 0x0f; }
    u8 GetB() const { return (ab >> 4) & 0x0f; }
    u8 GetC() const { return cd & 0x0f; }
    u8 GetD() const { return (cd >> 4) & 0x0f; }

    u8 GetScale() const { return (op >> 6) & 0x03; }
    u8 GetBias() const { return (op >> 4) & 0x03; }
    u8 GetOp() const { return op & 0x0f; }

    u8 GetKSel() const { return (cl >> 3) & 0x1f; }
    u8 GetOutReg() const { return (cl >> 1) & 0x03; }
    bool IsClamp() const { return static_cast<bool>(cl & 0x01); }

    // sethods
    void SetIn(u8 a, u8 b, u8 c, u8 d) {
        ab = a | b << 4;
        cd = c | d << 4;
    }

    void SetOp(u8 aOp, u8 bias, u8 scale, bool clamp, u8 outReg, u8 kSel) {
        op = aOp | bias << 4 | scale << 6;

        cl = BOOLIFY_TERNARY(clamp) | outReg << 1 | kSel << 3;
    }

    // members
  private:
    /*	struct
            {
                    GXTevColorArg	b	: 4; // may also double as GXTevAlphaArg
                    GXTevColorArg	a	: 4; // may also double as GXTevAlphaArg
            };
    */
    u8 ab; // offset 0x00, size 0x01
    /*	struct
            {
                    GXTevColorArg	d	: 4; // may also double as GXTevAlphaArg
                    GXTevColorArg	c	: 4; // may also double as GXTevAlphaArg
            };
    */
    u8 cd; // offset 0x01, size 0x01
    /*	struct
            {
                    GXTevScale	scale	: 2;
                    GXTevBias	bias	: 2;
                    GXTevOp		op		: 4;
            };
    */
    u8 op; // offset 0x02, size 0x01
    /*	struct
            {
                    GXTevKColorSel	kSel	: 5; // may also double as GXTevKAlphaSel
                    GXTevRegID		outReg	: 2;
                    u8				clamp	: 1;
            };
    */
    u8 cl; // offset 0x03, size 0x01
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f3a0
class TevStage {
    // methods
  public:
    // cdtors
    TevStage() {
        SetOrder(GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        SetColorIn(GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        SetAlphaIn(GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        SetColorOp(GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV, GX_TEV_KCSEL_K0);
        SetAlphaOp(GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV, GX_TEV_KASEL_K0_R);
        SetIndirect(GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_OFF, GX_ITW_OFF, GX_ITW_OFF, false, false,
                    GX_ITBA_OFF);
    }

    // methods

    // i do not care and you should not either
    // clang-format off
		GXTexCoordID GetTexCoordGen() const { return static_cast<GXTexCoordID>(texCoordGen); }
		GXChannelID GetColorChan() const { return static_cast<GXChannelID>(colChan); }

		// shift left is for swapSel.texMapDisable -> GX_TEXMAP_DISABLE
		GXTexMapID GetTexMap() const { return static_cast<GXTexMapID>((swapSel & 1) << 8 | texMap); }

		GXTevSwapSel GetTexSwapSel() const { return static_cast<GXTevSwapSel>((swapSel >> 3) & 0x03); }
		GXTevSwapSel GetRasSwapSel() const { return static_cast<GXTevSwapSel>((swapSel >> 1) & 0x03); }

		GXTevColorArg GetColorInA() const { return static_cast<GXTevColorArg>(colIn.GetA()); }
		GXTevColorArg GetColorInB() const { return static_cast<GXTevColorArg>(colIn.GetB()); }
		GXTevColorArg GetColorInC() const { return static_cast<GXTevColorArg>(colIn.GetC()); }
		GXTevColorArg GetColorInD() const { return static_cast<GXTevColorArg>(colIn.GetD()); }

		GXTevOp GetColorOp() const { return static_cast<GXTevOp>(colIn.GetOp()); }
		GXTevBias GetColorBias() const { return static_cast<GXTevBias>(colIn.GetBias()); }
		GXTevScale GetColorScale() const { return static_cast<GXTevScale>(colIn.GetScale()); }
		bool IsColorClamp() const { return colIn.IsClamp(); }
		GXTevRegID GetColorOutReg() const { return static_cast<GXTevRegID>(colIn.GetOutReg()); }
		GXTevKColorSel GetKColorSel() const { return static_cast<GXTevKColorSel>(colIn.GetKSel()); }

		GXTevAlphaArg GetAlphaInA() const { return static_cast<GXTevAlphaArg>(alpIn.GetA()); }
		GXTevAlphaArg GetAlphaInB() const { return static_cast<GXTevAlphaArg>(alpIn.GetB()); }
		GXTevAlphaArg GetAlphaInC() const { return static_cast<GXTevAlphaArg>(alpIn.GetC()); }
		GXTevAlphaArg GetAlphaInD() const { return static_cast<GXTevAlphaArg>(alpIn.GetD()); }

		GXTevOp GetAlphaOp() const { return static_cast<GXTevOp>(alpIn.GetOp()); }
		GXTevBias GetAlphaBias() const { return static_cast<GXTevBias>(alpIn.GetBias()); }
		GXTevScale GetAlphaScale() const { return static_cast<GXTevScale>(alpIn.GetScale()); }
		bool IsAlphaClamp() const { return alpIn.IsClamp(); }
		GXTevRegID GetAlphaOutReg() const { return static_cast<GXTevRegID>(alpIn.GetOutReg()); }
		GXTevKAlphaSel GetKAlphaSel() const { return static_cast<GXTevKAlphaSel>(alpIn.GetKSel()); }

		GXIndTexStageID GetIndStage() const { return static_cast<GXIndTexStageID>(indStage); }
		GXIndTexMtxID GetIndMtxSel() const { return static_cast<GXIndTexMtxID>((indBiMt >> 3) & 0x0f); }
		GXIndTexBiasSel GetIndBiasSel() const { return static_cast<GXIndTexBiasSel>(indBiMt & 0x07); }
		GXIndTexWrap GetIndWrapS() const { return static_cast<GXIndTexWrap>(indWrap & 0x07); }
		GXIndTexWrap GetIndWrapT() const { return static_cast<GXIndTexWrap>((indWrap >> 3) & 0x07); }
		GXIndTexAlphaSel GetIndAlphaSel() const { return static_cast<GXIndTexAlphaSel>((indFoAdUtAl >> 4) & 0x03); }
		bool IsIndUtcLod() const { return static_cast<bool>((indFoAdUtAl >> 3) & 0x01); }
		bool IsIndAddPrev() const { return static_cast<bool>((indFoAdUtAl >> 2) & 0x01); }
		GXIndTexFormat GetIndFormat() const { return static_cast<GXIndTexFormat>(indFoAdUtAl & 0x03); }
    // clang-format on

    void SetOrder(GXTexCoordID aTexCoordGen, GXTexMapID aTexMap, GXChannelID aColChan, GXTevSwapSel rasSel,
                  GXTevSwapSel texSel) {
        texCoordGen = aTexCoordGen;
        colChan = aColChan;
        texMap = aTexMap;
        // shift right is for aTexMap == GX_TEX_DISABLE
        swapSel = aTexMap >> 8 | rasSel << 1 | texSel << 3;
    }

    void SetColorIn(GXTevColorArg a, GXTevColorArg b, GXTevColorArg c, GXTevColorArg d) { colIn.SetIn(a, b, c, d); }

    void SetColorOp(GXTevOp op, GXTevBias bias, GXTevScale scale, bool clamp, GXTevRegID outReg, GXTevKColorSel kSel) {
        colIn.SetOp(op, bias, scale, clamp, outReg, kSel);
    }

    void SetAlphaIn(GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c, GXTevAlphaArg d) { alpIn.SetIn(a, b, c, d); }

    void SetAlphaOp(GXTevOp op, GXTevBias bias, GXTevScale scale, bool clamp, GXTevRegID outReg, GXTevKAlphaSel kSel) {
        alpIn.SetOp(op, bias, scale, clamp, outReg, kSel);
    }

    void SetIndirect(GXIndTexStageID stage, GXIndTexFormat format, GXIndTexBiasSel biasSel, GXIndTexMtxID mtxSel,
                     GXIndTexWrap wrapS, GXIndTexWrap wrapT, bool addPrev, bool utcLod, GXIndTexAlphaSel alphaSel) {
        indStage = stage;
        indBiMt = biasSel | mtxSel << 3;
        indWrap = wrapS | wrapT << 3;
        indFoAdUtAl = format | BOOLIFY_TERNARY(addPrev) << 2 | BOOLIFY_TERNARY(utcLod) << 3 | alphaSel << 4;
    }

    // members
  private:
    u8 texCoordGen; // offset 0x00, size 0x01 // GXTexCoordID
    u8 colChan; // offset 0x01, size 0x01 // GXChannelID
    u8 texMap; // offset 0x02, size 0x01 // GXTexMap
    /*	struct
            {
                    // 3 bits padding
                    GXTevSwapSel	texSel			: 2;
                    GXTevSwapSel	rasSel			: 2;
                    u8				texMapDisable	: 1;
            };
    */
    u8 swapSel; // offset 0x03, size 0x01
    TevStageInOp colIn; // offset 0x04, size 0x04
    TevStageInOp alpIn; // offset 0x08, size 0x04
    u8 indStage; // offset 0x0c, size 0x01 // GXIndTexStageID
    /*	struct
            {
                    // 1 bit padding
                    GXIndTexWrap	mtxSel	: 4;
                    GXIndTexWrap	biasSel	: 3;
            };
    */
    u8 indBiMt; // offset 0x0d, size 0x01
    /*	struct
            {
                    // 2 bits padding
                    u8	wrapT	: 3;
                    u8	wrapS	: 3;
            };
    */
    u8 indWrap; // offset 0x0e, size 0x01
    /*	struct
            {
                    // 2 bits padding
                    GXIndTexAlphaSel	alphaSel	: 2;
                    u8					utcLod		: 1;
                    u8					addPrev		: 1;
                    GXIndTexFomat		format		: 2;
            };
            // 2009 Winner of the Goofiest Variable Name
    */
    u8 indFoAdUtAl; // offset 0x0f, size 0x01
}; // size 0x10

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f292
class ChanCtrl {
    // methods
  public:
    // cdtors
    ChanCtrl() : reserve1(0), reserve2(0) { Set(GX_SRC_VTX, GX_SRC_VTX); }

    // methods
    GXColorSrc GetColorSrc() const { return static_cast<GXColorSrc>(matSrcCol); }
    GXColorSrc GetAlphaSrc() const { return static_cast<GXColorSrc>(matSrcAlp); }

    void Set(GXColorSrc colSrc, GXColorSrc alpSrc) {
        matSrcCol = colSrc;
        matSrcAlp = alpSrc;
    }

    // members
  private:
    u8 matSrcCol; // offset 0x00, size 0x01 // GXColorSrc
    u8 matSrcAlp; // offset 0x01, size 0x01 // GXColorSrc
    u8 reserve1 ATTRIBUTE_UNUSED; // offset 0x02, size 0x01
    u8 reserve2 ATTRIBUTE_UNUSED; // offset 0x03, size 0x01
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f4bc
class AlphaCompare {
    // methods
  public:
    // cdtors
    AlphaCompare() { Set(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0); }

    // methods
    GXCompare GetComp0() const { return static_cast<GXCompare>(comp & 0x0f); }
    GXCompare GetComp1() const { return static_cast<GXCompare>((comp >> 4) & 0x0f); }

    GXAlphaOp GetOp() const { return static_cast<GXAlphaOp>(op); }

    u8 GetRef0() const { return ref0; }
    u8 GetRef1() const { return ref1; }

    void Set(GXCompare aComp0, u8 aRef0, GXAlphaOp aOp, GXCompare aComp1, u8 aRef1) {
        comp = aComp0 | aComp1 << 4;
        op = aOp;
        ref0 = aRef0;
        ref1 = aRef1;
    }

    // members
  private:
    u8 comp; // offset 0x00, size 0x01
    /*	struct
            {
                    GXCompare	comp1	: 4;
                    GXCompare	comp0	: 4;
            };
    */
    u8 op; // offset 0x01, size 0x01 // GXAlphaOp
    u8 ref0; // offset 0x02, size 0x01
    u8 ref1; // offset 0x03, size 0x01
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f510
class BlendMode {
    // methods
  public:
    // cdtors
    BlendMode() { Set(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET); }

    // gethods
    GXBlendMode GetType() const { return static_cast<GXBlendMode>(type); }
    GXBlendFactor GetSrcFactor() const { return static_cast<GXBlendFactor>(srcFactor); }
    GXBlendFactor GetDstFactor() const { return static_cast<GXBlendFactor>(dstFactor); }
    GXLogicOp GetOp() const { return static_cast<GXLogicOp>(op); }

    // sethods
    void Set(GXBlendMode aType, GXBlendFactor aSrcFactor, GXBlendFactor aDstFactor, GXLogicOp aOp) {
        type = aType;
        srcFactor = aSrcFactor;
        dstFactor = aDstFactor;
        op = aOp;
    }

    // members
  private:
    u8 type; // offset 0x00, size 0x01 // GXBlendMode
    u8 srcFactor; // offset 0x01, size 0x01 // GXBlendFactor
    u8 dstFactor; // offset 0x02, size 0x01 // GXBlendFactor
    u8 op; // offset 0x03, size 0x01 // GXLogicOp
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f131
class MaterialResourceNum {
    // methods
  public:
    // methods
    u8 GetTexMapNum() const { return detail::GetBits<>(bits, 0, 4); }
    u8 GetTexSRTNum() const { return detail::GetBits<>(bits, 4, 4); }
    u8 GetTexCoordGenNum() const { return detail::GetBits<>(bits, 8, 4); }
    bool HasTevSwapTable() const { return detail::TestBit<>(bits, 12); }
    u8 GetIndTexSRTNum() const { return detail::GetBits<>(bits, 13, 2); }
    u8 GetIndTexStageNum() const { return detail::GetBits<>(bits, 15, 3); }
    u8 GetTevStageNum() const { return detail::GetBits<>(bits, 18, 5); }
    bool HasAlphaCompare() const { return detail::TestBit<>(bits, 23); }
    bool HasBlendMode() const { return detail::TestBit<>(bits, 24); }
    u8 GetChanCtrlNum() const { return detail::GetBits<>(bits, 25, 1); }
    u8 GetMatColNum() const { return detail::GetBits<>(bits, 27, 1); }

    // members
  private:
    // Why was this not just a detail::BitGXNums
    /*	struct
            {
                    u8		texMap		: 4;
                    u8		texSRT		: 4;
                    u8		texCoordGen	: 4;
                    u8		tevSwap		: 1;
                    u8		indSRT		: 2;
                    u8		indStage	: 3;
                    u8		tevStage	: 5;
                    u8		alpComp		: 1;
                    u8		blendMode	: 1;
                    u8		chanCtrl	: 1;
                    // 1 bit padding
                    u8		matCol		: 1;
                    // 5 bits padding
            };
    */
    u32 bits;
}; // size 0x04

namespace res {
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f229
struct Texture {
    u32 nameStrOffset; // offset 0x00, size 0x04
    u8 type; // offset 0x04, size 0x01
    byte1_t padding[3];
}; // size 0x08

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f0af
struct Material {
    char name[20]; // offset 0x00, size 0x14
    GXColorS10 tevCols[3]; // offset 0x14, size 0x18
    GXColor tevKCols[4]; // offset 0x2c, size 0x10
    MaterialResourceNum resNum; // offset 0x3c, size 0x00
}; // size 0x40

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x48f167
struct TexMap {
    u16 texIdx; // offset 0x00, size 0x02
    u8 wrapS; // offset 0x02, size 0x01
    u8 wrapT; // offset 0x03, size 0x01
}; // size 0x04
} // namespace res

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x476427
class Material {
    // methods
  public:
    // cdtors
    Material(const res::Material* pRes, const ResBlockSet& resBlockSet);
    virtual ~Material();

    // virtual function ordering
    // vtable Material
    virtual bool SetupGX(bool bModVtxCol, u8 alpha);
    virtual void BindAnimation(AnimTransform* pAnimTrans);
    virtual void UnbindAnimation(AnimTransform* pAnimTrans);
    virtual void UnbindAllAnimation();
    virtual void Animate();
    virtual AnimationLink* FindAnimationLink(AnimTransform* pAnimTrans);
    virtual void SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable);

    // methods
    const char* GetName() const { return mName; }
    GXColorS10 GetTevColor(u32 idx) const { return mTevCols[idx]; }

    u8 GetTextureCap() const { return mGXMemCap.texMap; }
    u8 GetTexSRTCap() const { return mGXMemCap.texSRT; }
    u8 GetTexCoordGenCap() const { return mGXMemCap.texCoordGen; }
    u8 GetIndTexSRTCap() const { return mGXMemCap.indSRT; }
    bool IsTevSwapCap() const { return static_cast<bool>(mGXMemCap.tevSwap); }
    bool IsBlendModeCap() const { return static_cast<bool>(mGXMemCap.blendMode); }
    bool IsAlphaCompareCap() const { return static_cast<bool>(mGXMemCap.alpComp); }
    bool IsMatColorCap() const { return static_cast<bool>(mGXMemCap.matCol); }
    bool IsChanCtrlCap() const { return static_cast<bool>(mGXMemCap.chanCtrl); }

    u8 GetTextureNum() const { return mGXMemNum.texMap; }

    bool IsUserAllocated() const { return mbUserAllocated; }

    void SetName(const char* name);

    /* all const get methods except GetTexMapAry:
     * [SGLE41]/MAP/GormitiDebug.MAP:24425-24451, for weak function ordering
     */
    const GXTexObj* GetTexMapAry() const;
    const TexSRT* GetTexSRTAry() const;
    const TexCoordGen* GetTexCoordGenAry() const;
    const ChanCtrl* GetChanCtrlAry() const;
    const ut::Color* GetMatColAry() const;
    const TevSwapMode* GetTevSwapAry() const;
    const AlphaCompare* GetAlphaComparePtr() const;
    const BlendMode* GetBlendModePtr() const;
    const IndirectStage* GetIndirectStageAry() const;
    const TexSRT* GetIndTexSRTAry() const;
    const TevStage* GetTevStageAry() const;

    GXTexObj* GetTexMapAry();
    TexSRT* GetTexSRTAry();
    TexCoordGen* GetTexCoordGenAry();
    ChanCtrl* GetChanCtrlAry();
    ut::Color* GetMatColAry();
    TevSwapMode* GetTevSwapAry();
    AlphaCompare* GetAlphaComparePtr();
    BlendMode* GetBlendModePtr();
    IndirectStage* GetIndirectStageAry();
    TexSRT* GetIndTexSRTAry();
    TevStage* GetTevStageAry();

    void GetTexture(GXTexObj* pTexObj, u8 texMapIdx) const;

    void SetTextureNum(u8 num);
    void SetTexCoordGenNum(u8 num);
    void SetIndStageNum(u8 num);
    void SetTevStageNum(u8 num);

    void SetTexture(u8 texMapIdx, const GXTexObj& texObj);
    void SetTexture(u8 texMapIdx, TPLPalette* pTplRes);
    void SetTextureNoWrap(u8 texMapIdx, TPLPalette* pTplRes);
    void SetColorElement(u32 colorType, s16 value);

    void SetTexCoordGen(u32 idx, TexCoordGen value) { GetTexCoordGenAry()[idx] = value; }

    void SetTexSRTElement(u32 texSRTIdx, u32 eleIdx, f32 value) {
        f32* srtAry = reinterpret_cast<f32*>(&GetTexSRTAry()[texSRTIdx]);
        srtAry[eleIdx] = value;
    }

    void SetIndTexSRTElement(u32 texSRTIdx, u32 eleIdx, f32 value) {
        f32* srtAry = reinterpret_cast<f32*>(&GetIndTexSRTAry()[texSRTIdx]);
        srtAry[eleIdx] = value;
    }

    void Init();
    void InitBitGXNums(detail::BitGXNums* ptr);

    void ReserveGXMem(u8 texMapNum, u8 texSRTNum, u8 texCoordGenNum, u8 tevStageNum, bool allocTevSwap, u8 indStageNum,
                      u8 indSRTNum, bool allocChanCtrl, bool allocMatCol, bool allocAlpComp, bool allocBlendMode);

    void AddAnimationLink(AnimationLink* pAnimationLink);

    // members
  private:
    char mName[20]; // offset 0x04, size 0x14
    AnimationLink::LinkList mAnimList; // offset 0x18, size 0x0c
    GXColorS10 mTevCols[3]; // offset 0x24, size 0x18
    ut::Color mTevKCols[4]; // offset 0x3c, size 0x10
    detail::BitGXNums mGXMemCap; // offset 0x4c, size 0x04
    detail::BitGXNums mGXMemNum; // offset 0x50, size 0x04
    bool mbUserAllocated; // offset 0x54, size 0x01
    /* 3 bytes padding */
    void* mpGXMem; // offset 0x58, size 0x04
}; // size 0x5c
} // namespace lyt
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_LYT_MATERIAL_HPP
