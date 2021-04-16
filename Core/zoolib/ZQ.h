// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#ifndef __ZooLib_ZQ_h__
#define __ZooLib_ZQ_h__
#include "zconfig.h"

#include "zoolib/Compat_operator_bool.h"
#include "zoolib/Compat_algorithm.h" // For std::swap
#include "zoolib/CtorDtor.h" // For placement ctor/copy/dtor/assign
#include "zoolib/Default.h"
#include "zoolib/Not.h"
#include "zoolib/Util_Relops.h"

#include "zoolib/ZDebug.h"
#include "zoolib/ZTypes.h" // For null

namespace ZooLib {

// =================================================================================================
#pragma mark - ZQ

// The 'Q' stands for 'Questionable' or 'Queryable'.
// It is pronounced 'Quid', as in the latin interrogative.

// c.f. boost::optional, Haskell's 'Data.Maybe', Scala's 'Option'.

template <class T>
class ZQ
	{
	static
	void spSwap(ZQ<T>& ioNoValue, ZQ<T>& ioHasValue)
		{
		sCtorFromVoidStar_T<T>(ioNoValue.fBytes, ioHasValue.fBytes);
		ioNoValue.fHasValue = true;
		ioHasValue.fHasValue = false;
		sDtor_T<T>(ioHasValue.fBytes);
		}

public:
	void swap(ZQ<T>& ioOther)
		{
		if (fHasValue)
			{
			if (ioOther.fHasValue)
				{
				using std::swap;
				swap(*sFetch_T<T>(fBytes), *sFetch_T<T>(ioOther.fBytes));
				}
			else
				{
				spSwap(ioOther, *this);
				}
			}
		else if (ioOther.fHasValue)
			{
			spSwap(*this, ioOther);
			}
		}

// -----------------

	ZQ()
	:	fHasValue(false)
		{}

	ZQ(const ZQ& iOther)
	:	fHasValue(iOther.fHasValue)
		{
		if (fHasValue)
			sCtorFromVoidStar_T<T>(fBytes, iOther.fBytes);
		}

	~ZQ()
		{
		if (fHasValue)
			sDtor_T<T>(fBytes);
		}

	ZQ& operator=(const ZQ& iOther)
		{
		if (fHasValue)
			{
			if (iOther.fHasValue)
				{
				sAssignFromVoidStar_T<T>(fBytes, iOther.fBytes);
				}
			else
				{
				fHasValue = false;
				sDtor_T<T>(fBytes);
				}
			}
		else if (iOther.fHasValue)
			{
			sCtorFromVoidStar_T<T>(fBytes, iOther.fBytes);
			fHasValue = true;
			}
		return *this;
		}

// -----------------

	template <class OtherT>
	ZQ(const ZQ<OtherT>& iOther)
		{
		if (const OtherT* theOther = iOther.PGet())
			{
			sCtor_T<T,OtherT>(fBytes, *theOther);
			fHasValue = true;
			}
		else
			{
			fHasValue = false;
			}
		}

	template <class OtherT>
	ZQ& operator=(const ZQ<OtherT>& iOther)
		{
		if (fHasValue)
			{
			if (const OtherT* theOther = iOther.PGet())
				{
				sAssign_T<T>(fBytes, *theOther);
				}
			else
				{
				fHasValue = false;
				sDtor_T<T>(fBytes);
				}			
			}
		else if (const OtherT* theOther = iOther.PGet())
			{
			sCtor_T<T,OtherT>(fBytes, *theOther);
			fHasValue = true;
			}
		return *this;
		}

// -----------------

	ZQ(const null_t&)
	:	fHasValue(false)
		{}

	ZQ& operator=(const null_t&)
		{
		if (fHasValue)
			{
			fHasValue = false;
			sDtor_T<T>(fBytes);
			}
		return *this;
		}

// -----------------

	template <class P0>
	ZQ(const P0& i0)
	:	fHasValue(true)
		{ sCtor_T<T, P0>(fBytes, i0); }

	template <class P0>
	ZQ& operator=(const P0& iValue)
		{
		if (fHasValue)
			{
			sAssign_T<T>(fBytes, iValue);
			}
		else
			{
			sCtor_T<T,P0>(fBytes, iValue);
			fHasValue = true;
			}
		return *this;
		}

// -----------------

	template <class P0, class P1>
	ZQ(const P0& i0, const P1& i1)
	:	fHasValue(true)
		{ sCtor_T<T,P0,P1>(fBytes, i0, i1); }

// -----------------

	ZMACRO_operator_bool_T(ZQ, operator_bool) const
		{ return operator_bool_gen::translate(fHasValue); }

	bool HasValue() const
		{ return fHasValue; }

	T& operator*()
		{
		ZAssert(fHasValue);
		return *sFetch_T<T>(fBytes);
		}
	
	const T& operator*() const
		{
		ZAssert(fHasValue);
		return *sFetch_T<T>(fBytes);
		}
	
	T* operator->()
		{
		ZAssert(fHasValue);
		return sFetch_T<T>(fBytes);
		}

	const T* operator->() const
		{
		ZAssert(fHasValue);
		return sFetch_T<T>(fBytes);
		}

// -----------------

	void Clear()
		{
		if (fHasValue)
			{
			fHasValue = false;
			sDtor_T<T>(fBytes);
			}
		}

	const T* PGet() const
		{
		if (fHasValue)
			return sFetch_T<T>(fBytes);
		return nullptr;
		}

	const T& DGet(const T& iDefault) const
		{
		if (fHasValue)
			return *sFetch_T<T>(fBytes);
		return iDefault;
		}

	const T& Get() const
		{
		if (fHasValue)
			return *sFetch_T<T>(fBytes);
		return sDefault<T>();
		}

	T* PMut()
		{
		if (fHasValue)
			return sFetch_T<T>(fBytes);
		return nullptr;
		}

	T& DMut(const T& iDefault)
		{
		if (not fHasValue)
			{
			sCtor_T<T>(fBytes, iDefault);
			fHasValue = true;
			}
		return *sFetch_T<T>(fBytes);
		}

	T& Mut()
		{
		if (not fHasValue)
			{
			sCtor_T<T>(fBytes);
			fHasValue = true;
			}
		return *sFetch_T<T>(fBytes);
		}

	T& Set(const T& iVal)
		{
		if (fHasValue)
			{
			return sAssign_T<T>(fBytes, iVal);
			}
		else
			{
			sCtor_T<T>(fBytes, iVal);
			fHasValue = true;
			return *sFetch_T<T>(fBytes);
			}
		}

	bool QSet(const T& iVal)
		{
		if (fHasValue)
			return false;
		sCtor_T<T>(fBytes, iVal);
		fHasValue = true;
		return true;
		}

	T& OParam()
		{
		if (fHasValue)
			{
			fHasValue = false;
			sDtor_T<T>(fBytes);
			}

		sCtor_T<T>(fBytes);
		fHasValue = true;
		return *sFetch_T<T>(fBytes);
		}

private:
	#if ZCONFIG_CPP >= 2011
	union
		{
		T fAsT;
		char fBytes[1];
		};
	#else
		char fBytes[sizeof(T)] ZMACRO_Attribute_Aligned;
	#endif

protected:
	bool fHasValue;
	};

template <class T>
bool operator==(const ZQ<T>& iL, const ZQ<T>& iR)
	{
	if (const T* ll = iL.PGet())
		{
		if (const T* rr = iR.PGet())
			return *ll == *rr;
		return false;
		}
	return not iR.HasValue();
	}

template <class T>
bool operator<(const ZQ<T>& iL, const ZQ<T>& iR)
	{
	if (const T* ll = iL.PGet())
		{
		if (const T* rr = iR.PGet())
			return *ll < *rr;
		return false;
		}
	return iR.HasValue();
	}

template <class T>
struct RelopsTraits_HasEQ<ZQ<T>> : public RelopsTraits_Has {};

template <class T>
struct RelopsTraits_HasLT<ZQ<T>> : public RelopsTraits_Has {};

// =================================================================================================
#pragma mark - ZQ (specialized for void)

template <>
class ZQ<void>
	{
public:
	void swap(ZQ& iOther)
		{ std::swap(fHasValue, iOther.fHasValue); }

// -----------------

	ZQ()
	:	fHasValue(false)
		{}

	ZQ(const ZQ<void>& iOther)
	:	fHasValue(iOther.fHasValue)
		{}

	~ZQ()
		{}

	ZQ& operator=(const ZQ<void>& iOther)
		{
		fHasValue = iOther.fHasValue;
		return *this;
		}

// -----------------

	ZQ(const null_t&)
	:	fHasValue(false)
		{}

	ZQ& operator=(const null_t&)
		{
		fHasValue = false;
		return *this;
		}

// -----------------

	ZQ(const notnull_t&)
	:	fHasValue(true)
		{}

	ZQ& operator=(const notnull_t&)
		{
		fHasValue = true;
		return *this;
		}

// -----------------

	ZMACRO_operator_bool_T(ZQ, operator_bool) const
		{ return operator_bool_gen::translate(fHasValue); }

	bool HasValue() const
		{ return fHasValue; }

	void operator*() const
		{ ZAssert(fHasValue); }

	void Clear()
		{ fHasValue = false; }

	const void Get() const
		{}

	void Set()
		{ fHasValue = true; }

	bool QSet()
		{
		if (fHasValue)
			return false;
		fHasValue = true;
		return true;
		}

protected:
	bool fHasValue;
	};

inline
bool operator==(const ZQ<void>& iL, const ZQ<void>& iR)
	{
	if (iL.HasValue())
		return iR.HasValue();
	return not iR.HasValue();
	}

inline
bool operator<(const ZQ<void>& iL, const ZQ<void>& iR)
	{ return not iL.HasValue() && iR.HasValue(); }

// =================================================================================================
#pragma mark - NotQ

template <class T> using NotQ = Not<ZQ<T>>;

// =================================================================================================
#pragma mark - Pseudo-ctor

template <class T>
ZQ<T> sQ()
	{ return ZQ<T>(); }

template <class T>
ZQ<T> sQ(const T& iT)
	{ return ZQ<T>(iT); }

template <class T, class P0>
ZQ<T> sQ(const P0& i0)
	{ return ZQ<T>(i0); }

template <class T, class P0, class P1>
ZQ<T> sQ(const P0& i0, const P1& i1)
	{ return ZQ<T>(i0, i1); }

// =================================================================================================
#pragma mark - Accessor functions

template <class T>
const T* sPGet(const ZQ<T>& iQ)
	{ return iQ.PGet(); }

template <class T>
const T& sDGet(const T& iDefault, const ZQ<T>& iQ)
	{ return iQ.DGet(iDefault); }

template <class T>
const T& sGet(const ZQ<T>& iQ)
	{ return iQ.Get(); }

template <class T>
T* sPMut(ZQ<T>& ioQ)
	{ return ioQ.PMut(); }

template <class T>
T& sDMut(const T& iDefault, ZQ<T>& ioQ)
	{ return ioQ.DMut(iDefault); }

template <class T>
T& sMut(ZQ<T>& ioQ)
	{ return ioQ.Mut(); }

template <class T>
T& sSet(ZQ<T>& ioQ, const T& iVal)
	{ return ioQ.Set(iVal); }

// =================================================================================================
#pragma mark -

template <class T>
void sClear(ZQ<T>& ioQ)
	{ ioQ.Clear(); }

template <class T>
T sGetClear(ZQ<T>& ioQ)
	{
	const T result = ioQ.Get();
	ioQ.Clear();
	return result;
	}

template <class T>
ZQ<T> sQGetClear(ZQ<T>& ioQ)
	{
	if (ioQ)
		{
		const T result = ioQ.Get();
		ioQ.Clear();
		return result;
		}
	return null;
	}

// =================================================================================================
#pragma mark - swap

template <class T>
void swap(ZQ<T>& a, ZQ<T>& b)
	{ a.swap(b); }

} // namespace ZooLib

#endif // __ZooLib_ZQ_h__
