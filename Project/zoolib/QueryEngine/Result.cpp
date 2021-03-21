// Copyright (c) 2010 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/QueryEngine/Result.h"

#include "zoolib/Compare_vector.h"

using std::map;
using std::pair;
using std::vector;

namespace ZooLib {

// =================================================================================================
#pragma mark - QueryEngine::Result

namespace QueryEngine {

using RelationalAlgebra::RelHead;

Result::Result(const Result& iOther)
:	fRelHead(iOther.fRelHead)
,	fPackedRows(iOther.fPackedRows)
	{}

Result::Result(RelHead* ioRelHead,
	vector<Val_DB>* ioPackedRows)
	{
	ioRelHead->swap(fRelHead);
	ioPackedRows->swap(fPackedRows);
	}

Result::Result(const RelHead& iRelHead,
	vector<Val_DB>* ioPackedRows)
:	fRelHead(iRelHead)
	{
	ioPackedRows->swap(fPackedRows);
	}

Result::Result(const ZP<Result>& iOther, size_t iRow)
:	fRelHead(iOther->GetRelHead())
	{
	if (iRow < iOther->Count())
		{
		const Val_DB* theVals = iOther->GetValsAt(iRow);
		fPackedRows.insert(fPackedRows.end(), theVals, theVals + fRelHead.size());
		}
	}

Result::~Result()
	{}

const RelHead& Result::GetRelHead()
	{ return fRelHead; }

size_t Result::Count()
	{
	if (const size_t theSize = fRelHead.size())
		return fPackedRows.size() / theSize;
	return 0;
	}

const Val_DB* Result::GetValsAt(size_t iIndex)
	{
	const size_t theOffset = fRelHead.size() * iIndex;
	ZAssert(theOffset < fPackedRows.size() + fRelHead.size());
	return &fPackedRows[theOffset];
	}

int Result::Compare(const ZP<Result>& iOther) const
	{
	if (int compare = sCompare_T(fRelHead, iOther->fRelHead))
		return compare;
	return sCompare_T(fPackedRows, iOther->fPackedRows);
	}

ZP<Result> Result::Fresh()
	{ return new Result(*this); }

// =================================================================================================
#pragma mark - ResultDeltas

ResultDeltas::ResultDeltas()
	{}

ResultDeltas::~ResultDeltas()
	{}

// =================================================================================================
#pragma mark - Comparer_t (anonymous)

namespace { // anonymous

pair<int,size_t> spCompare(const vector<size_t>& iOffsets,
	const Val_DB* iVals_Left, const Val_DB* iVals_Right)
	{
	const size_t offsetsCount = iOffsets.size();
	for (size_t yy = 0; yy < offsetsCount; ++yy)
		{
		const size_t theCol = iOffsets[yy];
		if (int compare = sCompare_T(iVals_Left[theCol], iVals_Right[theCol]))
			return pair<int,size_t>(compare, yy);
		}
	return pair<int,size_t>(0, offsetsCount);
	}

struct Comparer_t
	{
	Comparer_t(const vector<size_t>& iOffsets, const ZP<Result>& iResult)
	:	fOffsets(iOffsets)
	,	fResult(iResult)
		{}

	bool operator()(const size_t& iLeft, const size_t& iRight) const
		{
		return 0 > spCompare(fOffsets,
			fResult->GetValsAt(iLeft),
			fResult->GetValsAt(iRight)).first;
		}

	const vector<size_t>& fOffsets;
	ZP<Result> fResult;
	};

} // anonymous namespace

// =================================================================================================
#pragma mark - ResultDiffer

ResultDiffer::ResultDiffer(const RelHead& iIdentity,
	const RelHead& iSignificant,
	bool iEmitDummyChanges)
:	fIdentity(iIdentity)
,	fSignificant(iSignificant)
,	fEmitDummyChanges(iEmitDummyChanges)
	{
	// If you don't have any identity or significant fields, then I'm
	// not even sure what we're doing.
	ZAssert(not fIdentity.empty() || not fSignificant.empty());
	}

// * oRemoved indices are relative to the prior list.
// * oAdded pairs have the index at which the entry should be inserted, and the corresponding
//   index of the value in iResult.
// * oChanged triples have f0 as the index in the new list, f1 is
//   the index into the prior result (so the prior value is available) and f2
//   is the index into iResult for the new value.

// To mutate an external list you would erase every position in oRemoved,
// insert everything in oAdded, and then apply changes in oChanged after both.

void ResultDiffer::Apply(const ZP<Result>& iResult,
	ZP<Result>* oPriorResult,
	const ZP<ResultDeltas>& iResultDeltas,
	ZP<Result>* oCurResult,
	vector<size_t>* oRemoved,
	vector<pair<size_t,size_t>>* oAdded,
	vector<Multi3<size_t,size_t,size_t>>* oChanged)
	{
	ZP<Result> theResult = iResult;

	if (iResultDeltas)
		{
		ZAssert(fResult_Prior);

		theResult = fResult_Prior->Fresh();

		const size_t theColCount = theResult->GetRelHead().size();
		for (size_t xx = 0; xx < iResultDeltas->fMapping.size(); ++xx)
			{
			const size_t target = iResultDeltas->fMapping[xx];

			std::copy_n(&iResultDeltas->fPackedRows[xx * theColCount],
				theColCount,
				&theResult->fPackedRows[target * theColCount]);
			}
		}

	ZAssert(theResult);

	if (theResult != fResult_Prior)
		{
		const RelHead& theRH = theResult->GetRelHead();

		ZAssert(not fResult_Prior || fResult_Prior->GetRelHead() == theRH);

		if (not fResult_Prior)
			{
			// This is the first time we get to see a query's relhead. Initialize fPermute so we
			// know in which order to examine columns when we sort and compare result rows.

			RelHead::const_iterator iter_Identity = fIdentity.begin();
			size_t index_Identity = 0;

			RelHead::const_iterator iter_Significant = fSignificant.begin();
			size_t index_Significant = 0;

			RelHead::const_iterator iter_RH = theRH.begin();
			size_t index_Other = 0;

			size_t index_RH = 0;

			fPermute.resize(theRH.size());

			for (const RelHead::const_iterator end_RH = theRH.end();
				iter_RH != end_RH;
				++iter_RH, ++index_RH)
				{
				if (iter_Identity != fIdentity.end() && *iter_Identity == *iter_RH)
					{
					fPermute[index_Identity] = index_RH;
					++index_Identity;
					++iter_Identity;
					}
				else if (iter_Significant != fSignificant.end() && *iter_Significant == *iter_RH)
					{
					fPermute[fIdentity.size() + index_Significant] = index_RH;
					++index_Significant;
					++iter_Significant;
					}
				else
					{
					fPermute[fIdentity.size() + fSignificant.size() + index_Other] = index_RH;
					++index_Other;
					}
				}

			// We must have consumed the entirety of our identity relhead
			ZAssert(iter_Identity == fIdentity.end());
			ZAssert(index_Identity == fIdentity.size());

			// And significant
			ZAssert(iter_Significant == fSignificant.end());
			ZAssert(index_Significant == fSignificant.size());
			}

		const size_t theCount = theResult->Count();

		vector<size_t> theSort_New;
		theSort_New.reserve(theCount);
		for (size_t xx = 0; xx < theCount; ++xx)
			theSort_New.push_back(xx);

		sort(theSort_New.begin(), theSort_New.end(), Comparer_t(fPermute, theResult));

		if (not fResult_Prior)
			{
			// This is our first result, everything is an add.
			if (oAdded)
				{
				for (size_t yy = 0; yy < theSort_New.size(); ++yy)
					oAdded->push_back(pair<size_t, size_t>(yy, theSort_New[yy]));
				}
			}
		else
			{
			// We have a prior result, do the diff.

			size_t theIndex_Prior = 0;
			const size_t theCount_Prior = fSort_Prior.size();

			size_t theIndex_New = 0;
			const size_t theCount_New = theSort_New.size();

			for (;;)
				{
				if (theIndex_New >= theCount_New)
					{
					// Anything remaining in prior when new is exhausted is a removal.
					if (oRemoved)
						{
						oRemoved->reserve(oRemoved->size() + theCount_Prior - theIndex_Prior);
						while (theCount_Prior > theIndex_Prior)
							oRemoved->push_back(fSort_Prior[theIndex_Prior++]);
						}
					break;
					}

				if (theIndex_Prior >= theCount_Prior)
					{
					// Anything remaining in new when prior is exhausted is an addition.
					if (oAdded)
						{
						oAdded->reserve(oAdded->size() + theCount_New - theIndex_New);
						while (theCount_New > theIndex_New)
							{
							oAdded->push_back(pair<size_t,size_t>(theIndex_New, theSort_New[theIndex_New]));
							++theIndex_New;
							}
						}
					break;
					}

				// Match current prior against current new
				const pair<int,size_t> result = spCompare(fPermute,
					fResult_Prior->GetValsAt(fSort_Prior[theIndex_Prior]),
					theResult->GetValsAt(theSort_New[theIndex_New]));

				if (result.second < fIdentity.size())
					{
					// Comparison was terminated in the 'identity' portion of the values,
					// and so the values can't be equal.
					ZAssert(result.first != 0);

					if (result.first < 0)
						{
						// Prior is less than new, so prior is not in new, and this is a removal.
						if (oRemoved)
							oRemoved->push_back(fSort_Prior[theIndex_Prior]);
						++theIndex_Prior;
						}
					else
						{
						// Contrariwise.
						if (oAdded)
							oAdded->push_back(pair<size_t,size_t>(theIndex_New, theSort_New[theIndex_New]));
						++theIndex_New;
						}
					}
				else
					{
					if (oChanged
						&& (fEmitDummyChanges
							|| (result.second < fIdentity.size() + fSignificant.size())))
						{
						// We care about changes, and comparison was terminated in the 'significant'
						// portion of the values. So they matched in the identity portion, and thus
						// reference the same entity, but differ in the significant portion, thus
						// this is a change.
						oChanged->push_back(
							Multi3<size_t,size_t,size_t>(
								theIndex_New, fSort_Prior[theIndex_Prior], theSort_New[theIndex_New]));
						}
					++theIndex_New;
					++theIndex_Prior;
					}
				}
			}
		swap(fSort_Prior, theSort_New);
		}

	if (oPriorResult)
		*oPriorResult = fResult_Prior;

	if (oCurResult)
		*oCurResult = theResult;

	fResult_Prior = theResult;
	}

// =================================================================================================
#pragma mark - ResultDiffer

void sBuildBindings(ZP<Result> iResult, map<string8,size_t>& oResult)
	{
	const RelHead& theRH = iResult->GetRelHead();
	size_t index = 0;
	for (auto&& entry: theRH)
		oResult.insert(pair<string8,size_t>(entry, index++));
	}

} // namespace QueryEngine
} // namespace ZooLib
