// Copyright (c) 2018 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/Apple/PullPush_CF.h"

#include "zoolib/Apple/Chan_UTF_CFString.h"
#include "zoolib/Apple/Data_CF.h"
#include "zoolib/Apple/Util_CF.h"
#include "zoolib/Apple/Util_CF_Any.h"
#include "zoolib/Apple/Util_CF_ZZ.h"
#include "zoolib/Apple/Val_CF.h"

#include "zoolib/Channer_Bin.h"
#include "zoolib/Channer_UTF.h"
#include "zoolib/Chan_Bin_Data.h"
#include "zoolib/NameUniquifier.h"
#include "zoolib/Util_Chan.h"

namespace ZooLib {

using std::string;

using namespace PullPush;
using namespace Util_CF;

// =================================================================================================
#pragma mark - 

static void spGatherContents(const void* iKey, const void* iValue, void* iRefcon)
	{
	const ChanW_PPT& theChanW = *((const ChanW_PPT*)iRefcon);
	sPush(sName(sAsUTF8(static_cast<CFStringRef>(iKey))), theChanW);
	sFromCF_Push_PPT(static_cast<CFTypeRef>(iValue), theChanW);
	}

bool sFromCF_Push_PPT(CFTypeRef iCFTypeRef, const ChanW_PPT& iChanW)
	{
	if (not iCFTypeRef)
		{
		sPush(PPT(), iChanW);
		return true;
		}

	const CFTypeID theTypeID = ::CFGetTypeID(iCFTypeRef);

	if (theTypeID == ::CFStringGetTypeID())
		{
		sPull_UTF_Push_PPT(ChanR_UTF_CFString((CFStringRef)iCFTypeRef), iChanW);
		return true;
		}

	if (theTypeID == ::CFDataGetTypeID())
		{
		PullPushPair<byte> thePullPushPair = sMakePullPushPair<byte>();
		sPush(sGetClear(thePullPushPair.second), iChanW);
		sFlush(iChanW);

		if (size_t theLength = ::CFDataGetLength((CFDataRef)iCFTypeRef))
			sWriteMemFully(*thePullPushPair.first, ::CFDataGetBytePtr((CFDataRef)iCFTypeRef), theLength);

		sDisconnectWrite(*thePullPushPair.first);
		return true;
		}

	if (theTypeID == ::CFArrayGetTypeID())
		{
		CFArrayRef theCFArrayRef = (CFArrayRef)iCFTypeRef;
		sPush_Start_Seq(iChanW);
		for (size_t xx = 0, theCount = ::CFArrayGetCount(theCFArrayRef); xx < theCount; ++xx)
			{
			if (not sFromCF_Push_PPT(::CFArrayGetValueAtIndex(theCFArrayRef, xx), iChanW))
				return false;
			}
		sPush_End(iChanW);
		return true;
		}

	if (theTypeID == ::CFDictionaryGetTypeID())
		{
		sPush_Start_Map(iChanW);
		::CFDictionaryApplyFunction((CFDictionaryRef)iCFTypeRef, spGatherContents, (void*)&iChanW);
		sPush_End(iChanW);
		return true;
		}

	if (ZQ<Any> theQ = sQSimpleAsAny(theTypeID, iCFTypeRef))
		{
		sPush(theQ->As<PPT>(), iChanW);
		return true;
		}

	return false;
	}

// =================================================================================================
#pragma mark - 

static bool sPull_PPT_AsCF(const PPT& iPPT, const ChanR_PPT& iChanR, ZP<CFTypeRef>& oCFTypeRef)
	{
	if (const string* theString = sPGet<string>(iPPT))
		{
		oCFTypeRef = sString(*theString);
		return true;
		}

	if (ZP<ChannerR_UTF> theChanner = sGet<ZP<ChannerR_UTF>>(iPPT))
		{
		ZP<CFMutableStringRef> theStringRef = sStringMutable();
		sCopyAll(*theChanner, ChanW_UTF_CFString(theStringRef));
		oCFTypeRef = theStringRef;
		return true;
		}

	if (ZP<ChannerR_Bin> theChanner = sGet<ZP<ChannerR_Bin>>(iPPT))
		{
		oCFTypeRef = sReadAll_T<Data_CF>(*theChanner);
		return true;
		}

	if (sIsStart_Map(iPPT))
		{
		Map_CF theMap;
		for (;;)
			{
			if (NotQ<Name> theNameQ = sQEReadNameOrEnd(iChanR))
				{
				break;
				}
			else
				{
				ZP<CFTypeRef> theCFTypeRef;
				if (not sPull_PPT_AsCF(iChanR, theCFTypeRef))
					return false;
				theMap.Set(*theNameQ, theCFTypeRef);
				}
			}
		oCFTypeRef = theMap;
		return true;
		}

	if (sIsStart_Seq(iPPT))
		{
		Seq_CF theSeq;
		for (;;)
			{
			ZQ<PPT> theQ = sQRead(iChanR);
			if (not theQ)
				return false;
			if (sIsEnd(*theQ))
				break;
			ZP<CFTypeRef> theCFTypeRef;
			if (not sPull_PPT_AsCF(*theQ, iChanR, theCFTypeRef))
				return false;
			theSeq.Append(theCFTypeRef);
			}
		oCFTypeRef = theSeq;
		return true;
		}

	oCFTypeRef = sAsCFType(iPPT.As<Val_ZZ>());

	return true;
	}

bool sPull_PPT_AsCF(const ChanR_PPT& iChanR, ZP<CFTypeRef>& oCFTypeRef)
	{
	ZQ<PPT> theQ = sQRead(iChanR);
	if (not theQ)
		return false;
	return sPull_PPT_AsCF(*theQ, iChanR, oCFTypeRef);
	}

ZP<CFTypeRef> sAsCF(const ChanR_PPT& iChanR)
	{
	if (ZQ<PPT> theQ = sQRead(iChanR))
		{
		ZP<CFTypeRef> theCFTypeRef;
		if (sPull_PPT_AsCF(*theQ, iChanR, theCFTypeRef))
			return theCFTypeRef;
		}
	return null;
	}

} // namespace ZooLib
