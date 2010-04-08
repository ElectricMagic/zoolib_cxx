/* -------------------------------------------------------------------------------------------------
Copyright (c) 2008 Andrew Green
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

#include "zoolib/ZYad_Std.h"
#include "zoolib/ZYad_ZooLib.h"

NAMESPACE_ZOOLIB_BEGIN

using std::pair;
using std::string;
using std::vector;

// =================================================================================================
#pragma mark -
#pragma mark * Helper functions

static bool spIsSimpleString(const ZYadOptions& iOptions, const string& iString)
	{
	if (iOptions.fStringLineLength && iString.size() > iOptions.fStringLineLength)
		{
		// We have a non-zero desired line length, and the string contains
		// more characters than that. This doesn't account for increases in width
		// due to escaping etc.
		return false;
		}

	if (iOptions.fBreakStrings)
		{
		// We've been asked to break strings at line ending characters,
		// which means (here and in ZStrimW_Escaped) LF and CR. Strictly
		// speaking we should use ZUnicode::sIsEOL().
		if (string::npos != iString.find_first_of("\n\r"))
			return false;
		}

	return true;
	}

static bool spIsSimple(const ZYadOptions& iOptions, const ZVal_ZooLib& iVal)
	{
	switch (iVal.TypeOf())
		{
		case eZType_Raw:
			{
			return iVal.GetRaw().GetSize() <= iOptions.fRawChunkSize;
			}
		case eZType_Vector:
			{
			const ZSeq_ZooLib& theSeq = iVal.GetSeq();
			if (!theSeq.Count())
				return true;

			if (theSeq.Count() == 1)
				return spIsSimple(iOptions, theSeq.Get(0));

			return false;
			}
		case eZType_Tuple:
			{
			const ZMap_ZooLib& theMap = iVal.GetMap();
			if (theMap.Empty())
				return true;

			if (theMap.Count() == 1)
				return spIsSimple(iOptions, *theMap.PGet(theMap.Begin()));

			return false;
			}
		case eZType_String:
			{
			return spIsSimpleString(iOptions, iVal.GetString());
			}
		default:
			{
			return true;
			}
		}
	}

// =================================================================================================
#pragma mark -
#pragma mark * ZYadPrimR_ZooLib

ZYadPrimR_ZooLib::ZYadPrimR_ZooLib()
	{}

ZYadPrimR_ZooLib::ZYadPrimR_ZooLib(const ZVal_ZooLib& iVal)
:	ZYadR_ZooLib(iVal)
	{}

ZYadPrimR_ZooLib::ZYadPrimR_ZooLib(ZType iType, const ZStreamR& iStreamR)
:	ZYadR_ZooLib(ZVal_ZooLib(iType, iStreamR))
	{}

bool ZYadPrimR_ZooLib::IsSimple(const ZYadOptions& iOptions)
	{ return spIsSimple(iOptions, fVal); }

ZAny ZYadPrimR_ZooLib::AsAny()
	{ return fVal.AsAny(); }

// =================================================================================================
#pragma mark -
#pragma mark * ZYadSeqRPos_ZooLib

ZYadSeqRPos_ZooLib::ZYadSeqRPos_ZooLib(const ZSeq_ZooLib& iSeq)
:	ZYadR_ZooLib(iSeq)
,	YadSeqBase_t(iSeq)
	{}

ZYadSeqRPos_ZooLib::ZYadSeqRPos_ZooLib(const ZSeq_ZooLib& iSeq, uint64 iPosition)
:	ZYadR_ZooLib(iSeq)
,	YadSeqBase_t(iSeq, iPosition)
	{}

bool ZYadSeqRPos_ZooLib::IsSimple(const ZYadOptions& iOptions)
	{ return spIsSimple(iOptions, fVal); }

// =================================================================================================
#pragma mark -
#pragma mark * ZYadMapRPos_ZooLib

ZYadMapRPos_ZooLib::ZYadMapRPos_ZooLib(const ZMap_ZooLib& iMap)
:	ZYadR_ZooLib(iMap)
,	YadMapBase_t(iMap)
	{}

ZYadMapRPos_ZooLib::ZYadMapRPos_ZooLib(const ZMap_ZooLib& iMap, const Index_t& iIndex)
:	ZYadR_ZooLib(iMap)
,	YadMapBase_t(iMap, iIndex)
	{}

bool ZYadMapRPos_ZooLib::IsSimple(const ZYadOptions& iOptions)
	{ return spIsSimple(iOptions, fVal); }

// =================================================================================================
#pragma mark -
#pragma mark * sMakeYadR

ZRef<ZYadR> sMakeYadR(const ZVal_ZooLib& iVal)
	{
	switch (iVal.TypeOf())
		{
		case eZType_Vector: return sMakeYadR(iVal.GetSeq());
		case eZType_Tuple: return sMakeYadR(iVal.GetMap());
		case eZType_Raw: return new ZYadStreamRPos_ZooLib(iVal.GetData());
		case eZType_String: return sMakeYadR(iVal.GetString());
		}

	return new ZYadPrimR_ZooLib(iVal);
	}

ZRef<ZYadSeqRPos> sMakeYadR(const ZSeq_ZooLib& iSeq)
	{ return new ZYadSeqRPos_ZooLib(iSeq); }

ZRef<ZYadMapRPos> sMakeYadR(const ZMap_ZooLib& iMap)
	{ return new ZYadMapRPos_ZooLib(iMap); }

// =================================================================================================
#pragma mark -
#pragma mark * sFromYadR

namespace ZANONYMOUS {

class Visitor_GetVal : public ZVisitor_Yad
	{
public:
	Visitor_GetVal(const ZVal_ZooLib& iDefault);

// From ZVisitor_Yad
	virtual bool Visit_YadPrimR(ZRef<ZYadPrimR> iYadPrimR);
	virtual bool Visit_YadStreamR(ZRef<ZYadStreamR> iYadStreamR);
	virtual bool Visit_YadStrimR(ZRef<ZYadStrimR> iYadStrimR);
	virtual bool Visit_YadSeqR(ZRef<ZYadSeqR> iYadSeqR);
	virtual bool Visit_YadMapR(ZRef<ZYadMapR> iYadMapR);

// Our protocol
	ZVal_ZooLib GetVal(ZRef<ZYadR> iYadR);

private:
	ZVal_ZooLib fDefault;
	ZVal_ZooLib fOutput;
	};

} // anonymous namespace

Visitor_GetVal::Visitor_GetVal(const ZVal_ZooLib& iDefault)
:	fDefault(iDefault)
	{}

bool Visitor_GetVal::Visit_YadPrimR(ZRef<ZYadPrimR> iYadPrimR)
	{
	fOutput = ZVal_ZooLib::sDFromAny(fDefault, iYadPrimR->AsAny());
	return true;
	}

bool Visitor_GetVal::Visit_YadStreamR(ZRef<ZYadStreamR> iYadStreamR)
	{
	fOutput = sReadAll_T<ZData_ZooLib>(iYadStreamR->GetStreamR());
	return true;
	}

bool Visitor_GetVal::Visit_YadStrimR(ZRef<ZYadStrimR> iYadStrimR)
	{
	fOutput = iYadStrimR->GetStrimR().ReadAll8();
	return true;
	}

bool Visitor_GetVal::Visit_YadSeqR(ZRef<ZYadSeqR> iYadSeqR)
	{
	ZSeq_ZooLib theSeq;

	while (ZRef<ZYadR> theChild = iYadSeqR->ReadInc())
		theSeq.Append(this->GetVal(theChild));

	fOutput = theSeq;
	return true;
	}

bool Visitor_GetVal::Visit_YadMapR(ZRef<ZYadMapR> iYadMapR)
	{
	ZMap_ZooLib theMap;

	string theName;
	while (ZRef<ZYadR> theChild = iYadMapR->ReadInc(theName))
		theMap.Set(theName, this->GetVal(theChild));

	fOutput = theMap;
	return true;
	}

ZVal_ZooLib Visitor_GetVal::GetVal(ZRef<ZYadR> iYadR)
	{
	ZVal_ZooLib result;
	if (iYadR)
		{
		iYadR->Accept(*this);
		std::swap(result, fOutput);
		}
	return result;
	}

ZVal_ZooLib sFromYadR(const ZVal_ZooLib& iDefault, ZRef<ZYadR> iYadR)
	{
	if (ZRef<ZYadR_ZooLib> theYadR = ZRefDynamicCast<ZYadR_ZooLib>(iYadR))
		return theYadR->GetVal();

	return Visitor_GetVal(iDefault).GetVal(iYadR);
	}

NAMESPACE_ZOOLIB_END
