#include "revolution/hbm/nw4hbm/lyt/lyt_layout.hpp"

/* TODO: clean up CONVERT_OFFSET_TO_PTR macros, use constexpr variables instead
 * of macros and loose constants
 */

/*******************************************************************************
 * headers
 */

#include "new.hpp"

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/lyt/lyt_animation.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_bounding.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_common.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_group.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_pane.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_picture.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_textBox.hpp"
#include "revolution/hbm/nw4hbm/lyt/lyt_types.hpp" // detail::ConvertOffsToPtr
#include "revolution/hbm/nw4hbm/lyt/lyt_window.hpp"

#include "revolution/hbm/nw4hbm/ut/ut_LinkList.hpp" // IWYU pragma: keep (NW4HBM_RANGE_FOR)
#include "revolution/hbm/nw4hbm/ut/ut_Rect.hpp"
#include "revolution/hbm/nw4hbm/ut/ut_RuntimeTypeInfo.hpp"

#include "revolution/mem/mem_allocator.h"

/*******************************************************************************
 * macros
 */

// Kind of annoying
// I'll just kinda borrow the lyt function
#define CONVERT_OFFSET_TO_PTR(type_, ptr_, offset_) reinterpret_cast<type_*>(reinterpret_cast<u32>(ptr_) + offset_)

/*******************************************************************************
 * local function declarations
 */

namespace {
// pretend this is nw4hbm::lyt
using namespace nw4hbm;
using namespace nw4hbm::lyt;

void SetTagProcessorImpl(Pane* pPane, ut::TagProcessorBase<wchar_t>* pTagProcesssor);

// wait until these guys hear about c++11
template <class T> T* CreateObject() {
    void* pMem = Layout::AllocMemory(sizeof(T));

    if (pMem) {
        return new (pMem) T();
    } else {
        return nullptr;
    }
}

template <class T, typename Param1> T* CreateObject(Param1 p1) {
    void* pMem = Layout::AllocMemory(sizeof(T));

    if (pMem) {
        return new (pMem) T(p1);
    } else {
        return nullptr;
    }
}

template <class T, typename Param1, typename Param2> T* CreateObject(Param1 p1, Param2 p2) {
    void* pMem = Layout::AllocMemory(sizeof(T));

    if (pMem) {
        return new (pMem) T(p1, p2);
    } else {
        return nullptr;
    }
}
} // unnamed namespace

/*******************************************************************************
 * variables
 */

namespace nw4hbm {
namespace lyt {
// .bss
MEMAllocator* Layout::mspAllocator;
} // namespace lyt
} // namespace nw4hbm

/*******************************************************************************
 * functions
 */

namespace {

void SetTagProcessorImpl(Pane* pPane, ut::TagProcessorBase<wchar_t>* pTagProcessor) {
    if (TextBox* pTextBox = ut::DynamicCast<TextBox*>(pPane)) {
        pTextBox->SetTagProcessor(pTagProcessor);
    }

    NW4HBM_RANGE_FOR(it, pPane->GetChildList())
    SetTagProcessorImpl(&(*it), pTagProcessor);
}

} // anonymous namespace

namespace nw4hbm {
namespace lyt {

Layout::Layout() : mpRootPane(nullptr), mpGroupContainer(nullptr), mLayoutSize(0.0f, 0.0f), mOriginType(0) {}

Layout::~Layout() {
    if (mpGroupContainer) {
        mpGroupContainer->~GroupContainer();
        FreeMemory(mpGroupContainer);
    }

    if (mpRootPane && !mpRootPane->IsUserAllocated()) {
        mpRootPane->~Pane();
        FreeMemory(mpRootPane);
    }

    NW4HBM_RANGE_FOR_NO_AUTO_INC(it, mAnimTransList) {
        DECLTYPE(it) currIt = it++;

        mAnimTransList.Erase(currIt);
        currIt->~AnimTransform();
        FreeMemory(&(*currIt));
    }
}

bool Layout::Build(const void* lytResBuf, ResourceAccessor* pResAcsr) {
    const res::BinaryFileHeader* fileHead = static_cast<const res::BinaryFileHeader*>(lytResBuf);

    if (!detail::TestFileHeader(*fileHead, FILE_HEADER_SIGNATURE_LAYOUT)) {
        return false;
    }

    ResBlockSet resBlockSet = {};
    resBlockSet.pResAccessor = pResAcsr;

    Pane* pParentPane = nullptr;
    Pane* pLastPane = nullptr;
    bool bReadRootGroup = false;
    int groupNestLevel = 0;
    void* dataPtr = CONVERT_OFFSET_TO_PTR(void, lytResBuf, fileHead->headerSize);

    for (int i = 0; i < fileHead->dataBlocks; i++) {
        res::DataBlockHeader* pDataBlockHead = static_cast<res::DataBlockHeader*>(dataPtr);

        switch (detail::GetSignatureInt(pDataBlockHead->kind)) {
            case 'lyt1': {
                res::Layout* pResLyt = static_cast<res::Layout*>(dataPtr);

                mOriginType = pResLyt->originType != 0; // ?
                mLayoutSize = pResLyt->layoutSize;
            }

            break;

            case 'txl1':
                resBlockSet.pTextureList = static_cast<res::TextureList*>(dataPtr);
                break;

            case 'fnl1':
                resBlockSet.pFontList = static_cast<res::FontList*>(dataPtr);
                break;

            case 'mat1':
                resBlockSet.pMaterialList = static_cast<res::MaterialList*>(dataPtr);
                break;

            case 'pan1':
            case 'bnd1':
            case 'pic1':
            case 'txt1':
            case 'wnd1':
                if (Pane* pPane = BuildPaneObj(detail::GetSignatureInt(pDataBlockHead->kind), dataPtr, resBlockSet)) {
                    if (!mpRootPane) {
                        mpRootPane = pPane;
                    }

                    if (pParentPane) {
                        pParentPane->AppendChild(pPane);
                    }

                    pLastPane = pPane;
                }

                break;

            case 'pas1': // pane start?
                pParentPane = pLastPane;
                break;

            case 'pae1': // pane end?
                pLastPane = pParentPane;
                pParentPane = pLastPane->GetParent();
                break;

            case 'grp1':
                if (!bReadRootGroup) {
                    bReadRootGroup = true;
                    mpGroupContainer = CreateObject<GroupContainer>();
                } else if (mpGroupContainer && groupNestLevel == 1) {
                    if (Group* pGroup =
                            CreateObject<Group>(reinterpret_cast<const res::Group*>(pDataBlockHead), mpRootPane)) {
                        mpGroupContainer->AppendGroup(pGroup);
                    }
                }
                break;

            case 'grs1': // group start?
                groupNestLevel++;
                break;

            case 'gre1': // group end?
                groupNestLevel--;
                break;
        }

        dataPtr = CONVERT_OFFSET_TO_PTR(void, dataPtr, pDataBlockHead->size);
    }

    return true;
}

AnimTransform* Layout::CreateAnimTransform(const void* anmResBuf, ResourceAccessor* pResAcsr) {
    const res::BinaryFileHeader* pFileHead = static_cast<const res::BinaryFileHeader*>(anmResBuf);

    if (!detail::TestFileHeader(*pFileHead)) {
        return nullptr;
    }

    const res::AnimationBlock* pInfoBlock = nullptr;
    const res::DataBlockHeader* pDataBlockHead =
        detail::ConvertOffsToPtr<res::DataBlockHeader>(pFileHead, pFileHead->headerSize);

    AnimTransform* ret = nullptr;

    for (int i = 0; i < pFileHead->dataBlocks; i++) {
        switch (detail::GetSignatureInt(pDataBlockHead->kind)) {
            case 'pai1': // painting? idk
                switch (detail::GetSignatureInt(pFileHead->signature)) {
                    case 'RLAN':
                    case res::AnimationInfo::ANIM_INFO_PANE_PAIN_SRT:
                    case res::AnimationInfo::ANIM_INFO_PANE_VISIBILITY:
                    case res::AnimationInfo::ANIM_INFO_PANE_VERTEX_COLOR:
                    case res::AnimationInfo::ANIM_INFO_MATERIAL_COLOR:
                    case res::AnimationInfo::ANIM_INFO_MATERIAL_TEXTURE_SRT:
                    case res::AnimationInfo::ANIM_INFO_MATERIAL_TEXTURE_PATTERN:
                        if (AnimTransformBasic* pAnimTrans = CreateObject<AnimTransformBasic>()) {
                            pInfoBlock = reinterpret_cast<const res::AnimationBlock*>(pDataBlockHead);

                            pAnimTrans->SetResource(pInfoBlock, pResAcsr);
                            ret = pAnimTrans;
                        }
                }

                if (ret) {
                    mAnimTransList.PushBack(ret);
                }
        }

        pDataBlockHead = detail::ConvertOffsToPtr<res::DataBlockHeader>(pDataBlockHead, pDataBlockHead->size);
    }

    return ret;
}

void Layout::BindAnimation(AnimTransform* pAnimTrans) {
    if (mpRootPane) {
        mpRootPane->BindAnimation(pAnimTrans, true);
    }
}

void Layout::UnbindAnimation(AnimTransform* pAnimTrans) {
    if (mpRootPane) {
        mpRootPane->UnbindAnimation(pAnimTrans, true);
    }
}

void Layout::UnbindAllAnimation() { UnbindAnimation(nullptr); }

void Layout::SetAnimationEnable(AnimTransform* pAnimTrans, bool bEnable) {
    if (mpRootPane) {
        mpRootPane->SetAnimationEnable(pAnimTrans, bEnable, true);
    }
}

void Layout::CalculateMtx(const DrawInfo& drawInfo) {
    if (mpRootPane) {
        mpRootPane->CalculateMtx(drawInfo);
    }
}

void Layout::Draw(const DrawInfo& drawInfo) {
    if (mpRootPane) {
        mpRootPane->Draw(drawInfo);
    }
}

void Layout::Animate(u32 option) {
    if (mpRootPane) {
        mpRootPane->Animate(option);
    }
}

ut::Rect Layout::GetLayoutRect() const {
    if (mOriginType == 1) {
        return ut::Rect(-mLayoutSize.width / 2.0f, mLayoutSize.height / 2.0f, mLayoutSize.width / 2.0f,
                        -mLayoutSize.height / 2.0f);
    } else {
        return ut::Rect(0.0f, 0.0f, mLayoutSize.width, mLayoutSize.height);
    }
}

void Layout::SetTagProcessor(ut::TagProcessorBase<wchar_t>* pTagProcessor) {
    SetTagProcessorImpl(mpRootPane, pTagProcessor);
}

Pane* Layout::BuildPaneObj(s32 kind, const void* dataPtr, const ResBlockSet& resBlockSet) {
    switch (kind) {
        case OBJECT_KIND_PANE: {
            const res::Pane* pResPane = static_cast<const res::Pane*>(dataPtr);

            return CreateObject<Pane>(pResPane);
        }

        case OBJECT_KIND_PICTURE: {
            const res::Picture* pResPic = static_cast<const res::Picture*>(dataPtr);

            return CreateObject<Picture>(pResPic, resBlockSet);
        }

        case OBJECT_KIND_TEXT_BOX: {
            // block?
            const res::TextBox* pBlock = static_cast<const res::TextBox*>(dataPtr);

            return CreateObject<TextBox>(pBlock, resBlockSet);
        }

        case OBJECT_KIND_WINDOW: {
            // block?
            const res::Window* pBlock = static_cast<const res::Window*>(dataPtr);

            return CreateObject<Window>(pBlock, resBlockSet);
        }

        case OBJECT_KIND_BOUNDING: {
            const res::Bounding* pResBounding = static_cast<const res::Bounding*>(dataPtr);

            return CreateObject<Bounding>(pResBounding, resBlockSet);
        }

        default:
            return nullptr;
    }
}

} // namespace lyt
} // namespace nw4hbm
