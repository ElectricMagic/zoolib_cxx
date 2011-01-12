/* -------------------------------------------------------------------------------------------------
Copyright (c) 2009 Andrew Green
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

#ifndef __ZRef__
#define __ZRef__ 1
#include "zconfig.h"

#include "zoolib/ZAtomic.h" // For ZAtomic_CompareAndSwapPtr
#include "zoolib/ZCompat_algorithm.h" // For std::swap
#include "zoolib/ZCompat_operator_bool.h"
#include "zoolib/ZDebug.h"
#include "zoolib/ZTypes.h" // For Adopt_T

namespace ZooLib {

// =================================================================================================
#pragma mark -
#pragma mark * ZRef

template <class T, bool Sense = true>
class ZRef
	{
private:
	static void spRetain(T* iP)
		{
		if (iP)
			sRetain(*iP);
		}

	static void spRelease(T* iP)
		{
		if (iP)
			sRelease(*iP);
		}

	static void spCheck(T* iP)
		{ ZAssertStop(1, iP); }

public:
	#ifdef __OBJC__

		operator bool() const { return Sense == (fP && true); }

		operator T*() const { return fP; }

	#else

		ZOOLIB_DEFINE_OPERATOR_BOOL_TYPES_T(ZRef,
			operator_bool_generator_type, operator_bool_type);

		operator operator_bool_type() const
			{ return operator_bool_generator_type::translate(Sense == (fP && true)); }

	#endif

	void swap(ZRef& ioOther)
		{ std::swap(fP, ioOther.fP); }

	ZRef()
	:	fP(nullptr)
		{}

	ZRef(const ZRef& iOther)
	:	fP(iOther.Get())
		{ spRetain(fP); }

	~ZRef()
		{ spRelease(fP); }

	ZRef& operator=(const ZRef& iOther)
		{
		T* otherP = iOther.Get();
		std::swap(otherP, fP);
		spRetain(fP);
		spRelease(otherP);
		return *this;
		}

	ZRef(const null_t&)
	:	fP(nullptr)
		{}

	ZRef(T* iP)
	:	fP(iP)
		{ spRetain(fP); }

	ZRef& operator=(T* iP)
		{
		std::swap(iP, fP);
		spRetain(fP);
		spRelease(iP);
		return *this;
		}

	template <class O, bool OtherSense>
	ZRef(const ZRef<O, OtherSense>& iOther)
	:	fP(iOther.Get())
		{ spRetain(fP); }

	template <class O, bool OtherSense>
	ZRef& operator=(const ZRef<O, OtherSense>& iOther)
		{
		T* otherP = iOther.Get();
		std::swap(otherP, fP);
		spRetain(fP);
		spRelease(otherP);
		return *this;
		}

	template <class O>
	ZRef(const Adopt_T<O>& iAdopt)
	:	fP(iAdopt.Get())
		{}

	template <class O>
	ZRef& operator=(const Adopt_T<O>& iAdopt)
		{
		T* otherP = iAdopt.Get();
		std::swap(otherP, fP);
		spRelease(otherP);
		return *this;
		}

	template <class O>
	bool operator==(O* iP) const
		{ return fP == iP; }

	template <class O>
	bool operator!=(O* iP) const
		{ return fP != iP; }

	template <class O, bool OtherSense>
	bool operator==(const ZRef<O, OtherSense>& iOther) const
		{ return fP == iOther.Get(); }

	template <class O, bool OtherSense>
	bool operator!=(const ZRef<O, OtherSense>& iOther) const
		{ return fP != iOther.Get(); }

	template <class O, bool OtherSense>
	bool operator<(const ZRef<O, OtherSense>& iOther) const
		{ return fP < iOther.Get(); }

	T* operator->() const
		{
		spCheck(fP);
		return fP;
		}

	T* Get() const
		{ return fP; }

	T* Copy() const
		{
		spRetain(fP);
		return fP;
		}

	T* Orphan()
		{
		T* otherP = nullptr;
		std::swap(otherP, fP);
		return otherP;
		}

	void Clear()
		{
		T* otherP = nullptr;
		std::swap(otherP, fP);
		spRelease(otherP);
		}

	T*& OParam()
		{
		this->Clear();
		return fP;
		}

	template <class O>
	O* DynamicCast() const
		{ return dynamic_cast<O*>(fP); }

	template <class O>
	O* StaticCast() const
		{ return static_cast<O*>(fP); }

	bool AtomicCompareAndSwap(T* iPrior, T* iNew)
		{
		if (!ZAtomic_CompareAndSwapPtr(&fP, iPrior, iNew))
			return false;
		spRetain(iNew);
		spRelease(iPrior);
		return true;
		}

	bool AtomicSetIfNull(T* iNew)
		{ return this->AtomicCompareAndSwap(0, iNew); }

private:
	T* fP;
	};

template <class T>
void sRefCopy(void* oDest, T* iP)
	{
	*static_cast<T**>(oDest) = iP;
	if (iP)
		sRetain(*iP);
	}

// =================================================================================================
#pragma mark -
#pragma mark * ZRef partially specialized for pointer types

template <class T> void sRetain_T(T*& ioPtr);
template <class T> void sRelease_T(T* iPtr);

template <class T>
class ZRef<T*, true>
	{
private:
	static void spRetain(T*& iP)
		{
		if (iP)
			sRetain_T(iP);
		}

	static void spRelease(T* iP)
		{
		if (iP)
			sRelease_T(iP);
		}

public:
	void swap(ZRef& iOther)
		{ std::swap(fP, iOther.fP); }

	ZRef()
	:	fP(nullptr)
		{}

	ZRef(const ZRef& iOther)
	:	fP(iOther.Get())
		{ spRetain(fP); }

	~ZRef()
		{ spRelease(fP); }

	ZRef& operator=(const ZRef& iOther)
		{
		T* otherP = iOther.Get();
		std::swap(otherP, fP);
		spRetain(fP);
		spRelease(otherP);
		return *this;
		}

	ZRef(const null_t&)
	:	fP(nullptr)
		{}

	ZRef(T* iP)
	:	fP(iP)
		{ spRetain(fP); }

	ZRef& operator=(T* iP)
		{
		std::swap(iP, fP);
		spRetain(fP);
		spRelease(iP);
		return *this;
		}

	template <class O>
	ZRef(const ZRef<O*>& iOther)
	:	fP(iOther.Get())
		{ spRetain(fP); }

	template <class O>
	ZRef& operator=(const ZRef<O*>& iOther)
		{
		T* otherP = iOther.Get();
		std::swap(otherP, fP);
		spRetain(fP);
		spRelease(otherP);
		return *this;
		}

	template <class O>
	ZRef(const Adopt_T<O*>& iAdopt)
	:	fP(iAdopt.Get())
		{}

	template <class O>
	ZRef& operator=(const Adopt_T<O*>& iAdopt)
		{
		T* otherP = iAdopt.Get();
		std::swap(otherP, fP);
		spRelease(otherP);
		return *this;
		}

	template <class O>
	bool operator==(O* iP) const
		{ return fP == iP; }

	template <class O>
	bool operator!=(O* iP) const
		{ return fP != iP; }

	template <class O>
	bool operator==(const ZRef<O*>& iOther) const
		{ return fP == iOther.Get(); }

	template <class O>
	bool operator!=(const ZRef<O*>& iOther) const
		{ return fP != iOther.Get(); }

	template <class O>
	bool operator<(const ZRef<O*>& iOther) const
		{ return fP < iOther.Get(); }

	operator T*()
		{ return fP; }

	operator T*() const
		{ return fP; }

	T* Get() const
		{ return fP; }

	T* Copy() const
		{
		spRetain(fP);
		return fP;
		}

	T* Orphan()
		{
		T* otherP = nullptr;
		std::swap(otherP, fP);
		return otherP;
		}

	void Clear()
		{
		T* otherP = nullptr;
		std::swap(otherP, fP);
		spRelease(otherP);
		}

	T*& OParam()
		{
		this->Clear();
		return fP;
		}

	template <class O>
	O StaticCast() const
		{ return static_cast<O>(fP); }

private:
	T* fP;
	};

// =================================================================================================
#pragma mark -
#pragma mark * MakeRef

const struct MakeRef_t
	{
	template <class T>
	ZRef<T> operator()(T* iT) const { return ZRef<T>(iT); }
	} MakeRef = {};

const struct TempRef_t
	{
	template <class T>
	ZRef<T> operator&(T* iT) const { return ZRef<T>(Adopt_T<T>(iT)); }
	
	template <class T>
	ZRef<T> operator()(T* iT) const { return ZRef<T>(Adopt_T<T>(iT)); }
	} TempRef = {};

// =================================================================================================
#pragma mark -
#pragma mark *

template <class T>
void swap(ZRef<T>& a, ZRef<T>& b)
	{ a.swap(b); }

} // namespace ZooLib

#endif // __ZRef__
