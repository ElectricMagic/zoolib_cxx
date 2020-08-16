// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_Compat_MSVCStaticLib_h__
#define __ZooLib_Compat_MSVCStaticLib_h__ 1
#include "zconfig.h"

/**
\file
\sa FunctionChain.h
\section Background

Static variables in a translation unit are initialized before any regular code in the
translation unit executes. In practice the initialization happens when the containing
executable or dynamic library is loaded. When your \c main() is called, or your call to
<code>LoadLibrary()/dlopen()</code> completes, any static variables will have been initialized.

\section TheProblem The Problem
As described by <a href="http://msdn.microsoft.com/en-us/library/5tkz6s71(v=VS.80).aspx">MSDN</a>:
	\par
	<em>Constructors and assignment by global function or static methods in the declaration do
	not create a reference and will not prevent /OPT:REF elimination. Side effects from such
	code should not be depended on when no other references to the data exist.</em>

It can be convenient to place the object code from multiple translation units in a single
file, a static library conventionally named with a \c .lib or \c .a suffix. The MSVC linker does
dependency analysis on static libraries and will not include code that is not referenced
by the including entity.

The common pattern of using a static variable to declare and cause the registration of
a factory object can fail in this circumstance -- the MSVC linker deems the static as
being unreachable and strips it from the result.

\section Solutions
A useful google search: http://www.google.com/search?q=msvc+factory+static+library

One solution is to set the <code>/OPT:NOREF</code> linker flag on the including entity. However,
this is an all or nothing setting, and will require that all included libraries be fully linkable.

If something in the file containing the static is referenced (directly or indirectly) by
the including entity, then by the language rules the static itself must be preserved.

The most basic approach is to put a dummy function in the file, and reference that from
somewhere known to be considered reachable.

Another approach is to use the \c /INCLUDE linker flag to reference an entity in the problem file.
Assuming an entity named \c DummyForLinkProblem, this can be done in the including entity's source:
\code #pragma comment(linker, "/include:DummyForLinkProblem") \endcode

\section ZooLibSolution ZooLib's Solution
ZooLib entities currently affected by this problem are those in
ZFile_Win.cpp, ZGRgnRep_HRGN.cpp, ZNet_Internet_WinSock.cpp, ZStreamRWCon_SSL_Win.cpp,
ZTextCoder_Win.cpp and ZUnicode_Normalize_Win.cpp.

We \#include ZCompat_MSVCStaticLib.h in the corresponding header files, and put in each a
\c ZMACRO_MSVCStaticLib_Reference(ModifiedFileName). In the cpp files we put a
\c ZMACRO_MSVCStaticLib_cpp(ModifiedFileName). The ModifiedFileName is generally the
filename with the leading Z and file extension removed, the same style as
used in \c ZCONFIG_API_XXX macros.

To ensure that your executable or library does not strip these entities, simply \#include
the appropriate header file from known referenced code in your including entity. This will
cause a non-executing reference to occur, and things will work as expected.

2014-10-28. This problem also occurs with gcc/clang static libs on iOS.
*/

// =================================================================================================
#pragma mark - ZCompat_MSVCStaticLib

#if not defined(ZMACRO_MSVCStaticLib_Reference)
	#define ZMACRO_MSVCStaticLib_Reference(distinguisher_p) \
		namespace ZooLib { \
		namespace MSVCStaticLib { \
		namespace distinguisher_p { \
		void sDummyFunction(); \
		static struct DummyClass \
			{ DummyClass() { sDummyFunction();} } sDummyClass; \
		} /* namespace distinguisher_p */ \
		} /* namespace MSVCStaticLib */ \
		} /* namespace ZooLib */
#endif

#if not defined(ZMACRO_MSVCStaticLib_cpp)
	#define ZMACRO_MSVCStaticLib_cpp(distinguisher_p) \
		namespace ZooLib { \
		namespace MSVCStaticLib { \
		namespace distinguisher_p { \
		void sDummyFunction() {} \
		} /* namespace distinguisher_p */ \
		} /* namespace MSVCStaticLib */ \
		} /* namespace ZooLib */
#endif

#endif // __ZooLib_Compat_MSVCStaticLib_h__
