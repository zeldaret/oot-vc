#include "revolution/hbm/nw4hbm/db/DbgPrintBase.h>

/*******************************************************************************
 * headers
 */

#include "cstdarg.hpp"
#include "cstddef" // offsetof

#include "macros.h"
#include "revolution/types.h"

#include "revolution/hbm/nw4hbm/ut/Color.h>
#include "revolution/hbm/nw4hbm/ut/list.h>
#include "revolution/hbm/nw4hbm/ut/Font.h>
#include "revolution/hbm/nw4hbm/ut/TextWriterBase.h>

#include "revolution/GX/GXTransform.h"
#include "revolution/GX/GXEnum.h" // GX_ORTHOGRAPHIC
#include "revolution/MEM/mem_heapCommon.h"
#include "revolution/MEM/mem_expHeap.h"
#include "revolution/MTX/mtx.h"

#include "revolution/hbm/nw4hbm/NW4RAssert.h>

/*******************************************************************************
 * variables
 */

namespace nw4hbm { namespace db
{
	template <typename charT>
	MEMiHeapHead *DbgPrintBase<charT>::mHeapHandle;

	template <typename charT>
	DbgPrintBase<charT> *DbgPrintBase<charT>::mInstance;
}} // namespace nw4hbm::db

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace db {

template <typename charT>
DbgPrintBase<charT>::DbgPrintBase(ut::Color textColor, ut::Font const *font):
	mFont		(font),
	mTextColor	(textColor),
	mFontSize	(-1.0f), // ?
	mVisible	(true)
{
	ut::List_Init(&mTextList, offsetof(DbgText, link));
}

template <typename charT>
DbgPrintBase<charT>::~DbgPrintBase() {}

template <typename charT>
DbgPrintBase<charT> *DbgPrintBase<charT>::GetInstance()
{
	return mInstance;
}

template <typename charT>
void DbgPrintBase<charT>::Initialize(void *buffer, u32 size,
                                     ut::Color textColor)
{
	NW4RAssertPointerValid_Line(123, buffer);

	/* specifically not the source variant */
	NW4RAssertHeaderMinimumValue_Line(124, size, 1);

	SetBuffer(buffer, size);
	mInstance = new DbgPrintBase(textColor, nullptr);
}

template <typename charT>
void DbgPrintBase<charT>::Initialize(void *buffer, u32 size,
                                     ut::Font const &font, ut::Color textColor)
{
	NW4RAssertPointerValid_Line(150, buffer);

	/* specifically not the source variant */
	NW4RAssertHeaderMinimumValue_Line(151, size, 1);
	NW4RAssertPointerValid_Line(152, & font);

	SetBuffer(buffer, size);
	mInstance = new DbgPrintBase(textColor, &font);
}

template <typename charT>
void DbgPrintBase<charT>::SetFont(ut::Font const &font)
{
	NW4RAssertPointerValid_Line(177, this);
	NW4RAssertPointerValid_Line(178, & font);

	mFont = &font;
}

template <typename charT>
ut::Font const *DbgPrintBase<charT>::GetFont() const
{
	NW4RAssertPointerValid_Line(195, this);

	return mFont;
}

template <typename charT>
void DbgPrintBase<charT>::SetTextColor(ut::Color color)
{
	NW4RAssertPointerValid_Line(212, this);

	mTextColor = color;
}

template <typename charT>
f32 DbgPrintBase<charT>::GetFontSize() const
{
	NW4RAssertPointerValid_Line(229, this);

	return mFontSize;
}

template <typename charT>
void DbgPrintBase<charT>::SetFontSize(f32 size)
{
	NW4RAssertPointerValid_Line(246, this);

	mFontSize = size;
}

template <typename charT>
ut::Color DbgPrintBase<charT>::GetTextColor() const
{
	NW4RAssertPointerValid_Line(263, this);

	return mTextColor;
}

template <typename charT>
void DbgPrintBase<charT>::SetVisible(bool bVisible)
{
	NW4RAssertPointerValid_Line(280, this);

	mVisible = bVisible;
}

template <typename charT>
bool DbgPrintBase<charT>::IsVisible() const
{
	NW4RAssertPointerValid_Line(297, this);

	return mVisible;
}

template <typename charT>
void DbgPrintBase<charT>::Registerf(int x, int y, charT const *format, ...)
{
	NW4RAssertPointerValid_Line(325, format);

	std::va_list args;

	va_start(args, format);
	VRegisterf(x, y, 1, format, args);
	va_end(args);
}

template <typename charT>
void DbgPrintBase<charT>::Registerf(int x, int y, int time, charT const *format,
                                    ...)
{
	NW4RAssertPointerValid_Line(351, format);

	std::va_list args;

	va_start(args, format);
	VRegisterf(x, y, time, format, args);
	va_end(args);
}

template <typename charT>
void DbgPrintBase<charT>::VRegisterf(int x, int y, int time,
                                     charT const *format, std::va_list args)
{
	NW4RAssertPointerValid_Line(377, format);

	int alignment = time == 1 ? -4 : 4;
	u32 maxAvailable = MEMGetAllocatableSizeForExpHeap(mHeapHandle);
	u32 bufferSize = (maxAvailable - 28) / 2;
	charT *buffer;

	MEMSetGroupIDForExpHeap(mHeapHandle, 0);

	buffer = static_cast<charT *>(
		MEMAllocFromExpHeapEx(mHeapHandle, bufferSize, alignment));
	if (buffer)
	{
		int length = ut::TextWriterBase<charT>::VSNPrintf(buffer, bufferSize,
		                                                  format, args);

		Register(x, y, time, buffer, length);
		MEMFreeToExpHeap(mHeapHandle, buffer);
	}
}

template <typename charT>
void DbgPrintBase<charT>::Register(int x, int y, int time, const charT *string,
                                   int length)
{
	NW4RAssertPointerValid_Line(414, this);
	NW4RAssertPointerValid_Line(415, string);

	/* specifically not the source variant */
	NW4RAssertHeaderMinimumValue_Line(416, length, 0);

	NW4RAssert_Line(417, mHeapHandle != MEM_HEAP_INVALID_HANDLE);

	u32 size = static_cast<u32>(length + 24); // TODO: where is 24 from?
	int alignment = time == 1 ? 4 : -4;
	DbgText *dbgText;

	MEMSetGroupIDForExpHeap(mHeapHandle, 0);

	dbgText = static_cast<DbgText *>(
		MEMAllocFromExpHeapEx(mHeapHandle, size, alignment));
	if (dbgText)
	{
		dbgText->x = x;
		dbgText->y = y;
		dbgText->time = time;
		dbgText->length = length;
		memcpy(&dbgText->text, string, static_cast<u32>(length));

		ut::List_Append(&mTextList, dbgText);
	}
}

template <typename charT>
void DbgPrintBase<charT>::Unregister(DbgText *dbgText)
{
	NW4RAssertPointerValid_Line(452, this);
	NW4RAssertPointerValid_Line(453, dbgText);
	NW4RAssert_Line(454, mHeapHandle != MEM_HEAP_INVALID_HANDLE);

	ut::List_Remove(&mTextList, dbgText);
	MEMFreeToExpHeap(mHeapHandle, dbgText);
}

template <typename charT>
void DbgPrintBase<charT>::Reset()
{
	NW4RAssertPointerValid_Line(482, this);

	if (mHeapHandle)
	{
		DbgText *dbgText = GetFirstText();
		while (dbgText)
		{
			DbgText *next = GetNextText(dbgText);

			Unregister(dbgText);

			dbgText = next;
		}
	}

	ut::List_Init(&mTextList, offsetof(DbgText, link));
}

template <typename charT>
void DbgPrintBase<charT>::Flush()
{
	NW4RAssertPointerValid_Line(514, this);

	DbgText *dbgText;
	ut::TextWriterBase<charT> writer;

	ensure(mFont);

	writer.SetFont(*mFont);
	writer.SetTextColor(mTextColor);

	if (mFontSize >= 0.0f)
		writer.SetFontSize(mFontSize);

	writer.SetupGX();

	dbgText = GetFirstText();
	while (dbgText)
	{
		DbgText *next = GetNextText(dbgText);

		if (mVisible)
		{
			writer.SetCursor(static_cast<f32>(dbgText->x),
			                 static_cast<f32>(dbgText->y));

			writer.Print(dbgText->text, dbgText->length);
		}

		dbgText->time--;
		if (dbgText->time <= 0)
			Unregister(dbgText);

		dbgText = next;
	}
}

template <typename charT>
void DbgPrintBase<charT>::Flush(int x, int y, int w, int h)
{
	NW4RAssertPointerValid_Line(571, this);

	{ // 39d461 wants lexical_block
		Mtx_4x4 matrix;
		f32 near	= 0.0f;
		f32 far		= 1.0f;
		f32 left	= static_cast<f32>(x);
		f32 top		= static_cast<f32>(y);
		f32 right	= static_cast<f32>(x + w);
		f32 bottom	= static_cast<f32>(y + h);

		MTXOrtho(matrix, top, bottom, left, right, near, far);
		GXSetProjection(matrix, GX_ORTHOGRAPHIC);
	}

	{ // 39d4ff wants lexical_block
		Mtx_3x4 matrix;
		MTXIdentity(matrix);

		GXLoadPosMtxImm(matrix, 0);
		GXSetCurrentMtx(0);
	}

	Flush();
}

template <typename charT>
void DbgPrintBase<charT>::SetBuffer(void *buffer, u32 size)
{
	mHeapHandle = MEMCreateExpHeap(buffer, size);
}

template <typename charT>
void *DbgPrintBase<charT>::ReleaseBuffer()
{
	void *addr = nullptr;

	if (mHeapHandle)
	{
		delete mInstance;

		addr = MEMGetHeapStartAddress(mHeapHandle);

		MEMDestroyExpHeap(mHeapHandle);
		mHeapHandle = nullptr;
		mInstance = nullptr;
	}

	return addr;
}

template <typename charT>
void *DbgPrintBase<charT>::operator new(u32 size)
{
	NW4RAssert_Line(674, mHeapHandle != MEM_HEAP_INVALID_HANDLE);

	MEMSetGroupIDForExpHeap(mHeapHandle, 0);
	return MEMAllocFromExpHeap(mHeapHandle, size);
}

template <typename charT>
void DbgPrintBase<charT>::operator delete(void *ptr)
{
	MEMFreeToExpHeap(mHeapHandle, ptr);
}

template <typename charT>
void DbgPrintBase<charT>::Registf(int x, int y, const charT *format, ...)
{
	std::va_list args;

	va_start(args, format);
	VRegisterf(x, y, 1, format, args);
	va_end(args);
}

template <typename charT>
void DbgPrintBase<charT>::Registf(int x, int y, int time, const charT *format,
                                  ...)
{
	std::va_list args;

	va_start(args, format);
	VRegisterf(x, y, time, format, args);
	va_end(args);
}

template <typename charT>
void DbgPrintBase<charT>::VRegistf(int x, int y, int time, const charT *format,
                                   std::va_list args)
{
	VRegisterf(x, y, time, format, args);
}

template <typename charT>
void DbgPrintBase<charT>::Regist(int x, int y, int time, const charT *string,
                                 int length)
{
	Register(x, y, time, string, length);
}

}} // namespace nw4hbm::db

/*******************************************************************************
 * explicit template instantiations
 */

namespace nw4hbm { namespace db
{
	template class DbgPrintBase<char>;
	template class DbgPrintBase<wchar_t>;
}} // namespace nw4hbm::db
