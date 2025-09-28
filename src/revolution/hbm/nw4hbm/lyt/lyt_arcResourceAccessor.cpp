#include "revolution/hbm/nw4hbm/lyt/arcResourceAccessor.h"

#include "revolution/arc.h"
#include "revolution/hbm/nw4hbm/ut.h"

#include "string.h"

namespace {
// pretend this is nw4hbm::lyt
using namespace nw4hbm;
using namespace nw4hbm::lyt;

s32 FindNameResource(ARCHandle* pArcHandle, const char* resName);
void* GetResourceSub(ARCHandle* pArcHandle, const char* resRootDir, u32 resType, const char* name, u32* pSize);
} // namespace

namespace {

#pragma push
#pragma dont_inline on

s32 FindNameResource(ARCHandle* pArcHandle, const char* resName) {
    s32 entryNum = -1;

    ARCDir dir;
    bool bSuccess ATTRIBUTE_UNUSED = ARCOpenDir(pArcHandle, ".", &dir);
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

#pragma pop

static char unused1[] = "NW4HBM:Failed assertion std::strlen(name) < FONTNAMEBUF_MAX";

void* GetResourceSub(ARCHandle* pArcHandle, const char* resRootDir, u32 resType, const char* name, u32* pSize) {
    (void)unused1; // necessary

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
                bool bSuccess ATTRIBUTE_UNUSED = ARCChangeDir(pArcHandle, "..");
                NW4HBMAssert_Line(bSuccess, 117);
            }
        }

        bool bSuccess ATTRIBUTE_UNUSED = ARCChangeDir(pArcHandle, "..");
        NW4HBMAssert_Line(bSuccess, 123);
    }

    if (entryNum != -1) {
        ARCFileInfo arcFileInfo;
        bool bSuccess ATTRIBUTE_UNUSED = ARCFastOpen(pArcHandle, entryNum, &arcFileInfo);
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

} // unnamed namespace

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

ArcResourceAccessor::ArcResourceAccessor() : mArcBuf(nullptr) {}

bool ArcResourceAccessor::Attach(void* archiveStart, const char* resourceRootDirectory) {
    // clang-format off
    NW4HBMAssert_Line(! IsAttached(), 220);
    NW4HBMAssertPointerNonnull_Line(archiveStart, 221);
    NW4HBMAssertPointerNonnull_Line(resourceRootDirectory, 222);
    // clang-format on

    bool bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

    if (!bSuccess) {
        return false;
    }

    mArcBuf = archiveStart;
    std::strncpy(mResRootDir, resourceRootDirectory, ARRAY_COUNT(mResRootDir) - 1);
    mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

    return true;
}

static char unused2[] = "NW4HBM:Failed assertion IsAttached()";
static char unused3[] = "NW4HBM:Pointer must not be NULL (pLink)";

void* ArcResourceAccessor::GetResource(u32 resType, const char* name, u32* pSize) {
    // necessary
    (void)unused2;
    (void)unused3;

    return GetResourceSub(&mArcHandle, mResRootDir, resType, name, pSize);
}

bool ArcResourceLink::Set(void* archiveStart, const char* resourceRootDirectory) {
    bool bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

    if (!bSuccess) {
        return false;
    }

    std::strncpy(mResRootDir, resourceRootDirectory, ARRAY_COUNT(mResRootDir) - 1);
    mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

    return true;
}

ut::Font* ArcResourceAccessor::GetFont(const char* name) { return detail::FindFont(&mFontList, name); }

MultiArcResourceAccessor::MultiArcResourceAccessor() {}

MultiArcResourceAccessor::~MultiArcResourceAccessor() { DetachAll(); }

void MultiArcResourceAccessor::Attach(ArcResourceLink* pLink) { mArcList.PushBack(pLink); }

void* MultiArcResourceAccessor::GetResource(u32 resType, const char* name, u32* pSize) {
    for (ArcResourceLinkList::Iterator it = mArcList.GetBeginIter(); it != mArcList.GetEndIter(); it++) {
        ARCHandle* pArcHandle = it->GetArcHandle();
        if (void* resPtr = GetResourceSub(pArcHandle, it->GetResRootDir(), resType, name, pSize)) {
            return resPtr;
        }
    }

    return nullptr;
}

ut::Font* MultiArcResourceAccessor::GetFont(const char* name) { return detail::FindFont(&mFontList, name); }

} // namespace lyt
} // namespace nw4hbm
