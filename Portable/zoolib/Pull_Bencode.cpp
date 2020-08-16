// Copyright (c) 2018 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/Pull_Bencode.h"

#include "zoolib/ChanR_Bin.h"
#include "zoolib/ChanR_Bin_More.h"
#include "zoolib/Data_ZZ.h"
#include "zoolib/NameUniquifier.h" // For sName
#include "zoolib/ParseException.h"
#include "zoolib/Unicode.h"
#include "zoolib/Util_Chan.h"

#include "zoolib/ZMACRO_foreach.h"

namespace ZooLib {

using namespace PullPush;
using std::string;

// =================================================================================================
#pragma mark - Helpers

namespace { // anonymous

bool spTryRead_Byte(const ChanRU_Bin& iChanRU, uint8 iByte)
	{ return sTryRead<uint8>(iByte, iChanRU); }

bool spTryRead_Digit(const ChanRU_Bin& s, int& oDigit)
	{
	uint8 theByte;
	if (1 != sRead(s, &theByte, 1))
		return false;

	if (theByte >= '0' && theByte <= '9')
		{
		oDigit = theByte - '0';
		return true;
		}

	sUnread(s, theByte);
	return false;
	}

bool spTryRead_DecimalInteger(const ChanRU_Bin& s, int64& oInteger)
	{
	oInteger = 0;
	bool isValid = false;
	for (;;)
		{
		int curDigit;
		if (not spTryRead_Digit(s, curDigit))
			return isValid;
		isValid = true;
		oInteger *= 10;
		oInteger += curDigit;
		}
	}

bool spTryRead_SignedInteger(const ChanRU_Bin& s, int64& oInteger)
	{
	const bool isNegative = spTryRead_Byte(s, '-');

	if (not spTryRead_DecimalInteger(s, oInteger))
		return false;

	if (isNegative)
		oInteger = -oInteger;

	return true;
	}

int64 spRead_PositiveInteger(const ChanRU_Bin& s)
	{
	int64 result;
	if (not spTryRead_DecimalInteger(s, result))
		sThrow_ParseException("Expected unsigned length");
	return result;
	}

string spReadString(const ChanRU_Bin& s)
	{
	const size_t theLength = size_t(spRead_PositiveInteger(s));
	if (not spTryRead_Byte(s, ':'))
		sThrow_ParseException("Expected ':' terminator for string length");

	return sReadString(s, theLength);
	}

void spReadStringish(const ChanRU_Bin& s, const ChanW_PPT& iChanW)
	{
	const size_t theLength = size_t(spRead_PositiveInteger(s));
	if (not spTryRead_Byte(s, ':'))
		sThrow_ParseException("Expected ':' terminator for string/binary length");

	if (not theLength)
		sPush(null, iChanW);

	const string8 theString = sReadString(s, theLength);
	string::const_iterator current = theString.begin();
	string::const_iterator end = theString.end();
	size_t countSkipped = 0;
	UTF32 dummyCP;
	while (Unicode::sReadInc(current, end, dummyCP, countSkipped) && !countSkipped)
		{}

	if (countSkipped == 0)
		{
		// We skipped no code units, so theString is valid UTF8.
		sPush(theString, iChanW);
		}
	else
		{
		sPush(Data_ZZ(theString.data(), theLength), iChanW);
		}
	}

void spRead(const ChanRU_Bin& iChanRU, const ChanW_PPT& iChanW)
	{
	uint8 theType = sEReadBE<uint8>(iChanRU);
	switch (theType)
		{
		case 'd':
			{
			sPush_Start_Map(iChanW);
			for (;;)
				{
				if (spTryRead_Byte(iChanRU, 'e'))
					{
					sPush_End(iChanW);
					return;
					}
				string theName = spReadString(iChanRU);
				sPush(sName(theName), iChanW);

				spRead(iChanRU, iChanW);
				}
			}
		case 'l':
			{
			sPush_Start_Seq(iChanW);
			for (;;)
				{
				if (spTryRead_Byte(iChanRU, 'e'))
					{
					sPush_End(iChanW);
					return;
					}
				spRead(iChanRU, iChanW);
				}
			}
		case 'i':
			{
			int64 theInteger;
			if (not spTryRead_SignedInteger(iChanRU, theInteger))
				sThrow_ParseException("Expected signed decimal integer");
			if (not spTryRead_Byte(iChanRU, 'e'))
				sThrow_ParseException("Expected 'e' terminator for integer");
			sPush(theInteger, iChanW);
			return;;
			}
		default:
			{
			sUnread(iChanRU, theType);
			// It must be a 'string'.

			// It could be valid UTF-8, or could be
			// arbitrary bytes, so we call sReadStringish.
			spReadStringish(iChanRU, iChanW);
			}
		}
	}

} // anonymous namespace

// =================================================================================================
#pragma mark -

bool sPull_Bencode_Push_PPT(const ChanRU_Bin& iChanRU, const ChanW_PPT& iChanW)
	{
	if (ZQ<byte> theQ = sQReadBE<uint8>(iChanRU))
		{
		sUnread(iChanRU, *theQ);
		spRead(iChanRU, iChanW);
		return true;
		}
	return false;
	}

} // namespace ZooLib
