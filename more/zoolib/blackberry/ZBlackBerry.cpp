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

#include "zoolib/blackberry/ZBlackBerry.h"

#include "zoolib/ZLog.h"

using std::string;

namespace ZooLib {

namespace ZBlackBerry {

// =================================================================================================
#pragma mark -
#pragma mark * ZBlackBerry::Manager

/**
\class Manager
\ingroup BlackBerry
*/

Manager::Manager()
	{}

Manager::~Manager()
	{}

void Manager::RegisterManagerChanged(ZRef<CB_ManagerChanged> iCallback)
	{ fCallbacks.Register(iCallback); }

void Manager::UnregisterManagerChanged(ZRef<CB_ManagerChanged> iCallback)
	{ fCallbacks.Unregister(iCallback); }

void Manager::pChanged()
	{ fCallbacks.Invoke(this); }

// =================================================================================================
#pragma mark -
#pragma mark * ZBlackBerry::Device

/**
\class Device
\ingroup BlackBerry
*/

Device::Device()
	{}

Device::~Device()
	{}

ZRef<Channel> Device::Open(
	const string& iName, const PasswordHash* iPasswordHash, Error* oError)
	{ return this->Open(false, iName, iPasswordHash, oError); }

uint32 Device::GetPIN()
	{ return 0; }

void Device::RegisterDeviceFinished(ZRef<CB_DeviceFinished> iCallback)
	{ fCallbacks.Register(iCallback); }

void Device::UnregisterDeviceFinished(ZRef<CB_DeviceFinished> iCallback)
	{ fCallbacks.Unregister(iCallback); }

void Device::pFinished()
	{ fCallbacks.Invoke(this); }

// =================================================================================================
#pragma mark -
#pragma mark * ZBlackBerry::Channel

/**
\class Channel
\ingroup BlackBerry
*/

Channel::Channel()
	{}

Channel::~Channel()
	{}

size_t Channel::GetIdealSize_Read()
	{
	return 4096;
	}

size_t Channel::GetIdealSize_Write()
	{
	return 4096;
	}

} // namespace ZBlackBerry

} // namespace ZooLib
