// Copyright (c) 2011 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_Server_Roster_h__
#define __ZooLib_Server_Roster_h__ 1
#include "zconfig.h"

#include "zoolib/Callable.h"
#include "zoolib/Compat_NonCopyable.h"

#include <set>

namespace ZooLib {

// =================================================================================================
#pragma mark - Roster

class Roster
:	public Counted
,	NonCopyable
	{
public:
	class Entry;

	Roster();

	Roster(const ZP<Callable_Void>& iCallable_Change,
		const ZP<Callable_Void>& iCallable_Gone);

	virtual ~Roster();

// From Counted
	virtual void Finalize();

// Our protocol
	ZP<Entry> MakeEntry();

	ZP<Entry> MakeEntry(const ZP<Callable_Void>& iCallable_Broadcast,
		const ZP<Callable_Void>& iCallable_Gone);
	
	void Broadcast();

	size_t Count();
	void Wait(size_t iCount);
	bool WaitFor(double iTimeout, size_t iCount);
	bool WaitUntil(double iDeadline, size_t iCount);

private:
	void pFinalizeEntry(Entry* iEntry, const ZP<Callable_Void>& iCallable_Gone);

	ZMtx fMtx;
	ZCnd fCnd;
	std::set<Entry*> fEntries;
	const ZP<Callable_Void> fCallable_Change;
	const ZP<Callable_Void> fCallable_Gone;

	friend class Entry;
	};

// =================================================================================================
#pragma mark - Roster::Entry

class Roster::Entry
:	public Counted
,	NonCopyable
	{
private:
	Entry(
		const ZP<Roster>& iRoster,
		const ZP<Callable_Void>& iCallable_Broadcast,
		const ZP<Callable_Void>& iCallable_Gone);

public:
	virtual ~Entry();

// From Counted
	virtual void Finalize();

private:
	WP<Roster> fRoster;

	const ZP<Callable_Void> fCallable_Broadcast;
	const ZP<Callable_Void> fCallable_Gone;

	friend class Roster;
	};

} // namespace ZooLib

#endif // __ZooLib_Server_Roster_h__
