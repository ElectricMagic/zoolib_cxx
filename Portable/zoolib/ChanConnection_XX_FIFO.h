// Copyright (c) 2007 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_ChanConnection_XX_FIFO_h__
#define __ZooLib_ChanConnection_XX_FIFO_h__ 1
#include "zconfig.h"

#include "zoolib/Chan.h"
#include "zoolib/ZThread.h"

#include <deque>

namespace ZooLib {

// =================================================================================================
#pragma mark - ChanConnection_XX_FIFO

/** A RW stream that queues written data and returns it in FIFO order when read. */

template <class EE>
class ChanConnection_XX_FIFO
:	public virtual ChanConnection<EE>
	{
public:
	ChanConnection_XX_FIFO()
	:	fClosed(false)
	,	fMaxSize(64)
	,	fUserCount(0)
		{}

	ChanConnection_XX_FIFO(size_t iMaxSize)
	:	fClosed(false)
	,	fMaxSize(iMaxSize)
	,	fUserCount(0)
		{}

	~ChanConnection_XX_FIFO()
		{
		ZAcqMtx acq(fMutex);

		fClosed = true;
		fCondition_Read.Broadcast();
		fCondition_Write.Broadcast();

		while (fUserCount)
			fCondition_UserCount.Wait(fMutex);
		}

// From ChanAspect_Abort
	virtual void Abort()
		{
		ZAcqMtx acq(fMutex);
		fClosed = true;
		fBuffer.clear();
		fCondition_Read.Broadcast();
		fCondition_Write.Broadcast();
		}

// From ChanAspect_DisconnectRead
	virtual bool DisconnectRead(double iTimeout)
		{
		double deadline = Time::sSystem() + iTimeout;
		ZAcqMtx acq(fMutex);
		for (;;)
			{
			if (fBuffer.size())
				{
				fBuffer.clear();
				fCondition_Write.Broadcast();
				}

			if (fClosed)
				return true;

			if (not fCondition_Read.WaitUntil(fMutex, deadline))
				return false;
			}
		}

// From ChanAspect_DisconnectWrite
	virtual void DisconnectWrite()
		{
		ZAcqMtx acq(fMutex);
		if (not fClosed)
			{
			fClosed = true;
			fCondition_Read.Broadcast();
			fCondition_Write.Broadcast();
			}
		}

// From ChanAspect_Read<EE>
	virtual size_t Read(EE* oDest, size_t iCount)
		{
		ZAcqMtx acq(fMutex);
		EE* localDest = oDest;
		++fUserCount;
		if (iCount)
			{
			for (;;)
				{
				const size_t countToCopy = std::min(iCount, fBuffer.size());
				if (countToCopy == 0)
					{
					if (fClosed)
						break;
					fCondition_Read.Wait(fMutex);
					}
				else
					{
					std::copy_n(fBuffer.begin(), countToCopy, localDest);
					fBuffer.erase(fBuffer.begin(), fBuffer.begin() + countToCopy);
					localDest += countToCopy;
					fCondition_Write.Broadcast();
					break;
					}
				}
			}

		--fUserCount;
		fCondition_UserCount.Broadcast();
		return localDest - oDest;
		}

	virtual size_t Readable()
		{
		ZAcqMtx acq(fMutex);
		return fBuffer.size();
		}

// From ChanAspect_WaitReadable
	virtual bool WaitReadable(double iTimeout)
		{
		const double deadline = Time::sSystem() + iTimeout;
		ZAcqMtx acq(fMutex);
		for (;;)
			{
			if (fBuffer.size())
				return true;
			if (not fCondition_Read.WaitUntil(fMutex, deadline))
				return false;
			}
		}

// From ChanAspect_Write<EE>
	virtual size_t Write(const EE* iSource, size_t iCount)
		{
		ZAcqMtx acq(fMutex);
		++fUserCount;
		const EE* localSource = iSource;
		while (iCount && not fClosed)
			{
			size_t countToInsert = iCount;
			if (fMaxSize)
				{
				if (fMaxSize >= fBuffer.size())
					countToInsert = std::min(iCount, fMaxSize - fBuffer.size());
				else
					countToInsert = 0;
				}

			if (countToInsert == 0)
				{
				fCondition_Write.Wait(fMutex);
				}
			else
				{
				fBuffer.insert(fBuffer.end(), localSource, localSource + countToInsert);
				localSource += countToInsert;
				iCount -= countToInsert;
				fCondition_Read.Broadcast();
				}
			}
		--fUserCount;
		fCondition_UserCount.Broadcast();
		return localSource - iSource;
		}

// Our protocol
	void Close();
	bool IsClosed();
	void Reset();

private:
	ZMtx fMutex;
	ZCnd fCondition_UserCount;
	ZCnd fCondition_Read;
	ZCnd fCondition_Write;
	bool fClosed;
	size_t fMaxSize;
	std::deque<EE> fBuffer;
	size_t fUserCount;
	};

} // namespace ZooLib

#endif // __ZooLib_ChanConnection_XX_FIFO_h__
