// Copyright (c) 2011 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_Apple_Callable_Block_h__
#define __ZooLib_Apple_Callable_Block_h__ 1
#include "zconfig.h"

#include "zoolib/Callable.h"

#if defined(__BLOCKS__)

#include <Block.h>

namespace ZooLib {

// =================================================================================================
#pragma mark - Callable

template <class Signature> class Callable_Block;

// =================================================================================================
#pragma mark - Callable (specialization for 0 params, void return)
// This is the only variant usable before Clang 2.0.

template <>
class Callable_Block<void()>
:	public Callable<void()>
	{
public:
	typedef void (^BlockPtr_t)();

	Callable_Block(BlockPtr_t iBlockPtr)
	:	fBlockPtr(Block_copy(iBlockPtr))
		{}

	virtual ~Callable_Block()
		{ Block_release(fBlockPtr); }

// From Callable
	virtual bool QCall()
		{
		fBlockPtr();
		return true;
		}

private:
	BlockPtr_t fBlockPtr;
	};

#if defined(__clang_major__) && __clang_major__ >= 2

// =================================================================================================
#pragma mark - Callable (specialization for 0 params)

template <class R>
class Callable_Block<R()>
:	public Callable<R()>
	{
public:
	typedef R (^BlockPtr_t)();

	Callable_Block(BlockPtr_t iBlockPtr)
	:	fBlockPtr(Block_copy(iBlockPtr))
		{}

	virtual ~Callable_Block()
		{ Block_release(fBlockPtr); }

// From Callable
	virtual ZQ<R> QCall()
		{ return fBlockPtr(); }

private:
	BlockPtr_t fBlockPtr;
	};

// =================================================================================================
#pragma mark - Callable (specialization for 1 param)

#define ZMACRO_Callable_Callable(X) \
\
template <class R, ZMACRO_Callable_Class_P##X> \
class Callable_Block<R(ZMACRO_Callable_P##X)> \
:	public Callable<R(ZMACRO_Callable_P##X)> \
	{ \
public: \
	typedef R (^BlockPtr_t)(ZMACRO_Callable_P##X); \
\
	Callable_Block(BlockPtr_t iBlockPtr) \
	:	fBlockPtr(Block_copy(iBlockPtr)) \
		{} \
\
	virtual ~Callable_Block() \
		{ Block_release(fBlockPtr); } \
\
	virtual ZQ<R> QCall(ZMACRO_Callable_Pi##X) \
		{ return fBlockPtr(ZMACRO_Callable_i##X); } \
\
private: \
	BlockPtr_t fBlockPtr; \
	};

ZMACRO_Callable_Callable(0)
ZMACRO_Callable_Callable(1)
ZMACRO_Callable_Callable(2)
ZMACRO_Callable_Callable(3)
ZMACRO_Callable_Callable(4)
ZMACRO_Callable_Callable(5)
ZMACRO_Callable_Callable(6)
ZMACRO_Callable_Callable(7)
ZMACRO_Callable_Callable(8)
ZMACRO_Callable_Callable(9)
ZMACRO_Callable_Callable(A)
ZMACRO_Callable_Callable(B)
ZMACRO_Callable_Callable(C)
ZMACRO_Callable_Callable(D)
ZMACRO_Callable_Callable(E)
ZMACRO_Callable_Callable(F)

#undef ZMACRO_Callable_Callable

// =================================================================================================
#pragma mark - sCallable

template <class R>
ZP<Callable<R()>>
sCallable(R(^iBlockPtr)())
	{
	if (iBlockPtr)
		return new Callable_Block<R()>(iBlockPtr);
	return null;
	}

#define ZMACRO_Callable_sCallable(X) \
\
template <class R, ZMACRO_Callable_Class_P##X> \
ZP<Callable<R(ZMACRO_Callable_P##X)>> \
sCallable(R(^iBlockPtr)(ZMACRO_Callable_P##X)) \
	{ \
	if (iBlockPtr) \
		return new Callable_Block<R(ZMACRO_Callable_P##X)>(iBlockPtr); \
	return null; \
	}

ZMACRO_Callable_sCallable(0)
ZMACRO_Callable_sCallable(1)
ZMACRO_Callable_sCallable(2)
ZMACRO_Callable_sCallable(3)
ZMACRO_Callable_sCallable(4)
ZMACRO_Callable_sCallable(5)
ZMACRO_Callable_sCallable(6)
ZMACRO_Callable_sCallable(7)
ZMACRO_Callable_sCallable(8)
ZMACRO_Callable_sCallable(9)
ZMACRO_Callable_sCallable(A)
ZMACRO_Callable_sCallable(B)
ZMACRO_Callable_sCallable(C)
ZMACRO_Callable_sCallable(D)
ZMACRO_Callable_sCallable(E)
ZMACRO_Callable_sCallable(F)

#undef ZMACRO_Callable_sCallable

#endif // defined(__clang_major__) && __clang_major__ >= 2

} // namespace ZooLib

#endif // defined(__BLOCKS__)

#endif // __ZooLib_Apple_Callable_Block_h__
