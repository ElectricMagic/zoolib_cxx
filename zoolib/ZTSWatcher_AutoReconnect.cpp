/* -------------------------------------------------------------------------------------------------
Copyright (c) 2007 Andrew Green and Learning in Motion, Inc.
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

#include "ZTSWatcher_AutoReconnect.h"

#include "ZDebug.h"
#include "ZLog.h"
#include "ZUtil_STL.h"

using ZUtil_STL::sFirstOrNil;

using std::set;
using std::vector;

#define kDebug_ZTSWatcher_AutoReconnect_LogRegistrations 1

// =================================================================================================
#pragma mark -
#pragma mark * ZTSWatcher_AutoReconnect

/*
The ZTSWatcher protocol is synchronous, generally driven by the sync thread of a ZTSoup. In
many circumstances a ZTSWatcher_AutoReconnect will be called not by a ZTSoup, but by a
ZTSWatcherMUX::Watcher or another complex ZTSWatcher, which cannot recover from the failure
of an underlying ZTSWatcher. So a call to ZTSWatcher_AutoReconnect::Sync or AllocateIDs will
succeed or fail irrevocably.

Internally ZTSWatcher_AutoReconnect is in one of three states:
  Dead, Failed, Established

We're [Failed] when just created, or when our underlying connection has gone
away and we're trying to reestablish it.

We're [Established] when we have a connection.

We're [Dead] when the underlying connection could not be established, or reestablished after
failure, and that this ZTSWatcher is no longer usable, and its owner should just give up.

During the execution of Sync or AllocateIDs we may temporarily transition to
[Failed], but by the time we return we'll be [Established] or [Dead].

State transitions are notified by calling the NotifyStatusChange virtual method. Subclasses
of ZTSWatcher_AutoReconnect can do what they want in NotifyStatusChange -- most commonly
trigger some UI to indicate that there's a problem, and then dismiss it when it clears up.

     Failed --> Established
Established --> Failed
     Failed --> Dead

*/

ZTSWatcher_AutoReconnect::ZTSWatcher_AutoReconnect(ZRef<ZTSWatcherFactory> iFactory)
:	fFactory(iFactory),
	fConnectionFailed(false),
	fCallback(nil)
	{}

ZTSWatcher_AutoReconnect::~ZTSWatcher_AutoReconnect()
	{}

bool ZTSWatcher_AutoReconnect::AllocateIDs(size_t iCount, uint64& oBaseID, size_t& oCountIssued)
	{
	ZMutexLocker locker(fMutex);

	if (!this->pEnsureConnection())
		return false;

	for (;;)
		{
		if (this->pAllocateIDs(iCount, oBaseID, oCountIssued))
			return true;

		if (!this->pReestablishConnection())
			return false;
		}
	}

bool ZTSWatcher_AutoReconnect::Sync(
	const uint64* iRemovedIDs, size_t iRemovedIDsCount,
	const uint64* iAddedIDs, size_t iAddedIDsCount,
	const int64* iRemovedQueries, size_t iRemovedQueriesCount,
	const AddedQueryCombo* iAddedQueries, size_t iAddedQueriesCount,
	vector<uint64>& oAddedIDs,
	vector<uint64>& oChangedTupleIDs, vector<ZTuple>& oChangedTuples,
	const uint64* iWrittenTupleIDs, const ZTuple* iWrittenTuples, size_t iWrittenTuplesCount,
	map<int64, vector<uint64> >& oChangedQueries)
	{
	ZMutexLocker locker(fMutex);

	// Update our record of our client's registrations.
	this->pUpdateRegistrations(iRemovedIDs, iRemovedIDsCount,
			iAddedIDs, iAddedIDsCount,
			iRemovedQueries, iRemovedQueriesCount,
			iAddedQueries, iAddedQueriesCount);
	
	if (!this->pEnsureConnection())
		return false;

	for (;;)
		{
		// We must have a tswatcher.
		if (fConnectionFailed)
			{
			// It's newly established, so we must send all our registrations.
			vector<uint64> addedIDs(fTupleIDs.begin(), fTupleIDs.end());
			vector<AddedQueryCombo> addedQueries(fQueries.begin(), fQueries.end());

			// Clear out our results, in case the last pSync touched them.
			oAddedIDs.clear();
			oChangedTupleIDs.clear();
			oChangedTuples.clear();
			oChangedQueries.clear();

			if (this->pSync(nil, 0,
					sFirstOrNil(addedIDs), addedIDs.size(),
					nil, 0,
					sFirstOrNil(addedQueries), addedQueries.size(),
					oAddedIDs,
					oChangedTupleIDs, oChangedTuples,
					iWrittenTupleIDs, iWrittenTuples, iWrittenTuplesCount,
					oChangedQueries))
				{
				fConnectionFailed = false;
				return true;
				}
			}
		else
			{
			if (this->pSync(iRemovedIDs, iRemovedIDsCount,
				iAddedIDs, iAddedIDsCount,
				iRemovedQueries, iRemovedQueriesCount,
				iAddedQueries, iAddedQueriesCount,
				oAddedIDs,
				oChangedTupleIDs, oChangedTuples,
				iWrittenTupleIDs, iWrittenTuples, iWrittenTuplesCount,
				oChangedQueries))
				{
				return true;
				}
			}

		if (!this->pReestablishConnection())
			return false;
		}
	}

void ZTSWatcher_AutoReconnect::SetCallback(Callback_t iCallback, void* iRefcon)
	{
	fCallback = iCallback;
	fRefcon = iRefcon;
	}

void ZTSWatcher_AutoReconnect::NotifyStatusChange(EStatus iStatus)
	{}

bool ZTSWatcher_AutoReconnect::pEnsureConnection()
	{
	if (fTSWatcher)
		{
		// We already have a connection.
		return true;
		}

	// We don't have a tswatcher.
	if (fConnectionFailed)
		{
		// But that's because an extant connection failed and could not be re-established.
		return false;
		}

	// We get here only when we're newly created and have never had a connection.
	this->NotifyStatusChange(eStatus_Failed);

	// fConnectionFailed will be set false by Sync when it has done a
	// full sync (sending all known tuple and query registrations).
	fConnectionFailed = true;

	if (this->pEstablishConnection())
		{
		this->NotifyStatusChange(eStatus_Established);
		return true;
		}
	else
		{
		this->NotifyStatusChange(eStatus_Dead);
		return false;
		}
	}

bool ZTSWatcher_AutoReconnect::pReestablishConnection()
	{
	ZAssert(!fTSWatcher);

	// The connection failed whilst we were using it.
	this->NotifyStatusChange(eStatus_Failed);

	// Remember that, so we do a full sync next time round.
	fConnectionFailed = true;

	if (this->pEstablishConnection())
		{
		this->NotifyStatusChange(eStatus_Established);
		return true;
		}
	else
		{
		this->NotifyStatusChange(eStatus_Dead);
		return false;
		}
	}

bool ZTSWatcher_AutoReconnect::pEstablishConnection()
	{
	ZAssert(!fTSWatcher);

	if (fFactory)
		fTSWatcher = fFactory->MakeTSWatcher();

	if (!fTSWatcher)
		return false;

	// Hmm. Some watchers will invoke the callback from SetCallback if one
	// is already pending. We're holding fMutex, is there the potential for deadlock?
	// I don't think so, as all tswatchers are purely reactive, so the scope
	// nests from outside in, but that may change when we switch to an async
	// ZTSWatcher_Client implementation.

	fTSWatcher->SetCallback(sCallback, this);
	return true;
	}

void ZTSWatcher_AutoReconnect::pUpdateRegistrations(
	const uint64* iRemovedIDs, size_t iRemovedIDsCount,
	const uint64* iAddedIDs, size_t iAddedIDsCount,
	const int64* iRemovedQueries, size_t iRemovedQueriesCount,
	const AddedQueryCombo* iAddedQueries, size_t iAddedQueriesCount)
	{
	if (kDebug_ZTSWatcher_AutoReconnect_LogRegistrations)
		{
		if (iRemovedIDsCount)
			{
			if (const ZLog::S& s = ZLog::S(ZLog::eDebug, "TSWatcher_AutoConnect"))
				{
				s << "pUpdateRegistrations, removed IDs: ";
				for (size_t x = 0; x < iRemovedIDsCount; ++x)
					s.Writef("%llX, ", iRemovedIDs[x]);
				}
			}

		if (iAddedIDsCount)
			{
			if (const ZLog::S& s = ZLog::S(ZLog::eDebug, "TSWatcher_AutoConnect"))
				{
				s << "pUpdateRegistrations, added IDs: ";
				for (size_t x = 0; x < iAddedIDsCount; ++x)
					s.Writef("%llX, ", iAddedIDs[x]);
				}
			}

		if (iRemovedQueriesCount)
			{
			if (const ZLog::S& s = ZLog::S(ZLog::eDebug, "TSWatcher_AutoConnect"))
				{
				s << "pUpdateRegistrations, removed queries: ";
				for (size_t x = 0; x < iRemovedQueriesCount; ++x)
					s.Writef("%llX, ", iRemovedQueries[x]);
				}
			}

		if (iAddedQueriesCount)
			{
			if (const ZLog::S& s = ZLog::S(ZLog::eDebug, "TSWatcher_AutoConnect"))
				{
				s << "pUpdateRegistrations, added queries: ";
				for (size_t x = 0; x < iAddedQueriesCount; ++x)
					s.Writef("%llX, ", iAddedQueries[x].fRefcon);
				}
			}
		}

	while (iRemovedIDsCount--)
		{
		ZUtil_STL::sEraseMustContain(kDebug, fTupleIDs, *iRemovedIDs++);
		}

	while (iAddedIDsCount--)
		{
		ZUtil_STL::sInsertMustNotContain(kDebug, fTupleIDs, *iAddedIDs++);
		}

	if (iRemovedQueriesCount)
		{
		AddedQueryCombo dummy;
		while (iRemovedQueriesCount--)
			{
			dummy.fRefcon = *iRemovedQueries++;
			ZUtil_STL::sEraseMustContain(kDebug, fQueries, dummy);
			}
		}
	
	while (iAddedQueriesCount--)
		{
		ZUtil_STL::sInsertMustNotContain(kDebug, fQueries, *iAddedQueries++);
		}
	}

bool ZTSWatcher_AutoReconnect::pAllocateIDs(size_t iCount, uint64& oBaseID, size_t& oCountIssued)
	{
	if (fTSWatcher && fTSWatcher->AllocateIDs(iCount, oBaseID, oCountIssued))
		return true;

	fTSWatcher.Clear();
	return false;
	}

bool ZTSWatcher_AutoReconnect::pSync(
	const uint64* iRemovedIDs, size_t iRemovedIDsCount,
	const uint64* iAddedIDs, size_t iAddedIDsCount,
	const int64* iRemovedQueries, size_t iRemovedQueriesCount,
	const AddedQueryCombo* iAddedQueries, size_t iAddedQueriesCount,
	vector<uint64>& oAddedIDs,
	vector<uint64>& oChangedTupleIDs, vector<ZTuple>& oChangedTuples,
	const uint64* iWrittenTupleIDs, const ZTuple* iWrittenTuples, size_t iWrittenTuplesCount,
	map<int64, vector<uint64> >& oChangedQueries)
	{
	if (fTSWatcher && fTSWatcher->Sync(
		iRemovedIDs, iRemovedIDsCount,
		iAddedIDs, iAddedIDsCount,
		iRemovedQueries, iRemovedQueriesCount,
		iAddedQueries, iAddedQueriesCount,
		oAddedIDs,
		oChangedTupleIDs, oChangedTuples,
		iWrittenTupleIDs, iWrittenTuples, iWrittenTuplesCount,
		oChangedQueries))
		{
		// Augment fTupleIDs with values from oAddedIDs.
		if (!oAddedIDs.empty())
			{
			if (kDebug_ZTSWatcher_AutoReconnect_LogRegistrations)
				{
				if (const ZLog::S& s = ZLog::S(ZLog::eDebug, "TSWatcher_AutoConnect"))
					{
					s << "pSync, server added IDs: ";
					for (vector<uint64>::iterator i = oAddedIDs.begin(); i != oAddedIDs.end(); ++i)
						s.Writef("%llX, ", *i);
					}
				}

			for (vector<uint64>::iterator i = oAddedIDs.begin(); i != oAddedIDs.end(); ++i)
				{
				ZUtil_STL::sInsertMustNotContain(kDebug, fTupleIDs, *i);
				}
			}

		return true;
		}

	fTSWatcher.Clear();
	return false;
	}

void ZTSWatcher_AutoReconnect::Callback()
	{
	if (fCallback)
		fCallback(fRefcon);
	}

void ZTSWatcher_AutoReconnect::sCallback(void* iRefcon)
	{ static_cast<ZTSWatcher_AutoReconnect*>(iRefcon)->Callback(); }

// =================================================================================================
#pragma mark -
#pragma mark * ZTSWatcherFactory_AutoReconnect

ZTSWatcherFactory_AutoReconnect::ZTSWatcherFactory_AutoReconnect(ZRef<ZTSWatcherFactory> iFactory)
:	fFactory(iFactory)
	{}

ZTSWatcherFactory_AutoReconnect::~ZTSWatcherFactory_AutoReconnect()
	{}

ZRef<ZTSWatcher> ZTSWatcherFactory_AutoReconnect::MakeTSWatcher()
	{ return new ZTSWatcher_AutoReconnect(fFactory); }
