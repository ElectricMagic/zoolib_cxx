// Copyright (c) 2006 Andrew Green and Learning in Motion, Inc.
// MIT License. http://www.zoolib.org

#include "zoolib/Chan_Bin_ASCIIStrim.h"

#include "zoolib/Util_Chan_UTF.h"

namespace ZooLib {

// =================================================================================================
#pragma mark - ChanR_Bin_ASCIIStrim

ChanR_Bin_ASCIIStrim::ChanR_Bin_ASCIIStrim(const ChanR_UTF& iChanR)
:	fChanR_UTF(iChanR)
	{}

size_t ChanR_Bin_ASCIIStrim::Read(byte* oDest, size_t iCount)
	{
	UTF8* localDest = reinterpret_cast<UTF8*>(oDest);

	if (iCount < 6)
		{
		// When reading UTF8 we must be prepared to read up to six code units.
		UTF8 buffer[6];
		while (iCount)
			{
			size_t countCURead;
			size_t countCPRead;
			sRead(fChanR_UTF, buffer, 6, &countCURead, iCount, &countCPRead);
			if (countCURead == 0)
				break;

			// Transcribe only ASCII code units/points.
			for (UTF8* readFrom = &buffer[0], *destEnd = &buffer[countCURead];
				readFrom < destEnd; ++readFrom)
				{
				if (*readFrom & 0x80)
					continue;
				*localDest++ = *readFrom;
				--iCount;
				}
			}
		}
	else
		{
		while (iCount)
			{
			// Top up our buffer with UTF8 code points.
			size_t countRead;
			sRead(fChanR_UTF, localDest, iCount, &countRead, iCount, nullptr);
			if (countRead == 0)
				break;

			// Scan till we find a non-ASCII code point (if any).
			for (UTF8* readFrom = localDest, *destEnd = localDest + countRead;
				readFrom < destEnd; ++readFrom)
				{
				if (*readFrom & 0x80)
					{
					// We found a problem, so start transcribing only those bytes that are okay.
					for (UTF8* writeTo = readFrom; readFrom < destEnd; ++readFrom)
						{
						if (not (*readFrom & 0x80))
							*writeTo++ = *readFrom;
						}
					break;
					}
				}
			localDest += countRead;
			iCount -= countRead;
			}
		}

	return localDest - reinterpret_cast<UTF8*>(oDest);
	}

// =================================================================================================
#pragma mark - ChanW_Bin_ASCIIStrim

ChanW_Bin_ASCIIStrim::ChanW_Bin_ASCIIStrim(const ChanW_UTF& iChanW)
:	fChanW_UTF(iChanW)
	{}

size_t ChanW_Bin_ASCIIStrim::Write(const byte* iSource, size_t iCount)
	{
	const byte* localSource = iSource;
	while (iCount--)
		{
		UTF32 current = *localSource++;
		if (int32(current) >= 0 && current <= 127)
			{
			if (not sQWrite(fChanW_UTF, current))
				break;
			}
		}
	return localSource - iSource;
	}

} // namespace ZooLib
