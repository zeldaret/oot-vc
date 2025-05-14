#include "revolution/hbm/nw4hbm/lyt/pane.h"

/*******************************************************************************
 * headers
 */

#include "cstring.hpp" // std::strncpy

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/lyt_animation.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_drawInfo.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_layout.hpp"
#include "revolution/hbm/nw4hbm/lyt/material.h"

#include "revolution/hbm/nw4hbm/math/math_types.hpp"
#include "revolution/hbm/nw4hbm/ut/LinkList.h" // IWYU pragma: keep (NW4HBM_RANGE_FOR)
#include "revolution/hbm/nw4hbm/ut/ut_Color.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_Rect.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"

#include "revolution/gx/GXTransform.h"
#include "revolution/mtx/mtx.h"

/*******************************************************************************
 * local function declarations
 */

namespace {
// pretend this is nw4hbm::lyt
using namespace nw4hbm;
using namespace nw4hbm::lyt;

void ReverseYAxis(math::MTX34* pMtx);
} // unnamed namespace

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace lyt {
const ut::detail::RuntimeTypeInfo Pane::typeInfo(nullptr);
}
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace {

void ReverseYAxis(math::MTX34* pMtx) {
    pMtx->m[0][1] = -pMtx->m[0][1];
    pMtx->m[1][1] = -pMtx->m[1][1];
    pMtx->m[2][1] = -pMtx->m[2][1];
}

} // unnamed namespace

namespace nw4hbm {
namespace lyt {

detail::PaneBase::PaneBase() {}
detail::PaneBase::~PaneBase() {}

extern void __deadstrip1();
extern void __deadstrip1() {
    (void)0.0f;
    (void)1.0f;
}

Pane::Pane(const res::Pane* pBlock) {
    Init();

    mBasePosition = pBlock->basePosition;
    SetName(pBlock->name);
    SetUserData(pBlock->userData);

    mTranslate = pBlock->translate;
    mRotate = pBlock->rotate;
    mScale = pBlock->scale;
    mSize = pBlock->size;
    mAlpha = pBlock->alpha;
    mGlbAlpha = mAlpha;
    mFlag = pBlock->flag;
}

void Pane::Init() {
    mpParent = nullptr;
    mpMaterial = nullptr;
    mbUserAllocated = false;
}

Pane::~Pane() {
    NW4HBM_RANGE_FOR_NO_AUTO_INC(it, mChildList) {
        LinkList::Iterator currIt = it++;
        mChildList.Erase(currIt);

        if (!currIt->IsUserAllocated()) {
            currIt->~Pane();
            Layout::FreeMemory(&(*currIt));
        }
    }

    UnbindAnimationSelf(nullptr);

    if (mpMaterial && !mpMaterial->IsUserAllocated()) {
        mpMaterial->~Material();
        Layout::FreeMemory(mpMaterial);
    }
}

void Pane::SetName(const char* name) { std::strncpy(mName, name, sizeof mName); }

void Pane::SetUserData(const char* userData) { std::strncpy(mUserData, userData, sizeof mUserData); }

void Pane::AppendChild(Pane* pChild) { InsertChild(mChildList.GetEndIter(), pChild); }

extern void _deadstrip2();
extern void _deadstrip2() { Pane::LinkList::GetNodeFromPointer(nullptr); }

void Pane::InsertChild(LinkList::Iterator next, Pane* pChild) {
    mChildList.Insert(next, pChild);
    pChild->mpParent = this;
}

const ut::Rect Pane::GetPaneRect(const DrawInfo& drawInfo) const {
    ut::Rect ret;
    math::VEC2 basePt = GetVtxPos();

    ret.left = basePt.x;
    ret.top = basePt.y;
    ret.right = ret.left + mSize.width;
    ret.bottom = ret.top + mSize.height;

    if (drawInfo.IsYAxisUp()) {
        ret.top = -ret.top;
        ret.bottom = -ret.bottom;
    }

    return ret;
}

const ut::Color Pane::GetVtxColor(u32) const { return ut::Color(0xffffffff); }

void Pane::SetVtxColor(u32, ut::Color) { /* ... */ }

u8 Pane::GetColorElement(u32 idx) const {
    switch (idx) {
        case 16:
            return mAlpha;

        default:
            return GetVtxColorElement(idx);
    }
}

void Pane::SetColorElement(u32 idx, u8 value) {
    switch (idx) {
        case 16:
            mAlpha = value;
            break;

        default:
            SetVtxColorElement(idx, value);
            break;
    }
}

u8 Pane::GetVtxColorElement(u32) const { return 0xff; }

void Pane::SetVtxColorElement(u32, u8) { /* ... */ }

Pane* Pane::FindPaneByName(const char* findName, bool bRecursive) {
    if (detail::EqualsPaneName(mName, findName)) {
        return this;
    }

    if (bRecursive) {
        NW4HBM_RANGE_FOR(it, mChildList) {
            if (Pane* pPane = it->FindPaneByName(findName, true)) {
                return pPane;
            }
        }
    }

    return nullptr;
}

Material* Pane::FindMaterialByName(const char* findName, bool bRecursive) {
    if (mpMaterial && detail::EqualsMaterialName(mpMaterial->GetName(), findName)) {
        return mpMaterial;
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

void Pane::CalculateMtx(const DrawInfo& drawInfo) {
    // this doesnt even get referenced, an entirely new constant is generated lol
    const f32 invAlpha = 1.0f / 255.0f;

    if (!detail::TestBit(mFlag, 0) && !drawInfo.IsInvisiblePaneCalculateMtx()) {
        return;
    }

    { // 0x498f0f wants lexical_block
        math::MTX34 mtx1;
        math::MTX34 mtx2;
        math::MTX34 rotateMtx;

        { // 0x498f5e wants lexical_block
            math::VEC2 scale(mScale);

            if (drawInfo.IsLocationAdjust() && detail::TestBit(mFlag, 2)) {
                scale.x *= drawInfo.GetLocationAdjustScale().x;
                scale.y *= drawInfo.GetLocationAdjustScale().y;
            }

            PSMTXScale(mtx2, scale.x, scale.y, 1.0f);
        }

        PSMTXRotRad(rotateMtx, 'x', DEG_TO_RAD(mRotate.x));
        PSMTXConcat(rotateMtx, mtx2, mtx1);

        PSMTXRotRad(rotateMtx, 'y', DEG_TO_RAD(mRotate.y));
        PSMTXConcat(rotateMtx, mtx1, mtx2);

        PSMTXRotRad(rotateMtx, 'z', DEG_TO_RAD(mRotate.z));
        PSMTXConcat(rotateMtx, mtx2, mtx1);

        PSMTXTransApply(mtx1, mMtx, mTranslate.x, mTranslate.y, mTranslate.z);
    }

    if (mpParent) {
        math::MTX34Mult(&mGlbMtx, &mpParent->mGlbMtx, &mMtx);
    } else if (drawInfo.IsMultipleViewMtxOnDraw()) {
        mGlbMtx = mMtx;
    } else {
        math::MTX34Mult(&mGlbMtx, &drawInfo.GetViewMtx(), &mMtx);
    }

    if (drawInfo.IsInfluencedAlpha() && mpParent) {
        mGlbAlpha = mAlpha * drawInfo.GetGlobalAlpha();
    } else {
        mGlbAlpha = mAlpha;
    }

    // cr = const ref?
    f32 crGlobalAlpha = drawInfo.GetGlobalAlpha();
    bool bCrInfluenced = drawInfo.IsInfluencedAlpha();

    bool bModDrawInfo = detail::TestBit(mFlag, 1) && mAlpha != 0xff;

    if (bModDrawInfo) {
        // mt = mutable, probably
        DrawInfo& mtDrawInfo = const_cast<DrawInfo&>(drawInfo);

        mtDrawInfo.SetGlobalAlpha(crGlobalAlpha * mAlpha * invAlpha);
        mtDrawInfo.SetInfluencedAlpha(true);
    }

    CalculateMtxChild(drawInfo);

    // restore changed values if applicable
    if (bModDrawInfo) {
        DrawInfo& mtDrawInfo = const_cast<DrawInfo&>(drawInfo);

        mtDrawInfo.SetGlobalAlpha(crGlobalAlpha);
        mtDrawInfo.SetInfluencedAlpha(bCrInfluenced);
    }
}

void Pane::CalculateMtxChild(const DrawInfo& drawInfo) {
    NW4HBM_RANGE_FOR(it, mChildList)
    it->CalculateMtx(drawInfo);
}

void Pane::Draw(const DrawInfo& drawInfo) {
    if (detail::TestBit(mFlag, 0)) {
        DrawSelf(drawInfo);

        NW4HBM_RANGE_FOR(it, mChildList)
        it->Draw(drawInfo);
    }
}

void Pane::DrawSelf(const DrawInfo& drawInfo) {
    if (mpParent && drawInfo.IsDebugDrawMode()) {
        LoadMtx(drawInfo);
        detail::DrawLine(GetVtxPos(), mSize, 0x00ff00ff); // green
    }
}

void Pane::Animate(u32 option) {
    AnimateSelf(option);

    if (detail::TestBit(mFlag, 0) || !(option & 1)) {
        NW4HBM_RANGE_FOR(it, mChildList)
        it->Animate(option);
    }
}

void Pane::AnimateSelf(u32 option) {
    NW4HBM_RANGE_FOR(it, mAnimList) {
        if (it->IsEnable()) {
            AnimTransform* animTrans = it->GetAnimTransform();

            animTrans->Animate(it->GetIndex(), this);
        }
    }

    if ((detail::TestBit(mFlag, 0) || !(option & 1)) && mpMaterial) {
        mpMaterial->Animate();
    }
}

void Pane::BindAnimation(AnimTransform* pAnimTrans, bool bRecursive) { pAnimTrans->Bind(this, bRecursive); }

void Pane::UnbindAnimation(AnimTransform* pAnimTrans, bool bRecursive) {
    UnbindAnimationSelf(pAnimTrans);

    if (bRecursive) {
        NW4HBM_RANGE_FOR(it, mChildList)
        it->UnbindAnimation(pAnimTrans, bRecursive);
    }
}

void Pane::UnbindAllAnimation(bool bRecursive) { UnbindAnimation(nullptr, bRecursive); }

void Pane::UnbindAnimationSelf(AnimTransform* pAnimTrans) {
    if (mpMaterial) {
        mpMaterial->UnbindAnimation(pAnimTrans);
    }

    NW4HBM_RANGE_FOR_NO_AUTO_INC(it, mAnimList) {
        DECLTYPE(it) currIt = it++;

        // nullptr is for UnbindAllAnimations
        if (pAnimTrans == nullptr || currIt->GetAnimTransform() == pAnimTrans) {
            mAnimList.Erase(currIt);
            currIt->Reset();
        }
    }
}

void Pane::AddAnimationLink(AnimationLink* pAnimationLink) { mAnimList.PushBack(pAnimationLink); }

AnimationLink* Pane::FindAnimationLink(AnimTransform* pAnimTrans) {
    if (AnimationLink* ret = detail::FindAnimationLink(&mAnimList, pAnimTrans)) {
        return ret;
    }

    if (mpMaterial) {
        if (AnimationLink* ret = mpMaterial->FindAnimationLink(pAnimTrans)) {
            return ret;
        }
    }

    return nullptr;
}

void Pane::SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable, bool bRecursive) {
    if (AnimationLink* pAnimLink = detail::FindAnimationLink(&mAnimList, pAnimTrans)) {
        pAnimLink->SetEnable(bEnable);
    }

    if (mpMaterial) {
        mpMaterial->SetAnimationEnable(pAnimTrans, bEnable);
    }

    if (bRecursive) {
        NW4HBM_RANGE_FOR(it, mChildList)
        it->SetAnimationEnable(pAnimTrans, bEnable, bRecursive);
    }
}

void Pane::LoadMtx(const DrawInfo& drawInfo) {
    math::MTX34 mtx;
    MtxPtr mtxPtr = nullptr;

    if (drawInfo.IsMultipleViewMtxOnDraw()) {
        math::MTX34Mult(&mtx, &drawInfo.GetViewMtx(), &mGlbMtx);

        if (drawInfo.IsYAxisUp()) {
            ReverseYAxis(&mtx);
        }

        mtxPtr = mtx;
    } else if (drawInfo.IsYAxisUp()) {
        math::MTX34Copy(&mtx, &mGlbMtx);
        ReverseYAxis(&mtx);
        mtxPtr = mtx;
    } else {
        mtxPtr = mGlbMtx;
    }

    GXLoadPosMtxImm(mtxPtr, 0);
    GXSetCurrentMtx(0);
}

math::VEC2 Pane::GetVtxPos() const {
    math::VEC2 basePt(0.0f, 0.0f);

    switch (mBasePosition % 3) {
        default:
        case 0:
            basePt.x = 0.0f;
            break;

        case 1:
            basePt.x = -mSize.width / 2.0f;
            break;

        case 2:
            basePt.x = -mSize.width;
            break;
    }

    switch (mBasePosition / 3) {
        default:
        case 0:
            basePt.y = 0.0f;
            break;

        case 1:
            basePt.y = -mSize.height / 2.0f;
            break;

        case 2:
            basePt.y = -mSize.height;
            break;
    }

    return basePt;
}

Material* Pane::GetMaterial() const { return mpMaterial; }

} // namespace lyt
} // namespace nw4hbm
