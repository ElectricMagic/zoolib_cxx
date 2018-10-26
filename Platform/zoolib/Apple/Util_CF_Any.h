/* -------------------------------------------------------------------------------------------------
Copyright (c) 2014 Andrew Green
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

#ifndef __ZooLib_Apple_Util_CF_Any_h__
#define __ZooLib_Apple_Util_CF_Any_h__ 1
#include "zconfig.h"
#include "zoolib/ZCONFIG_SPI.h"

#if ZCONFIG_SPI_Enabled(CFType)

#include "zoolib/Val_Any.h"

#include "zoolib/Apple/ZRef_CF.h"

namespace ZooLib {
namespace Util_CF {

// =================================================================================================
#pragma mark -
#pragma mark Util_CF

ZQ<Any> sQSimpleAsAny(CFTypeID iTypeID, CFTypeRef iVal);
ZQ<Any> sQAsAny(CFTypeRef iVal);
Any sDAsAny(const Any& iDefault, CFTypeRef iVal);
Any sAsAny(CFTypeRef iVal);

ZRef<CFTypeRef> sDAsCFType(CFTypeRef iDefault, const Any& iVal);
ZRef<CFTypeRef> sAsCFType(const Any& iVal);

Seq_Any sAsSeq_Any(const Any& iDefault, CFArrayRef iCFArray);
Map_Any sAsMap_Any(const Any& iDefault, CFDictionaryRef iCFDictionary);

} // namespace Util_CF
} // namespace ZooLib

#endif // ZCONFIG_SPI_Enabled(CFType)

#endif // __ZooLib_Apple_Util_CF_Any_h__
