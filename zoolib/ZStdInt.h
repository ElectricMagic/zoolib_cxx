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

#ifndef __ZStdInt__
#define __ZStdInt__
#include "zconfig.h"

// The usual suite of types with known sizes and signedness.

#if defined(ZProjectHeader_StdInt)

	// zconfig.h has defined the name of a project-specific header
	// providing definitions of our standard integer types.

	#include ZProjectHeader_StdInt

#else // defined(ZProjectHeader_StdInt)

#include "zoolib/ZCONFIG_SPI.h"

#if ZCONFIG_SPI_Enabled(MacOSX) \
	|| ZCONFIG_SPI_Enabled(MacClassic) \
	|| ZCONFIG_SPI_Enabled(Carbon)

	#if __arm__
	#	include <MacTypes.h>
	#else
	#	include ZMACINCLUDE3(CoreServices,CarbonCore,MacTypes.h)
	#endif

#endif

#if ZCONFIG(Compiler, MSVC)

	typedef __int8 int8;
	typedef unsigned __int8 uint8;

	typedef __int16 int16;
	typedef unsigned __int16 uint16;

	#ifndef _INT32
		#define _INT32 1
		typedef __int32 int32;
	#endif

	#ifndef _UINT32
		#define _UINT32 1
		typedef unsigned __int32 uint32;
	#endif

	typedef __int64 int64;
	typedef unsigned __int64 uint64;

	#define ZINT64_C(v) (v##i64)
	#define ZUINT64_C(v) (v##ui64)

#else

	#include <stdint.h>

	typedef int8_t int8;
	typedef uint8_t uint8;

	typedef int16_t int16;
	typedef uint16_t uint16;

	#ifndef _INT32
		#define _INT32 1
		typedef int32_t int32;
	#endif

	#ifndef _UINT32
		#define _UINT32 1
		typedef uint32_t uint32;
	#endif

	typedef int64_t int64;
	typedef uint64_t uint64;

	#define ZINT64_C(v) (v##LL)
	#define ZUINT64_C(v) (v##ULL)

#endif

typedef int64 bigtime_t;

#endif // defined(ZStdInt_ProjectHeader)

#endif // __ZStdInt__
