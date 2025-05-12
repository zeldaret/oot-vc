#include "revolution/hbm/nw4hbm/lyt/lyt_arcResourceAccessor.hpp"

/*******************************************************************************
 * headers
 */

#include "cstring.hpp"
#include "string.h" // stricmp

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/ut_LinkList.hpp" // IWYU pragma: keep (NW4HBM_RANGE_FOR)

#include "revolution/arc/arc.h"

/*******************************************************************************
 * local function declarations
 */

namespace
{
	// pretend this is nw4hbm::lyt
	using namespace nw4hbm;
	using namespace nw4hbm::lyt;

	s32 FindNameResource(ARCHandle *pArcHandle, const char *resName);
	void *GetResourceSub(ARCHandle *pArcHandle, const char *resRootDir,
	                     u32 resType, const char *name, u32 *pSize);
}

/*******************************************************************************
 * functions
 */

namespace {

#pragma push
#pragma dont_inline on

s32 FindNameResource(ARCHandle *pArcHandle, const char *resName)
{
	s32 entryNum = -1;

	ARCDir dir;
	bool bSuccess ATTRIBUTE_UNUSED = ARCOpenDir(pArcHandle, ".", &dir);

	ARCEntry dirEntry;
	while (ARCReadDir(&dir, &dirEntry))
	{
		if (dirEntry.type == ARC_ENTRY_FOLDER)
		{
			bSuccess = ARCChangeDir(pArcHandle, dirEntry.name);
			entryNum = FindNameResource(pArcHandle, resName);
			bSuccess = ARCChangeDir(pArcHandle, "..");

			if (entryNum != -1)
				break;
		}
		else if (stricmp(resName, dirEntry.name) == 0)
		{
			entryNum = dirEntry.path;
			break;
		}
	}

	bSuccess = ARCCloseDir(&dir);
	return entryNum;
}

#pragma pop

void *GetResourceSub(ARCHandle *pArcHandle, const char *resRootDir, u32 resType,
                     const char *name, u32 *pSize)
{
	s32 entryNum = -1;

	if (ARCConvertPathToEntrynum(pArcHandle, resRootDir) != -1
	    && ARCChangeDir(pArcHandle, resRootDir))
	{
		if (!resType)
		{
			entryNum = FindNameResource(pArcHandle, name);
		}
		else
		{
			char resTypeStr[5];
			resTypeStr[0] = resType >> 24;
			resTypeStr[1] = resType >> 16;
			resTypeStr[2] = resType >> 8;
			resTypeStr[3] = resType;
			resTypeStr[4] = '\0';

			if (ARCConvertPathToEntrynum(pArcHandle, resTypeStr) != -1
			    && ARCChangeDir(pArcHandle, resTypeStr))
			{
				entryNum = ARCConvertPathToEntrynum(pArcHandle, name);
				bool bSuccess ATTRIBUTE_UNUSED = ARCChangeDir(pArcHandle, "..");
			}
		}

		bool bSuccess ATTRIBUTE_UNUSED = ARCChangeDir(pArcHandle, "..");
	}

	if (entryNum != -1)
	{
		ARCFileInfo arcFileInfo;
		bool bSuccess ATTRIBUTE_UNUSED =
			ARCFastOpen(pArcHandle, entryNum, &arcFileInfo);

		void *resPtr = ARCGetStartAddrInMem(&arcFileInfo);

		if (pSize)
			*pSize = ARCGetLength(&arcFileInfo);

		ARCClose(&arcFileInfo);

		return resPtr;
	}

	return nullptr;
}

} // unnamed namespace

namespace nw4hbm { namespace lyt {

ut::Font *detail::FindFont(FontRefLink::LinkList *pFontRefList,
                           const char *name)
{
	NW4HBM_RANGE_FOR(it, *pFontRefList)
	{
		if (std::strcmp(name, it->GetFontName()) == 0)
			return it->GetFont();
	}

	return nullptr;
}

ArcResourceAccessor::ArcResourceAccessor():
	mArcBuf(nullptr)
{}

bool ArcResourceAccessor::Attach(void *archiveStart,
                                 const char *resourceRootDirectory)
{
	bool bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

	if (!bSuccess)
		return false;

	mArcBuf = archiveStart;
	std::strncpy(mResRootDir, resourceRootDirectory,
	             ARRAY_COUNT(mResRootDir) - 1);
	mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

	return true;
}

void *ArcResourceAccessor::GetResource(u32 resType, const char *name,
                                       u32 *pSize)
{
	return GetResourceSub(&mArcHandle, mResRootDir, resType, name, pSize);
}

/* [SPQE7T]/ISpyD.elf:.debug_info::0x3bd83a
 * nw4r version
 */
bool ArcResourceLink::Set(void *archiveStart, const char *resourceRootDirectory)
{
	bool bSuccess = ARCInitHandle(archiveStart, &mArcHandle);

	if (!bSuccess)
		return false;

	std::strncpy(mResRootDir, resourceRootDirectory,
	             ARRAY_COUNT(mResRootDir) - 1);
	mResRootDir[ARRAY_COUNT(mResRootDir) - 1] = '\0';

	return true;
}

ut::Font *ArcResourceAccessor::GetFont(const char *name)
{
	return detail::FindFont(&mFontList, name);
}

/* [SPQE7T]/ISpyD.elf:.debug_info::0x3bdb6b
 * nw4r version
 */
MultiArcResourceAccessor::MultiArcResourceAccessor() {}

/* [SPQE7T]/ISpyD.elf:.debug_info::0x3bdc4e
 * nw4r version
 */
MultiArcResourceAccessor::~MultiArcResourceAccessor()
{
	DetachAll();
}

/* [SPQE7T]/ISpyD.elf:.debug_info::0x3bdcc4
 * nw4r version
 */
void MultiArcResourceAccessor::Attach(ArcResourceLink *pLink)
{
	mArcList.PushBack(pLink);
}

/* [SPQE7T]/ISpyD.elf:.debug_info::0x3be484
 * nw4r version
 */
void *MultiArcResourceAccessor::GetResource(u32 resType, const char *name,
                                            u32 *pSize)
{
	NW4HBM_RANGE_FOR(it, mArcList)
	{
		ARCHandle *pArcHandle = it->GetArcHandle();

		if (void *resPtr = GetResourceSub(pArcHandle, it->GetResRootDir(),
		                                  resType, name, pSize))
		{
			return resPtr;
		}
	}

	return nullptr;
}

ut::Font *MultiArcResourceAccessor::GetFont(const char *name)
{
	return detail::FindFont(&mFontList, name);
}

}} // namespace nw4hbm::lyt
