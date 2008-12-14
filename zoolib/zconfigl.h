/* -------------------------------------------------------------------------------------------------
Copyright (c) 2000 Andrew Green and Learning in Motion, Inc.
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

#ifndef __zconfigl__
#define __zconfigl__ 1

// Ensure our definitions have been set up
#include "zoolib/zconfigd.h"

// ==================================================
#define ZCONFIG(a, b)\
	((ZCONFIG_##a##_##b) && (((ZCONFIG_##a) & (ZCONFIG_##a##_##b))==(ZCONFIG_##a##_##b)))

// Disable some ZCONFIG_XXX stuff, to flush out older code.
#undef ZCONFIG_OS
#define ZCONFIG_OS $@ZCONFIG_OS_Is_Disabled

#undef ZCONFIG_API_Graphics
#define ZCONFIG_API_Graphics $@ZCONFIG_API_Graphics_Is_Disabled

#undef ZCONFIG_API_OSWindow
#define ZCONFIG_API_OSWindow $@ZCONFIG_API_Graphics_Is_Disabled


// ==================================================
// Compiler

#ifndef ZCONFIG_Compiler
#	if 0
#	elif defined(__MWERKS__)
#		define ZCONFIG_Compiler ZCONFIG_Compiler_CodeWarrior
#	elif defined(__GNUC__)
#		define ZCONFIG_Compiler ZCONFIG_Compiler_GCC
#	elif defined(_MSC_VER)
#		define ZCONFIG_Compiler ZCONFIG_Compiler_MSVC
#	endif
#endif
#ifndef ZCONFIG_Compiler
#	error "Don't know what compiler we're using."
#endif

// ==================================================
// Which processor?

#ifndef ZCONFIG_Processor
#	if 0
#	elif defined(__MWERKS__)
#		if __POWERPC__
#			define ZCONFIG_Processor ZCONFIG_Processor_PPC
#		elif __MC68K__
#			define ZCONFIG_Processor ZCONFIG_Processor_68K
#		elif __INTEL__
#			define ZCONFIG_Processor ZCONFIG_Processor_x86
#		endif
#	elif defined(__GNUC__)
#		if defined(__i386__)
#			define ZCONFIG_Processor ZCONFIG_Processor_x86
#		elif defined(__ppc__)
#			define ZCONFIG_Processor ZCONFIG_Processor_PPC
#		elif defined(__arm__)
#			define ZCONFIG_Processor ZCONFIG_Processor_ARM
#		endif
#	elif defined(_MSC_VER)
#		define ZCONFIG_Processor ZCONFIG_Processor_x86
#	endif
#endif

#ifndef ZCONFIG_Processor
#	error "Don't know what processor we're using."
#endif

// ==================================================
// Byte order

#ifndef ZCONFIG_Endian
#	if (ZCONFIG_Processor == ZCONFIG_Processor_x86)
#		define ZCONFIG_Endian ZCONFIG_Endian_Little
#	elif (ZCONFIG_Processor == ZCONFIG_Processor_ARM)
#		define ZCONFIG_Endian ZCONFIG_Endian_Little
#	else
#		define ZCONFIG_Endian ZCONFIG_Endian_Big
#	endif
#endif

// ==================================================
// We have two different defines for debugging. ZCONFIG_DebugLevel is what type of debugging
// is wanted for a debug build (one with SYM information being generated). 0 generally means
// almost no debug code will be generated, although it's legal to have assertions that are always
// present in the code, even in a level 0 build.
// Level 1 is for lighter weight debugging -- useful for code issued to external sites, code which
// has had all the serious panic situations handled for heavily-executed code, but which may still
// have lurking problems in less busy parts of the app. Level 2 is for development. Remember, these
// are suggested values only, and the ZDebug suite of macros lets you determine whether we should
// panic and kill the app, or just fprintf(stderr, ..) and go on our way.

#ifndef ZCONFIG_DebugLevel
#	define ZCONFIG_DebugLevel 1
#endif

// The second define is ZCONFIG_Debug, which takes its value from ZCONFIG_DebugLevel if we're doing
// a SYM build, otherwise it  is set to 0. You can override ZCONFIG_Debug by setting its value in
// your zconfig.h before including this file.
#ifndef ZCONFIG_Debug
#	if 0
#	elif defined(__MWERKS__)
#		if __option(sym)
#			define ZCONFIG_Debug ZCONFIG_DebugLevel
#		else
#			define ZCONFIG_Debug 0
#		endif
#	elif defined(_MSC_VER)
#		if defined(_DEBUG)
#			define ZCONFIG_Debug ZCONFIG_DebugLevel
#		else
#			define ZCONFIG_Debug 0
#		endif
#	else
#		if defined(DEBUG)
#			define ZCONFIG_Debug ZCONFIG_DebugLevel
#		else
#			define ZCONFIG_Debug 0
#		endif
#	endif
#endif

#ifndef ZCONFIG_Debug
#	error "Can't figure out if we're debugging."
#endif

// ==================================================
// Declare namespace std and use it -- injecting all std names into global
// namespace. A necessary hack until we've gone through all source and done this on
// a case by case basis. This can be switched *off* by defining ZCONFIG_NamespaceHack
// as zero in your zconfig.h file.

#ifndef ZCONFIG_NamespaceHack
#	define ZCONFIG_NamespaceHack 1
#endif

#ifndef SKIPOMPARSE
#	if ZCONFIG_NamespaceHack && defined(__cplusplus)
		namespace std {}
		using namespace std;
#	endif
#endif

// Object Master doesn't parse anonymous namespaces, so we define
// this symbol and use it instead.
#define ZANONYMOUS

// ==================================================
// Some extra bits to patch up some CodeWarrior issues.
#if defined(__MWERKS__)

#	if __option(precompile)
#		error "Don't precompile zconfig"
// because settings on individual files will not necessarily match claimed configuration settings.
#	endif

// This definition causes some problematic math-related stuff to drop out. Seems to
// be only a CW/10.3.9 problem.
#	define __NOEXTENSIONS__

#	ifndef NEWMODE
#		define NEWMODE NEWMODE_MALLOC
#	endif

#	define MSL_USE_PRECOMPILED_HEADERS 0

#	if ZCONFIG_Debug
		// Switch off (v. nice) collapse of T* to void* in containers, so we can
		// actually work with the contents when debugging.
#		define _Inhibit_Container_Optimization
		// Switch off the placement of Red/Black flag as a single bit in the parent pointer.
#		define _Inhibit_Optimize_RB_bit
		// Enable debugging checks. This doesn't work for me yet, so I'm leaving it off.
//#		define _MSL_DEBUG
#	endif

	// Apple's headers on 10.3 define SCHAR_MAX and CHAR_BIT in terms of __SCHAR_MAX__
	// and __CHAR_BIT__, which are compiler-provided by gcc 3.3. There's a fixup for
	// SHRT, INT, LONG and LONG_LONG in /usr/include/gcc/darwin/3.3/machine/limit.h,
	// but not for these two. So we'll do it ourselves for now.
#	define __SCHAR_MAX__ 127
#	define __CHAR_BIT__ 8

#endif

// ==================================================
// Some extra bits to patch up some MSVC issues.
#if defined(_MSC_VER)

#	if !defined(_MT)
#		error "_MT is not defined. ZooLib requires thread safe libraries and headers."
#	endif

	// Ghastly workaround for MSVC non-compliance with C++ standard
#	define for if (0) {} else for

	// Disable warning "illegal pragma" -- we use pragma mark fairly extensively
#	pragma warning(disable:4068)

	// Disable warning "debug info truncated to 255 characters"
	// (STL template expansion will often generate this)
#	pragma warning(disable:4786)

	// Disable warning "'this' used in base member initializer list" -- we
	// know what we're doing when we do this.
#	pragma warning(disable:4355)

	// Disable warning "forcing value to bool 'true' or 'false'"
#	pragma warning(disable:4800)

#endif

// ==================================================
// Define _REENTRANT for all platforms, although it's only significant for POSIX.
#ifndef _REENTRANT
#	define _REENTRANT
#endif

// ==================================================
/* We use the macro 'nil' extensively in ZooLib. It's something of a historical accident,
basically the result of an early collaborator (Cyrus Ghalambor) having previously worked in
a pascal-centric environment. NULL might more normally be expected, but its official C
definition makes it problematic as a subsitute for a null pointer. And there's a problem
with having nil be simply zero, in that it then cannot be distinguished from a zero integer.
Herb Sutter's column in "C/C++ User's Journal", May 2004 discusses 'nullptr', which will likely
end up part of C++ in the next standardization cycle. In the meantime I'm re-defining nil from
being zero to something like nullptr. */

#ifdef __cplusplus
class nil_t
	{
public:
	template <class T> operator T*() const { return 0; }
// CW doesn't like this templated member function pointer conversion. And
// we don't really use member function pointers currently.
//	template <class C, class T> operator T C::*() const { return 0; }
private:
	void operator&() const;
	};

#	ifdef nil
#		undef nil
#	endif
#	define nil nil_t()
#else
#	ifndef nil
#		define nil 0
#	endif
#endif

// ==================================================
#if __MACH__
	#define ZMACINCLUDE2(a,b) <a/b>
	#if __MWERKS__
		#define ZMACINCLUDE3(a,b,c) <b/c>
	#else
		#define ZMACINCLUDE3(a,b,c) <a/../Frameworks/b.framework/Headers/c>
	#endif
#else
	#define ZMACINCLUDE2(a,b) <b>
	#define ZMACINCLUDE3(a,b,c) <c>
#endif

#endif // __zconfigl__
