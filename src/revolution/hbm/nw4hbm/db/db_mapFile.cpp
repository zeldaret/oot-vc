#include "revolution/hbm/nw4hbm/db/mapFile.hpp"

/*******************************************************************************
 * headers
 */

// #include "cstddef.hpp"

#include "macros.h"
#include "revolution/types.h"

// #include "revolution/os/__OSGlobals.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/os/OSLink.h"
#include "revolution/dvd/dvd.h"
#include "revolution/dvd/dvdfs.h"

#include "revolution/hbm/HBMAssert.hpp"

/*******************************************************************************
 * types
 */

typedef u8 GetCharFunc(u8 const *buf);

/*******************************************************************************
 * local function declarations
 */

namespace nw4hbm { namespace db
{
#if !defined(NDEBUG)
	static void StringForce__(void);
#endif // !defined(NDEBUG)

	static u8 GetCharOnMem_(const u8 *buf);
	static u8 GetCharOnDvd_(u8 const *buf);

	static u8 *SearchNextLine_(u8 *buf, s32 lines);
	static u8 *SearchNextSection_(u8 *buf);
	static u8 *SearchParam_(u8 *lineTop, u32 argNum, u8 splitter);

	static u32 XStrToU32_(u8 const *str);
	static u32 CopySymbol_(u8 const *buf, u8 *str, u32 strLenMax, u8 splitter);

	static bool QuerySymbolToMapFile_(u8 *buf, OSModuleInfo const *moduleInfo,
                                      u32 address, u8 *strBuf, u32 strBufSize);
	static bool QuerySymbolToSingleMapFile_(MapFile *pMapFile, u32 address,
	                                        u8 *strBuf, u32 strBufSize);
}} // namespace nw4hbm::db

/*******************************************************************************
 * variables
 */

namespace nw4hbm { namespace db
{
	static u8 sMapBuf[0x200];
	static s32 sMapBufOffset = -1;
	static DVDFileInfo sFileInfo;
	static u32 sFileLength;
	static MapFile *sMapFileList;
	static GetCharFunc *GetCharPtr_;
}} // namespace nw4hbm::db

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace db {

#if !defined(NDEBUG)
static void StringForce__(void)
{
	// whatever values
	MapFile *pMapFile = nullptr;
	void *buffer = nullptr;
	void *mapDataBuf = nullptr;
	void *filePath = nullptr;

	NW4RAssertPointerNonnull(pMapFile);
	NW4RAssert(sMapFileList->moduleInfo != NULL);
	NW4RAssertPointerNonnull(buffer);
	NW4RAssertPointerNonnull(mapDataBuf);
	NW4RAssertPointerNonnull(filePath);
	NW4RAssert(pMapFile->fileEntry >= 0);
}
#endif // !defined(NDEBUG)

bool MapFile_Exists(void)
{
	return BOOLIFY_TERNARY_TYPE(bool, sMapFileList);
}

static u8 GetCharOnMem_(u8 const *buf)
{
	return *buf;
}

static u8 GetCharOnDvd_(u8 const *buf)
{
	s32 address = (s32)(reinterpret_cast<u32>(buf) & ~0x80000000);
	s32 offset = address - sMapBufOffset;

	ensure((u32)address < sFileLength, false);

	if (sMapBufOffset < 0 || offset < 0 || (int)ARRAY_COUNT(sMapBuf) <= offset)
	{
		s32 len;
		s32 size = ARRAY_COUNT(sMapBuf);

		sMapBufOffset = ROUND_DOWN(address, 32);
		offset = address - sMapBufOffset;

		if ((u32)sMapBufOffset + ARRAY_COUNT(sMapBuf) >= sFileLength)
			size = (s32)ROUND_UP(sFileLength - (u32)sMapBufOffset, 32);

		int intrStatus = OSEnableInterrupts(); /* int enabled; */

		len = DVDReadAsyncPrio(&sFileInfo, sMapBuf, size, sMapBufOffset,
		                       nullptr, 2);

		while (DVDGetCommandBlockStatus(&sFileInfo.block))
			{ /* ... */ }

		OSRestoreInterrupts(intrStatus);

		ensure(len > 0, false);
	}

	return static_cast<u8>(sMapBuf[offset]);
}

static u8 *SearchNextLine_(u8 *buf, s32 lines)
{
	u8 c;

	NW4RAssertPointerNonnull_Line(363, GetCharPtr_);

	ensure(buf, nullptr);

	for (; (c = (*GetCharPtr_)(buf)) != '\0'; buf++)
	{
		if (c == '\n')
		{
			if (--lines <= 0)
				return buf + 1;
		}
	}

	return nullptr;
}

static u8 *SearchNextSection_(u8 *buf)
{
	NW4RAssertPointerNonnull_Line(399, GetCharPtr_);

	do
	{
		buf = SearchNextLine_(buf, 1);

		if (!buf)
			return nullptr;
	} while ((*GetCharPtr_)(buf) != '.');

	return buf;
}


static u8 *SearchParam_(u8 *lineTop, u32 argNum, u8 splitter)
{
	int inArg = 0;
	u8 *buf = lineTop;

	NW4RAssertPointerNonnull_Line(434, GetCharPtr_);

	ensure(buf, nullptr);

	while (true)
	{
		u8 c = (*GetCharPtr_)(buf);

		if (c == '\0' || c == '\n')
			return 0;

		if (inArg)
		{
			if (c == splitter)
				inArg = 0;
		}
		else if (c != splitter)
		{
			if (!argNum--)
				return buf;

			inArg = 1;
		}

		buf++;
	}

	return 0;
}

static u32 XStrToU32_(u8 const *str)
{
	u32 val = 0;

	NW4RAssertPointerNonnull_Line(488, str);
	NW4RAssertPointerNonnull_Line(489, GetCharPtr_);

	while (true)
	{
		u32 num;
		u8 c;

		c = (*GetCharPtr_)(str);

		if ('0' <= c && c <= '9')
			num = static_cast<u32>(c - '0');
		else if ('a' <= c && c <= 'z')
			num = static_cast<u32>(c - ('a' - 10)); // ?
		else if ('A' <= c && c <= 'Z')
			num = static_cast<u32>(c - ('A' - 10)); // What's the - 10 for
		else
			return val;

		if (val >= 0x10000000)
			return 0;

		val = num + (val << 4);
		str++;
	}

	return 0;
}

#if defined(NDEBUG)
inline
#endif // defined(NDEBUG)
static u32 CopySymbol_(const u8 *buf, u8 *str, u32 strLenMax, u8 splitter)
{
	u32 cnt = 0;

	NW4RAssertPointerNonnull_Line(546, buf);
	NW4RAssertPointerNonnull_Line(547, str);
	NW4RAssertPointerNonnull_Line(548, GetCharPtr_);

	while (true)
	{
		u8 c = (*GetCharPtr_)(buf++);

		if (c == splitter || c == '\0' || c == '\n')
		{
			*str = '\0';
			return cnt;
		}

		*str = c;
		str++;
		cnt++;

		if (cnt >= strLenMax - 1)
		{
			*str = '\0';
			return cnt;
		}
	}

	return 0;
}

static bool QuerySymbolToMapFile_(u8 *buf, OSModuleInfo const *moduleInfo,
                                  u32 address, u8 *strBuf, u32 strBufSize)
{
	OSSectionInfo *sectionInfo = nullptr;
	u32 sectionCnt;

	NW4RAssertPointerNonnull_Line(604, strBuf);
	NW4RAssert_Line(605, strBufSize > 0);

	if (moduleInfo)
	{
		sectionInfo =
			reinterpret_cast<OSSectionInfo *>(moduleInfo->sectionInfoOffset);
		sectionCnt = moduleInfo->numSections;
	}

	do
	{
		u32 offset = 0;

		buf = SearchNextSection_(buf);
		buf = SearchNextLine_(buf, 3);

		if (sectionInfo)
		{
			offset = sectionInfo->offset;

			if (address < offset)
				goto get_next_section_info;

			if (address >= offset + sectionInfo->size)
				goto get_next_section_info;
		}

		while (true)
		{
			u8 *param;
			u32 startAddr;
			u32 size;

			buf = SearchNextLine_(buf, 1);
			if (!buf)
				return false;

			param = SearchParam_(buf, 1, ' ');
			if (!param)
				break;

			size = XStrToU32_(param);
			param = SearchParam_(buf, 2, ' ');
			if (!param)
				break;

			startAddr = XStrToU32_(param);
			if (!startAddr)
				continue;

			startAddr = startAddr + offset;
			if (address < startAddr || startAddr + size <= address)
				continue;

			param = SearchParam_(buf, 5, ' ');
			if (!param)
			{
				*strBuf = '\0';
				return true;
			}

			if ((*GetCharPtr_)(param) == '.')
				continue;

			CopySymbol_(param, strBuf, strBufSize, ' ');
			return true;

		}

	get_next_section_info:
		if (sectionInfo)
		{
			if (!--sectionCnt)
				return false;

			sectionInfo++;
		}
	} while (true);

	return false;
}

static bool QuerySymbolToSingleMapFile_(MapFile *pMapFile, u32 address,
                                        u8 *strBuf, u32 strBufSize)
{
	NW4RAssertPointerNonnull_Line(725, pMapFile);
	NW4RAssertPointerNonnull_Line(726, strBuf);

	if (pMapFile->mapBuf)
	{
		GetCharPtr_ = &GetCharOnMem_;
		return QuerySymbolToMapFile_(pMapFile->mapBuf, pMapFile->moduleInfo,
		                             address, strBuf, strBufSize);
	}

	if (pMapFile->fileEntry >= 0)
	{
		u8 *buf = reinterpret_cast<u8 *>(OS_BOOT_INFO.diskID.game);
		bool ret;

		if (DVDFastOpen(pMapFile->fileEntry, &sFileInfo))
		{
			sFileLength = sFileInfo.size;
			GetCharPtr_ = &GetCharOnDvd_;
			ret = QuerySymbolToMapFile_(buf, pMapFile->moduleInfo,
			                            address, strBuf, strBufSize);

			DVDClose(&sFileInfo);
			return ret;
		}
	}

	*strBuf = '\0';
	return false;
}

bool MapFile_QuerySymbol(u32 address, u8 *strBuf, u32 strBufSize)
{
	MapFile *pMap;
	for (pMap = sMapFileList; pMap; pMap = pMap->next)
	{
		if (QuerySymbolToSingleMapFile_(pMap, address, strBuf, strBufSize))
			return true;
	}

	return false;
}

}} // namespace nw4hbm::db
