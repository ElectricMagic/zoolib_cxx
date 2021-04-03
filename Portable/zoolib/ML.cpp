// Copyright (c) 2004 Andrew Green and Learning in Motion, Inc.
// MIT License. http://www.zoolib.org

#include "zoolib/ML.h"

#include "zoolib/Chan_UTF_string.h"
#include "zoolib/Memory.h"
#include "zoolib/Stringf.h"
#include "zoolib/Unicode.h"
#include "zoolib/Util_Chan_UTF.h"
#include "zoolib/Util_STL.h"
#include "zoolib/Util_STL_vector.h"
#include "zoolib/Util_string.h"
#include "zoolib/Util_Time.h"

#include <stdio.h> // For sprintf

namespace ZooLib {
namespace ML {

#define kDebug_StrimW_ML 1

using std::min;
using std::pair;
using std::string;
using std::vector;

using namespace Util_Chan;
using namespace Util_STL;

// =================================================================================================
#pragma mark - Static helper functions

string sReadReference(const ChanRU_UTF& iChanRU, const ZP<Callable_Entity>& iCallable)
	{
	string result;

	if (sTryRead_CP(iChanRU, '#'))
		{
		// It's a character reference.
		int64 theInt;
		bool gotIt = false;
		if (sTryRead_CP(iChanRU, 'x') || sTryRead_CP(iChanRU, 'X'))
			gotIt = sTryRead_HexInteger(iChanRU, theInt);
		else
			gotIt = sTryRead_DecimalInteger(iChanRU, theInt);

		if (gotIt && sTryRead_CP(iChanRU, ';'))
			result += UTF32(theInt);
		}
	else
		{
		string theEntity;
		theEntity.reserve(8);
		for (;;)
			{
			if (NotQ<UTF32> theCPQ = sQRead(iChanRU))
				{
				theEntity.clear();
				break;
				}
			else if (*theCPQ == ';')
				{ break; }
			else if (Unicode::sIsWhitespace((*theCPQ)))
				{
				theEntity.clear();
				break;
				}
			else
				{
				theEntity += *theCPQ;
				}
			}

		if (not theEntity.empty())
			{
			if (iCallable)
				result = iCallable->Call(theEntity);
			else if (theEntity == "quot")
				result = "\"";
			else if (theEntity == "lt")
				result = "<";
			else if (theEntity == "gt")
				result = ">";
			else if (theEntity == "amp")
				result = "&";
			else if (theEntity == "apos")
				result = "'";
			else if (theEntity == "nbsp")
				result = "\xC2\xA0";
			}
		}

	return result;
	}

bool sReadIdentifier(const ChanRU_UTF& iChanRU, string& oText)
	{
	oText.resize(0);

	if (NotQ<UTF32> theCPQ = sQRead(iChanRU))
		{ return false; }
	else if (not Unicode::sIsAlpha(*theCPQ) && *theCPQ != '_' && *theCPQ != '?' && *theCPQ != '!')
		{
		sUnread(iChanRU, *theCPQ);
		return false;
		}
	else
		{
		oText += *theCPQ;

		for (;;)
			{
			if (NotQ<UTF32> theCPQ = sQRead(iChanRU))
				{
				break;
				}
			else if (not Unicode::sIsAlphaDigit(*theCPQ)
				&& *theCPQ != '_' && *theCPQ != '-' && *theCPQ != ':')
				{
				sUnread(iChanRU, *theCPQ);
				break;
				}
			else
				{
				oText += *theCPQ;
				}
			}
		}

	return true;
	}

static bool spReadUntil(const ChanRU_UTF& iChanRU, UTF32 iTerminator, string& oText)
	{ return sCopy_Until(iChanRU, iTerminator, ChanW_UTF_string8(&oText)); }

bool sReadAttributeName(const ChanRU_UTF& iChanRU, string& oName)
	{
	oName.resize(0);

	if (NotQ<UTF32> theCPQ = sQRead(iChanRU))
		{ return false; }
	else if (*theCPQ == '"')
		{ return spReadUntil(iChanRU, '"', oName); }
	else if (*theCPQ == '\'')
		{ return spReadUntil(iChanRU, '\'', oName); }
	else
		{
		if (not Unicode::sIsAlpha(*theCPQ) && *theCPQ != '_' && *theCPQ != '?' && *theCPQ != '!')
			{
			sUnread(iChanRU, *theCPQ);
			return false;
			}

		oName += *theCPQ;
		for (;;)
			{
			if (NotQ<UTF32> theCPQ = sQRead(iChanRU))
				{ return true; }
			else if (not Unicode::sIsAlphaDigit(*theCPQ)
				&& *theCPQ != '_' && *theCPQ != '-' && *theCPQ != ':')
				{
				sUnread(iChanRU, *theCPQ);
				return true;
				}
			else
				{ oName += *theCPQ; }
			}
		}
	}

bool sReadAttributeValue(
	const ChanRU_UTF& iChanRU,
	bool iRecognizeEntities, const ZP<Callable_Entity>& iCallable,
	string& oValue)
	{
	oValue.resize(0);

	if (NotQ<UTF32> theCPQ = sQRead(iChanRU))
		{ return false; }
	else if (*theCPQ == '"')
		{ return spReadUntil(iChanRU, '"', oValue); }
	else if (*theCPQ == '\'')
		{ return spReadUntil(iChanRU, '\'', oValue); }
	else
		{
		sUnread(iChanRU, *theCPQ);

		sSkip_WS(iChanRU);

		for (;;)
			{
			if (NotQ<UTF32> theCPQ = sQRead(iChanRU))
				{ break; }
			else if (*theCPQ == '>')
				{
				sUnread(iChanRU, *theCPQ);
				break;
				}
			else if (Unicode::sIsWhitespace(*theCPQ))
				{ break; }
			else if (*theCPQ == '&' && iRecognizeEntities)
				{ oValue += sReadReference(iChanRU, iCallable); }
			else
				{ oValue += *theCPQ; }
			}
		}

	return true;
	}

} // namespace ML
} // namespace ZooLib
