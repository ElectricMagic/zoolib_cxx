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

#ifndef __ZooLib_Yad_JSON_h__
#define __ZooLib_Yad_JSON_h__ 1
#include "zconfig.h"

#include "zoolib/Chan_UTF_Escaped.h"
#include "zoolib/Chan_Bin_ASCIIStrim.h"
#include "zoolib/Chan_Bin_Base64.h"
#include "zoolib/ChanR_Bin_HexStrim.h"
#include "zoolib/ChanR_XX_Terminated.h"
#include "zoolib/ChanR_XX_Boundary.h"
#include "zoolib/CountedVal.h"
#include "zoolib/Yad_Std.h"

namespace ZooLib {
namespace Yad_JSON {

using std::string;

// =================================================================================================
#pragma mark -
#pragma mark ReadOptions

struct ReadOptions
	{
	ZQ<bool> fAllowUnquotedPropertyNames;
	ZQ<bool> fAllowEquals;
	ZQ<bool> fAllowSemiColons;
	ZQ<bool> fAllowTerminators;
	ZQ<bool> fLooseSeparators;
	ZQ<bool> fAllowBinary;
	};

ReadOptions sReadOptions_Extended();

// =================================================================================================
#pragma mark -
#pragma mark WriteOptions

struct WriteOptions : YadOptions
	{
	WriteOptions();
	WriteOptions(const YadOptions& iOther);
	WriteOptions(const WriteOptions& iOther);

	ZQ<bool> fUseExtendedNotation;
	ZQ<bool> fBinaryAsBase64;
	ZQ<bool> fPreferSingleQuotes;
	ZQ<bool> fNumberSequences;
	};

// =================================================================================================
#pragma mark -
#pragma mark ParseException

class ParseException : public YadParseException_Std
	{
public:
	ParseException(const std::string& iWhat);
	ParseException(const char* iWhat);
	};

// =================================================================================================
#pragma mark -
#pragma mark ParseException

string spPrettyName(const std::type_info& iTI);
bool spRead_Identifier(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU,
	string* oStringLC, string* oStringExact);
bool spTryRead_PropertyName(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU,
	string& oName, bool iAllowUnquoted);
bool spTryRead_JSONString(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU,
	string& oString);

void spWriteIndent(size_t iCount, const WriteOptions& iOptions, const ChanW_UTF& iChanW);
void spWriteLFIndent(size_t iCount, const WriteOptions& iOptions, const ChanW_UTF& iChanW);
void spWriteString(const string& iString, bool iPreferSingleQuotes, const ChanW_UTF& iChanW);
void spWriteString(const ChanR_UTF& iChanR, const ChanW_UTF& iChanW);
bool spContainsProblemChars(const string& iString);
void spWritePropName(const string& iString, bool iUseSingleQuotes, const ChanW_UTF& w);
void spToStrim_SimpleValue(const Any& iAny, const WriteOptions& iOptions, const ChanW_UTF& w);
void spToStrim_Stream(const ChanR_Bin& iChanR,
	size_t iLevel, const WriteOptions& iOptions, bool iMayNeedInitialLF,
	const ChanW_UTF& w);

// =================================================================================================
#pragma mark -
#pragma mark YadStreamerR_Hex

class YadStreamerR_Hex
:	public Channer_T<ChanR_Bin_HexStrim>
	{
	typedef Channer_T<ChanR_Bin_HexStrim> inherited;
public:
	YadStreamerR_Hex(ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU);

// From ZCounted
	virtual void Finalize();

private:
	ZRef<ChannerR_UTF> fChannerR;
	ZRef<ChannerU_UTF> fChannerU;
	};

// =================================================================================================
#pragma mark -
#pragma mark YadStreamerR_Base64

class YadStreamerR_Base64
:	public YadStreamerR
	{
public:
	YadStreamerR_Base64(const Base64::Decode& iDecode,
		ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU);

// From ZCounted
	virtual void Finalize();

// From ChanR
	virtual size_t Read(byte* oDest, size_t iCount);

private:
	ZRef<ChannerR_UTF> fChannerR;
	ZRef<ChannerU_UTF> fChannerU;
	ChanR_Bin_ASCIIStrim fChanR_Bin_ASCIIStrim;
	ChanR_XX_Terminated<byte> fChanR_Bin_Terminated;
	ChanR_Bin_Base64Decode fChanR;
	};

// =================================================================================================
#pragma mark -
#pragma mark YadStrimmerR_JSON

class YadStrimmerR_JSON
:	public YadStrimmerR
	{
public:
	YadStrimmerR_JSON(ZRef<ChannerRU_UTF> iChanner);
	YadStrimmerR_JSON(ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU);

	virtual ~YadStrimmerR_JSON();

// From ZCounted
	virtual void Finalize();

// From ChanR_UTF via YadStrimmerR
	virtual size_t Read(UTF32* oDest, size_t iCount);

private:
	ZRef<ChannerR_UTF> fChannerR;
	ZRef<ChannerU_UTF> fChannerU;
	ChanR_XX_Boundary<UTF32> fChanR_Boundary;
	size_t fQuotesSeen;
	};

// =================================================================================================
#pragma mark -
#pragma mark YadSeqR_JSON

class ChanR_RefYad_JSON
:	public ChanR_RefYad_Std
	{
public:
	ChanR_RefYad_JSON(const ZRef<CountedVal<ReadOptions> >& iRO,
		ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU);

// From ChanR_RefYad_Std
	virtual void Imp_ReadInc(bool iIsFirst, ZRef<YadR>& oYadR);

private:
	const ZRef<CountedVal<ReadOptions> > fRO;
	ZRef<ChannerR_UTF> fChannerR;
	ZRef<ChannerU_UTF> fChannerU;
	};

typedef Channer_T<ChanR_RefYad_JSON> YadSeqR_JSON;

// =================================================================================================
#pragma mark -
#pragma mark YadMapR_JSON

class ChanR_NameRefYad_JSON
:	public ChanR_NameRefYad_Std
	{
public:
	ChanR_NameRefYad_JSON(const ZRef<CountedVal<ReadOptions> >& iRO,
		ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU);

// From ChanR_NameRefYad_Std
	virtual void Imp_ReadInc(bool iIsFirst, Name& oName, ZRef<YadR>& oYadR);

private:
	const ZRef<CountedVal<ReadOptions> > fRO;
	ZRef<ChannerR_UTF> fChannerR;
	ZRef<ChannerU_UTF> fChannerU;
	};

typedef Channer_T<ChanR_NameRefYad_JSON> YadMapR_JSON;

// =================================================================================================
#pragma mark -
#pragma mark sYadR, sToStrim and sPush

ZRef<YadR> sYadR(ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU);

ZRef<YadR> sYadR(const ReadOptions& iReadOptions,
	ZRef<ChannerR_UTF> iChannerR, ZRef<ChannerU_UTF> iChannerU);

void sToChan(ZRef<YadR> iYadR, const ChanW_UTF& w);

void sToChan(size_t iInitialIndent, const WriteOptions& iOptions,
	ZRef<YadR> iYadR, const ChanW_UTF& w);

ZQ<string8> sQRead_PropName(const ChanR_UTF& iChanR, const ChanU_UTF& iChanU);

void sWrite_PropName(const string8& iPropName, const ChanW_UTF& w);

} // namespace Yad_JSON
} // namespace ZooLib

#endif // __ZooLib_Yad_JSON_h__
