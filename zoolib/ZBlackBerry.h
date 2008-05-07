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

#ifndef __ZBlackBerry__
#define __ZBlackBerry__ 1
#include "zconfig.h"

#include "ZMemoryBlock.h"
#include "ZStreamer.h"

#include <set>

namespace ZBlackBerry {

struct PasswordHash
	{
	uint8 fData[20];
	};

class Device;
class Channel;

// =================================================================================================
#pragma mark -
#pragma mark * ZBlackBerry::Manager

class Manager : public ZRefCountedWithFinalization
	{
protected:
	Manager();

public:
	virtual ~Manager();

// From ZRefCountedWithFinalization
	virtual void Initialize();
	virtual void Finalize();

// Our protocol
	virtual void Start();
	virtual void Stop();

	virtual void GetDeviceIDs(vector<uint64>& oDeviceIDs) = 0;
	virtual ZRef<Device> Open(uint64 iDeviceID) = 0;

	class Observer
		{
	public:
		virtual void ManagerChanged(ZRef<Manager> iManager) = 0;
		};

	void ObserverAdd(Observer* iObserver);
	void ObserverRemove(Observer* iObserver);

protected:
	void pNotifyObservers();

	void pStarted();
	void pStopped();

private:
	ZMutex fMutex;
	ZCondition fCondition;
	int fStartCount;
	set<Observer*> fObservers;
	};

// =================================================================================================
#pragma mark -
#pragma mark * ZBlackBerry::Device

class Device : public ZRefCountedWithFinalization
	{
protected:
	Device();

public:
	virtual ~Device();

// From ZRefCountedWithFinalization
	virtual void Initialize();
	virtual void Finalize();

// Our protocol
	virtual void Start();
	virtual void Stop();

	enum Error
		{
		error_None,
		error_DeviceClosed,
		error_UnknownChannel,
		error_PasswordNeeded,
		error_PasswordExhausted,
		error_PasswordIncorrect
		};

	virtual ZRef<Channel> Open(
		const string& iName, const PasswordHash* iPasswordHash, Error* oError) = 0;
	virtual ZMemoryBlock GetAttribute(uint16 iObject, uint16 iAttribute) = 0;

	class Observer
		{
	public:
		virtual void Finished(ZRef<Device> iDevice) = 0;
		};

	void ObserverAdd(Observer* iObserver);
	void ObserverRemove(Observer* iObserver);

protected:
	void pFinished();

private:
	ZMutex fMutex;
	ZCondition fCondition;
	int fStartCount;
	set<Observer*> fObservers;
	};

// =================================================================================================
#pragma mark -
#pragma mark * ZBlackBerry::Channel

class Channel : public ZStreamerRWCon
	{
protected:
	Channel();

public:
	virtual ~Channel();
	};

} // namespace ZBlackBerry

#endif // __ZBlackBerry__
