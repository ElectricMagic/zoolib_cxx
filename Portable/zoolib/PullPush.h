// Copyright (c) 2018 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_PullPush_h__
#define __ZooLib_PullPush_h__ 1
#include "zconfig.h"

#include "zoolib/Any_T.h"
#include "zoolib/Callable_Bind.h"
#include "zoolib/Callable_Function.h"
#include "zoolib/Chan.h"
#include "zoolib/Chan_XX_PipePair.h"
#include "zoolib/Channer.h"
#include "zoolib/ChanR.h"
#include "zoolib/ChanR_Bin.h"
#include "zoolib/ChanR_UTF.h"
#include "zoolib/ChanW.h"
#include "zoolib/Name.h"
#include "zoolib/StartOnNewThread.h"

namespace ZooLib {

// =================================================================================================
#pragma mark - 

namespace PullPush {

struct Tag_PPT;
typedef Any_T<Tag_PPT> PPT;

class Marker : public CountedWithoutFinalize {};

// -----

class Start : public virtual Marker
	{
protected:
	Start() {}

public:
	static const PPT sPPT;
	static bool sIs(const PPT& iPPT);
	};

// -----

class Start_Map : public Start
	{
protected:
	Start_Map() {}

public:
	static const PPT sPPT;
	static bool sIs(const PPT& iPPT);
	};

// -----

class Start_Seq : public Start
	{
protected:
	Start_Seq() {}

public:
	static const PPT sPPT;
	static bool sIs(const PPT& iPPT);
	};

// -----

class End : public virtual Marker
	{
protected:
	End() {}

public:
	static const PPT sPPT;
	static bool sIs(const PPT& iPPT);
	};

// -----

bool sIsStart(const PPT& iPPT);
bool sIsStart_Map(const PPT& iPPT);
bool sIsStart_Seq(const PPT& iPPT);
bool sIsEnd(const PPT& iPPT);

} // namespace PullPush

// =================================================================================================
#pragma mark -

using PPT = PullPush::PPT;

using ChanR_PPT = ChanR<PPT>;
using ChannerR_PPT = Channer<ChanR_PPT>;

using ChanW_PPT = ChanW<PPT>;
using ChannerW_PPT = Channer<ChanW_PPT>;

using ChanWCon_PPT = ChanWCon<PPT>;
using ChannerWCon_PPT = Channer<ChanWCon_PPT>;

void sPush_Start_Map(const ChanW_PPT& iChanW);
void sPush_Start_Seq(const ChanW_PPT& iChanW);
void sPush_End(const ChanW_PPT& iChanW);

void sPush_Marker(const ZP<PullPush::Marker>& iMarker, const ChanW_PPT& iChanW);

void sPush(const PPT& iVal, const ChanW_PPT& iChanW);

template <class T>
void sPush(const T& iVal, const ChanW_PPT& iChanW)
	{ sEWrite<PPT>(iChanW, PPT(iVal)); }

template <class T>
void sPush(const Name& iName, const T& iVal, const ChanW_PPT& iChanW)
	{
	sPush(iName, iChanW);
	sPush(iVal, iChanW);
	}

// Passing a Name as the value is unlikely to be intentional, the caller probably
// thinks it's passing a string. If a Name followed by a Name is actually wanted
// then that's better done explicitly with two separate pushes.
template <>
void sPush<Name>(const Name& iName, const Name&, const ChanW_PPT& iChanW) = delete;

void sPull_UTF_Push_PPT(const ChanR_UTF& iChanR, const ChanW_PPT& iChanW);
void sPull_UTF_Push_PPT(const ChanR_UTF& iChanR, uint64 iCount, const ChanW_PPT& iChanW);

void sPull_Bin_Push_PPT(const ChanR_Bin& iChanR, const ChanW_PPT& iChanW);
void sPull_Bin_Push_PPT(const ChanR_Bin& iChanR, uint64 iCount, const ChanW_PPT& iChanW);

// ----------

bool sCopy_Node(const ChanR_PPT& iChanR, size_t iStartDepth, const ChanW_PPT& iChanW);
bool sSkip_Node(const ChanR_PPT& iChanR);

bool sSkip_Node(const ChanR_PPT& iChanR, size_t iStartDepth);

// ----------

bool sTryPull_Start_Map(const ChanRU<PPT>& iChanRU);
bool sTryPull_Start_Seq(const ChanRU<PPT>& iChanRU);
bool sTryPull_End(const ChanRU<PPT>& iChanRU);
bool sTryPull_Name(const Name& iName, const ChanRU<PPT>& iChanRU);

void sEPull_End(const ChanR<PPT>& iChanR);

// ----------

// Throw if the chan is empty, return a false ZQ<PPT> if we hit an end, otherwise return the PPT.
ZQ<PPT> sQEReadPPTOrEnd(const ChanR<PPT>& iChanR);

// Throw if the chan is empty, return a false ZQ<Name> if we hit an end, otherwise return the Name.
ZQ<Name> sQEReadNameOrEnd(const ChanR<PPT>& iChanR);

// ----------

template <class EE>
using PullPushPair = std::pair<ZP<ChannerWCon<EE>>,ZP<ChannerR<EE>>>;

// ----------

template <class EE>
void sMakePullPushPair(ZP<ChannerWCon<EE>>& oChannerW, ZP<ChannerR<EE>>& oChannerR)
	{
	ZP<ImpPipePair<EE>> theImp = new ImpPipePair<EE>;
	oChannerW = sChanner_T<ChanWCon_XX_PipePair<EE>>(theImp);
	oChannerR = sChanner_T<ChanR_XX_PipePair<EE>>(theImp);
	}

template <class EE>
PullPushPair<EE> sMakePullPushPair()
	{
	PullPushPair<EE> thePair;
	sMakePullPushPair<EE>(thePair.first, thePair.second);
	return thePair;
	}

// ----------

template <class Pull_p, class Push_p>
void sRunPullPush_Channer(
	const ZP<Callable<void(const ChanR<Pull_p>&,const ChanW<Push_p>&)>>& iCallable,
	const ZP<ChannerR<Pull_p>>& iChannerR,
	const ZP<ChannerWCon<Push_p>>& iChannerWCon)
	{
	try
		{
		sCall(iCallable, *iChannerR, *iChannerWCon);
		}
	catch (std::exception& ex)
		{}
	sDisconnectWrite(*iChannerWCon);
	}

template <class Pull_p, class Push_p>
ZP<ChannerR<Push_p>> sStartPullPush(
	const ZP<Callable<void(const ChanR<Pull_p>&,const ChanW<Push_p>&)>>& iCallable,
	const ZP<ChannerR<Pull_p>>& iChannerR)
	{
	PullPushPair<Push_p> thePullPushPair = sMakePullPushPair<Push_p>();
	sStartOnNewThread
		(
		sBindR
			(
			sCallable(sRunPullPush_Channer<Pull_p,Push_p>),
			iCallable,
			iChannerR,
			sGetClear(thePullPushPair.first)
			)
		);
	return thePullPushPair.second;
	}

} // namespace ZooLib

#endif // __ZooLib_PullPush_h__
