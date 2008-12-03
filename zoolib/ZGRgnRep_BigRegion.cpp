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

#include "zoolib/ZGRgnRep_BigRegion.h"

#include "zoolib/ZFactoryChain.h"

using namespace ZooLib;

// =================================================================================================
#pragma mark -
#pragma mark * Helper functions

namespace ZANONYMOUS {

bool sDecomposeRepProc(const ZRect& iRect, void* iRefcon)
	{
	// Use ZAccumulator_T at some point.
	ZBigRegion* theBigRegion = static_cast<ZBigRegion*>(iRefcon);
	*theBigRegion |= iRect;

	// Return false to indicate we don't want to abort.
	return false;
	}

ZGRgnRep_BigRegion* sDecomposeRepIntoBigRegion(const ZRef<ZGRgnRep>& iRep)
	{
	ZGRgnRep_BigRegion* theRep = new ZGRgnRep_BigRegion;
	iRep->Decompose(sDecomposeRepProc, &theRep->GetBigRegion());
	return theRep;
	}

ZGRgnRep_BigRegion* sMakeRep(const ZRef<ZGRgnRep>& iRep)
	{
	if (ZRef<ZGRgnRep_BigRegion> other = ZRefDynamicCast<ZGRgnRep_BigRegion>(iRep))
		return new ZGRgnRep_BigRegion(other->GetBigRegion());

	return sDecomposeRepIntoBigRegion(iRep);
	}

} // anonymous namespace

// =================================================================================================
#pragma mark -
#pragma mark * Factory functions

namespace ZANONYMOUS {

class Make_Rect
:	public ZFactoryChain_T<ZRef<ZGRgnRep>, const ZRect&>
	{
public:
	Make_Rect()
	:	ZFactoryChain_T<Result_t, Param_t>(false)
		{}

	virtual bool Make(Result_t& oResult, Param_t iParam)
		{
		oResult = new ZGRgnRep_BigRegion(ZBigRegion(iParam));
		return true;
		}	
	} sMaker0;

template <>
ZRef<ZGRgnRep> ZGRgnRepCreator_T<ZBigRegion>::sCreate(ZBigRegion iNative, bool iAdopt)
	{
	return new ZGRgnRep_BigRegion(iNative);
	}

} // anonymous namespace

// =================================================================================================
#pragma mark -
#pragma mark * ZGRgnRep_BigRegion

ZRef<ZGRgnRep_BigRegion> ZGRgnRep_BigRegion::sGetRep(const ZRef<ZGRgnRep>& iRep)
	{
	if (ZRef<ZGRgnRep_BigRegion> other = ZRefDynamicCast<ZGRgnRep_BigRegion>(iRep))
		return other;

	return sDecomposeRepIntoBigRegion(iRep);
	}

ZGRgnRep_BigRegion::ZGRgnRep_BigRegion()
	{}

ZGRgnRep_BigRegion::ZGRgnRep_BigRegion(const ZBigRegion& iBigRegion)
:	fBigRegion(iBigRegion)
	{}

ZGRgnRep_BigRegion::~ZGRgnRep_BigRegion()
	{}

namespace ZANONYMOUS {

struct DecomposeBigRegion_t
	{
	ZGRgnRep::DecomposeProc fDecomposeProc;
	void* fRefcon;
	};

bool sDecompose_BigRegion(const ZRect_T<int32>& iRect, void* iRefcon)
	{
	DecomposeBigRegion_t* theStruct = static_cast<DecomposeBigRegion_t*>(iRefcon);
	return theStruct->fDecomposeProc(iRect, theStruct->fRefcon);
	}

} // anonymous namespace

size_t ZGRgnRep_BigRegion::Decompose(DecomposeProc iProc, void* iRefcon)
	{
	DecomposeBigRegion_t theStruct;
	theStruct.fDecomposeProc = iProc;
	theStruct.fRefcon = iRefcon;
	return fBigRegion.Decompose(sDecompose_BigRegion, &theStruct);
	}

bool ZGRgnRep_BigRegion::Contains(ZCoord iH, ZCoord iV)
	{ return fBigRegion.Contains(iH, iV); }

bool ZGRgnRep_BigRegion::IsEmpty()
	{ return fBigRegion.IsEmpty(); }

ZRect ZGRgnRep_BigRegion::Bounds()
	{ return fBigRegion.Bounds(); }

bool ZGRgnRep_BigRegion::IsSimpleRect()
	{ return fBigRegion.IsSimpleRect(); }

bool ZGRgnRep_BigRegion::IsEqualTo(const ZRef<ZGRgnRep>& iRep)
	{ return fBigRegion == sGetRep(iRep)->GetBigRegion(); }

void ZGRgnRep_BigRegion::Inset(ZCoord iH, ZCoord iV)
	{ fBigRegion.MakeInset(iH, iV); }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Insetted(ZCoord iH, ZCoord iV)
	{
	ZGRgnRep_BigRegion* result = new ZGRgnRep_BigRegion(fBigRegion);
	result->fBigRegion.MakeInset(iH, iV);
	return result;
	}

void ZGRgnRep_BigRegion::Offset(ZCoord iH, ZCoord iV)
	{ fBigRegion += ZPoint_T<int32>(iH, iV); }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Offsetted(ZCoord iH, ZCoord iV)
	{
	ZGRgnRep_BigRegion* result = new ZGRgnRep_BigRegion(fBigRegion);
	result->fBigRegion += ZPoint_T<int32>(iH, iV);
	return result;
	}

void ZGRgnRep_BigRegion::Include(const ZRect& iRect)
	{ fBigRegion |= iRect; }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Including(const ZRect& iRect)
	{
	ZGRgnRep_BigRegion* result = new ZGRgnRep_BigRegion(fBigRegion);
	result->fBigRegion |= iRect;
	return result;
	}

void ZGRgnRep_BigRegion::Intersect(const ZRect& iRect)
	{ fBigRegion &= iRect; }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Intersecting(const ZRect& iRect)
	{
	ZGRgnRep_BigRegion* result = new ZGRgnRep_BigRegion(fBigRegion);
	result->fBigRegion &= iRect;
	return result;
	}

void ZGRgnRep_BigRegion::Exclude(const ZRect& iRect)
	{ fBigRegion -= iRect; }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Excluding(const ZRect& iRect)
	{
	ZGRgnRep_BigRegion* result = new ZGRgnRep_BigRegion(fBigRegion);
	result->fBigRegion -= iRect;
	return result;
	}

void ZGRgnRep_BigRegion::Include(const ZRef<ZGRgnRep>& iRep)
	{ fBigRegion |= sGetRep(iRep)->GetBigRegion(); }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Including(const ZRef<ZGRgnRep>& iRep)
	{
	ZGRgnRep_BigRegion* result = sMakeRep(iRep);
	result->fBigRegion |= fBigRegion;
	return result;
	}

void ZGRgnRep_BigRegion::Intersect(const ZRef<ZGRgnRep>& iRep)
	{ fBigRegion &= sGetRep(iRep)->GetBigRegion(); }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Intersecting(const ZRef<ZGRgnRep>& iRep)
	{
	ZGRgnRep_BigRegion* result = sMakeRep(iRep);
	result->fBigRegion &= fBigRegion;
	return result;
	}

void ZGRgnRep_BigRegion::Exclude(const ZRef<ZGRgnRep>& iRep)
	{ fBigRegion -= sGetRep(iRep)->GetBigRegion(); }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Excluding(const ZRef<ZGRgnRep>& iRep)
	{
	ZGRgnRep_BigRegion* result = sMakeRep(iRep);
	result->fBigRegion -= fBigRegion;
	return result;
	}

void ZGRgnRep_BigRegion::Xor(const ZRef<ZGRgnRep>& iRep)
	{ fBigRegion ^= sGetRep(iRep)->GetBigRegion(); }

ZRef<ZGRgnRep> ZGRgnRep_BigRegion::Xoring(const ZRef<ZGRgnRep>& iRep)
	{
	ZGRgnRep_BigRegion* result = sMakeRep(iRep);
	result->fBigRegion ^= fBigRegion;
	return result;
	}

ZBigRegion& ZGRgnRep_BigRegion::GetBigRegion()
	{ return fBigRegion; }
