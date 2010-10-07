/* -------------------------------------------------------------------------------------------------
Copyright (c) 2002 Andrew Green and Learning in Motion, Inc.
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

#ifndef __ZUtil_STL__
#define __ZUtil_STL__
#include "zconfig.h"

#include "zoolib/ZCompat_algorithm.h"
#include "zoolib/ZDebug.h"

#include <map>
#include <set>
#include <vector>

namespace ZooLib {
namespace ZUtil_STL {

// ==================================================

/** Invoke delete on all elements between begin and end. */
template <class InputIterator>
void sDeleteAll(InputIterator begin, InputIterator end)
	{
	while (begin != end)
		{
		delete *begin;
		++begin;
		}
	}

template <class T>
T* sFirstOrNil(std::vector<T>& iVec)
	{
	return iVec.empty() ? nullptr : &iVec[0];
	}

template <class T>
const T* sFirstOrNil(const std::vector<T>& iVec)
	{
	return iVec.empty() ? nullptr : &iVec[0];
	}

template <typename T>
typename std::set<T>::iterator sEraseInc(std::set<T>& ioSet, typename std::set<T>::iterator iter)
	{
	if (ioSet.end() != iter)
		{
		const T theVal = *iter;
		ioSet.erase(iter);
		iter = ioSet.lower_bound(theVal);
		}
	return iter;
	}

// ==================================================

/** Returns true if iVector contains iElement. iVector is
assumed to be unordered, so we use find to make the determination. */
template <typename Base, typename Derived>
bool sContains(const std::vector<Base>& iVector, Derived iElement)
	{ return iVector.end() != std::find(iVector.begin(), iVector.end(), iElement); }


/** Returns true if iSet contains iElement. */
template <typename Base, typename Derived>
bool sContains(const std::set<Base>& iSet, Derived iElement)
	{ return iSet.end() != iSet.find(iElement); }


/** Returns true if iMap has an element at iKey. */
template <typename KBase, typename KDerived, typename Value>
bool sContains(const std::map<KBase, Value>& iMap, KDerived iKey)
	{ return iMap.end() != iMap.find(iKey); }


/** Appends iElement to ioVec by calling push_back. If iElement was already contained in ioVec
then false is returned and no change is made to ioVec. */
template <typename Base, typename Derived>
bool sPushBackIfNotContains(std::vector<Base>& ioVec, Derived iElement)
	{
	typename std::vector<Base>::iterator i = std::find(ioVec.begin(), ioVec.end(), iElement);
	if (i != ioVec.end())
		return false;
	ioVec.push_back(iElement);
	return true;
	}


/** Appends iElement to ioVec by calling push_back. We first assert, controlled
by iDebugLevel, that iElement is not already present in ioVec. */
template <typename Base, typename Derived>
void sPushBackMustNotContain(const int iDebugLevel, std::vector<Base>& ioVec, Derived iElement)
	{
	ZAssertStop(iDebugLevel, ioVec.end() == std::find(ioVec.begin(), ioVec.end(), iElement));
	ioVec.push_back(iElement);
	}


/** Inserts iElement to ioSet. We first assert, controlled
by iDebugLevel, that iElement is not already present in ioSet. */
template <typename Base, typename Derived>
void sInsertMustNotContain(const int iDebugLevel, std::set<Base>& ioSet, Derived iElement)
	{
	ZAssertStop(iDebugLevel, ioSet.end() == ioSet.find(iElement));
	ioSet.insert(iElement);
	}


/** Inserts iElement in ioSet, if it's not already contained. */
template <typename Base, typename Derived>
bool sInsertIfNotContains(std::set<Base>& ioSet, Derived iElement)
	{
	typename std::set<Base>::iterator i = ioSet.lower_bound(iElement);
	if (ioSet.end() != i && *i == iElement)
		return false;
	ioSet.insert(i, iElement);
	return true;
	}


/** If the unordered vector ioVec contains iElement then it is
removed and true returned. Otherwise no change is made to ioVec and
false is returned. */
template <typename Base, typename Derived>
bool sEraseIfContains(std::vector<Base>& ioVec, Derived iElement)
	{
	typename std::vector<Base>::iterator i = std::find(ioVec.begin(), ioVec.end(), iElement);
	if (i == ioVec.end())
		return false;
	ioVec.erase(i);
	return true;
	}


/** If ioSet contains iElement then it is removed and true returned.
Otherwise no change is made to ioSet and false is returned. */
template <typename Base, typename Derived>
bool sEraseIfContains(std::set<Base>& ioSet, Derived iElement)
	{
	typename std::set<Base>::iterator i = ioSet.find(iElement);
	if (i == ioSet.end())
		return false;
	ioSet.erase(i);
	return true;
	}


/** Removes iElement from ioVec, asserting that it is present and
returning an iterator referencing the position at which iElement was found. */
template <typename Base, typename Derived>
typename std::vector<Base>::iterator sEraseMustContain(const int iDebugLevel,
	std::vector<Base>& ioVec, Derived iElement)
	{
	typename std::vector<Base>::iterator i = std::find(ioVec.begin(), ioVec.end(), iElement);
	ZAssertStop(iDebugLevel, i != ioVec.end());
	return ioVec.erase(i);
	}


/** Removes iElement from ioSet, asserting that it is present. */
template <typename Base, typename Derived>
void sEraseMustContain(const int iDebugLevel, std::set<Base>& ioSet, Derived iElement)
	{
	typename std::set<Base>::iterator i = ioSet.find(iElement);
	ZAssertStop(iDebugLevel, i != ioSet.end());
	ioSet.erase(i);
	}

// ==================================================

/** The contents of iVector are assumed to be sorted by less<Base>. Returns
true if iVector contains iElement. */
template <typename Base, typename Derived>
bool sSortedContains(const std::vector<Base>& iVector, Derived iElement)
	{
	typename std::vector<Base>::const_iterator i =
		lower_bound(iVector.begin(), iVector.end(), iElement);
	return i != iVector.end() && *i == iElement;
	}


/** The contents of ioVec are assumed to be sorted by less<Base>. If iElement
was already contained in ioVec then false is returned and no change is made
to ioVec. Otherwise it is inserted using lower_bound and less<Base> and true
is returned. */
template <typename Base, typename Derived>
bool sSortedInsertIfNotContains(std::vector<Base>& ioVec, Derived iElement)
	{
	typename std::vector<Base>::iterator i = lower_bound(ioVec.begin(), ioVec.end(), iElement);

	if (i != ioVec.end() && *i == iElement)
		return false;

	ioVec.insert(i, iElement);
	return true;
	}


/** The contents of ioVec are assumed to be sorted by less<Base>. Returns false
if ioVec does not contain iElement. If ioVec does contain
iElement then it is removed and true returned. */
template <typename Base, typename Derived>
bool sSortedEraseIfContains(std::vector<Base>& ioVec, Derived iElement)
	{
	typename std::vector<Base>::iterator i = lower_bound(ioVec.begin(), ioVec.end(), iElement);

	if (i == ioVec.end() || !(*i == iElement))
		return false;

	ioVec.erase(i);
	return true;
	}


/** The contents of ioVec are assumed to be sorted by less<Base>. We first
assert, under the control of iDebugLevel, that ioVec does not contain iElement.
We then insert iElement using lower_bound and less<Base>. */
template <typename Base, typename Derived>
void sSortedInsertMustNotContain(const int iDebugLevel, std::vector<Base>& ioVec, Derived iElement)
	{
	typename std::vector<Base>::iterator i = lower_bound(ioVec.begin(), ioVec.end(), iElement);
	ZAssertStop(iDebugLevel, i == ioVec.end() || !(*i == iElement));
	ioVec.insert(i, iElement);
	}


/** The contents of ioVec are assumed to be sorted by less<Base>. We first
assert, under the control of iDebugLevel, that ioVec contains iElement.
We then remove iElement using lower_bound and less<Base>. */
template <typename Base, typename Derived>
void sSortedEraseMustContain(const int iDebugLevel, std::vector<Base>& ioVec, Derived iElement)
	{
	typename std::vector<Base>::iterator i = lower_bound(ioVec.begin(), ioVec.end(), iElement);
	ZAssertStop(iDebugLevel, i != ioVec.end() && *i == iElement);
	ioVec.erase(i);
	}

// ==================================================

template <typename KBase, typename KDerived, typename Value>
bool sEraseIfContains(std::map<KBase, Value>& ioMap, KDerived iKey)
	{
	typename std::map<KBase, Value>::iterator i = ioMap.find(iKey);
	if (i == ioMap.end())
		return false;
	ioMap.erase(i);
	return true;
	}


template <typename KBase, typename KDerived, typename Value>
void sEraseMustContain(const int iDebugLevel, std::map<KBase, Value>& ioMap, KDerived iKey)
	{
	typename std::map<KBase, Value>::iterator i = ioMap.find(iKey);
	ZAssertStop(iDebugLevel, i != ioMap.end());
	ioMap.erase(i);
	}


template <typename KBase, typename KDerived, typename Value>
Value sEraseAndReturn(const int iDebugLevel, std::map<KBase, Value>& ioMap, KDerived iKey)
	{
	typename std::map<KBase, Value>::iterator iter = ioMap.find(iKey);
	ZAssertStop(iDebugLevel, ioMap.end() != iter);
	Value result = (*iter).second;
	ioMap.erase(iter);
	return result;
	}


template <typename KBase, typename KDerived, typename Value>
void sInsertMustNotContain(const int iDebugLevel,
	std::map<KBase, Value>& ioMap, KDerived iKey, Value iValue)
	{
	ZAssertStop(iDebugLevel, ioMap.end() == ioMap.find(iKey));
	ioMap.insert(typename std::map<KBase, Value>::value_type(iKey, iValue));
	}

template <typename KBase, typename KDerived, typename Value>
void sSetMustContain(const int iDebugLevel,
	std::map<KBase, Value>& ioMap, KDerived iKey, Value iValue)
	{
	typename std::map<KBase, Value>::iterator i = ioMap.find(iKey);
	ZAssertStop(iDebugLevel, ioMap.end() != i);
	i->second = iValue;
	}

template <typename KBase, typename KDerived, typename Value>
bool sSetIfContains(std::map<KBase, Value>& ioMap, KDerived iKey, Value iValue)
	{
	typename std::map<KBase, Value>::iterator i = ioMap.find(iKey);
	if (ioMap.end() == i)
		return false;
	i->second = iValue;
	return true;
	}

template <typename KBase, typename KDerived, typename Value>
Value sGetMustContain(const int iDebugLevel,
	std::map<KBase, Value>& ioMap, KDerived iKey)
	{
	typename std::map<KBase, Value>::iterator i = ioMap.find(iKey);
	ZAssertStop(iDebugLevel, ioMap.end() != i);
	return i->second;
	}

} // namespace ZUtil_STL
} // namespace ZooLib

#endif // __ZUtil_STL__
