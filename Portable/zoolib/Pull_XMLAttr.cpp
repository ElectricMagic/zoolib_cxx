// Copyright (c) 2018 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/Pull_XMLAttr.h"

#include "zoolib/Log.h"
#include "zoolib/NameUniquifier.h" // For sName
#include "zoolib/ParseException.h"
#include "zoolib/Pull_ML.h"
#include "zoolib/ZMACRO_foreach.h"

/*
sPull_XMLAttr_Push turns this:
  <tag1>
    <tag2>val3</tag2>
    <tag4 at5="5" at6="6"/>
    <tag7 at8="8">
      <tag9/>
      <tag10></tag10>
    </tag7>
  </tag1>

into this:
{
tag1 =
  {
  "!text" = "\n    ";
  tag2 = "val3";
  tag4 =
    {
    at5 = "5";
    at6 = "6";
    };
  tag7 =
    {
    at8 = "8";
    "!text" = "\n      ";
    tag9 =
      {
      };
    tag10 = "";
    };
  };
};
*/

namespace ZooLib {

using namespace PullPush;
using namespace Pull_ML;
using std::string;

// =================================================================================================
#pragma mark -

static void spPush_Attrs(const ML::Attrs_t& iAttrs, const ChanW_PPT& iChanW)
	{
	foreacha (entry, iAttrs)
		sPush(entry.first, entry.second, iChanW);
	}

// =================================================================================================
#pragma mark - sPull_XMLAttr_Push_PPT (from ChanR_PPT)

#if 0

Text after an end or an empty could just be emitted as a !text, rather than only the text
that occurs after a begin and before any other tag. I think that would then let us stream the
content rather than having to speculatively accumulate it in case it's meaningful.

Actually no. Text after a begin has to be captured until we see an end, when we can emit the
begin/text/end as a simple property.
Any text after an end or an empty can be streamed as a !text though.

Could we handle attributes with no value as being booleans? and emit them as such? Might need
to enhance the Attrs_t/Attr_t definition, pair<Name, ZQ<string>> perhaps. Absent string is a
boolean attribute.



static void spPull_XMLAttr_Push_PPT(const Name& iOuterName,
	const PPT& iPPT,
	const ChanR_PPT& iChanR,
	const ChanW_PPT& iChanW)
	{
	PPT thePPT = iPPT;

	for (;;)
		{
		if (NotQ<PPT> thePPTQ = sQSkipText_Read(iChanR))
			{
			if (sNotEmpty(iOuterName))
				sThrow_ParseException("Unexpected end of source (1)");
			break;
			}
		else if (ZP<TagEnd> theTagEnd = TagEnd::sAs(*thePPTQ))
			{
			if (iOuterName != theTagEnd->GetName())
				{
				sThrow_ParseException("Expected end tag '"
					+ string(iOuterName) + "', got '" + string(theTagEnd->GetName()) + "' (1)");
				}
			break;
			}
		else if (ZP<TagEmpty> theTagEmpty = TagEmpty::sAs(*thePPTQ))
			{
			sPush(sName(theTagEmpty->GetName()), iChanW);
			sPush_Start_Map(iChanW);
				spPush_Attrs(theTagEmpty->GetAttrs_t(), iChanW);
			sPush_End(iChanW);
			}
		else
			{
			ZP<TagBegin> theTagBegin = TagBegin::sAs(*thePPTQ)
			if (not theTagBegin)
				sThrow_ParseException("Expected begin tag");

			const Name theName = theTagBegin->GetName();
			sPush(theName, iChanW);

			if (NotQ<PPT> theNextPPTQ = sQRead(iChanR))
				{
				sThrow_ParseException("Unexpected end of source (2)");
				}
			else
				{
				ZQ<PPT> thePPTQ = theNextPPTQ;
				ZQ<string> theTextQ = sQAsString(*thePPTQ);
				if (theTextQ)
					thePPTQ = sQRead(iChanR);

				if (not thePPTQ)
					{
					sThrow_ParseException("Unexpected end of source (3)");
					}
				else
					{
					if (ZP<TagEnd> theTagEnd = TagEnd::sAs(*thePPTQ))
						{
						if (theName != theTagEnd->GetName())
							{
							sThrow_ParseException("Expected end tag '"
								+ string(iOuterName) + "', got '" + string(theTagEnd->GetName()) + "' (2)");
							}
						if (sIsEmpty(theTagBegin->GetAttrs())
							{
							if (theTextQ)
								sPush(*theTextQ, iChanW);
							}
						else
							{
							sPush_Start_Map(iChanW);
								spPush_Attrs(theTagBegin->GetAttrs(), iChanW);
								if (theTextQ)
									sPush("!text", *theTextQ, iChanW);
							sPush_End(iChanW);
							}
						}
					else if (ioChanRU.Current() == ML::eToken_TagBegin
						|| ioChanRU.Current() == ML::eToken_TagEmpty)
						{
						sPush_Start_Map(iChanW);
							spPush_Attrs(theAttrs, iChanW);
							if (theText.size())
								sPush("!text", theText, iChanW);
							sPull_XMLAttr_Push_PPT(theName, ioChanRU, iChanW);
						sPush_End(iChanW);
						}
			}
		}
	}

bool sPull_XMLAttr_Push_PPT(const ChanR_PPT& iChanR, const ChanW_PPT& iChanW)
	{
	sSkipText(ioChanRU);
	if (ioChanRU.Current() == ML::eToken_Exhausted)
		return false;

	sPush_Start_Map(iChanW);
		spPull_XMLAttr_Push_PPT(string(), ioChanRU, iChanW);
	sPush_End(iChanW);
	return true;
	}
#endif // 0

// =================================================================================================
#pragma mark - sPull_XMLAttr_Push_PPT (from ChanRU_UTF_ML)

/*
We enter the function having supposed been passed a tagname. Which will be empty for the top most.

for (;;)
	{
	<Name/> --> Name={}
	<Name a=b c=d/> --> Name={ a="b"; c="d"; }

	<Name>text</Name>, --> Name="text"
	<Name a=b c=d>text</Name>, --> Name={ a="b";c="d"; !text="text"}

	<Name> || <BeginOrEmpty/> --> Name={ --> recurse passing Name, BeginOrEmpty to read
	<Name>text || <BeginOrEmpty/> --> Name={!text=text --> recurse passing Name, BeginOrEmpty to read
	<Name a=b c=d> || <BeginOrEmpty> --> Name={ a="b"; c="d" --> recurse passing Name, BeginOrEmpty to read
	<Name a=b c=d>text || <BeginOrEmpty> --> Name={ a="b"; c="d" !text=test, --> recurse passing Name, BeginOrEmpty to read

	</OuterName> --> } --> return
	Token_Exhausted, with OuterName == "" is legal end of document.
	}
*/

static void spPull_XMLAttr_Push_PPT(const string& iOuterName,
	ChanRU_UTF_ML& ioChanRU,
	const ChanW_PPT& iChanW)
	{
	for (;;)
		{
		sSkipText(ioChanRU);

		if (ioChanRU.Current() == ML::eToken_Exhausted)
			{
			if (not iOuterName.empty())
				sThrow_ParseException("Unexpected end of source");
			break;
			}
		else if (ioChanRU.Current() == ML::eToken_TagEnd)
			{
			if (iOuterName != ioChanRU.Name())
				{
				sThrow_ParseException("Expected end tag '"
					+ iOuterName + "', got '" + ioChanRU.Name() + "'");
				}
			ioChanRU.Advance();
			break;
			}
		else if (ioChanRU.Current() == ML::eToken_TagEmpty)
			{
			sPush(sName(ioChanRU.Name()), iChanW);
			sPush_Start_Map(iChanW);
				spPush_Attrs(ioChanRU.Attrs(), iChanW);
			sPush_End(iChanW);
			ioChanRU.Advance();
			}
		else
			{
			ZAssert(ioChanRU.Current() == ML::eToken_TagBegin);

			const string theName = ioChanRU.Name();
			sPush(sName(theName), iChanW);
			const ML::Attrs_t theAttrs = ioChanRU.Attrs();
			ioChanRU.Advance();

			const string theText = sReadAllUTF8(ioChanRU);
			sSkipText(ioChanRU); // Won't have text to skip, but will advance to next token.

			if (ioChanRU.Current() == ML::eToken_TagEnd)
				{
				if (ioChanRU.Name() != theName)
					{
					sThrow_ParseException("Expected end tag '"
						+ theName + "', got '" + ioChanRU.Name() + "'");
					}

				ioChanRU.Advance();

				if (theAttrs.empty())
					{
					sPush(theText, iChanW);
					}
				else
					{
					sPush_Start_Map(iChanW);
						spPush_Attrs(theAttrs, iChanW);
						sPush("!text", theText, iChanW);
					sPush_End(iChanW);
					}
				}
			else if (ioChanRU.Current() == ML::eToken_TagBegin
				|| ioChanRU.Current() == ML::eToken_TagEmpty)
				{
				sPush_Start_Map(iChanW);
					spPush_Attrs(theAttrs, iChanW);
					if (theText.size())
						sPush("!text", theText, iChanW);
					spPull_XMLAttr_Push_PPT(theName, ioChanRU, iChanW);
				sPush_End(iChanW);
				}
			else
				{
				ZAssert(ioChanRU.Current() == ML::eToken_Exhausted);
				sThrow_ParseException("Unexpected end of source");
				}
			}
		}
	}

bool sPull_XMLAttr_Push_PPT(ChanRU_UTF_ML& ioChanRU, const ChanW_PPT& iChanW)
	{
	sSkipText(ioChanRU);
	if (ioChanRU.Current() == ML::eToken_Exhausted)
		return false;

	sPush_Start_Map(iChanW);
		spPull_XMLAttr_Push_PPT(string(), ioChanRU, iChanW);
	sPush_End(iChanW);
	return true;
	}

// =================================================================================================
#pragma mark -

// We can't use the sStartPullPush mechanism here because we're preserving that it's
// a ChannerRU_UTF_ML we're working with.

static void spXMLAttrPullPush(const ZP<ChannerRU_UTF_ML>& iChannerRU,
	const ZP<ChannerWCon_PPT>& iChannerWCon)
	{
	ZThread::sSetName("spXMLAttrPullPush");
	try
		{
		sPull_XMLAttr_Push_PPT(*iChannerRU, *iChannerWCon);
		}
	catch (...)
		{
		sTryPush(std::current_exception(), *iChannerWCon);
		}
	sDisconnectWrite(*iChannerWCon);
	}

ZP<ChannerR_PPT> sChannerR_PPT_XMLAttr(const ZP<ChannerRU_UTF_ML>& iChannerRU)
	{
	auto ppp = sMakePullPushPair<PPT>();
	sStartOnNewThread(sBindR(sCallable(spXMLAttrPullPush), iChannerRU, sGetClear(ppp.first)));
	return ppp.second;
	}

} // namespace ZooLib
