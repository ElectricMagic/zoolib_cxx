// Copyright (c) 2018 Andrew Green
// http://www.zoolib.org

#ifndef __ZooLib_ChanR_XX_AbortOnSlowRead_h__
#define __ZooLib_ChanR_XX_AbortOnSlowRead_h__ 1
#include "zconfig.h"

#include "zoolib/Chan.h"

namespace ZooLib {

// =================================================================================================
#pragma mark - ChanR_XX_AbortOnSlowRead

template <class EE>
class ChanR_XX_AbortOnSlowRead
:	public virtual ChanR<EE>
	{
public:
	using ChanForRead = DeriveFrom<
		ChanAspect_Abort,
		ChanAspect_Read<EE>,
		ChanAspect_WaitReadable>;

	ChanR_XX_AbortOnSlowRead(const ChanForRead& iChan, double iTimeout)
	:	fChan(iChan)
	,	fTimeout(iTimeout)
		{}

// From ChanAspect_Read
	virtual size_t Read(EE* oDest, size_t iCount)
		{
		if (not sWaitReadable(fChan, fTimeout))
			{
			sAbort(fChan);
			sThrow_ExhaustedR();
			}

		return sRead(fChan, oDest, iCount);
		}

	const ChanForRead& fChan;
	const double fTimeout;
	};

} // namespace ZooLib

#endif // __ZooLib_ChanR_XX_AbortOnSlowRead_h__
