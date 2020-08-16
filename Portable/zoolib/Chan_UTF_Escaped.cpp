// Copyright (c) 2009 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/Chan_UTF_Escaped.h"

#include "zoolib/ParseException.h"
#include "zoolib/Unicode.h"
#include "zoolib/Util_Chan.h"
#include "zoolib/Util_Chan_UTF.h"

#include "zoolib/ZDebug.h"

namespace ZooLib {

// =================================================================================================
#pragma mark - ChanR_UTF_Escaped

ChanR_UTF_Escaped::ChanR_UTF_Escaped(UTF32 iDelimiter, const ChanRU_UTF& iChanRU)
:	fDelimiter(iDelimiter)
,	fChanRU(iChanRU)
	{}

ChanR_UTF_Escaped::~ChanR_UTF_Escaped()
	{}

size_t ChanR_UTF_Escaped::Read(UTF32* oDest, size_t iCountCU)
	{
	using namespace Util_Chan;

	UTF32* localDest = oDest;
	UTF32* localDestEnd = oDest + iCountCU;
	while (localDestEnd > localDest)
		{
		const ZQ<UTF32> theCPQ = sQRead(fChanRU);
		if (not theCPQ)
			sThrow_ParseException("Unexpected end of strim whilst parsing a string");

		UTF32 theCP = *theCPQ;

		if (theCP == fDelimiter)
			{
			sUnread(fChanRU, theCP);
			break;
			}

		if (Unicode::sIsEOL(theCP))
			{
			sUnread(fChanRU, theCP);
			sThrow_ParseException("Illegal end of line whilst parsing a string");
			}

		if (theCP == '\\')
			{
			const ZQ<UTF32> theCPQ = sQRead(fChanRU);
			if (not theCPQ)
				sThrow_ParseException("Unexpected end of strim after parsing escape");

			theCP = *theCPQ;

			switch (theCP)
				{
				case '\\':
					theCP = '\\';
					break;
				case 't':
					theCP = '\t';
					break;
				case 'n':
					theCP = '\n';
					break;
				case 'r':
					theCP = '\r';
					break;
				case 'b':
					theCP = '\b';
					break;
				case 'f':
					theCP = '\f';
					break;
				case '"':
					theCP = '\"';
					break;
				case '\'':
					theCP = '\'';
					break;
				case '/':
					theCP = '/';
					break;
				case 'x':
					{
					if (NotQ<int> theQ = sQRead_HexDigit(fChanRU))
						{
						sThrow_ParseException("Illegal non-hex digit following \"\\x\"");
						}
					else
						{
						theCP = *theQ;
						while (ZQ<int> theQ = sQRead_HexDigit(fChanRU))
							theCP = (theCP << 4) + *theQ;
						}
					break;
					}
				case 'u':
				case 'U':
					{
					int32 requiredChars = 4;
					if (theCP == 'U')
						requiredChars = 8;

					UTF32 resultCP = 0;
					while (requiredChars--)
						{
						if (NotQ<int> theQ = sQRead_HexDigit(fChanRU))
							{
							sThrow_ParseException(string8("Illegal non-hex digit in \"\\")
								+ char(theCP) + "\" escape sequence");
							}
						else
							{
							resultCP = (resultCP << 4) + *theQ;
							}
						}
					theCP = resultCP;
					break;
					}
				default:
					{
					// Gotta love escape sequences. This message
					// has "\" (quote, backslash, quote) at the end.
					sThrow_ParseException("Illegal character following \"\\\"");
					}
				}
			}

		*localDest++ = theCP;
		}

	return localDest - oDest;
	}

// =================================================================================================
#pragma mark - ChanW_UTF_Escaped::Options

ChanW_UTF_Escaped::Options::Options()
:	fQuoteQuotes(true)
,	fEscapeHighUnicode(true)
	{}

// =================================================================================================
#pragma mark - ChanW_UTF_Escaped

ChanW_UTF_Escaped::ChanW_UTF_Escaped(const Options& iOptions, const ChanW_UTF& iChanW)
:	fChanW(iChanW),
	fEOL(iOptions.fEOL),
	fQuoteQuotes(iOptions.fQuoteQuotes),
	fEscapeHighUnicode(iOptions.fEscapeHighUnicode),
	fLastWasCR(false)
	{}

ChanW_UTF_Escaped::ChanW_UTF_Escaped(const ChanW_UTF& iChanW)
:	fChanW(iChanW),
	fQuoteQuotes(true),
	fEscapeHighUnicode(true),
	fLastWasCR(false)
	{}

ChanW_UTF_Escaped::~ChanW_UTF_Escaped()
	{
	try
		{
		if (fLastWasCR)
			{
			sEWrite(fChanW, "\\r");
			sEWrite(fChanW, fEOL);
			}
		}
	catch (...)
		{}
	}

static UTF32 spAsHexCP(int inInt)
	{
	if (inInt < 10)
		return inInt + '0';
	return inInt - 10 + 'A';
	}

size_t ChanW_UTF_Escaped::Write(const UTF32* iSource, size_t iCountCU)
	{
	size_t localCount = iCountCU + 1;
	while (--localCount)
		{
		UTF32 theCP = *iSource++;
		bool lastWasCR = fLastWasCR;
		fLastWasCR = false;
		if (theCP < 0x20 || theCP == 0x7F)
			{
			switch (theCP)
				{
				case '\t':
					{
					sEWrite(fChanW, "\\t");
					break;
					}
				case '\n':
					{
					if (lastWasCR)
						sEWrite(fChanW, "\\r");
					sEWrite(fChanW, "\\n");
					sEWrite(fChanW, fEOL);
					break;
					}
				case '\r':
					{
					if (lastWasCR)
						{
						sEWrite(fChanW, "\\r");
						sEWrite(fChanW, fEOL);
						}
					fLastWasCR = true;
					break;
					}
				case '\b':
					{
					sEWrite(fChanW, "\\b");
					break;
					}
				case '\f':
					{
					sEWrite(fChanW, "\\f");
					break;
					}
				default:
					{
					sEWrite(fChanW, "\\x");
					sEWrite(fChanW, spAsHexCP(theCP >> 4));
					sEWrite(fChanW, spAsHexCP(theCP & 0xF));
					break;
					}
				}
			}
		else if (theCP < 0x80 || !fEscapeHighUnicode)
			{
			if (fQuoteQuotes && theCP == '\"')
				sEWrite(fChanW, "\\\"");
			else if (not fQuoteQuotes && theCP == '\'')
				sEWrite(fChanW, "\\\'");
			else if (theCP == '\\')
				sEWrite(fChanW, "\\\\");
			else
				sEWrite(fChanW, theCP);
			}
		else if (theCP < 0x10000)
			{
			sEWrite(fChanW, "\\u");
			sEWrite(fChanW, spAsHexCP((theCP >> 12) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 8) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 4) & 0xF));
			sEWrite(fChanW, spAsHexCP(theCP & 0xF));
			}
		else
			{
			sEWrite(fChanW, "\\U");
			sEWrite(fChanW, spAsHexCP((theCP >> 28) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 24) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 20) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 16) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 12) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 8) & 0xF));
			sEWrite(fChanW, spAsHexCP((theCP >> 4) & 0xF));
			sEWrite(fChanW, spAsHexCP(theCP & 0xF));
			}
		}
	return iCountCU - localCount;
	}

} // namespace ZooLib
