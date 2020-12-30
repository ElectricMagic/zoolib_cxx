// Copyright (c) 2016 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/Dataspace/Searcher_Datons.h"

#include "zoolib/Callable_PMF.h"
#include "zoolib/Compare.h"
#include "zoolib/Log.h"
#include "zoolib/Stringf.h"
#include "zoolib/Util_STL.h"
#include "zoolib/Util_STL_map.h"
#include "zoolib/Util_STL_vector.h"
#include "zoolib/Util_ZZ_JSON.h"

#include "zoolib/ZMACRO_foreach.h"

#include "zoolib/Dataspace/Daton_Val.h"

#include "zoolib/Expr/Util_Expr_Bool_CNF.h"

#include "zoolib/QueryEngine/ResultFromWalker.h"
#include "zoolib/QueryEngine/Util_Strim_Result.h"
#include "zoolib/QueryEngine/Util_Strim_Walker.h"
#include "zoolib/QueryEngine/Walker_Project.h"
#include "zoolib/QueryEngine/Walker_Result.h"
#include "zoolib/QueryEngine/Walker_Restrict.h"

#include "zoolib/RelationalAlgebra/RelHead.h"
#include "zoolib/RelationalAlgebra/Util_Strim_Rel.h"
#include "zoolib/RelationalAlgebra/Util_Strim_RelHead.h"

#include "zoolib/ValPred/ValPred_DB.h"
#include "zoolib/ValPred/Visitor_Expr_Bool_ValPred_DB_ToStrim.h"
#include "zoolib/ValPred/Visitor_Expr_Bool_ValPred_Do_GetNames.h"

namespace ZooLib {
namespace Dataspace {

using namespace Operators_ZZ_JSON;
using namespace Util_STL;

using std::make_pair;
using std::map;
using std::pair;
using std::set;
using std::vector;

namespace QE = QueryEngine;
namespace RA = RelationalAlgebra;

const ChanW_UTF& operator<<(const ChanW_UTF& ww, const Val_DB& iVal);
const ChanW_UTF& operator<<(const ChanW_UTF& ww, const Val_DB& iVal)
	{
	Util_ZZ_JSON::sWrite(ww, iVal.As<Val_ZZ>());
	return ww;
	}

// =================================================================================================
#pragma mark - Index

struct Searcher_Datons::Key
	{
	static const size_t kMaxCols = 4;
	const Searcher_Datons::Map_Thing::value_type* fMapEntryP;
	const Val_DB* fValues[kMaxCols];
	};

class Searcher_Datons::Index
	{
public:
	typedef Searcher_Datons::Key Key;

	// -----

	struct Comparer
		{
		Comparer(size_t iCount)
		:	fCount(iCount)
			{}

		bool spDoIt(const Key& iLeft, const Key& iRight) const
			{
			// When called by lower_bound iLeft is a key in the set, and iRight
			// is a key that's being looked for. So when iLeft has more entries
			// than iRight then iLeft is considered bigger than iRight.

			// When called by upper_bound iLeft is a key that's being looked for
			// and iRight is a key in the set. So when iLeft has fewer entries
			// than iRight then iLeft is is considered bigger than iRight.

			// The upshot being that when one or other vector of values is
			// exhausted we return false, indicating that iLeft is not smaller
			// than iRight.

			for (size_t xx = 0; xx < fCount; ++xx)
				{
				const Val_DB* valL = iLeft.fValues[xx];
				if (not valL)
					return false;
				const Val_DB* valR = iRight.fValues[xx];
				if (not valR)
					return false;

				if (const int compare = valL->Compare(*valR))
					return compare < 0;
				}
			// Tie-break on the *pointer*, just so keys are distinct.
			return iLeft.fMapEntryP < iRight.fMapEntryP;
			}

		static void spDump(bool result, const Key& iLeft, const Key& iRight);

		bool operator()(const Key& iLeft, const Key& iRight) const
			{
			bool result = spDoIt(iLeft, iRight);
			return result;
			}

		const size_t fCount;
		};

	// -----

	typedef std::set<Key,Comparer> Set;

	// -----

	Index(const IndexSpec& iIndexSpec)
	:	fCount(iIndexSpec.size())
	,	fSet(Comparer(fCount))
		{
		ZAssert(fCount <= Key::kMaxCols);
		std::copy_n(iIndexSpec.begin(), fCount, fColNames);
		}

	bool pAsKey(const Map_Thing::value_type* iMapEntryP, Key& oKey)
		{
		const Map_ZZ* asMap = iMapEntryP->second.PGet<Map_ZZ>();
		if (not asMap)
			{
			// iValPtr is not a map, can't index.
			return false;
			}

		const Val_DB* firstVal = asMap->PGet(fColNames[0]);
		if (not firstVal)
			{
			// The map does not have our first property, which we treat as a null. So it's
			// unlikely that anything will search for that with that as the first criteria
			// and we need not index it.
			return false;
			}

		const Val_DB* emptyValPtr = &sDefault<Val_DB>();

		oKey.fValues[0] = firstVal;
		for (size_t xx = 1; xx < fCount; ++xx)
			{
			if (const Val_DB* theVal = asMap->PGet(fColNames[xx]))
				oKey.fValues[xx] = theVal;
			else
				oKey.fValues[xx] = emptyValPtr;
			}

		for (size_t xx = fCount; xx < Key::kMaxCols; ++xx)
			oKey.fValues[xx] = nullptr;

		oKey.fMapEntryP = iMapEntryP;

		return true;
		}

	ColName fColNames[Key::kMaxCols];
	const size_t fCount;

	Set fSet;

	DListHead<DLink_PSearch_InIndex> fPSearch_InIndex;
	};

const ChanW_UTF& operator<<(const ChanW_UTF& ww, const Searcher_Datons::Index::Key& iKey);

void Searcher_Datons::Index::Comparer::spDump(bool iResult, const Key& iLeft, const Key& iRight)
	{
	if (ZLOGF(w, eDebug))
		w << iLeft << "/" << iResult<< "/" << iRight;
	}

// =================================================================================================
#pragma mark - Searcher_Datons::Walker_Map

class Searcher_Datons::Walker_Map
:	public QE::Walker
	{
public:
	Walker_Map(ZP<Searcher_Datons> iSearcher, const ConcreteHead& iConcreteHead)
	:	fSearcher(iSearcher)
	,	fConcreteHead(iConcreteHead)
		{}

	virtual ~Walker_Map()
		{}

// From QE::Walker
	virtual void Rewind()
		{
		this->Called_Rewind();
		fSearcher->pRewind(this);
		}

	virtual ZP<QE::Walker> Prime(const map<string8,size_t>& iOffsets,
		map<string8,size_t>& oOffsets,
		size_t& ioBaseOffset)
		{
		fSearcher->pPrime(this, iOffsets, oOffsets, ioBaseOffset);
		return this;
		}

	virtual bool QReadInc(Val_DB* ioResults)
		{
		this->Called_QReadInc();
		return fSearcher->pReadInc(this, ioResults);
		}

	const ZP<Searcher_Datons> fSearcher;
	const ConcreteHead fConcreteHead;
	size_t fBaseOffset;
	Map_Thing::const_iterator fCurrent;
	std::set<std::vector<Val_DB>> fPriors;
	};

// =================================================================================================
#pragma mark - Searcher_Datons::Walker_Index

class Searcher_Datons::Walker_Index
:	public QE::Walker
	{
public:
	Walker_Index(ZP<Searcher_Datons> iSearcher, Index* iIndex,
		size_t iUsableIndexNames,
		const ConcreteHead& iConcreteHead,
		Index::Set::const_iterator iBegin, Index::Set::const_iterator iEnd)
	:	fSearcher(iSearcher)
	,	fIndex(iIndex)
	,	fUsableIndexNames(iUsableIndexNames)
	,	fNameBoolVector(iConcreteHead.begin(), iConcreteHead.end())
	,	fBegin(iBegin)
	,	fEnd(iEnd)
	,	fPrior(iIndex->fCount + iConcreteHead.size())
		{}

	virtual ~Walker_Index()
		{}

// From QE::Walker
	virtual void Rewind()
		{
		this->Called_Rewind();
		fSearcher->pRewind(this);
		}

	virtual ZP<QE::Walker> Prime(const map<string8,size_t>& iOffsets,
		map<string8,size_t>& oOffsets,
		size_t& ioBaseOffset)
		{
		fSearcher->pPrime(this, iOffsets, oOffsets, ioBaseOffset);
		return this;
		}

	virtual bool QReadInc(Val_DB* ioResults)
		{
		this->Called_QReadInc();
		return fSearcher->pReadInc(this, ioResults);
		}

	const ZP<Searcher_Datons> fSearcher;
	Index* const fIndex;
	const size_t fUsableIndexNames;
	typedef pair<Name,bool> NameBool;
	typedef vector<NameBool> NameBoolVector;
	const NameBoolVector fNameBoolVector;
	size_t fBaseOffset;

	const Index::Set::const_iterator fBegin;
	const Index::Set::const_iterator fEnd;

	Index::Set::const_iterator fCurrent;
	std::vector<Val_DB> fPrior;
	};

// =================================================================================================
#pragma mark - Searcher_Datons::ClientSearch

class Searcher_Datons::DLink_ClientSearch_InPSearch
:	public DListLink<ClientSearch, DLink_ClientSearch_InPSearch, kDebug>
	{};

class Searcher_Datons::DLink_ClientSearch_NeedsWork
:	public DListLink<ClientSearch, DLink_ClientSearch_NeedsWork, kDebug>
	{};

class Searcher_Datons::ClientSearch
:	public DLink_ClientSearch_InPSearch
,	public DLink_ClientSearch_NeedsWork
	{
public:
	ClientSearch(int64 iRefcon, PSearch* iPSearch)
	:	fRefcon(iRefcon)
	,	fPSearch(iPSearch)
		{}

	int64 const fRefcon;
	PSearch* const fPSearch;
	};

// =================================================================================================
#pragma mark - Searcher_Datons::PSearch

typedef ZQ<pair<Val_DB,bool>> Bound_t; // Value, inclusive

class Searcher_Datons::DLink_PSearch_InIndex
:	public DListLink<PSearch, DLink_PSearch_InIndex, kDebug>
	{};

class Searcher_Datons::DLink_PSearch_NeedsWork
:	public DListLink<PSearch, DLink_PSearch_NeedsWork, kDebug>
	{};

class Searcher_Datons::PSearch
:	public DLink_PSearch_InIndex
,	public DLink_PSearch_NeedsWork
	{
public:
	PSearch(const SearchSpec& iSearchSpec)
	:	fSearchSpec(iSearchSpec)
	,	fIndex(nullptr)
		{}

	const SearchSpec fSearchSpec;

	size_t fUsableIndexNames;
	ConcreteHead fConcreteHead;
	RelHead fProjectionIfNecessary;

	Index* fIndex;

	vector<Val_DB> fValsEqual;
	Bound_t fRangeLo;
	Bound_t fRangeHi;
	ZP<Expr_Bool> fRestrictionRemainder;

	DListHead<DLink_ClientSearch_InPSearch> fClientSearch_InPSearch;

	ZP<QE::Result> fResult;
	};

// =================================================================================================
#pragma mark - Searcher_Datons

Searcher_Datons::Searcher_Datons(const vector<IndexSpec>& iIndexSpecs)
:	fChangeCount(0)
	{
	foreacha (entry, iIndexSpecs)
		fIndexes.push_back(new Index(entry));
	}

Searcher_Datons::~Searcher_Datons()
	{
	for (DListEraser<PSearch,DLink_PSearch_NeedsWork> eraser = fPSearch_NeedsWork;
		eraser; eraser.Advance())
		{}

	for (DListEraser<ClientSearch,DLink_ClientSearch_NeedsWork> eraser = fClientSearch_NeedsWork;
		eraser; eraser.Advance())
		{}

	sDeleteAll(fIndexes.begin(), fIndexes.end());
	}

bool Searcher_Datons::Intersects(const RelHead& iRelHead)
	{ return true; }

typedef ValComparator_Simple::EComparator EComparator;

static EComparator spFlipped(EComparator iEComparator)
	{
	switch (iEComparator)
		{
		case ValComparator_Simple::eLT: return ValComparator_Simple::eGT;
		case ValComparator_Simple::eLE: return ValComparator_Simple::eGE;
		case ValComparator_Simple::eEQ: return ValComparator_Simple::eEQ;
		case ValComparator_Simple::eNE: return ValComparator_Simple::eNE;
		case ValComparator_Simple::eGE: return ValComparator_Simple::eLE;
		case ValComparator_Simple::eGT: return ValComparator_Simple::eLT;
		}
	ZUnimplemented();
	}

// -----

static void spDump(const ChanW_UTF& ww,
	Searcher_Datons::Index* bestIndex,
	const vector<Val_DB>& bestValsEqual, const Bound_t& bestLo, const Bound_t& bestHi)
	{
	ww << "\n" << bestIndex << " ";
	if (size_t count = bestValsEqual.size())
		{
		ww << "(";
		for (size_t xx = 0; xx < count; ++xx)
			{
			if (xx)
				ww << " && ";
			ww << bestIndex->fColNames[xx] << " == " << bestValsEqual[xx];
			}
		ww << ")";
		}

	if (bestLo || bestHi)
		{
		ww << " Range(";
		if (bestLo)
			{
			ww << bestLo->first;
			if (bestLo->second)
				ww << " <= ";
			else
				ww << " < ";
			}

		ww << bestIndex->fColNames[bestValsEqual.size()];

		if (bestHi)
			{
			if (bestHi->second)
				ww << " <= ";
			else
				ww << " < ";
			ww << bestHi->first;
			}
		ww << ")";
		}
	}

void Searcher_Datons::pSetupPSearch(PSearch* ioPSearch)
	{
	using namespace Util_Expr_Bool;

	const SearchSpec& theSearchSpec = ioPSearch->fSearchSpec;

	const CNF theCNF = sAsCNF(theSearchSpec.GetRestriction());

	CNF bestDClauses;
	Index* bestIndex = nullptr;
	vector<Val_DB> bestValsEqual;
	Bound_t bestLo, bestHi;

	foreachv (Index* curIndex, fIndexes)
		{
		CNF curDClauses = theCNF;

		vector<Val_DB> valsEqual;

		Bound_t finalLo, finalHi;

		for (size_t xxColName = 0; xxColName < curIndex->fCount; ++xxColName)
			{
			const ColName& curColName = curIndex->fColNames[xxColName];

			Bound_t clausesLo, clausesHi;

			for (set<DClause>::iterator iterDClauses = curDClauses.begin();
				iterDClauses != curDClauses.end();
				/*no inc*/)
				{
				// Hack for now -- only start with success if we have a single clause.
				bool everyTermIsRelevant = iterDClauses->size() == 1;

				Bound_t termsLo, termsHi;

				for (set<Term>::iterator iterTerms = iterDClauses->begin();
					everyTermIsRelevant && iterTerms != iterDClauses->end();
					++iterTerms)
					{
					bool termIsRelevant = false;
					if (ZP<Expr_Bool_ValPred> theExpr = iterTerms->Get().DynamicCast<Expr_Bool_ValPred>())
						{
						const ValPred& theValPred = theExpr->GetValPred();

						if (ZP<ValComparator_Simple> theValComparator =
							theValPred.GetComparator().DynamicCast<ValComparator_Simple>())
							{
							EComparator theEComparator = theValComparator->GetEComparator();

							ZP<ValComparand_Const_DB> theComparand_Const =
								theValPred.GetRHS().DynamicCast<ValComparand_Const_DB>();

							ZP<ValComparand_Name> theComparand_Name =
								theValPred.GetLHS().DynamicCast<ValComparand_Name>();

							if (not theComparand_Const || not theComparand_Name)
								{
								theComparand_Const = theValPred.GetLHS().DynamicCast<ValComparand_Const_DB>();
								theComparand_Name = theValPred.GetRHS().DynamicCast<ValComparand_Name>();
								theEComparator = spFlipped(theEComparator);
								}

							if (theComparand_Const
								&& theComparand_Name
								&& theComparand_Name->GetName() == curColName)
								{
								termIsRelevant = true;

								const Val_DB& theVal = theComparand_Const->GetVal();

								switch (theEComparator)
									{
									case ValComparator_Simple::eLT:
										{
										termsLo.Clear();
										termsHi = Bound_t(theVal, false);
										break;
										}
									case ValComparator_Simple::eLE:
										{
										termsLo.Clear();
										termsHi = Bound_t(theVal, true);
										break;
										}
									case ValComparator_Simple::eEQ:
										{
										termsLo = Bound_t(theVal, true);
										termsHi = Bound_t(theVal, true);
										break;
										}
									case ValComparator_Simple::eGE:
										{
										termsLo = Bound_t(theVal, true);
										termsHi.Clear();
										break;
										}
									case ValComparator_Simple::eGT:
										{
										termsLo = Bound_t(theVal, false);
										termsHi.Clear();
										break;
										}
									default:
										{
										termIsRelevant = false;
										break;
										}
									}
								}
							}
						}

					everyTermIsRelevant = everyTermIsRelevant && termIsRelevant;
					} // iterTerms

				if (not everyTermIsRelevant)
					{
					++iterDClauses;
					}
				else
					{
					// Remove this DClause from further consideration -- its constraints will be
					// represented in curComparison.
					iterDClauses = sEraseInc(curDClauses, iterDClauses);

					if (not clausesLo)
						{
						clausesLo = termsLo;
						}
					else if (not termsLo)
						{}
					else if (termsLo->second == clausesLo->second)
						{
						// terms and clauses are both inclusive or exclusive.
						if (clausesLo->first < termsLo->first)
							clausesLo = termsLo;
						}
					else if (termsLo->second)
						{
						// clauses is inclusive and terms is exclusive. So C1 <= XX && T1 < XX
						if (clausesLo->first <= termsLo->first)
							clausesLo = termsLo;
						}
					else
						{
						// clauses is exclusive and terms is inclusive. So C1 < XX && T1 <= XX
						if (clausesLo->first < termsLo->first)
							clausesLo = termsLo;
						}

					if (not clausesHi)
						{
						clausesHi = termsHi;
						}
					else if (not termsHi)
						{}
					else if (termsHi->second == clausesHi->second)
						{
						// terms and clauses are both inclusive or exclusive.
						if (clausesHi->first > termsHi->first)
							clausesHi = termsHi;
						}
					else if (termsHi->second)
						{
						// clauses is exclusive and terms is inclusive. So XX < C1 && XX <= T1
						if (clausesHi->first > termsHi->first)
							clausesHi = termsHi;
						}
					else
						{
						// clauses is inclusive and terms is exclusive. So XX <= C1 && XX < T1
						if (clausesHi->first >= termsHi->first)
							clausesHi = termsHi;
						}
					} // not everyTermIsRelevant
				} // iterDClauses

			if (clausesLo && clausesHi
				&& clausesLo->second and clausesHi->second
				&& clausesLo->first == clausesHi->first)
				{
				// It's an equality.
				valsEqual.push_back(clausesLo->first);
				}
			else
				{
				finalLo = clausesLo;
				finalHi = clausesHi;
				break;
				}
			} // xxColName

		if (curDClauses.size() < theCNF.size())
			{
			// We were able to remove at least one clause.

			if (not bestIndex || curDClauses.size() < bestDClauses.size())
				{
				// This is the first usable index, or we've removed more clauses than the prior best index.
				bestIndex = curIndex;
				bestValsEqual = valsEqual;
				bestLo = finalLo;
				bestHi = finalHi;
				bestDClauses = curDClauses;
				}
			}
		}

	// We've got valsEqual filled in with stuff we're doing an equality search on, and
	// may have a comparison in finalLo/finalHi.

	const RelHead theRH_Wanted = RA::sRelHead(theSearchSpec.GetConcreteHead());

	RelHead theRH_Required, theRH_Optional;
	RA::sRelHeads(theSearchSpec.GetConcreteHead(), theRH_Required, theRH_Optional);

	// Add in any names in restriction that weren't provided in the searchspec's CH.
	ioPSearch->fConcreteHead = RA::sAugmentedOptional(
		theSearchSpec.GetConcreteHead(),
		sGetNames(theSearchSpec.GetRestriction()));

	if (theRH_Wanted != sRelHead(ioPSearch->fConcreteHead))
		{
		// There were names in the restriction that aren't in
		// the searchspec's CH, so we must project them out.
		ioPSearch->fProjectionIfNecessary = theRH_Wanted;
		}

	if (true && bestIndex)
		{
		ioPSearch->fIndex = bestIndex;

		sInsertBackMust(bestIndex->fPSearch_InIndex, ioPSearch);
		ioPSearch->fValsEqual.swap(bestValsEqual);
		ioPSearch->fRangeLo = bestLo;
		ioPSearch->fRangeHi = bestHi;
		ioPSearch->fRestrictionRemainder = sFromCNF(bestDClauses);

		ioPSearch->fUsableIndexNames = ioPSearch->fValsEqual.size();
		if (bestLo || bestHi)
			++ioPSearch->fUsableIndexNames;

		// Remove the indexed names from theCH.
		for (size_t xxColName = 0; xxColName < ioPSearch->fUsableIndexNames; ++xxColName)
			sQErase(ioPSearch->fConcreteHead, ioPSearch->fIndex->fColNames[xxColName]);
		}
	}

static void spDump(const ChanW_UTF& ww,
	const SearchSpec& theSearchSpec,
	const vector<Searcher_Datons::Index*>& fIndexes)
	{
	ww << "\n" << "ConcreteHead: " << theSearchSpec.GetConcreteHead();
	ww << "\n" << "Restriction: ";
	Visitor_Expr_Bool_ValPred_DB_ToStrim().ToStrim(ww,
		sDefault(), theSearchSpec.GetRestriction());

	foreacha (anIndex, fIndexes)
		{
		ww << "\n" << anIndex->fSet.size() << " entries, indexed on: ";
		for (size_t xx = 0; xx < anIndex->fCount; ++xx)
			ww << anIndex->fColNames[xx] << " ";

		foreacha (entry, anIndex->fSet)
			{
			ww << "\n";
			for (size_t xx = 0; xx < anIndex->fCount; ++xx)
				ww << *(entry.fValues[xx]) << " ";
			ww << "--> " << entry.fMapEntryP->second;
			}
		}
	}

void Searcher_Datons::ModifyRegistrations(
	const AddedSearch* iAdded, size_t iAddedCount,
	const int64* iRemoved, size_t iRemovedCount)
	{
	ZAcqMtx acq(fMtx);

	for (/*no init*/; iAddedCount--; ++iAdded)
		{
		const SearchSpec& theSearchSpec = iAdded->GetSearchSpec();

		const pair<Map_SearchSpec_PSearch::iterator,bool>
			iterPSearchPair = fMap_SearchSpec_PSearch.insert(
				Map_SearchSpec_PSearch::value_type(theSearchSpec, theSearchSpec));

		PSearch* thePSearch = &iterPSearchPair.first->second;

		if (iterPSearchPair.second)
			{
			// It's a new PSearch, so we'll need to work on it
			sInsertBackMust(fPSearch_NeedsWork, thePSearch);

			// and get it hooked up.
			this->pSetupPSearch(thePSearch);
			}

		const int64 theRefcon = iAdded->GetRefcon();

		const pair<map<int64,ClientSearch>::iterator,bool>
			iterClientSearchPair = fMap_Refcon_ClientSearch.insert(
				make_pair(theRefcon, ClientSearch(theRefcon, thePSearch)));
		ZAssert(iterClientSearchPair.second);

		ClientSearch* theClientSearch = &iterClientSearchPair.first->second;
		sInsertBackMust(thePSearch->fClientSearch_InPSearch, theClientSearch);

		sInsertBackMust(fClientSearch_NeedsWork, theClientSearch);
		}

	while (iRemovedCount--)
		{
		const int64 theRefcon = *iRemoved++;

		map<int64, ClientSearch>::iterator iterClientSearch =
			fMap_Refcon_ClientSearch.find(theRefcon);

		ZAssertStop(kDebug, iterClientSearch != fMap_Refcon_ClientSearch.end());

		ClientSearch* theClientSearch = &iterClientSearch->second;

		PSearch* thePSearch = theClientSearch->fPSearch;
		if (thePSearch->fIndex)
			sEraseMust(thePSearch->fIndex->fPSearch_InIndex, thePSearch);

		sEraseMust(thePSearch->fClientSearch_InPSearch, theClientSearch);
		if (sIsEmpty(thePSearch->fClientSearch_InPSearch))
			{
			sQErase(fPSearch_NeedsWork, thePSearch);
			sEraseMust(kDebug, fMap_SearchSpec_PSearch, thePSearch->fSearchSpec);
			}

		sQErase(fClientSearch_NeedsWork, theClientSearch);
		fMap_Refcon_ClientSearch.erase(iterClientSearch);
		}

	if (sNotEmpty(fClientSearch_NeedsWork) || sNotEmpty(fPSearch_NeedsWork))
		{
		ZRelMtx rel(fMtx);
		Searcher::pTriggerSearcherResultsAvailable();
		}
	}

namespace { // anonymous
template <class PP>
PP* spAllOnesPointer()
	{ return reinterpret_cast<PP*>((char*)(0)-1); }
} // anonymous namespace

void Searcher_Datons::CollectResults(vector<SearchResult>& oChanged, int64& oChangeCount)
	{
	Searcher::pCollectResultsCalled();

	ZAcqMtx acq(fMtx);

	oChanged.clear();
	oChangeCount = fChangeCount;

	for (DListEraser<PSearch,DLink_PSearch_NeedsWork> eraser = fPSearch_NeedsWork;
		eraser; eraser.Advance())
		{
		PSearch* thePSearch = eraser.Current();

		if (not thePSearch->fResult)
			{
			const SearchSpec& theSearchSpec = thePSearch->fSearchSpec;

			ZP<QE::Walker> theWalker;

			if (thePSearch->fIndex)
				{
				Index::Key theKey;

				const size_t countEqual = thePSearch->fValsEqual.size();
				const size_t countAll = Key::kMaxCols;//thePSearch->fIndex->fCount;
				ZAssert(countEqual <= countAll);
				for (size_t xx = 0; xx < countEqual; ++xx)
					theKey.fValues[xx] = &thePSearch->fValsEqual[xx];

				for (size_t xx = countEqual + 1; xx < countAll; ++xx)
					theKey.fValues[xx] = nullptr;

				Index::Set::const_iterator theBegin;
				if (not thePSearch->fRangeLo)
					{
					theKey.fValues[countEqual] = nullptr;
					theKey.fMapEntryP = nullptr;
					theBegin = thePSearch->fIndex->fSet.lower_bound(theKey);
					}
				else
					{
					theKey.fValues[countEqual] = &thePSearch->fRangeLo->first;
					if (thePSearch->fRangeLo->second)
						{
						theKey.fMapEntryP = nullptr;
						theBegin = thePSearch->fIndex->fSet.lower_bound(theKey);
						}
					else
						{
						theKey.fMapEntryP = spAllOnesPointer<Map_Thing::value_type>();
						theBegin = thePSearch->fIndex->fSet.upper_bound(theKey);
						}
					}

				Index::Set::const_iterator theEnd;
				if (not thePSearch->fRangeHi)
					{
					theKey.fValues[countEqual] = nullptr;
					theKey.fMapEntryP = spAllOnesPointer<Map_Thing::value_type>();
					theEnd = thePSearch->fIndex->fSet.upper_bound(theKey);
					}
				else
					{
					theKey.fValues[countEqual] = &thePSearch->fRangeHi->first;
					if (thePSearch->fRangeHi->second)
						{
						theKey.fMapEntryP = spAllOnesPointer<Map_Thing::value_type>();
						theEnd = thePSearch->fIndex->fSet.upper_bound(theKey);
						}
					else
						{
						theKey.fMapEntryP = nullptr;
						theEnd = thePSearch->fIndex->fSet.lower_bound(theKey);
						}
					}

				theWalker = new Walker_Index(this,
					thePSearch->fIndex, thePSearch->fUsableIndexNames, thePSearch->fConcreteHead,
					theBegin, theEnd);

				if (thePSearch->fRestrictionRemainder && thePSearch->fRestrictionRemainder != sTrue())
					theWalker = new QE::Walker_Restrict(theWalker, thePSearch->fRestrictionRemainder);
				}
			else
				{
				theWalker = new Walker_Map(this, thePSearch->fConcreteHead);

				const ZP<Expr_Bool>& theRestriction = theSearchSpec.GetRestriction();
				if (theRestriction && theRestriction != sTrue())
					theWalker = new QE::Walker_Restrict(theWalker, theRestriction);
				}

			if (sNotEmpty(thePSearch->fProjectionIfNecessary))
				theWalker = new QE::Walker_Project(theWalker, thePSearch->fProjectionIfNecessary);

			const double start = Time::sSystem();

			thePSearch->fResult = QE::sResultFromWalker(theWalker);

			const double elapsed = Time::sSystem() - start;

			if (elapsed > 50e-3)
				{
				if (ZLOGPF(ww, eDebug))
					{
					ww << "\nSlow PSearch " << elapsed * 1e3 << "ms: ";
					Visitor_Expr_Bool_ValPred_DB_ToStrim()
						.ToStrim(ww, sDefault(), theSearchSpec.GetRestriction());
					if (thePSearch->fRestrictionRemainder)
						{
						ww << "\nRestrictionRemainder: ";
						Visitor_Expr_Bool_ValPred_DB_ToStrim()
							.ToStrim(ww, sDefault(), thePSearch->fRestrictionRemainder);
						}

					ww << "\n";
					sToStrim(ww, thePSearch->fResult);

					sDumpWalkers(ww, theWalker);
					}
				}

			for (DListIterator<ClientSearch, DLink_ClientSearch_InPSearch>
				iter = thePSearch->fClientSearch_InPSearch; iter; iter.Advance())
				{ sQInsertBack(fClientSearch_NeedsWork, iter.Current()); }
			}
		}

	for (DListEraser<ClientSearch,DLink_ClientSearch_NeedsWork> eraser = fClientSearch_NeedsWork;
		eraser; eraser.Advance())
		{
		ClientSearch* theClientSearch = eraser.Current();
		PSearch* thePSearch = theClientSearch->fPSearch;
		oChanged.push_back(SearchResult(theClientSearch->fRefcon, thePSearch->fResult));
		}
	}

int64 Searcher_Datons::MakeChanges(
	const Daton* iAsserted, size_t iAssertedCount,
	const Daton* iRetracted, size_t iRetractedCount)
	{
	ZAcqMtx acq(fMtx);

	while (iAssertedCount--)
		{
		const Daton theDaton = *iAsserted++;
		Map_Thing::iterator iterLB = fMap_Thing.lower_bound(theDaton);

		if (iterLB == fMap_Thing.end() || theDaton != iterLB->first)
			{
			Map_Thing::const_iterator iter =
				fMap_Thing.insert(iterLB, make_pair(theDaton, sAsVal(theDaton)));
			this->pIndexInsert(&*iter);
			}
		}

	while (iRetractedCount--)
		{
		const Daton theDaton = *iRetracted++;
		Map_Thing::iterator iter = fMap_Thing.find(theDaton);
		if (iter != fMap_Thing.end())
			{
			this->pIndexErase(&*iter);
			fMap_Thing.erase(iter);
			}
		}

	// Invalidate all PSearches unattached to indexes.
	for (Map_SearchSpec_PSearch::iterator
		iter = fMap_SearchSpec_PSearch.begin(), end = fMap_SearchSpec_PSearch.end();
		iter != end; ++iter)
		{
		if (not iter->second.fIndex)
			{
			iter->second.fResult.Clear();
			sQInsertBack(fPSearch_NeedsWork, &iter->second);
			}
		}

	int64 theChangeCount = ++fChangeCount;

	if (sNotEmpty(fClientSearch_NeedsWork) || sNotEmpty(fPSearch_NeedsWork))
		{
		ZRelMtx rel(fMtx);
		Searcher::pTriggerSearcherResultsAvailable();
		}

	return theChangeCount;
	}

void Searcher_Datons::pInvalidateSearchIfAppropriate(PSearch* iPSearch, const Key& iKey)
	{
	if (iPSearch->fResult && not sContains(fPSearch_NeedsWork, iPSearch))
		{
		// We will invalidate only if iKey matches iPSearch.
		const size_t countEqual = iPSearch->fValsEqual.size();

		bool allMatch = true;

		if (allMatch)
			{
			if (iPSearch->fRangeLo)
				{
				if (iPSearch->fRangeLo->second)
					allMatch = iPSearch->fRangeLo->first <= *iKey.fValues[countEqual];
				else
					allMatch = iPSearch->fRangeLo->first < *iKey.fValues[countEqual];
				}
			}

		if (allMatch)
			{
			if (iPSearch->fRangeHi)
				{
				if (iPSearch->fRangeHi->second)
					allMatch = iPSearch->fRangeHi->first >= *iKey.fValues[countEqual];
				else
					allMatch = iPSearch->fRangeHi->first > *iKey.fValues[countEqual];
				}
			}

		if (allMatch)
			{
			for (size_t xx = countEqual; xx > 0;)
				{
				--xx;
				if (*iKey.fValues[xx] != iPSearch->fValsEqual[xx])
					{
					allMatch = false;
					break;
					}
				}
			}

		// We're ignoring iPSearch->fRestrictionRemainder, so we will have some false positives.

		if (allMatch)
			{
			sQInsertBack(fPSearch_NeedsWork, iPSearch);
			iPSearch->fResult.Clear();
			}
		}
	}

void Searcher_Datons::pIndexInsert(const Map_Thing::value_type* iMapEntryP)
	{
	foreacha (anIndex, fIndexes)
		{
		Key theKey;
		if (anIndex->pAsKey(iMapEntryP, theKey))
			{
			sInsertMust(anIndex->fSet, theKey);
			for (DListIterator<PSearch,DLink_PSearch_InIndex> iter = anIndex->fPSearch_InIndex;
				iter; iter.Advance())
				{
				this->pInvalidateSearchIfAppropriate(iter.Current(), theKey);
				}
			}
		}
	}

void Searcher_Datons::pIndexErase(const Map_Thing::value_type* iMapEntryP)
	{
	foreacha (anIndex, fIndexes)
		{
		Key theKey;
		if (anIndex->pAsKey(iMapEntryP, theKey))
			{
			sEraseMust(anIndex->fSet, theKey);
			for (DListIterator<PSearch,DLink_PSearch_InIndex> iter = anIndex->fPSearch_InIndex;
				iter; iter.Advance())
				{
				this->pInvalidateSearchIfAppropriate(iter.Current(), theKey);
				}
			}
		}
	}

void Searcher_Datons::pRewind(ZP<Walker_Map> iWalker_Map)
	{
	iWalker_Map->fCurrent = fMap_Thing.begin();
	}

void Searcher_Datons::pPrime(ZP<Walker_Map> iWalker_Map,
	const map<string8,size_t>& iOffsets,
	map<string8,size_t>& oOffsets,
	size_t& ioBaseOffset)
	{
	iWalker_Map->fCurrent = fMap_Thing.begin();
	iWalker_Map->fBaseOffset = ioBaseOffset;
	foreacha (entry, iWalker_Map->fConcreteHead)
		oOffsets[entry.first] = ioBaseOffset++;
	}

bool Searcher_Datons::pReadInc(ZP<Walker_Map> iWalker_Map, Val_DB* ioResults)
	{
	const ConcreteHead& theConcreteHead = iWalker_Map->fConcreteHead;

	while (iWalker_Map->fCurrent != fMap_Thing.end())
		{
		if (const Map_ZZ* theMap = iWalker_Map->fCurrent->second.PGet<Map_ZZ>())
			{
			bool gotAll = true;
			vector<Val_DB> subset;
			subset.reserve(theConcreteHead.size());
			size_t offset = iWalker_Map->fBaseOffset;
			for (ConcreteHead::const_iterator
				ii = theConcreteHead.begin(), end = theConcreteHead.end();
				ii != end; ++ii, ++offset)
				{
				const string8& theName = ii->first;
				if (theName.empty())
					{
					// Empty name indicates that we want the Daton itself.
					const Val_DB& theVal = iWalker_Map->fCurrent->first;
					ioResults[offset] = theVal;
					subset.push_back(theVal);
					}
				else if (const Val_DB* theVal = sPGet(*theMap, theName))
					{
					ioResults[offset] = *theVal;
					subset.push_back(*theVal);
					}
				else if (not ii->second)
					{
					ioResults[offset] = AbsentOptional_t();
					subset.push_back(AbsentOptional_t());
					}
				else
					{
					gotAll = false;
					break;
					}
				}

			if (gotAll && sQInsert(iWalker_Map->fPriors, subset))
				{
				++iWalker_Map->fCurrent;
				return true;
				}
			}
		++iWalker_Map->fCurrent;
		}

	return false;
	}

void Searcher_Datons::pRewind(ZP<Walker_Index> iWalker_Index)
	{ iWalker_Index->fCurrent = iWalker_Index->fBegin; }

void Searcher_Datons::pPrime(ZP<Walker_Index> iWalker_Index,
	const map<string8,size_t>& iOffsets,
	map<string8,size_t>& oOffsets,
	size_t& ioBaseOffset)
	{
	iWalker_Index->fCurrent = iWalker_Index->fBegin;
	iWalker_Index->fBaseOffset = ioBaseOffset;

	for (size_t xxColName = 0; xxColName < iWalker_Index->fUsableIndexNames; ++xxColName)
		oOffsets[iWalker_Index->fIndex->fColNames[xxColName]] = ioBaseOffset++;

	foreacha (entry, iWalker_Index->fNameBoolVector)
		oOffsets[entry.first] = ioBaseOffset++;
	}

static const Val_DB spVal_AbsentOptional = AbsentOptional_t();

bool Searcher_Datons::pReadInc(ZP<Walker_Index> iWalker_Index, Val_DB* ioResults)
	{
	const size_t theCount_Indexed = iWalker_Index->fUsableIndexNames;
	const auto& theNBV = iWalker_Index->fNameBoolVector;
	const size_t theCount_NBV = theNBV.size();
	vector<const Val_DB*> theValPtrs(theCount_Indexed + theCount_NBV);

	Val_DB theVal_Daton;

	while (iWalker_Index->fCurrent != iWalker_Index->fEnd)
		{
		const Map_Thing::value_type* theTarget = iWalker_Index->fCurrent->fMapEntryP;

		if (const Map_ZZ* theMap = theTarget->second.PGet<Map_ZZ>())
			{
			// It's a map, and thus usable.

			// Transcribe the values in the current key into theValPtrs.
			for (size_t xx = 0; xx < theCount_Indexed; ++xx)
				theValPtrs[xx] = iWalker_Index->fCurrent->fValues[xx];

			bool gotAll = true;
			for (size_t xx = 0; xx < theCount_NBV; ++xx)
				{
				const auto& theNB = theNBV[xx];
				const Name& theName = theNB.first;
				if (sIsEmpty(theName))
					{
					// Empty name indicates that we want the Daton itself.
					theVal_Daton = theTarget->first;
					theValPtrs[theCount_Indexed + xx] = &theVal_Daton;
					}
				else if (const Val_DB* theValPtr = sPGet(*theMap, theName))
					{
					theValPtrs[theCount_Indexed + xx] = theValPtr;
					}
				else if (not theNB.second)
					{
					theValPtrs[theCount_Indexed + xx] = &spVal_AbsentOptional;
					}
				else
					{
					gotAll = false;
					break;
					}
				}

			if (gotAll)
				{
				bool allMatch = true;
				const size_t theCount = theValPtrs.size();
				const Val_DB* iterPrior = &iWalker_Index->fPrior[theCount-1];
				const Val_DB** iterCurr = &theValPtrs[theCount-1];
				for (size_t count = theCount + 1; --count; /*no inc*/)
					{
					if (*iterPrior-- != **iterCurr--)
						{
						allMatch = false;
						break;
						}
					}

				if (not allMatch)
					{
					for (size_t xx = 0; xx < theCount; ++xx)
						{
						const Val_DB* theValPtr = theValPtrs[xx];
						ioResults[iWalker_Index->fBaseOffset + xx] = *theValPtr;
						iWalker_Index->fPrior[xx] = *theValPtr;
						}
					++iWalker_Index->fCurrent;
					return true;
					}
				}
			}
		else
			{
			// Not a Map, can't do anything with it.
			}

		// But must advance the walker regardless.
		++iWalker_Index->fCurrent;
		}

	return false;
	}

// =================================================================================================
#pragma mark - XCode function popup chokes if this is earlier

const ChanW_UTF& operator<<(const ChanW_UTF& ww, const Searcher_Datons::Index::Key& iKey)
	{
	ww << sStringf("%p", iKey.fMapEntryP) << ": ";
	for (size_t xx = 0; xx < countof(iKey.fValues); ++xx)
		{
		if (not iKey.fValues[xx])
			break;
		if (xx)
			ww << ", ";
		ww << *iKey.fValues[xx];
		}
	return ww;
	}

} // namespace Dataspace
} // namespace ZooLib
