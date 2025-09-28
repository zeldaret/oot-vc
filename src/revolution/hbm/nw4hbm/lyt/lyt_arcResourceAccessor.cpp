#include "revolution/hbm/nw4hbm/lyt/arcResourceAccessor.h"

#include "revolution/arc.h"
#include "revolution/hbm/nw4hbm/ut.h"

#include "decomp.h"
#include "string.h"

namespace {

s32 FindNameResource(ARCHandle* pArcHandle, const char* resName) NO_INLINE {
    s32 entryNum = -1;

    ARCDir dir;
    BOOL bSuccess = ARCOpenDir(pArcHandle, ".", &dir);
    NW4HBMAssert_Line(bSuccess, 48);

    ARCEntry dirEntry;

    while (ARCReadDir(&dir, &dirEntry)) {
        if (dirEntry.type != ARC_ENTRY_FILE) {
            bSuccess = ARCChangeDir(pArcHandle, dirEntry.name);
            NW4HBMAssert_Line(bSuccess, 57);

            entryNum = FindNameResource(pArcHandle, resName);
            bSuccess = ARCChangeDir(pArcHandle, "..");
            NW4HBMAssert_Line(bSuccess, 60);

            if (entryNum != -1) {
                break;
            }
        } else if (stricmp(resName, dirEntry.name) == 0) {
            entryNum = dirEntry.path;
            break;
        }
    }

    bSuccess = ARCCloseDir(&dir);
    NW4HBMAssert_Line(bSuccess, 77);
    return entryNum;
}

void* GetResourceSub(ARCHandle* pArcHandle, const char* resRootDir, u32 resType, const char* name, u32* pSize) {
    s32 entryNum = -1;

    if (ARCConvertPathToEntrynum(pArcHandle, resRootDir) != -1 && ARCChangeDir(pArcHandle, resRootDir)) {
        if (!resType) {
            entryNum = FindNameResource(pArcHandle, name);
        } else {
            char resTypeStr[5];
            resTypeStr[0] = resType >> 24;
            resTypeStr[1] = resType >> 16;
            resTypeStr[2] = resType >> 8;
            resTypeStr[3] = resType;
            resTypeStr[4] = '\0';

            if (ARCConvertPathToEntrynum(pArcHandle, resTypeStr) != -1 && ARCChangeDir(pArcHandle, resTypeStr)) {
                entryNum = ARCConvertPathToEntrynum(pArcHandle, name);
                BOOL bSuccess = ARCChangeDir(pArcHandle, "..");
                NW4HBMAssert_Line(bSuccess, 117);
            }
        }

        BOOL bSuccess = ARCChangeDir(pArcHandle, "..");
        NW4HBMAssert_Line(bSuccess, 123);
    }

    if (entryNum != -1) {
        ARCFileInfo arcFileInfo;
        BOOL bSuccess = ARCFastOpen(pArcHandle, entryNum, &arcFileInfo);
        NW4HBMAssert_Line(bSuccess, 131);

        void* resPtr = ARCGetStartAddrInMem(&arcFileInfo);

        if (pSize) {
            *pSize = ARCGetLength(&arcFileInfo);
        }

        ARCClose(&arcFileInfo);

        return resPtr;
    }

    return nullptr;
}
} // namespace

namespace nw4hbm {
namespace lyt {
ut::Font* detail::FindFont(FontRefLinkList* pFontRefList, const char* name) {
    for (FontRefLinkList::Iterator it = pFontRefList->GetBeginIter(); it != pFontRefList->GetEndIter(); it++) {
        if (strcmp(name, it->GetFontName()) == 0) {
            return it->GetFont();
        }
    }
    return nullptr;
}

FontRefLink::FontRefLink() : mpFont(nullptr) {}

void FontRefLink::Set(const char* name, ut::Font* pFont) {
    strcpy(mFontName, name);
    mpFont = pFont;
}

ArcResourceAccessor::ArcResourceAccessor() : mArcBuf(NULL) {}

DECOMP_FORCE(NW4HBMAssert_String(std::strlen(name) < FONTNAMEBUF_MAX));

bool ArcResourceAccessor::Attach(void* archiveStart, const char* resourceRootDirectory) {
    // clang-format off
    NW4HBMAssert_Line(! IsAttached(), 220);
    NW4HBMAssertPointerNonnull_Line(archiveStart, 221);
    NW4HBMAssertPointerNonnull_Line(resourceRootDirectory, 222);
    // clang-format on
    
    BOOL bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

    if (!bSuccess) {
        return false;
    }

    mArcBuf = archiveStart;

    strncpy(mResRootDir, resourceRootDirectory, ARRAY_COUNT(mResRootDir) - 1);
    mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

    return true;
}

DECOMP_FORCE(NW4HBMAssert_String(IsAttached()));
DECOMP_FORCE(NW4HBMAssertPointerNonnull_String(pLink));

void* ArcResourceAccessor::GetResource(u32 resType, const char* name, u32* pSize) {
    return GetResourceSub(&mArcHandle, mResRootDir, resType, name, pSize);
}

bool ArcResourceLink::Set(void* archiveStart, const char* resRootDirectory) {
    BOOL bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

    if (!bSuccess) {
        return false;
    }

    strncpy(mResRootDir, resRootDirectory, ARRAY_COUNT(mResRootDir) - 1);
    mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

    return true;
}

ut::Font* ArcResourceAccessor::GetFont(const char* name) { return detail::FindFont(&mFontList, name); }

MultiArcResourceAccessor::MultiArcResourceAccessor() {}

MultiArcResourceAccessor::~MultiArcResourceAccessor() { DetachAll(); }

void MultiArcResourceAccessor::Attach(ArcResourceLink* pLink) { mArcList.PushBack(pLink); }

// it requires a type that wasn't used before to generate the string and avoid having it stripped
typedef ut::LinkList<void*, 0> DummyLinkList;
DECOMP_FORCE_CLASS_METHOD(nw4hbm::lyt::DummyLinkList, GetNodeFromPointer(nullptr));

void* MultiArcResourceAccessor::GetResource(u32 resType, const char* name, u32* pSize) {
    for (ArcResourceLinkList::Iterator it = mArcList.GetBeginIter(); it != mArcList.GetEndIter(); it++) {
        ARCHandle* pArcHandle = it->GetArcHandle();
        if (void* resPtr = GetResourceSub(pArcHandle, it->GetResRootDir(), resType, name, pSize)) {
            return resPtr;
        }
    }
    return nullptr;
}

void MultiArcResourceAccessor::RegistFont(FontRefLink* pLink) { mFontList.PushBack(pLink); }

ut::Font* MultiArcResourceAccessor::GetFont(const char* name) { return detail::FindFont(&mFontList, name); }

} // namespace lyt
} // namespace nw4r
