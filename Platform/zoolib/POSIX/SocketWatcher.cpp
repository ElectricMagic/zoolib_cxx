// Copyright (c) 2013 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/POSIX/SocketWatcher.h"

#include "zoolib/ZMACRO_foreach.h"

// We may want to override FD_SETSIZE, so we can handle > 1024 fds at once.

// See comment in Solaris' /usr/include/sys/ioctl.h
#if __sun__
	#define BSD_COMP
#endif

#include <sys/ioctl.h>
#include <sys/select.h>

#include <set>

#ifndef FD_COPY
	#include "zoolib/Memory.h"
	#define FD_COPY(a, b) sMemCopy(a, b, sizeof(fd_set))
#endif

namespace ZooLib {

using std::pair;
using std::set;

// =================================================================================================
#pragma mark - SocketWatcher

SocketWatcher::SocketWatcher()
:	fThreadRunning(false)
	{}

bool SocketWatcher::QInsert(const Pair_t& iPair)
	{
	ZAcqMtx acq(fMtx);
	if (not fSet.insert(iPair).second)
		return false;

	if (not fThreadRunning)
		{
		fThreadRunning = true;
		ZThread::sStart_T<SocketWatcher*>(spRun, this);
		}
	fCnd.Broadcast();
	return true;
	}

bool SocketWatcher::QErase(const Pair_t& iPair)
	{
	ZAcqMtx acq(fMtx);
	return fSet.erase(iPair);
	}

bool SocketWatcher::QInsert(int iSocket, const ZP<Callable_Void>& iCallable)
	{ return this->QInsert(Pair_t(iSocket, iCallable)); }

bool SocketWatcher::QErase(int iSocket, const ZP<Callable_Void>& iCallable)
	{ return this->QErase(Pair_t(iSocket, iCallable)); }

void SocketWatcher::pRun()
	{
	ZAcqMtx acq(fMtx);
	for (;;)
		{
		if (fSet.empty())
			{
			// Nothing pending, wait 100ms in case something else comes along.
			fCnd.WaitFor(fMtx, 0.1);
			if (fSet.empty())
				{
				// Still nothing pending, exit thread.
				fThreadRunning = false;
				break;
				}
			}
		else
			{
			fd_set readSet;
			FD_ZERO(&readSet);
			int largest = 0;
			foreacha (entry, fSet)
				{
				const int theFD = entry.first;
				if (largest < theFD)
					largest = theFD;
				FD_SET(entry.first, &readSet);
				}

			fd_set exceptSet;
			FD_COPY(&readSet, &exceptSet);

			struct timeval timeout;
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			int count;
			{
			ZRelMtx rel(fMtx);
			count = ::select(largest + 1, &readSet, nullptr, &exceptSet, &timeout);
			}

			if (count < 1)
				{
				// Didn't get any
				continue;
				}

			// Gather the callables
			set<ZP<Callable_Void>> toCall;

			for (int fd = 1; fd <= largest; ++fd)
				{
				if (FD_ISSET(fd, &readSet))
					{
					--count;
					if (FD_ISSET(fd, &exceptSet))
						--count;
					}
				else if (FD_ISSET(fd, &exceptSet))
					{ --count; }
				else
					{ continue; }

				const Set_t::iterator iterBegin = fSet.lower_bound(Pair_t(fd, null));
				Set_t::iterator iter = iterBegin;
				const Set_t::iterator iterEnd = fSet.end();
				while (iter->first == fd and iter != iterEnd)
					{
					if (iter->second)
						toCall.insert(iter->second);
					++iter;
					}
				fSet.erase(iterBegin, iterEnd);

				if (count <= 0)
					break;
				}

			ZRelMtx rel(fMtx);
			foreacha (entry, toCall)
				{
				try { entry->Call(); }
				catch (...) {}
				}
			}
		}
	}

void SocketWatcher::spRun(SocketWatcher* iSocketWatcher)
	{
	ZThread::sSetName("SocketWatcher");

	iSocketWatcher->pRun();
	}

} // namespace ZooLib
