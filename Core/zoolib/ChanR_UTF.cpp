// Copyright (c) 2014 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/ChanR_UTF.h"
#include "zoolib/Memory.h"
#include "zoolib/Unicode.h"

namespace ZooLib {

static const size_t kBufSize = sStackBufferSize;

using std::min;

// =================================================================================================
#pragma mark -

void sRead(const ChanR_UTF& iChanR,
	UTF32* oDest,
	size_t iCountCU, size_t* oCountCU, size_t iCountCP, size_t* oCountCP)
	{
	const size_t countRead = sRead(iChanR, oDest, std::min(iCountCU, iCountCP));

	if (oCountCU)
		*oCountCU = countRead;
	if (oCountCP)
		*oCountCP = countRead;
	}

void sRead(const ChanR_UTF& iChanR,
	UTF16* oDest,
	size_t iCountCU, size_t* oCountCU, size_t iCountCP, size_t* oCountCP)
	{
	ZAssert(iCountCU >= 2);

	UTF32 utf32Buffer[kBufSize];
	UTF16* localDest = oDest;
	size_t localCountCP = iCountCP;
	// If we've got space for two code units we are assured of
	// being able to read at least one code point.
	while (iCountCU >= 2 && localCountCP)
		{
		const size_t utf32Read =
			sRead(iChanR, utf32Buffer, min(kBufSize, min(localCountCP, iCountCU)));

		if (utf32Read == 0)
			break;

		size_t utf32Consumed;
		size_t utf16Generated;
		size_t cpGenerated;
		Unicode::sUTF32ToUTF16(
			utf32Buffer, utf32Read,
			&utf32Consumed, nullptr,
			localDest, iCountCU,
			&utf16Generated,
			localCountCP, &cpGenerated);

		ZAssert(utf32Consumed == utf32Read);

		localCountCP -= cpGenerated;
		iCountCU -= utf16Generated;
		localDest += utf16Generated;
		}
	if (oCountCP)
		*oCountCP = iCountCP - localCountCP;
	if (oCountCU)
		*oCountCU = localDest - oDest;
	}

void sRead(const ChanR_UTF& iChanR,
	UTF8* oDest,
	size_t iCountCU, size_t* oCountCU, size_t iCountCP, size_t* oCountCP)
	{
	ZAssert(iCountCU >= 6);

	UTF32 utf32Buffer[kBufSize];
	UTF8* localDest = oDest;
	size_t localCountCP = iCountCP;
	// If we've got space for six code units we are assured of
	// being able to read at least one code point.
	while (iCountCU >= 6 && localCountCP)
		{
		const size_t utf32Read =
			sRead(iChanR, utf32Buffer, min(kBufSize, min(localCountCP, iCountCU)));

		if (utf32Read == 0)
			break;

		size_t utf32Consumed;
		size_t utf8Generated;
		size_t cpGenerated;
		Unicode::sUTF32ToUTF8(
			utf32Buffer, utf32Read,
			&utf32Consumed, nullptr,
			localDest, iCountCU,
			&utf8Generated,
			localCountCP, &cpGenerated);

		ZAssert(utf32Consumed == utf32Read);

		localCountCP -= cpGenerated;
		iCountCU -= utf8Generated;
		localDest += utf8Generated;
		}
	if (oCountCP)
		*oCountCP = iCountCP - localCountCP;
	if (oCountCU)
		*oCountCU = localDest - oDest;
	}

// =================================================================================================
#pragma mark -

ZQ<string32> sQReadUTF32(const ChanR_UTF& iChanR, size_t iCountCP)
	{
	string32 result;
	size_t destGenerated = 0;
	while (iCountCP)
		{
		result.resize(destGenerated + iCountCP);
		UTF32* dest = sNonConst(result.data()) + destGenerated;
		const size_t cuRead = sRead(iChanR, dest, iCountCP);
		const size_t cpRead = Unicode::sCUToCP(dest, cuRead);

		if (cuRead == 0)
			return null;
		iCountCP -= cpRead;
		destGenerated += cuRead;
		}
	result.resize(destGenerated);
	return result;
	}

ZQ<string16> sQReadUTF16(const ChanR_UTF& iChanR, size_t iCountCP)
	{
	string16 result;
	size_t destGenerated = 0;
	while (iCountCP)
		{
		result.resize(destGenerated + iCountCP + 1);
		size_t cuRead, cpRead;
		sRead(iChanR, sNonConst(result.data()) + destGenerated,
			iCountCP + 1, &cuRead, iCountCP, &cpRead);

		if (cuRead == 0)
			return null;
		iCountCP -= cpRead;
		destGenerated += cuRead;
		}
	result.resize(destGenerated);
	return result;
	}

ZQ<string8> sQReadUTF8(const ChanR_UTF& iChanR, size_t iCountCP)
	{
	string8 result;
	size_t destGenerated = 0;
	while (iCountCP)
		{
		result.resize(destGenerated + iCountCP + 5);
		size_t cuRead, cpRead;
		sRead(iChanR, sNonConst(result.data()) + destGenerated,
			iCountCP + 5, &cuRead, iCountCP, &cpRead);

		if (cuRead == 0)
			return null;
		iCountCP -= cpRead;
		destGenerated += cuRead;
		}
	result.resize(destGenerated);
	return result;
	}

// -----

string32 sEReadUTF32(const ChanR_UTF& iChanR, size_t iCountCP)
	{
	if (const ZQ<string32> theQ = sQReadUTF32(iChanR, iCountCP))
		return *theQ;
	sThrow_ExhaustedR();
	}

string16 sEReadUTF16(const ChanR_UTF& iChanR, size_t iCountCP)
	{
	if (const ZQ<string16> theQ = sQReadUTF16(iChanR, iCountCP))
		return *theQ;
	sThrow_ExhaustedR();
	}

string8 sEReadUTF8(const ChanR_UTF& iChanR, size_t iCountCP)
	{
	if (const ZQ<string8> theQ = sQReadUTF8(iChanR, iCountCP))
		return *theQ;
	sThrow_ExhaustedR();
	}

// -----

string32 sReadAllUTF32(const ChanR_UTF& iChanR)
	{
	static const size_t kGrowthSize = 1024;
	string32 result;
	size_t destGenerated = 0;
	for (;;)
		{
		result.resize(destGenerated + kGrowthSize);
		size_t cuRead;
		sRead(iChanR, sNonConst(result.data()) + destGenerated,
			kGrowthSize, &cuRead, kGrowthSize, nullptr);

		if (cuRead == 0)
			break;
		destGenerated += cuRead;
		}
	result.resize(destGenerated);
	return result;
	}

string16 sReadAllUTF16(const ChanR_UTF& iChanR)
	{
	static const size_t kGrowthSize = 1024;
	string16 result;
	size_t destGenerated = 0;
	for (;;)
		{
		result.resize(destGenerated + kGrowthSize);
		size_t cuRead;
		sRead(iChanR, sNonConst(result.data()) + destGenerated,
			kGrowthSize, &cuRead, kGrowthSize, nullptr);

		if (cuRead == 0)
			break;
		destGenerated += cuRead;
		}
	result.resize(destGenerated);
	return result;
	}

string8 sReadAllUTF8(const ChanR_UTF& iChanR)
	{
	static const size_t kGrowthSize = 1024;
	string8 result;
	size_t destGenerated = 0;
	for (;;)
		{
		result.resize(destGenerated + kGrowthSize);
		size_t cuRead;
		sRead(iChanR, sNonConst(result.data()) + destGenerated,
			kGrowthSize, &cuRead, kGrowthSize, nullptr);

		if (cuRead == 0)
			break;
		destGenerated += cuRead;
		}
	result.resize(destGenerated);
	return result;
	}

// =================================================================================================
#pragma mark - ChanR_UTF_Native16

size_t ChanR_UTF_Native16::Read(UTF32* oDest, size_t iCountCU)
	{
	UTF16 utf16Buffer[kBufSize];
	size_t utf16Buffered = 0;

	UTF32* localDest = oDest;

	while (iCountCU)
		{
		size_t countCURead;
		this->ReadUTF16(utf16Buffer,
			kBufSize - utf16Buffered, &countCURead,
			iCountCU, nullptr);

		if (countCURead == 0)
			break;

		utf16Buffered += countCURead;

		size_t utf16Consumed;
		size_t utf32Generated;
		Unicode::sUTF16ToUTF32(
			utf16Buffer, countCURead,
			&utf16Consumed, nullptr,
			localDest, iCountCU,
			&utf32Generated);

		iCountCU -= utf32Generated;
		localDest += utf32Generated;

		if (utf16Consumed && utf16Buffered > utf16Consumed)
			{
			sMemMove(utf16Buffer, utf16Buffer + utf16Consumed, utf16Buffered - utf16Consumed);
			utf16Buffered -= utf16Consumed;
			}
		}
	return localDest - oDest;
	}

// =================================================================================================
#pragma mark - ChanR_UTF_Native8

size_t ChanR_UTF_Native8::Read(UTF32* oDest, size_t iCountCU)
	{
	UTF8 utf8Buffer[kBufSize];
	size_t utf8Buffered = 0;

	UTF32* localDest = oDest;

	while (iCountCU)
		{
		size_t countCURead;
		this->ReadUTF8(utf8Buffer,
			kBufSize - utf8Buffered, &countCURead,
			iCountCU, nullptr);

		if (countCURead == 0)
			break;

		utf8Buffered += countCURead;

		size_t utf8Consumed;
		size_t utf32Generated;
		Unicode::sUTF8ToUTF32(
			utf8Buffer, countCURead,
			&utf8Consumed, nullptr,
			localDest, iCountCU,
			&utf32Generated);

		iCountCU -= utf32Generated;
		localDest += utf32Generated;

		if (utf8Consumed && utf8Buffered > utf8Consumed)
			{
			sMemMove(utf8Buffer, utf8Buffer + utf8Consumed, utf8Buffered - utf8Consumed);
			utf8Buffered -= utf8Consumed;
			}
		}
	return localDest - oDest;
	}

} // namespace ZooLib
