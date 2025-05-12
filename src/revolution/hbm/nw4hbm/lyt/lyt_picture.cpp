#include "revolution/hbm/nw4hbm/lyt/lyt_picture.hpp"

/*******************************************************************************
 * headers
 */

#include "new.hpp"

#include "macros.h" // ARRAY_COUNT
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_layout.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_material.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_types.hpp" // detail::ConvertOffsToPtr

#include "revolution/hbm/nw4hbm/ut/ut_Color.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_inlines.hpp" // ut::Min
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"

#include "revolution/tpl/TPL.h"
#include "revolution/gx/GXTypes.h"

/*******************************************************************************
 * variables
 */

namespace nw4hbm { namespace lyt
{
	const ut::detail::RuntimeTypeInfo Picture::typeInfo(&Pane::typeInfo);
}} // namespace nw4hbm::lyt

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace lyt {

Picture::Picture(const res::Picture *pResPic, const ResBlockSet &resBlockSet):
	Pane(pResPic)
{
	u8 texCoordNum = ut::Min<u8>(pResPic->texCoordNum, 8);

	Init(texCoordNum);

	for (int i = 0; i < (int)ARRAY_COUNT(mVtxColors); i++)
		mVtxColors[i] = pResPic->vtxCols[i];

	if (texCoordNum && !mTexCoordAry.IsEmpty())
		mTexCoordAry.Copy(&pResPic[1], texCoordNum);

	if (Material *pMemMaterial =
	        static_cast<Material *>(Layout::AllocMemory(sizeof *pMemMaterial)))
	{
		const u32 *matOffsTbl =
			detail::ConvertOffsToPtr<u32>(resBlockSet.pMaterialList, 0xc);
		const res::Material *pResMaterial =
			detail::ConvertOffsToPtr<res::Material>(
				resBlockSet.pMaterialList, matOffsTbl[pResPic->materialIdx]);

		mpMaterial = new (pMemMaterial) Material(pResMaterial, resBlockSet);
	}
}

void Picture::Init(u8 texNum)
{
	if (texNum)
		ReserveTexCoord(texNum);
}

Picture::~Picture()
{
	if (mpMaterial && !mpMaterial->IsUserAllocated())
	{
		mpMaterial->~Material();
		Layout::FreeMemory(mpMaterial);
		mpMaterial = nullptr;
	}

	mTexCoordAry.Free();
}

void Picture::Append(TPLPalette *pTplRes)
{
	GXTexObj texObj;
	detail::InitGXTexObjFromTPL(&texObj, pTplRes, 0);

	Append(texObj);
}

void Picture::Append(const GXTexObj &texObj)
{
	if (mpMaterial->GetTextureNum() >= mpMaterial->GetTextureCap()
	    || mpMaterial->GetTextureNum() >= mpMaterial->GetTexCoordGenCap())
	{
		return;
	}

	u8 texIdx = mpMaterial->GetTextureNum();
	mpMaterial->SetTextureNum(texIdx + 1);
	mpMaterial->SetTexture(texIdx, texObj);
	mpMaterial->SetTexCoordGenNum(mpMaterial->GetTextureNum());
	mpMaterial->SetTexCoordGen(texIdx, TexCoordGen());

	SetTexCoordNum(mpMaterial->GetTextureNum());

	if (mSize == Size(0.0f, 0.0f) && mpMaterial->GetTextureNum() == 1)
		mSize = detail::GetTextureSize(mpMaterial, 0);
}

void Picture::ReserveTexCoord(u8 num)
{
	mTexCoordAry.Reserve(num);
}

void Picture::SetTexCoordNum(u8 num)
{
	mTexCoordAry.SetSize(num);
}

const ut::Color Picture::GetVtxColor(u32 idx) const
{
	return mVtxColors[idx];
}

void Picture::SetVtxColor(u32 idx, ut::Color value)
{
	mVtxColors[idx] = value;
}

u8 Picture::GetVtxColorElement(u32 idx) const
{
	return detail::GetVtxColorElement(mVtxColors, idx);
}

void Picture::SetVtxColorElement(u32 idx, u8 value)
{
	detail::SetVtxColorElement(mVtxColors, idx, value);
}

void Picture::DrawSelf(const DrawInfo &drawInfo)
{
	if (!mpMaterial)
		return;

	LoadMtx(drawInfo);

	bool bUseVtxCol = mpMaterial->SetupGX(
		detail::IsModulateVertexColor(mVtxColors, mGlbAlpha), mGlbAlpha);
	nw4hbm::lyt::detail::SetVertexFormat(bUseVtxCol, mTexCoordAry.GetSize());

	detail::DrawQuad(GetVtxPos(), mSize, mTexCoordAry.GetSize(),
	                 mTexCoordAry.GetArray(), bUseVtxCol ? mVtxColors : nullptr,
	                 mGlbAlpha);
}

}} // namespace nw4hbm::lyt
