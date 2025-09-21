#include "revolution/hbm/nw4hbm/lyt/lyt_window.hpp"

/*******************************************************************************
 * headers
 */

#include "new.hpp"

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_layout.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_types.hpp"
#include "revolution/hbm/nw4hbm/lyt/material.h"
#include "revolution/hbm/nw4hbm/lyt/pane.h"

#include "revolution/hbm/nw4hbm/math/math_types.hpp"
#include "revolution/hbm/nw4hbm/ut/Color.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp"

/*******************************************************************************
 * types
 */

// probably local
// [SGLEA4]/GormitiDebug.elf:.debug_info::0x49dde9
struct TextureFlipInfo {
    u8 coords[4][2]; // size 0x08, offset 0x00
    u8 idx[2]; // size 0x02, offset 0x08
}; // size 0x0a

/*******************************************************************************
 * local function declarations
 */

namespace {
// pretend this is nw4hbm::lyt
using namespace nw4hbm;
using namespace nw4hbm::lyt;

// NOTE the misspelling of GetTextureFlipInfo
TextureFlipInfo& GetTexutreFlipInfo(u8 textureFlip);

void GetLTFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize);
void GetLTTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip);
void GetRTFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize);
void GetRTTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip);
void GetLBFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize);
void GetLBTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip);
void GetRBFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize);
void GetRBTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip);
} // unnamed namespace

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace lyt {
// .bss
const ut::detail::RuntimeTypeInfo Window::typeInfo(&Pane::typeInfo);
} // namespace lyt
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

// TODO clean up

namespace {

TextureFlipInfo& GetTexutreFlipInfo(u8 textureFlip) {
    // clang-format off
    static TextureFlipInfo flipInfos[] =			//    0    1    2    3
	{									// in order of    LT    RT    LB    RB
		{{{0, 0}, {1, 0}, {0, 1}, {1, 1}}, {0, 1}},	//    0    1    2    3			    no flip
		{{{1, 0}, {0, 0}, {1, 1}, {0, 1}}, {0, 1}},	//    1    0    3    2			    horizontal flip
		{{{0, 1}, {1, 1}, {0, 0}, {1, 0}}, {0, 1}},	//    2    3    0    1			    vertical flip
		{{{0, 1}, {0, 0}, {1, 1}, {1, 0}}, {1, 0}},	//    2    0    3    1, index flip    cw  90 deg
		{{{1, 1}, {0, 1}, {1, 0}, {0, 0}}, {0, 1}},	//    3    2    1    0			    cw 180 deg
		{{{1, 0}, {1, 1}, {0, 0}, {0, 1}}, {1, 0}}	//    1    3    0    2, index flip    cw 270 deg (ccw 90 deg)
	};
    // clang-format on

    NW4HBMAssert_Line(textureFlip < TEXTUREFLIP_MAX, 50);
    return flipInfos[textureFlip];
}

void GetLTFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize) {
    *pPt = basePt;

    pSize->width = winSize.width - frameSize.r;
    pSize->height = frameSize.t;
}

void GetLTTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip) {
    TextureFlipInfo& flipInfo = GetTexutreFlipInfo(textureFlip);
    int ix = flipInfo.idx[0];
    int iy = flipInfo.idx[1];
    const math::VEC2 tSz(texSize.width, texSize.height);

    /*
            // easier to follow versions in these blocks
            // also TODO: understand lol

            u8 flipSubX = flipInfo.coords[1][ix] - flipInfo.coords[0][ix];
            u8 flipSubY = flipInfo.coords[2][iy] - flipInfo.coords[0][iy];

            f32 flipSubXMultSize = flipSubX * tSz[ix];
            f32 flipSubYMultSize = flipSubY * tSz[iy];

            f32 flipSubXMultSizePolDiv = polSize.width  / flipSubXMultSize;
            f32 flipSubYMultSizePolDiv = polSize.height / flipSubYMultSize;

            texCds[0][ix] = flipInfo.coords[0][ix];
            texCds[0][iy] = flipInfo.coords[0][iy];

            texCds[1][ix] = flipInfo.coords[0][ix] + flipSubXMultSizePolDiv;
            texCds[1][iy] = flipInfo.coords[0][iy];

            texCds[2][ix] = flipInfo.coords[0][ix];
            texCds[2][iy] = flipInfo.coords[0][iy] + flipSubYMultSizePolDiv;

            texCds[3][ix] = flipInfo.coords[0][ix] + flipSubXMultSizePolDiv;
            texCds[3][iy] = flipInfo.coords[0][iy] + flipSubYMultSizePolDiv;
    */

    texCds[0][ix] = texCds[2][ix] = flipInfo.coords[0][ix];
    texCds[0][iy] = texCds[1][iy] = flipInfo.coords[0][iy];

    texCds[3][ix] = texCds[1][ix] =
        flipInfo.coords[0][ix] + polSize.width / (tSz[ix] * (flipInfo.coords[1][ix] - flipInfo.coords[0][ix]));

    texCds[3][iy] = texCds[2][iy] =
        flipInfo.coords[0][iy] + polSize.height / (tSz[iy] * (flipInfo.coords[2][iy] - flipInfo.coords[0][iy]));
}

void GetRTFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize) {
    *pPt = math::VEC2(basePt.x + winSize.width - frameSize.r, basePt.y);

    pSize->width = frameSize.r;
    pSize->height = winSize.height - frameSize.b;
}

void GetRTTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip) {
    TextureFlipInfo& flipInfo = GetTexutreFlipInfo(textureFlip);
    int ix = flipInfo.idx[0];
    int iy = flipInfo.idx[1];
    const math::VEC2 tSz(texSize.width, texSize.height);

    /*
            u8 flipSubX = flipInfo.coords[0][ix] - flipInfo.coords[1][ix];
            u8 flipSubY = flipInfo.coords[3][iy] - flipInfo.coords[1][iy];

            f32 flipSubXMultSize = flipSubX * tSz[ix];
            f32 flipSubYMultSize = flipSubY * tSz[iy];

            f32 flipSubXMultSizePolDiv = polSize.width  / flipSubXMultSize;
            f32 flipSubYMultSizePolDiv = polSize.height / flipSubYMultSize;

            texCds[0][ix] = flipInfo.coords[1][ix] + flipSubXMultSizePolDiv;
            texCds[0][iy] = flipInfo.coords[1][iy];

            texCds[1][ix] = flipInfo.coords[1][ix];
            texCds[1][iy] = flipInfo.coords[1][iy];

            texCds[2][ix] = flipInfo.coords[1][ix] + flipSubXMultSizePolDiv;
            texCds[2][iy] = flipInfo.coords[1][iy] + flipSubYMultSizePolDiv;

            texCds[3][ix] = flipInfo.coords[1][ix];
            texCds[3][iy] = flipInfo.coords[1][iy] + flipSubYMultSizePolDiv;
    */

    texCds[1][ix] = texCds[3][ix] = flipInfo.coords[1][ix];
    texCds[1][iy] = texCds[0][iy] = flipInfo.coords[1][iy];

    texCds[2][ix] = texCds[0][ix] =
        flipInfo.coords[1][ix] + polSize.width / (tSz[ix] * (flipInfo.coords[0][ix] - flipInfo.coords[1][ix]));

    texCds[2][iy] = texCds[3][iy] =
        flipInfo.coords[1][iy] + polSize.height / (tSz[iy] * (flipInfo.coords[3][iy] - flipInfo.coords[1][iy]));
}

void GetLBFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize) {
    *pPt = math::VEC2(basePt.x, basePt.y + frameSize.t);

    pSize->width = frameSize.l;
    pSize->height = winSize.height - frameSize.t;
}

void GetLBTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip) {
    TextureFlipInfo& flipInfo = GetTexutreFlipInfo(textureFlip);
    int ix = flipInfo.idx[0];
    int iy = flipInfo.idx[1];
    const math::VEC2 tSz(texSize.width, texSize.height);

    /*
            u8 flipSubX = flipInfo.coords[3][ix] - flipInfo.coords[2][ix];
            u8 flipSubY = flipInfo.coords[0][iy] - flipInfo.coords[2][iy];

            f32 flipSubXMultSize = flipSubX * tSz[ix];
            f32 flipSubYMultSize = flipSubY * tSz[iy];

            f32 flipSubXMultSizePolDiv = polSize.width  / flipSubXMultSize;
            f32 flipSubYMultSizePolDiv = polSize.height / flipSubYMultSize;

            texCds[0][ix] = flipInfo.coords[2][ix];
            texCds[0][iy] = flipInfo.coords[2][iy] + flipSubYMultSizePolDiv;

            texCds[1][ix] = flipInfo.coords[2][ix] + flipSubXMultSizePolDiv;
            texCds[1][iy] = flipInfo.coords[2][iy] + flipSubYMultSizePolDiv;

            texCds[2][ix] = flipInfo.coords[2][ix];
            texCds[2][iy] = flipInfo.coords[2][iy];

            texCds[3][ix] = flipInfo.coords[2][ix] + flipSubXMultSizePolDiv;
            texCds[3][iy] = flipInfo.coords[2][iy];
    */

    texCds[2][ix] = texCds[0][ix] = flipInfo.coords[2][ix];
    texCds[2][iy] = texCds[3][iy] = flipInfo.coords[2][iy];

    texCds[1][ix] = texCds[3][ix] =
        flipInfo.coords[2][ix] + polSize.width / (tSz[ix] * (flipInfo.coords[3][ix] - flipInfo.coords[2][ix]));

    texCds[1][iy] = texCds[0][iy] =
        flipInfo.coords[2][iy] + polSize.height / (tSz[iy] * (flipInfo.coords[0][iy] - flipInfo.coords[2][iy]));
}

void GetRBFrameSize(math::VEC2* pPt, Size* pSize, const math::VEC2& basePt, const Size& winSize,
                    const WindowFrameSize& frameSize) {
    *pPt = math::VEC2(basePt.x + frameSize.l, basePt.y + winSize.height - frameSize.b);

    pSize->width = winSize.width - frameSize.l;
    pSize->height = frameSize.b;
}

void GetRBTexCoord(math::VEC2* texCds, const Size& polSize, const Size& texSize, u8 textureFlip) {
    TextureFlipInfo& flipInfo = GetTexutreFlipInfo(textureFlip);
    int ix = flipInfo.idx[0];
    int iy = flipInfo.idx[1];
    const math::VEC2 tSz(texSize.width, texSize.height);

    /*
            u8 flipSubX = flipInfo.coords[1][ix] - flipInfo.coords[3][ix];
            u8 flipSubY = flipInfo.coords[2][iy] - flipInfo.coords[3][iy];

            f32 flipSubXMultSize = flipSubX * tSz[ix];
            f32 flipSubYMultSize = flipSubY * tSz[iy];

            f32 flipSubXMultSizePolDiv = polSize.width  / flipSubXMultSize;
            f32 flipSubYMultSizePolDiv = polSize.height / flipSubYMultSize;

            texCds[0][ix] = flipInfo.coords[3][ix] + flipSubXMultSizePolDiv;
            texCds[0][iy] = flipInfo.coords[3][iy] + flipSubYMultSizePolDiv;

            texCds[1][ix] = flipInfo.coords[3][ix];
            texCds[1][iy] = flipInfo.coords[3][iy] + flipSubYMultSizePolDiv;

            texCds[2][ix] = flipInfo.coords[3][ix] + flipSubXMultSizePolDiv;
            texCds[2][iy] = flipInfo.coords[3][iy];

            texCds[3][ix] = flipInfo.coords[3][ix];
            texCds[3][iy] = flipInfo.coords[3][iy];
    */

    texCds[3][ix] = texCds[1][ix] = flipInfo.coords[3][ix];
    texCds[3][iy] = texCds[2][iy] = flipInfo.coords[3][iy];

    texCds[0][ix] = texCds[2][ix] =
        flipInfo.coords[3][ix] + polSize.width / (tSz[ix] * (flipInfo.coords[2][ix] - flipInfo.coords[3][ix]));

    texCds[0][iy] = texCds[1][iy] =
        flipInfo.coords[3][iy] + polSize.height / (tSz[iy] * (flipInfo.coords[1][iy] - flipInfo.coords[3][iy]));
}

} // unnamed namespace

namespace nw4hbm {
namespace lyt {

Window::Window(const res::Window* pBlock, const ResBlockSet& resBlockSet) : Pane(pBlock) {
    mContentInflation = pBlock->inflation;

    NW4HBMAssertPointerNonnull_Line(resBlockSet.pMaterialList, 193);
    const u32* matOffsTbl = detail::ConvertOffsToPtr<u32>(resBlockSet.pMaterialList, sizeof(*resBlockSet.pMaterialList));

    const res::WindowContent* pResContent =
        detail::ConvertOffsToPtr<res::WindowContent>(pBlock, pBlock->contentOffset);

    for (int i = 0; i < (int)ARRAY_COUNT(mContent.vtxColors); i++) {
        mContent.vtxColors[i] = pResContent->vtxCols[i];
    }

    if (pResContent->texCoordNum) {
        u8 texCoordNum = ut::Min<u8>(pResContent->texCoordNum, 8);
        mContent.texCoordAry.Reserve(texCoordNum);

        if (!mContent.texCoordAry.IsEmpty()) {
            mContent.texCoordAry.Copy(&pResContent[1], texCoordNum);
        }
    }

    if (void* pMemMaterial = Layout::AllocMemory(sizeof(Material))) {
        const res::Material* pResMaterial = detail::ConvertOffsToPtr<res::Material>(
            resBlockSet.pMaterialList, matOffsTbl[pResContent->materialIdx]);

        mpMaterial = new (pMemMaterial) Material(pResMaterial, resBlockSet);
    }

    mFrameNum = 0;
    mFrames = nullptr;

    if (pBlock->frameNum) {
        if ((mFrames = static_cast<Frame*>(Layout::AllocMemory(sizeof *mFrames * pBlock->frameNum)))) {
            mFrameNum = pBlock->frameNum;
            const u32* frameOffsetTable = detail::ConvertOffsToPtr<u32>(pBlock, pBlock->frameOffsetTableOffset);

            for (int i = 0; i < mFrameNum; i++) {
                const res::WindowFrame* pResWindowFrame =
                    detail::ConvertOffsToPtr<res::WindowFrame>(pBlock, frameOffsetTable[i]);

                mFrames[i].textureFlip = pResWindowFrame->textureFlip;
                mFrames[i].pMaterial = nullptr;

                if (void* pMemMaterial = Layout::AllocMemory(sizeof(Material))) {
                    const res::Material* pResMaterial = detail::ConvertOffsToPtr<res::Material>(
                        resBlockSet.pMaterialList, matOffsTbl[pResWindowFrame->materialIdx]);

                    mFrames[i].pMaterial = new (pMemMaterial) Material(pResMaterial, resBlockSet);
                }
            }
        }
    }
}

Window::~Window() {
    if (mFrames) {
        for (int i = 0; i < mFrameNum; i++) {
            mFrames[i].pMaterial->~Material();
            Layout::FreeMemory(mFrames[i].pMaterial);
        }

        Layout::FreeMemory(mFrames);
    }

    if (mpMaterial && !mpMaterial->IsUserAllocated()) {
        mpMaterial->~Material();
        Layout::FreeMemory(mpMaterial);
        mpMaterial = nullptr;
    }

    mContent.texCoordAry.Free();
}

Material* Window::FindMaterialByName(const char* findName, bool bRecursive) {
    if (mpMaterial && detail::EqualsMaterialName(mpMaterial->GetName(), findName)) {
        return mpMaterial;
    }

    for (int i = 0; i < mFrameNum; i++) {
        if (detail::EqualsMaterialName(mFrames[i].pMaterial->GetName(), findName)) {
            return mFrames[i].pMaterial;
        }
    }

    if (bRecursive) {
        NW4HBM_RANGE_FOR(it, mChildList) {
            if (Material* pMat = it->FindMaterialByName(findName, true)) {
                return pMat;
            }
        }
    }

    return nullptr;
}

AnimationLink* Window::FindAnimationLink(AnimTransform* pAnimTrans) {
    if (AnimationLink* ret = Pane::FindAnimationLink(pAnimTrans)) {
        return ret;
    }

    for (int i = 0; i < mFrameNum; i++) {
        if (AnimationLink* ret = mFrames[i].pMaterial->FindAnimationLink(pAnimTrans)) {
            return ret;
        }
    }

    return nullptr;
}

void Window::SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable, bool bRecursive) {
    for (int i = 0; i < mFrameNum; i++) {
        mFrames[i].pMaterial->SetAnimationEnable(pAnimTrans, bEnable);
    }

    Pane::SetAnimationEnable(pAnimTrans, bEnable, bRecursive);
}

const ut::Color Window::GetVtxColor(u32 idx) const {
    NW4HBMAssert_Line(idx < VERTEXCOLOR_MAX, 360);
    return mContent.vtxColors[idx];
}

void Window::SetVtxColor(u32 idx, ut::Color value) {
    NW4HBMAssert_Line(idx < VERTEXCOLOR_MAX, 371);
    mContent.vtxColors[idx] = value;
}

u8 Window::GetVtxColorElement(u32 idx) const { return detail::GetVtxColorElement(mContent.vtxColors, idx); }

void Window::SetVtxColorElement(u32 idx, u8 value) { detail::SetVtxColorElement(mContent.vtxColors, idx, value); }

void Window::DrawSelf(const DrawInfo& drawInfo) {
    LoadMtx(drawInfo);
    WindowFrameSize frameSize = GetFrameSize(mFrameNum, mFrames);

    math::VEC2 basePt = GetVtxPos();

    DrawContent(basePt, frameSize, mGlbAlpha);

    switch (mFrameNum) {
        case 1:
            DrawFrame(basePt, *mFrames, frameSize, mGlbAlpha);
            break;

        case 4:
            DrawFrame4(basePt, mFrames, frameSize, mGlbAlpha);
            break;

        case 8:
            DrawFrame8(basePt, mFrames, frameSize, mGlbAlpha);
            break;
    }
}

void Window::AnimateSelf(u32 option) {
    Pane::AnimateSelf(option);

    if (detail::TestBit<>(mFlag, 0) || !(option & 1)) {
        for (int i = 0; i < mFrameNum; i++) {
            mFrames[i].pMaterial->Animate();
        }
    }
}

void Window::UnbindAnimationSelf(AnimTransform* pAnimTrans) {
    for (int i = 0; i < mFrameNum; i++) {
        mFrames[i].pMaterial->UnbindAnimation(pAnimTrans);
    }

    Pane::UnbindAnimationSelf(pAnimTrans);
}

void Window::DrawContent(const math::VEC2& basePt, const WindowFrameSize& frameSize, u8 alpha) {
    bool bUseVtxCol = mpMaterial->SetupGX(detail::IsModulateVertexColor(mContent.vtxColors, alpha), alpha);

    detail::SetVertexFormat(bUseVtxCol, mContent.texCoordAry.GetSize());

    // clang-format off
    detail::DrawQuad(
	    math::VEC2(basePt.x + frameSize.l - mContentInflation.l,
		           basePt.y + frameSize.t - mContentInflation.t),
	    Size(mSize.width    - frameSize.l + mContentInflation.l
		                    - frameSize.r + mContentInflation.r,
		     mSize.height   - frameSize.t + mContentInflation.t
		                    - frameSize.b + mContentInflation.b),
	    mContent.texCoordAry.GetSize(),
	    mContent.texCoordAry.GetArray(),
	    bUseVtxCol ? mContent.vtxColors : nullptr,
	    alpha
	);
    // clang-format on
}

// clang-format off
void Window::DrawFrame(const math::VEC2 &basePt, const Frame &frame,
                       const WindowFrameSize &frameSize, u8 alpha)
{
    bool bUseVtxCol = frame.pMaterial->SetupGX(
	    detail::IsModulateVertexColor(nullptr, alpha), alpha);
    detail::SetVertexFormat(bUseVtxCol, 1);

    Size texSize = detail::GetTextureSize(frame.pMaterial, 0);
    ut::Color vtxColors[4];
    detail::TexCoords texCds[1]; // i imagine this is to get pointer decay semantics on the DrawQuad call
    math::VEC2 polPt;
    Size polSize;

#define DRAW_FRAME_1_QUAD_(corner_, frameIdx_)	\
    do																	\
	{																	\
	    Get ## corner_ ## FrameSize(&polPt, &polSize, basePt, mSize,	\
		                            frameSize);							\
	    Get ## corner_ ## TexCoord(*texCds, polSize, texSize,			\
		                           frameIdx_);							\
																		\
	    detail::DrawQuad(polPt, polSize, 1, texCds,						\
		                 bUseVtxCol ? vtxColors : nullptr, alpha);		\
	} while (0)

    DRAW_FRAME_1_QUAD_(LT, 0);
    DRAW_FRAME_1_QUAD_(RT, 1);
    DRAW_FRAME_1_QUAD_(RB, 4);
    DRAW_FRAME_1_QUAD_(LB, 2);

#undef DRAW_FRAME_1_QUAD_
}

void Window::DrawFrame4(const math::VEC2 &basePt, const Frame *frames,
                        const WindowFrameSize &frameSize, u8 alpha)
{
    ut::Color vtxColors[4];
    detail::TexCoords texCds[1];
    math::VEC2 polPt;
    Size polSize;
    bool bModVtxCol = detail::IsModulateVertexColor(nullptr, alpha);

#define DRAW_FRAME_4_QUAD_(corner_, frameIdx_)	\
    do																	\
	{																	\
	    bool bUseVtxCol =												\
		    frames[frameIdx_].pMaterial->SetupGX(bModVtxCol, alpha);	\
																		\
	    Get ## corner_ ## FrameSize(&polPt, &polSize, basePt, mSize,	\
		                            frameSize);							\
	    Get ## corner_ ## TexCoord(										\
			*texCds, polSize,											\
		    detail::GetTextureSize(frames[frameIdx_].pMaterial, 0),		\
		    frames[frameIdx_].textureFlip);								\
																		\
	    detail::SetVertexFormat(bUseVtxCol, 1);							\
																		\
	    detail::DrawQuad(polPt, polSize, 1, texCds,						\
		                 bUseVtxCol ? vtxColors : nullptr, alpha);		\
	} while (0)

    DRAW_FRAME_4_QUAD_(LT, 0);
    DRAW_FRAME_4_QUAD_(RT, 1);
    DRAW_FRAME_4_QUAD_(RB, 3);
    DRAW_FRAME_4_QUAD_(LB, 2);

#undef DRAW_FRAME_4_QUAD_
}

void Window::DrawFrame8(const math::VEC2 &basePt, const Frame *frames,
                        const WindowFrameSize &frameSize, u8 alpha)
{
    ut::Color vtxColors[4];
    detail::TexCoords texCds[1];
    Size polSize;
    bool bModVtxCol = detail::IsModulateVertexColor(nullptr, alpha);

#define DRAW_FRAME_8_QUAD_(corner_, frameIdx_, polSizeInit_, basePtInit_)	\
    do																		\
	{																		\
	    bool bUseVtxCol =													\
		    frames[frameIdx_].pMaterial->SetupGX(bModVtxCol, alpha);		\
	    polSize = Size polSizeInit_;										\
																			\
	    Get ## corner_ ## TexCoord(											\
			*texCds, polSize,												\
		    detail::GetTextureSize(frames[frameIdx_].pMaterial, 0),			\
		    frames[frameIdx_].textureFlip);									\
																			\
	    detail::SetVertexFormat(bUseVtxCol, 1);								\
																			\
	    detail::DrawQuad(VEC_CTOR_ basePtInit_, polSize, 1, texCds,			\
		                 bUseVtxCol ? vtxColors : nullptr, alpha);			\
	} while (0)

#define VEC_CTOR_ // avoid copy construction specifically for this first call
    DRAW_FRAME_8_QUAD_(LT, 0, (frameSize.l, frameSize.t), basePt);
#undef VEC_CTOR_

#define VEC_CTOR_ math::VEC2

    DRAW_FRAME_8_QUAD_(LT, 6,
		(mSize.width - frameSize.l - frameSize.r, frameSize.t),
		(basePt.x + frameSize.l, basePt.y)
	);
    DRAW_FRAME_8_QUAD_(RT, 1,
		(frameSize.r, frameSize.t),
		(basePt.x + mSize.width - frameSize.r, basePt.y)
	);
    DRAW_FRAME_8_QUAD_(RT, 5,
		(frameSize.r, mSize.height - frameSize.t - frameSize.b),
		(basePt.x + mSize.width - frameSize.r, basePt.y + frameSize.t)
	);
    DRAW_FRAME_8_QUAD_(RB, 3, (frameSize.r, frameSize.b),
		(basePt.x + mSize.width - frameSize.r,
		 basePt.y + mSize.height - frameSize.b)
	);
    DRAW_FRAME_8_QUAD_(RB, 7,
		(mSize.width - frameSize.l - frameSize.r, frameSize.b),
		(basePt.x + frameSize.l, basePt.y + mSize.height - frameSize.b)
	);
    DRAW_FRAME_8_QUAD_(LB, 2,
		(frameSize.l, frameSize.b),
		(basePt.x, basePt.y + mSize.height - frameSize.b)
	);
    DRAW_FRAME_8_QUAD_(LB, 4,
		(frameSize.l, mSize.height - frameSize.t - frameSize.b),
		(basePt.x, basePt.y + frameSize.t)
	);

#undef VEC_CTOR_
#undef DRAW_FRAME_8_QUAD_
}
// clang-format on

WindowFrameSize Window::GetFrameSize(u8 frameNum, const Frame* frames) {
    WindowFrameSize ret = {};

    switch (frameNum) {
        case 1: {
            Size texSize = detail::GetTextureSize(frames->pMaterial, 0);
            ret.l = texSize.width;
            ret.t = texSize.height;

            ret.r = texSize.width;
            ret.b = texSize.height;
        } break;

        case 4:
        case 8: {
            Size texSize = detail::GetTextureSize(frames[0].pMaterial, 0);
            ret.l = texSize.width;
            ret.t = texSize.height;

            texSize = detail::GetTextureSize(frames[3].pMaterial, 0);
            ret.r = texSize.width;
            ret.b = texSize.height;
        } break;
    }

    return ret;
}

Material* Window::GetFrameMaterial(u32 frameIdx) const {
    NW4HBMAssert_Line(frameIdx < WINDOWFRAME_MAX, 658);
    if (frameIdx >= mFrameNum) {
        return nullptr;
    }

    return mFrames[frameIdx].pMaterial;
}

Material* Window::GetContentMaterial() const { return GetMaterial(); }

} // namespace lyt
} // namespace nw4hbm
