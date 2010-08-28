/* -------------------------------------------------------------------------------------------------
Copyright (c) 2010 Andrew Green
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

#include "zoolib/ZWorkerRunner_EventLoop.h"

#include "zoolib/ZLog.h"
#include "zoolib/ZUtil_STL.h"

namespace ZooLib {

// =================================================================================================
#pragma mark -
#pragma mark * ZWorkerRunner_EventLoop::Worker_Waker

class ZWorkerRunner_EventLoop::Worker_Waker : public ZWorker
	{
public:
	Worker_Waker(ZRef<ZWorkerRunner_EventLoop> iRunner);

	virtual bool Work();

	ZWeakRef<ZWorkerRunner_EventLoop> fRunner;
	};

ZWorkerRunner_EventLoop::Worker_Waker::Worker_Waker(
	ZRef<ZWorkerRunner_EventLoop> iRunner)
:	fRunner(iRunner)
	{}

bool ZWorkerRunner_EventLoop::Worker_Waker::Work()
	{
	if (ZRef<ZWorkerRunner_EventLoop> theRunner = fRunner)
		return theRunner->pTriggerCallback();
	return false;
	}

// =================================================================================================
#pragma mark -
#pragma mark * ZWorkerRunner_EventLoop

/**
\class ZWorkerRunner_EventLoop
\ingroup Worker
\sa Worker
*/

ZWorkerRunner_EventLoop::ZWorkerRunner_EventLoop()
:	fCallbackTriggered(false)
	{}

ZWorkerRunner_EventLoop::~ZWorkerRunner_EventLoop()
	{
	if (fWorker_Waker)
		fWorker_Waker->Wake();
	}

void ZWorkerRunner_EventLoop::Wake(ZRef<ZWorker> iWorker)
	{ this->pWake(iWorker, 0); }

void ZWorkerRunner_EventLoop::WakeAt(ZRef<ZWorker> iWorker, ZTime iSystemTime)
	{ this->pWake(iWorker, iSystemTime); }

void ZWorkerRunner_EventLoop::WakeIn(ZRef<ZWorker> iWorker, double iInterval)
	{ this->pWake(iWorker, ZTime::sSystem() + iInterval); }

bool ZWorkerRunner_EventLoop::IsAwake(ZRef<ZWorker> iWorker)
	{
	ZAcqMtxR acq(fMtx);
	return fWorkersMap[iWorker] <= ZTime::sSystem();
	}

void ZWorkerRunner_EventLoop::pAttach(ZRef<ZWorker> iWorker)
	{
	ZAcqMtxR acq(fMtx);
	if (ZWorkerRunner::pAttachWorker(iWorker))
		{
		fWorkersSet.Insert(iWorker);
		ZUtil_STL::sInsertMustNotContain(1,
			fWorkersMap, iWorker, ZTime::sSystem() + ZTime::kYear);
		}
	}

void ZWorkerRunner_EventLoop::pCallback()
	{
	// Not all subclasses invoke this through a ZRef, so ensure our reference remains valid.
	ZRef<ZWorkerRunner_EventLoop> self = this;

	ZGuardRMtxR guard(fMtx);
	fCallbackTriggered = false;
	guard.Release();

	for (ZSafeSetIter<ZRef<ZWorker> > iter = fWorkersSet;;)
		{
		if (ZRef<ZWorker> theWorker = iter.ReadInc())
			{
			guard.Acquire();
			ZTime theTime = fWorkersMap[theWorker];
			guard.Release();

			if (theTime <= ZTime::sSystem())
				{
				if (!this->pInvokeWork(theWorker))
					{
					guard.Acquire();
					fWorkersSet.Erase(theWorker);
					ZUtil_STL::sEraseMustContain(1, fWorkersMap, theWorker);
					guard.Release();
					ZWorkerRunner::pDetachWorker(theWorker);
					}
				}
			}
		else
			{
			break;
			}
		}
	}

bool ZWorkerRunner_EventLoop::pTriggerCallback()
	{
	ZAcqMtxR acq(fMtx);
	if (!fCallbackTriggered)
		{
		fCallbackTriggered = true;
		this->pQueueCallback();
		}
	return true;
	}

void ZWorkerRunner_EventLoop::pWake(ZRef<ZWorker> iWorker, ZTime iSystemTime)
	{
	ZAcqMtxR acq(fMtx);
	fWorkersMap[iWorker] = iSystemTime;
	if (!fWorker_Waker)
		{
		fWorker_Waker = new Worker_Waker(this);
		sStartWorkerRunner(fWorker_Waker);
		}
	fWorker_Waker->WakeAt(iSystemTime);
	}

} // namespace ZooLib
