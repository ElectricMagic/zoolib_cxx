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

#ifndef __ZWorkerRunner_CFRunLoop__
#define __ZWorkerRunner_CFRunLoop__ 1
#include "zconfig.h"
#include "zoolib/ZCONFIG_SPI.h"

#include "zoolib/ZCaller_CFRunLoop.h"

#if ZCONFIG_API_Enabled(Caller_CFRunLoop)

#include "ZWorkerRunner_Caller.h"

namespace ZooLib {
namespace ZWorkerRunner_CFRunLoop {

// =================================================================================================
#pragma mark -
#pragma mark * ZWorkerRunner_CFRunLoop

ZRef<ZWorkerRunner_Caller> sMain();

} // namespace ZWorkerRunner_CFRunLoop
} // namespace ZooLib

#endif // ZCONFIG_API_Enabled(Caller_CFRunLoop)

#endif // __ZWorkerRunner_CFRunLoop__
