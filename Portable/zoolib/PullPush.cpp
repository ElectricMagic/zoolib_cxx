/* -------------------------------------------------------------------------------------------------
Copyright (c) 2018 Andrew Green
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

#include "zoolib/PullPush.h"
#include "zoolib/ChanConnection_XX_MemoryPipe.h"
#include "zoolib/Channer_Bin.h"
#include "zoolib/Channer_UTF.h"
#include "zoolib/Util_Chan.h" // For sCopyFully

namespace ZooLib {
namespace PullPush {

const Any kStartMap = Any(StartMap());
const Any kStartSeq = Any(StartSeq());

const Any kEnd = Any(End());

} // namespace PullPush

void sPush(const Any& iVal, const ChanW_Any& iChanW)
	{ sEWrite<Any>(iChanW, iVal); }

void sPull_UTF_Push(const ChanR_UTF& iChanR, const ChanW_Any& iChanW)
	{
	ZRef<Channer<ChanConnection<UTF32>>> theChannerPipe =
		new Channer_T<ChanConnection_XX_MemoryPipe<UTF32>>;
	sPush(ZRef<ChannerR_UTF>(theChannerPipe), iChanW);
	sCopyAll(iChanR, *theChannerPipe);
	sDisconnectWrite(*theChannerPipe);
	sSkipAll(iChanR);
	}

void sPull_UTF_Push(const ChanR_UTF& iChanR, uint64 iCount, const ChanW_Any& iChanW)
	{
	ZRef<Channer<ChanConnection<UTF32>>> theChannerPipe =
		new Channer_T<ChanConnection_XX_MemoryPipe<UTF32>>;
	sPush(ZRef<ChannerR_UTF>(theChannerPipe), iChanW);
	std::pair<uint64,uint64> counts = sCopyFully(iChanR, *theChannerPipe, iCount);
	sDisconnectWrite(*theChannerPipe);
	sSkipFully(iChanR, iCount - counts.first);
	}

void sPull_Bin_Push(const ChanR_Bin& iChanR, const ChanW_Any& iChanW)
	{
	ZRef<Channer<ChanConnection<byte>>> theChannerPipe =
		new Channer_T<ChanConnection_XX_MemoryPipe<byte>>;
	sPush(ZRef<ChannerR_Bin>(theChannerPipe), iChanW);
	sCopyAll(iChanR, *theChannerPipe);
	sDisconnectWrite(*theChannerPipe);
	sSkipAll(iChanR);
	}

void sPull_Bin_Push(const ChanR_Bin& iChanR, uint64 iCount, const ChanW_Any& iChanW)
	{
	ZRef<Channer<ChanConnection<byte>>> theChannerPipe =
		new Channer_T<ChanConnection_XX_MemoryPipe<byte>>;
	sPush(ZRef<ChannerR_Bin>(theChannerPipe), iChanW);
	std::pair<uint64,uint64> counts = sCopyFully(iChanR, *theChannerPipe, iCount);
	sDisconnectWrite(*theChannerPipe);
	sSkipFully(iChanR, iCount - counts.first);
	}

} // namespace ZooLib
