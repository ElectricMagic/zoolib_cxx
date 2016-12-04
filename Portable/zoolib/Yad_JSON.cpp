/* -------------------------------------------------------------------------------------------------
Copyright (c) 2009 Andrew Green
http://www.zoolib.org

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------------------------- */

#include "zoolib/Chan_UTF_Escaped.h"
#include "zoolib/ChanW_Bin_HexStrim.h"
#include "zoolib/Compat_algorithm.h" // ZSetRestore_T
#include "zoolib/Compat_cmath.h"
#include "zoolib/Unicode.h"
#include "zoolib/Util_Any.h"
#include "zoolib/Util_Chan_UTF.h"
#include "zoolib/Util_Chan_UTF_Operators.h"
#include "zoolib/Yad_JSON.h"

#include <vector>

#if ZCONFIG(Compiler,GCC)
	#include <cxxabi.h>
	#include <stdlib.h> // For free
#endif

namespace ZooLib {
namespace Yad_JSON {

using std::min;
using std::string;

/*! \namespace ZooLib::Yad_JSON
JSON is JavaScript Object Notation. See <http://www.crockford.com/JSON/index.html>.

Yad_JSON provides Yad facilities to read and write JSON source.

ZMap is isomorphic to JSON's object, ZSeq to JSON's array, and strings, booleans
and null translate back and forth without trouble. JSON's only other primitive is
the number, whereas ZVal explicitly stores and retrieves integers of different sizes,
floats and doubles, raw bytes and other composite types.

Yad_JSON writes all ZVal number types as JSON numbers. When reading, JSON numbers
are stored as int64s, unless the mantissa has a fractional component or exceeds 2^64,
or if there is an exponent,in which case a double is used.

The mappings are as follows:

<pre>

JSON          ZVal
----          -------
null          null
object        ZMap
array         ZSeq
boolean       bool
string        string
number        int64 or double


ZVal         JSON
----         ----
null         null
ZMap         object
ZSeq         array
bool         boolean
string       string
int8         number
int16        number
int32        number
int64        number
float        number
double       number
uint64 (ID)  number
time         number

all other types are written as nulls in maps and lists if the appropriate
preserve flag is passed, otherwise they are skipped.
raw              -
refcounted       -
rect             -
point            -
region           -
type             -
</pre>

*/

// =================================================================================================
#pragma mark -
#pragma mark Helpers

static string spPrettyName(const std::type_info& iTI)
	{
	#if ZCONFIG(Compiler,GCC)
	if (char* unmangled = abi::__cxa_demangle(iTI.name(), 0, 0, 0))
		{
		string result = unmangled;
		free(unmangled);
		return result;
		}
	#endif

	return iTI.name();
	}

static bool spRead_Identifier(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU,
	string* oStringLC, string* oStringExact)
	{
	if (oStringExact)
		oStringExact->reserve(32);

	if (oStringLC)
		oStringLC->reserve(32);

	bool gotAny = false;
	for (;;)
		{
		const ZQ<UTF32> theCPQ = sQRead(iChanR);
		if (not theCPQ)
			break;
		const UTF32 theCP = *theCPQ;
		if (not Unicode::sIsAlphaDigit(theCP) && theCP != '_')
			{
			sUnread(iChanU, theCP);
			break;
			}

		gotAny = true;

		if (oStringLC)
			*oStringLC += Unicode::sToLower(theCP);
		if (oStringExact)
			*oStringExact += theCP;
		}
	return gotAny;
	}

static bool spTryRead_JSONString(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU,
	string& oString)
	{
	using namespace Util_Chan;

	if (sTryRead_CP('"', iChanR, iChanU))
		{
		// We've got a string, delimited by ".
		for (;;)
			{
			string tempString;
			sRead_EscapedString('"', iChanR, iChanU, tempString);

			if (not sTryRead_CP('"', iChanR, iChanU))
				throw ParseException("Expected '\"' to close a string");

			oString += tempString;

			sSkip_WSAndCPlusPlusComments(iChanR, iChanU);

			if (not sTryRead_CP('"', iChanR, iChanU))
				return true;
			}
		}
	else if (sTryRead_CP('\'', iChanR, iChanU))
		{
		// We've got a string, delimited by '.
		for (;;)
			{
			string tempString;
			sRead_EscapedString( '\'', iChanR, iChanU, tempString);

			if (not sTryRead_CP('\'', iChanR, iChanU))
				throw ParseException("Expected \"'\" to close a string");

			oString += tempString;

			sSkip_WSAndCPlusPlusComments(iChanR, iChanU);

			if (not sTryRead_CP('\'', iChanR, iChanU))
				return true;
			}
		}

	return false;
	}

static bool spTryRead_PropertyName(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU,
	string& oName, bool iAllowUnquoted)
	{
	using namespace Util_Chan;

	if (sTryRead_EscapedString('"', iChanR, iChanU, oName))
		return true;

	if (sTryRead_EscapedString('\'', iChanR, iChanU, oName))
		return true;

	if (iAllowUnquoted && spRead_Identifier(iChanR, iChanU, nullptr, &oName))
		return true;

	return false;
	}

static ZQ<Any> spQFromChan_Val(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU)
	{
	using namespace Util_Chan;

	string theString;

	int64 asInt64;
	double asDouble;
	bool isDouble;

	if (spTryRead_JSONString(iChanR, iChanU, theString))
		{
		return theString;
		}
	else if (sTryRead_SignedGenericNumber(iChanR, iChanU, asInt64, asDouble, isDouble))
		{
		if (isDouble)
			return asDouble;
		else
			return asInt64;
		}
	else if (sTryRead_CaselessString("null", iChanR, iChanU))
		{
		return Any(null); //## Watch this
		}
	else if (sTryRead_CaselessString("false", iChanR, iChanU))
		{
		return false;
		}
	else if (sTryRead_CaselessString("true", iChanR, iChanU))
		{
		return true;
		}

//##	throw ParseException("Expected number, string or keyword");
	return null;
	}

static ZRef<YadR> spMakeYadR_JSON(const ZRef<CountedVal<ReadOptions> >& iRO,
	ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
	{
	using namespace Util_Chan;

	const ChanR_UTF& theChanR = sGetChan(iChannerR);
	const ChanU_UTF& theChanU = sGetChan(iChannerU);

	sSkip_WSAndCPlusPlusComments(theChanR, theChanU);

	if (sTryRead_CP('[', theChanR, theChanU))
		{
		return new YadSeqR_JSON(iRO, iChannerR, iChannerU);
		}
	else if (sTryRead_CP('{', theChanR, theChanU))
		{
		return new YadMapR_JSON(iRO, iChannerR, iChannerU);
		}
	else if (sTryRead_CP('"', theChanR, theChanU))
		{
		return new YadStrimmerR_JSON(iChannerR, iChannerU);
		}
	else if (iRO->Get().fAllowBinary.DGet(false) && sTryRead_CP('<', theChanR, theChanU))
		{
		sSkip_WSAndCPlusPlusComments(theChanR, theChanU);
		if (sTryRead_CP('=', theChanR, theChanU))
			{
			// It's Base64
			return new YadStreamerR_Base64(Base64::sDecode_Normal(), iChannerR, iChannerU);
			}
		else
			{
			// It's Hex
			return new YadStreamerR_Hex(iChannerR, iChannerU);
			}
		}
	else
		{
		if (ZQ<Any> theQ = spQFromChan_Val(theChanR, theChanU))
			return sMake_YadAtomR_Any(*theQ);
		}

	return null;
	}

// =================================================================================================
#pragma mark -
#pragma mark ReadOptions

ReadOptions sReadOptions_Extended()
	{
	ReadOptions theRO;
	theRO.fAllowUnquotedPropertyNames = true;
	theRO.fAllowEquals = true;
	theRO.fAllowSemiColons = true;
	theRO.fAllowTerminators = true;
	theRO.fLooseSeparators = true;
	theRO.fAllowBinary = true;

	return theRO;
	}

// =================================================================================================
#pragma mark -
#pragma mark WriteOptions

WriteOptions::WriteOptions()
:	YadOptions()
	{}

WriteOptions::WriteOptions(const YadOptions& iOther)
:	YadOptions(iOther)
	{}

WriteOptions::WriteOptions(const WriteOptions& iOther)
:	YadOptions(iOther)
,	fUseExtendedNotation(iOther.fUseExtendedNotation)
,	fBinaryAsBase64(iOther.fBinaryAsBase64)
,	fPreferSingleQuotes(iOther.fPreferSingleQuotes)
,	fNumberSequences(iOther.fNumberSequences)
	{}

// =================================================================================================
#pragma mark -
#pragma mark ParseException

ParseException::ParseException(const string& iWhat)
:	YadParseException_Std(iWhat)
	{}

ParseException::ParseException(const char* iWhat)
:	YadParseException_Std(iWhat)
	{}

// =================================================================================================
#pragma mark -
#pragma mark YadStreamerR_Hex

YadStreamerR_Hex::YadStreamerR_Hex(ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
:	ChanR_Bin_HexStrim(sGetChan(iChannerR), sGetChan(iChannerU))
,	fChannerR(iChannerR)
,	fChannerU(iChannerU)
	{}

void YadStreamerR_Hex::Finish()
	{
	using namespace Util_Chan;
	sSkipAll(*static_cast<ChanR_Bin_HexStrim*>(this));
	if (not sTryRead_CP('>', sGetChan(fChannerR), sGetChan(fChannerU)))
		throw ParseException("Expected '>' to close a binary data");
	}

// =================================================================================================
#pragma mark -
#pragma mark YadStreamerR_Base64

YadStreamerR_Base64::YadStreamerR_Base64(const Base64::Decode& iDecode,
	ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
:	fChannerR(iChannerR)
,	fChannerU(iChannerU)
,	fChanR_Bin_ASCIIStrim(sGetChan(iChannerR))
,	fChanR_Bin_Boundary('>', fChanR_Bin_ASCIIStrim)
,	fChanR(iDecode, fChanR_Bin_Boundary)
	{}

void YadStreamerR_Base64::Finish()
	{
	using namespace Util_Chan;
	sSkipAll(fChanR);
	if (not fChanR_Bin_Boundary.HitTerminator())
		throw ParseException("Expected '>' to close a base64 data");
	}

size_t YadStreamerR_Base64::QRead(byte* oDest, size_t iCount)
	{ return sQRead(fChanR, oDest, iCount); }

// =================================================================================================
#pragma mark -
#pragma mark YadStrimmerR

static const UTF32 spThreeQuotes[] = { '\"', '\"', '\"' };

YadStrimmerR_JSON::YadStrimmerR_JSON(ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
:	fChannerR(iChannerR)
,	fChannerU(iChannerU)
,	fChanR_Boundary(spThreeQuotes, 3, sGetChan(iChannerR))
,	fQuotesSeen(1) // We're initialized having seen a single quote.
	{}

void YadStrimmerR_JSON::Finish()
	{
	using namespace Util_Chan;
	sSkipAll(*this);
	if (fQuotesSeen)
		throw ParseException("Improperly closed string");
	}


size_t YadStrimmerR_JSON::QRead(UTF32* oDest, size_t iCount)
	{
	using namespace Util_Chan;

	const ChanR_UTF& theStrimR = sGetChan(fChannerR);
	const ChanU_UTF& theStrimU = sGetChan(fChannerU);

	UTF32* localDest = oDest;
	UTF32* const localDestEnd = oDest + iCount;
	bool exhausted = false;
	while (localDestEnd > localDest && not exhausted)
		{
		switch (fQuotesSeen)
			{
			case 0:
				{
				sSkip_WSAndCPlusPlusComments(theStrimR, theStrimU);

				if (sTryRead_CP('"', theStrimR, theStrimU))
					fQuotesSeen = 1;
				else
					exhausted = true;
				break;
				}
			case 1:
				{
				if (sTryRead_CP('"', theStrimR, theStrimU))
					{
					// We have two quotes in a row.
					fQuotesSeen = 2;
					}
				else
					{
					const size_t countRead = sQRead(ChanR_UTF_Escaped('"', theStrimR, theStrimU), localDest, localDestEnd - localDest);
					localDest += countRead;

					if (sTryRead_CP('"', theStrimR, theStrimU))
						fQuotesSeen = 0;
					else if (countRead == 0)
						throw ParseException("Expected \" to close a string");
					}
				break;
				}
			case 2:
				{
				if (sTryRead_CP('"', theStrimR, theStrimU))
					{
					// We have three quotes in a row.
					fQuotesSeen = 3;
					if (ZQ<UTF32> theCPQ = sQRead(theStrimR))
						{
						if (not Unicode::sIsEOL(*theCPQ))
							{
							// And the following character was *not* an EOL, so we'll treat it as
							// part of the string.
							sUnread(theStrimU, *theCPQ);
							}
						}
					}
				else
					{
					// We have two quotes in a row, followed by something
					// else, so we had an empty string segment.
					fQuotesSeen = 0;
					}
				break;
				}
			case 3:
				{
				// We've got three quotes in a row, and any trailing EOL
				// has been stripped.
				if (const size_t countRead = sQRead(fChanR_Boundary, localDest, localDestEnd - localDest))
					{
					localDest += countRead;
					}
				else if (not fChanR_Boundary.HitBoundary())
					{
					throw ParseException("Expected \"\"\" to close a string");
					}
				else
					{
					fChanR_Boundary.Reset();
					fQuotesSeen = 0;
					}
				break;
				}
			}
		}

	return localDest - oDest;
	}

// =================================================================================================
#pragma mark -
#pragma mark YadSeqR

YadSeqR_JSON::YadSeqR_JSON(const ZRef<CountedVal<ReadOptions> >& iRO,
	ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
:	fRO(iRO)
,	fChannerR(iChannerR)
,	fChannerU(iChannerU)
	{}

void YadSeqR_JSON::Imp_ReadInc(bool iIsFirst, ZRef<YadR>& oYadR)
	{
	using namespace Util_Chan;

	const ChanR_UTF& theChanR = sGetChan(fChannerR);
	const ChanU_UTF& theChanU = sGetChan(fChannerU);

	sSkip_WSAndCPlusPlusComments(theChanR, theChanU);

	if (sTryRead_CP(']', theChanR, theChanU))
		{
		// Reached end.
		return;
		}

	if (not iIsFirst)
		{
		// Must read a separator
		if (not sTryRead_CP(',', theChanR, theChanU))
			{
			if (not fRO->Get().fAllowSemiColons.DGet(false)
				|| not sTryRead_CP(';', theChanR, theChanU))
				{
				if (not fRO->Get().fLooseSeparators.DGet(false))
					throw ParseException("Require ',' to separate array elements");
				}
			}

		sSkip_WSAndCPlusPlusComments(theChanR, theChanU);

		if (fRO->Get().fAllowTerminators.DGet(false) && sTryRead_CP(']', theChanR, theChanU))
			{
			// The separator was actually a terminator, and we're done.
			return;
			}
		}

	if (not (oYadR = spMakeYadR_JSON(fRO, fChannerR, fChannerU)))
		throw ParseException("Expected a value");
	}

// =================================================================================================
#pragma mark -
#pragma mark YadMapR_JSON

YadMapR_JSON::YadMapR_JSON(const ZRef<CountedVal<ReadOptions> >& iRO,
	ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
:	fRO(iRO)
,	fChannerR(iChannerR)
,	fChannerU(iChannerU)
	{}

void YadMapR_JSON::Imp_ReadInc(bool iIsFirst, Name& oName, ZRef<YadR>& oYadR)
	{
	using namespace Util_Chan;

	const ChanR_UTF& theChanR = sGetChan(fChannerR);
	const ChanU_UTF& theChanU = sGetChan(fChannerU);

	sSkip_WSAndCPlusPlusComments(theChanR, theChanU);

	if (sTryRead_CP('}', theChanR, theChanU))
		{
		// Reached end, with no terminator
		return;
		}

	if (not iIsFirst)
		{
		// Must read a separator
		if (not sTryRead_CP(',', theChanR, theChanU))
			{
			if (not fRO->Get().fAllowSemiColons.DGet(false)
				|| not sTryRead_CP(';', theChanR, theChanU))
				{
				if (not fRO->Get().fLooseSeparators.DGet(false))
					throw ParseException("Require ',' to separate object elements");
				}
			}

		sSkip_WSAndCPlusPlusComments(theChanR, theChanU);

		if (fRO->Get().fAllowTerminators.DGet(false) && sTryRead_CP('}', theChanR, theChanU))
			{
			// The separator was actually a terminator, and we're done.
			return;
			}
		}

	string theName;
	if (not spTryRead_PropertyName(theChanR, theChanU,
		theName, fRO->Get().fAllowUnquotedPropertyNames.DGet(false)))
		{ throw ParseException("Expected a member name"); }
	oName = theName;

	sSkip_WSAndCPlusPlusComments(theChanR, theChanU);

	if (not sTryRead_CP(':', theChanR, theChanU))
		{
		if (not fRO->Get().fAllowEquals.DGet(false) || not sTryRead_CP('=', theChanR, theChanU))
			throw ParseException("Expected ':' after a member name");
		}

	if (not (oYadR = spMakeYadR_JSON(fRO, fChannerR, fChannerU)))
		throw ParseException("Expected value after ':'");
	}

// =================================================================================================
#pragma mark -
#pragma mark Static writing functions

static void spWriteIndent(size_t iCount, const WriteOptions& iOptions, const ChanW_UTF& iChanW)
	{
	while (iCount--)
		iChanW << iOptions.fIndentString;
	}

static void spWriteLFIndent(size_t iCount, const WriteOptions& iOptions, const ChanW_UTF& iChanW)
	{
	iChanW << iOptions.fEOLString;
	spWriteIndent(iCount, iOptions, iChanW);
	}

static void spWriteString(const string& iString, bool iPreferSingleQuotes, const ChanW_UTF& iChanW)
	{
	ChanW_UTF_Escaped::Options theOptions;
	theOptions.fEscapeHighUnicode = false;

	if (iPreferSingleQuotes)
		{
		iChanW << "'";

		theOptions.fQuoteQuotes = false;

		ChanW_UTF_Escaped(theOptions, iChanW) << iString;

		iChanW << "'";
		}
	else
		{
		iChanW << "\"";

		theOptions.fQuoteQuotes = true;

		ChanW_UTF_Escaped(theOptions, iChanW) << iString;

		iChanW << "\"";
		}
	}

static void spWriteString(const ChanR_UTF& iChanR, const ChanW_UTF& iChanW)
	{
	iChanW << "\"";

	ChanW_UTF_Escaped::Options theOptions;
	theOptions.fQuoteQuotes = true;
	theOptions.fEscapeHighUnicode = false;

	ChanW_UTF_Escaped(theOptions, iChanW) << iChanR;

	iChanW << "\"";
	}

static bool spContainsProblemChars(const string& iString)
	{
	if (iString.empty())
		{
		// An empty string can't be distinguished from no string at all, so
		// we treat it as if it has problem chars, so it will be wrapped in quotes.
		return true;
		}

	for (string::const_iterator ii = iString.begin(), end = iString.end();;)
		{
		UTF32 theCP;
		if (not Unicode::sReadInc(ii, end, theCP))
			break;
		if (not Unicode::sIsAlphaDigit(theCP) && '_' != theCP)
			return true;
		}

	return false;
	}

static void spWritePropName(const string& iString, bool iUseSingleQuotes, const ChanW_UTF& w)
	{
	if (spContainsProblemChars(iString))
		spWriteString(iString, iUseSingleQuotes, w);
	else
		w << iString;
	}

static void spToStrim_SimpleValue(const Any& iAny, const WriteOptions& iOptions, const ChanW_UTF& w)
	{
	if (false)
		{}
	else if (iAny.IsNull())
		{
		w << "null";
		}
	else if (const bool* theValue = iAny.PGet<bool>())
		{
		if (*theValue)
			w << "true";
		else
			w << "false";
		}
	else if (ZQ<int64> theQ = sQCoerceInt(iAny))
		{
		if (iOptions.fUseExtendedNotation.DGet(false) and (*theQ >= 1000000 || *theQ <= -1000000))
			sEWritef(w, "0x%016llX", (unsigned long long)*theQ);
		else
			w << *theQ;
		}
	else if (const float* asFloat = iAny.PGet<float>())
		{
		Util_Chan::sWriteExact(w, *asFloat);
		}
	else if (const double* asDouble = iAny.PGet<double>())
		{
		Util_Chan::sWriteExact(w, *asDouble);
		if (not fmod(*asDouble, 1.0))
			{
			// There's no fractional part, so we'll have written a sequence of digits with no
			// decimal point. So the number would likely be subsequently interpreted as an
			// integer, so we append a ".0".
			w << ".0";
			}
		}
//	else if (const ZTime* asTime = iAny.PGet<ZTime>())
//		{
//		Util_Chan::sWriteExact(asTime->fVal, w);
//		}
	else
		{
		w << "NULL" << " /*!! Unhandled: " << spPrettyName(iAny.Type()) << " !!*/";
		}
	}

static void spToStrim_Stream(const ChanR_Bin& iChanR,
	size_t iLevel, const WriteOptions& iOptions, bool iMayNeedInitialLF,
	const ChanW_UTF& w)
	{
	string chunkSeparator;
	size_t chunkSize = 0;
	if (iOptions.DoIndentation())
		{
		chunkSeparator = iOptions.fEOLString;
		for (size_t xx = 0; xx < iLevel; ++xx)
			chunkSeparator += iOptions.fIndentString;

		chunkSize = iOptions.fRawChunkSize;
		}

	if (iOptions.fBinaryAsBase64.DGet(false))
		{
		w << "<";
		w << "=";

		sCopyAll(iChanR,
			ChanW_Bin_Base64Encode(
				Base64::sEncode_Normal(),
				ChanW_Bin_ASCIIStrim(ChanW_UTF_InsertSeparator(chunkSize * 3, chunkSeparator, w))
				)
			);

		w << ">";
		}
	else if (iOptions.fRawAsASCII && chunkSize)
		{
		w << "<";
		std::vector<char> buffer(chunkSize, 0);
		for (;;)
			{
			const size_t countRead = sQReadMem(iChanR, &buffer[0], chunkSize);
			if (not countRead)
				break;
			const size_t countCopied = sQWriteMemFully(
				ChanW_Bin_HexStrim(iOptions.fRawByteSeparator, "", 0, w),
				&buffer[0], countRead);

			if (size_t extraSpaces = chunkSize - countCopied)
				{
				// We didn't write a complete line of bytes, so pad it out.
				while (extraSpaces--)
					{
					// Two spaces for the two nibbles
					w << "  ";
					// And then the separator sequence
					w << iOptions.fRawByteSeparator;
					}
				}

			w << " // ";
			for (size_t xx = 0; xx < countCopied; ++xx)
				{
				char theChar = buffer[xx];
				if (theChar < 0x20 || theChar > 0x7E)
					w << ".";
				else
					w << theChar;
				}
			spWriteLFIndent(iLevel, iOptions, w);
			}
		w << ">";
		}
	else
		{
		w << "<";

		sCopyAll(iChanR,
			ChanW_Bin_HexStrim(iOptions.fRawByteSeparator, chunkSeparator, chunkSize, w));

		w << ">";

		}
	}

// =================================================================================================
#pragma mark -
#pragma mark Visitor_Writer

Visitor_Writer::Visitor_Writer(
	size_t iIndent, const WriteOptions& iOptions, const ChanW_UTF& iChanW)
:	fIndent(iIndent)
,	fOptions(iOptions)
,	fChanW(iChanW)
,	fMayNeedInitialLF(false)
	{}

void Visitor_Writer::Visit_YadR(const ZRef<YadR>& iYadR)
	{
	fChanW << "NULL";
	if (fOptions.fBreakStrings)
		{
		fChanW << " /*!! Unhandled yad: "
			<< spPrettyName(typeid(*iYadR.Get()))
			<< " !!*/";
		}
	}

void Visitor_Writer::Visit_YadAtomR(const ZRef<YadAtomR>& iYadAtomR)
	{ spToStrim_SimpleValue(iYadAtomR->AsAny(), fOptions, fChanW); }

void Visitor_Writer::Visit_YadStreamerR(const ZRef<YadStreamerR>& iYadStreamerR)
	{
	spToStrim_Stream(sGetChan<ChanR_Bin>(iYadStreamerR),
		fIndent, fOptions, fMayNeedInitialLF, fChanW);
	}

void Visitor_Writer::Visit_YadStrimmerR(const ZRef<ZooLib::YadStrimmerR>& iYadStrimmerR)
	{ spWriteString(sGetChan<ChanR_UTF>(iYadStrimmerR), fChanW); }

void Visitor_Writer::Visit_YadSeqR(const ZRef<ZooLib::YadSeqR>& iYadSeqR)
	{
	bool needsIndentation = false;
	if (fOptions.DoIndentation())
		{
		// We're supposed to be indenting if we're complex, ie if any element is:
		// 1. A non-empty vector.
		// 2. A non-empty tuple.
		// or if iOptions.fBreakStrings is true, any element is a string with embedded
		// line breaks or more than iOptions.fStringLineLength characters.
		needsIndentation = not iYadSeqR->IsSimple(fOptions);
		}

	if (needsIndentation)
		{
		// We need to indent.
		if (fMayNeedInitialLF)
			{
			// We were invoked by a tuple which has already issued the property
			// name and equals sign, so we need to start a fresh line.
			spWriteLFIndent(fIndent, fOptions, fChanW);
			}

		SaveSetRestore<bool> ssr_MayNeedInitialLF(fMayNeedInitialLF, false);

		uint64 count = 0;
		fChanW << "[";
		for (bool isFirst = true; /*no test*/ ; isFirst = false)
			{
			if (ZRef<YadR,false> cur = iYadSeqR->ReadInc())
				{
				break;
				}
			else if (false && fOptions.fUseExtendedNotation.DGet(false))
				{
				spWriteLFIndent(fIndent, fOptions, fChanW);
				cur->Accept(*this);
				fChanW << ";";
				}
			else
				{
				if (not isFirst)
					fChanW << ",";
				spWriteLFIndent(fIndent, fOptions, fChanW);
				if (fOptions.fNumberSequences.DGet(false))
					fChanW << "/*" << count << "*/";
				cur->Accept(*this);
				}
			++count;
			}
		spWriteLFIndent(fIndent, fOptions, fChanW);
		fChanW << "]";
		}
	else
		{
		// We're not indenting, so we can just dump everything out on
		// one line, with just some spaces to keep things legible.
		fChanW << "[";
		for (bool isFirst = true; /*no test*/ ; isFirst = false)
			{
			if (ZRef<YadR,false> cur = iYadSeqR->ReadInc())
				{
				break;
				}
			else if (false && fOptions.fUseExtendedNotation.DGet(false))
				{
				if (not isFirst && fOptions.fBreakStrings)
					fChanW << " ";
				cur->Accept(*this);
				fChanW << ";";
				}
			else
				{
				if (not isFirst)
					{
					fChanW << ",";
					if (fOptions.fBreakStrings)
						fChanW << " ";
					}
				cur->Accept(*this);
				}
			}
		fChanW << "]";
		}
	}

void Visitor_Writer::Visit_YadMapR(const ZRef<ZooLib::YadMapR>& iYadMapR)
	{
	bool needsIndentation = false;
	if (fOptions.DoIndentation())
		{
		needsIndentation = not iYadMapR->IsSimple(fOptions);
		}

	const bool useSingleQuotes = fOptions.fPreferSingleQuotes.DGet(false);

	if (needsIndentation)
		{
		if (fMayNeedInitialLF)
			{
			// We're going to be indenting, but need to start
			// a fresh line to have our { and contents line up.
			spWriteLFIndent(fIndent, fOptions, fChanW);
			}

		fChanW << "{";
		for (bool isFirst = true; /*no test*/ ; isFirst = false)
			{
			Name curName;
			if (ZRef<YadR,false> cur = iYadMapR->ReadInc(curName))
				{
				break;
				}
			else if (fOptions.fUseExtendedNotation.DGet(false))
				{
				spWriteLFIndent(fIndent, fOptions, fChanW);
				spWritePropName(curName, useSingleQuotes, fChanW);
				fChanW << " = ";

				SaveSetRestore<size_t> ssr_Indent(fIndent, fIndent + 1);
				SaveSetRestore<bool> ssr_MayNeedInitialLF(fMayNeedInitialLF, true);
				cur->Accept(*this);
				fChanW << ";";
				}
			else
				{
				if (not isFirst)
					fChanW << ",";
				spWriteLFIndent(fIndent, fOptions, fChanW);
				spWriteString(curName, useSingleQuotes, fChanW);
				fChanW << ": ";

				SaveSetRestore<size_t> ssr_Indent(fIndent, fIndent + 1);
				SaveSetRestore<bool> ssr_MayNeedInitialLF(fMayNeedInitialLF, true);
				cur->Accept(*this);
				}
			}
		spWriteLFIndent(fIndent, fOptions, fChanW);
		fChanW << "}";
		}
	else
		{
		fChanW << "{";
		bool wroteAny = false;
		for (bool isFirst = true; /*no test*/ ; isFirst = false)
			{
			Name curName;
			if (ZRef<YadR,false> cur = iYadMapR->ReadInc(curName))
				{
				break;
				}
			else if (fOptions.fUseExtendedNotation.DGet(false))
				{
				if (not isFirst && fOptions.fBreakStrings)
					fChanW << " ";

				spWritePropName(curName, useSingleQuotes, fChanW);
				if (fOptions.fBreakStrings)
					fChanW << " = ";
				else
					fChanW << "=";

				SaveSetRestore<size_t> ssr_Indent(fIndent, fIndent + 1);
				SaveSetRestore<bool> ssr_MayNeedInitialLF(fMayNeedInitialLF, true);
				cur->Accept(*this);
				fChanW << ";";
				}
			else
				{
				if (not isFirst)
					fChanW << ",";
				if (fOptions.fBreakStrings)
					fChanW << " ";
				spWriteString(curName, useSingleQuotes, fChanW);
				fChanW << ":";
				if (fOptions.fBreakStrings)
					fChanW << " ";

				SaveSetRestore<size_t> ssr_Indent(fIndent, fIndent + 1);
				SaveSetRestore<bool> ssr_MayNeedInitialLF(fMayNeedInitialLF, true);
				cur->Accept(*this);
				}
			wroteAny = true;
			}
		if (wroteAny && fOptions.fBreakStrings)
			fChanW << " ";
		fChanW << "}";
		}
	}

// =================================================================================================
#pragma mark -
#pragma mark sYadR and sToChan

ZRef<YadR> sYadR(ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
	{
	if (iChannerR && iChannerU)
		return spMakeYadR_JSON(sCountedVal<ReadOptions>(), iChannerR, iChannerU);
	return null;
	}

ZRef<YadR> sYadR(const ReadOptions& iReadOptions,
	ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU)
	{
	if (iChannerR && iChannerU)
		return spMakeYadR_JSON(sCountedVal(iReadOptions), iChannerR, iChannerU);
	return null;
	}

void sToChan(ZRef<YadR> iYadR, const ChanW_UTF& w)
	{ sToChan(0, WriteOptions(), iYadR, w); }

void sToChan(size_t iInitialIndent, const WriteOptions& iOptions,
	ZRef<YadR> iYadR, const ChanW_UTF& w)
	{
	if (iYadR)
		iYadR->Accept(Visitor_Writer(iInitialIndent, iOptions, w));
	}

ZQ<string8> sQRead_PropName(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU)
	{
	string8 theString8;
	if (not spTryRead_PropertyName(iChanR, iChanU, theString8, true))
		return null;
	return theString8;
	}

void sWrite_PropName(const string& iPropName, const ChanW_UTF& w)
	{
	if (spContainsProblemChars(iPropName))
		{
		w << "\"";
		ChanW_UTF_Escaped(w) << iPropName;
		w << "\"";
		}
	else
		{
		w << iPropName;
		}
	}

} // namespace Yad_JSON
} // namespace ZooLib
