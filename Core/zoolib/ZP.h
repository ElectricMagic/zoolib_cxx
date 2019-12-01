// Copyright (c) 2019 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_ZP_h__
#define __ZooLib_ZP_h__ 1
#include "zconfig.h"

#if not defined(__OBJC__)
	#include "zoolib/Compat_operator_bool.h"
#endif

#include "zoolib/Atomic.h" // For sAtomicPtr_CAS
#include "zoolib/Compat_algorithm.h" // For std::swap
#include "zoolib/Util_Relops.h"

#include "zoolib/ZTypes.h" // For Adopt_T

namespace ZooLib {

// =================================================================================================
#pragma mark - ZP

template <class T>
class ZP
	{
private:
	#if defined(__APPLE__)
		typedef __unsafe_unretained T* TPtr;
	#else
		typedef T* TPtr;
	#endif

	TPtr fPtr;

	inline
	static void spRetain(TPtr iPtr) { if (iPtr) sRetain(*iPtr); }

	inline
	static void spRelease(TPtr iPtr) { if (iPtr) sRelease(*iPtr); }

public:
	#if defined(__OBJC__)
		operator bool() const { return true && fPtr; }
		operator TPtr() const { return fPtr; }
	#else
		ZMACRO_operator_bool_T(ZP, operator_bool) const
			{ return operator_bool_gen::translate(true && fPtr); }
	#endif

	template <class O>
	inline
	void swap(ZP<O>& ioOther)
		{
		using std::swap;
		swap(fPtr, ioOther.fPtr);
		}

	typedef T Type_t;
	typedef TPtr Ptr_t;

	T& operator *()
		{
		sCheck(fPtr);
		return *fPtr;
		}

	T& operator *() const
		{
		sCheck(fPtr);
		return *fPtr;
		}

// --

	inline
	ZP()
	:	fPtr(0)
		{}

	inline
	ZP(const ZP& iOther)
	:	fPtr(iOther.Copy())
		{}

	inline
	~ZP()
		{ spRelease(fPtr); }

	inline
	ZP& operator=(const ZP& iOther)
		{
		using std::swap;
		TPtr otherP = iOther.Copy();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	inline
	ZP(ZP&& iOther)
	:	fPtr(iOther.Orphan())
		{}

	inline
	ZP& operator=(ZP&& iOther)
		{
		using std::swap;
		swap(iOther.fPtr, fPtr);
		return *this;
		}

// --

	inline
	ZP(const null_t&)
	:	fPtr(0)
		{}

	inline
	ZP(TPtr iPtr)
	:	fPtr(iPtr)
		{ spRetain(fPtr); }

	inline
	ZP& operator=(TPtr iPtr)
		{
		using std::swap;
		swap(iPtr, fPtr);
		spRetain(fPtr);
		spRelease(iPtr);
		return *this;
		}

// --

	template <class O>
	inline
	ZP(const ZP<O>& iOther)
	:	fPtr(iOther.Copy())
		{}

	template <class O>
	inline
	ZP& operator=(const ZP<O>& iOther)
		{
		using std::swap;
		TPtr otherP = iOther.Copy();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	template <class O>
	inline
	ZP(ZP<O>&& iOther)
	:	fPtr(iOther.Orphan())
		{}

	template <class O>
	inline
	ZP& operator=(ZP<O>&& iOther)
		{
		using std::swap;
		TPtr otherP = iOther.Orphan();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	template <class O>
	inline
	ZP(const Adopt_T<O>& iAdopt)
	:	fPtr(iAdopt.Get())
		{}

	template <class O>
	inline
	ZP& operator=(const Adopt_T<O>& iAdopt)
		{
		using std::swap;
		TPtr otherP = iAdopt.Get();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	inline
	TPtr operator->() const
		{
		sCheck(fPtr);
		return fPtr;
		}

	inline
	TPtr Get() const
		{ return fPtr; }

	inline
	TPtr Copy() const
		{
		spRetain(fPtr);
		return fPtr;
		}

	inline
	TPtr Orphan()
		{
		using std::swap;
		TPtr otherP = 0;
		swap(otherP, fPtr);
		return otherP;
		}

	inline
	void Clear()
		{
		using std::swap;
		TPtr otherP = 0;
		swap(otherP, fPtr);
		spRelease(otherP);
		}

	template <class O>
	inline
	O* DynamicCast() const
		{ return dynamic_cast<O*>(fPtr); }

	template <class O>
	inline
	O* StaticCast() const
		{ return static_cast<O*>(fPtr); }

	inline
	bool AtomicCAS(TPtr iPrior, TPtr iNew)
		{
		if (not sAtomicPtr_CAS(&fPtr, iPrior, iNew))
			return false;
		spRetain(fPtr);
		spRelease(iPrior);
		return true;
		}

	inline
	bool CAS(TPtr iPrior, TPtr iNew)
		{
		if (fPtr != iPrior)
			return false;
		fPtr = iNew;
		spRetain(fPtr);
		spRelease(iPrior);
		return true;
		}

	inline
	static TPtr sCFAllocatorRetain(TPtr iPtr)
		{
		spRetain(iPtr);
		return iPtr;
		}

	inline
	static void sCFAllocatorRelease(TPtr iPtr)
		{ spRelease(iPtr); }

	inline
	void Retain()
		{ spRetain(fPtr); }

	inline
	void Release()
		{ spRelease(fPtr); }
	};

template <class T>
void sRefCopy(void* oDest, T* iPtr)
	{
	*static_cast<T**>(oDest) = iPtr;
	if (iPtr)
		sRetain(*iPtr);
	}

// =================================================================================================
#pragma mark - ZP partially specialized for pointer types

template <class T> void sRetain_T(T*& ioPtr);
template <class T> void sRelease_T(T* iPtr);

template <class T>
class ZP<T*>
	{
private:

	static void spRetain(T*& ioPtr) { if (ioPtr) sRetain_T(ioPtr); }
	static void spRelease(T* iPtr) { if (iPtr) sRelease_T(iPtr); }
	T* fPtr;

public:
	operator bool() const { return true && fPtr; }
	operator T*() const { return fPtr; }

	template <class O>
	void swap(ZP<O>& ioOther)
		{
		using std::swap;
		swap(fPtr, ioOther.fPtr);
		}

	typedef T* Type_t;
	typedef T* Ptr_t;

// --

	ZP()
	:	fPtr(0)
		{}

	ZP(const ZP& iOther)
	:	fPtr(iOther.Copy())
		{}

	~ZP()
		{ spRelease(fPtr); }

	ZP& operator=(const ZP& iOther)
		{
		using std::swap;
		T* otherP = iOther.Copy();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	ZP(ZP&& iOther)
	:	fPtr(iOther.Orphan())
		{}

	ZP& operator=(ZP&& iOther)
		{
		using std::swap;
		T* otherP = iOther.Orphan();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	ZP(const null_t&)
	:	fPtr(0)
		{}

	ZP(T* iPtr)
	:	fPtr(iPtr)
		{ spRetain(fPtr); }

	ZP& operator=(T* iPtr)
		{
		using std::swap;
		swap(iPtr, fPtr);
		spRetain(fPtr);
		spRelease(iPtr);
		return *this;
		}

// --

	template <class O>
	ZP(const ZP<O*>& iOther)
	:	fPtr(iOther.Copy())
		{}

	template <class O>
	ZP& operator=(const ZP<O*>& iOther)
		{
		using std::swap;
		T* otherP = iOther.Copy();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	template <class O>
	ZP(ZP<O*>&& iOther)
	:	fPtr(iOther.Orphan())
		{}

	template <class O>
	ZP& operator=(ZP<O*>&& iOther)
		{
		using std::swap;
		T* otherP = iOther.Orphan();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	template <class O>
	ZP(const Adopt_T<O*>& iAdopt)
	:	fPtr(iAdopt.Get())
		{}

	template <class O>
	ZP& operator=(const Adopt_T<O*>& iAdopt)
		{
		using std::swap;
		T* otherP = iAdopt.Get();
		swap(otherP, fPtr);
		spRelease(otherP);
		return *this;
		}

// --

	T* Get() const
		{ return fPtr; }

	T* Copy() const
		{
		T* result = fPtr;
		spRetain(result);
		return result;
		}

	T* Orphan()
		{
		using std::swap;
		T* otherP = 0;
		swap(otherP, fPtr);
		return otherP;
		}

	void Clear()
		{
		using std::swap;
		T* otherP = 0;
		swap(otherP, fPtr);
		spRelease(otherP);
		}

	template <class O>
	O StaticCast() const
		{ return static_cast<O>(fPtr); }

	bool AtomicCAS(T* iPrior, T* iNew)
		{
		if (not sAtomicPtr_CAS(&fPtr, iPrior, iNew))
			return false;
		spRetain(fPtr);
		spRelease(iPrior);
		return true;
		}

	bool CAS(T* iPrior, T* iNew)
		{
		if (fPtr != iPrior)
			return false;
		fPtr = iNew;
		spRetain(fPtr);
		spRelease(iPrior);
		return true;
		}

	inline
	static T* sCFAllocatorRetain(T* iPtr)
		{
		spRetain(iPtr);
		return iPtr;
		}

	inline
	static void sCFAllocatorRelease(T* iPtr)
		{ spRelease(iPtr); }

	inline
	void Retain()
		{ spRetain(fPtr); }

	inline
	void Release()
		{ spRelease(fPtr); }
	};

// =================================================================================================
#pragma mark - Pseudo-ctor

template <class T, class O>
bool operator==(const ZP<T>& iZP, O* iPtr)
	{ return iZP.Get() == iPtr; }

template <class T, class O>
bool operator!=(const ZP<T>& iZP, O* iPtr)
	{ return iZP.Get() != iPtr; }

template <class T, class O>
bool operator==(const ZP<T>& iL, const ZP<O>& iR)
	{ return iL.Get() == iR.Get(); }

template <class T, class O>
bool operator<(const ZP<T>& iL, const ZP<O>& iR)
	{ return iL.Get() < iR.Get(); }

template <class T> struct RelopsTraits_HasEQ<ZP<T>> : public RelopsTraits_Has {};
template <class T> struct RelopsTraits_HasLT<ZP<T>> : public RelopsTraits_Has {};

// =================================================================================================
#pragma mark - Pseudo-ctor

template <class T>
ZP<T> sZP(T* iPtr)
	{ return ZP<T>(iPtr); }

template <class T>
ZP<T> sZP(const ZP<T>& iP)
	{ return ZP<T>(iP); }

// =================================================================================================
#pragma mark - sClear

template <class T>
inline
void sClear(ZP<T>& ioP)
	{ ioP.Clear(); }

template <class T>
inline
ZP<T> sGetClear(ZP<T>& ioP)
	{
	const ZP<T> result = ioP;
	sClear(ioP);
	return result;
	}

// =================================================================================================
#pragma mark - swap

template <class T>
inline
void swap(ZP<T>& a, ZP<T>& b)
	{ a.swap(b); }

} // namespace ZooLib

#endif // __ZooLib_ZP_h__
