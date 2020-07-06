/* -------------------------------------------------------------------------------------------------
Copyright (c) 2014 Andrew Green
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

#ifndef __ZooLib_Dataspace_Searcher_h__
#define __ZooLib_Dataspace_Searcher_h__ 1
#include "zconfig.h"

#include "zoolib/Callable.h"
#include "zoolib/StdInt.h"
#include "zoolib/Util_Relops.h"
#include "zoolib/ValueOnce.h"

#include "zoolib/Expr/Expr_Bool.h"

#include "zoolib/Dataspace/Types.h"
#include "zoolib/QueryEngine/Result.h"

namespace ZooLib {
namespace Dataspace {

// =================================================================================================
#pragma mark - SearchSpec

class SearchSpec
	{
public:
	SearchSpec();
	SearchSpec(const SearchSpec& iOther);
	~SearchSpec();
	SearchSpec operator=(const SearchSpec& iOther);

	SearchSpec(const ConcreteHead& iConcreteHead,
		const ZP<Expr_Bool>& iRestriction);

	bool operator==(const SearchSpec& iOther) const;
	bool operator<(const SearchSpec& iOther) const;

	const ConcreteHead& GetConcreteHead() const;
	const ZP<Expr_Bool>& GetRestriction() const;

private:
	ConcreteHead fConcreteHead;
	ZP<Expr_Bool> fRestriction;
	};

// =================================================================================================
#pragma mark - AddedSearch

class AddedSearch
	{
public:
	AddedSearch();
	AddedSearch(const AddedSearch& iOther);
	~AddedSearch();
	AddedSearch& operator=(const AddedSearch& iOther);

	AddedSearch(int64 iRefcon, const SearchSpec& iSearchSpec);

	int64 GetRefcon() const;
	const SearchSpec& GetSearchSpec() const;

private:
	int64 fRefcon;
	SearchSpec fSearchSpec;
	};

// =================================================================================================
#pragma mark - SearchResult

class SearchResult
	{
public:
	SearchResult();
	SearchResult(const SearchResult& iOther);
	~SearchResult();
	SearchResult& operator=(const SearchResult& iOther);

	SearchResult(int64 iRefcon, const ZP<QueryEngine::Result>& iResult);

	int64 GetRefcon() const;
	ZP<QueryEngine::Result> GetResult() const;

private:
	int64 fRefcon;
	ZP<QueryEngine::Result> fResult;
	};

// =================================================================================================
#pragma mark - Searcher

class Searcher
:	public Counted
	{
public:
	Searcher();
	virtual ~Searcher();

	virtual bool Intersects(const RelHead& iRelHead) = 0;

	virtual void ModifyRegistrations(
		const AddedSearch* iAdded, size_t iAddedCount,
		const int64* iRemoved, size_t iRemovedCount) = 0;

	virtual void CollectResults(std::vector<SearchResult>& oChanged, int64& oChangeCount) = 0;

	typedef Callable<void(ZP<Searcher>)> Callable_SearcherResultsAvailable;
	void SetCallable_SearcherResultsAvailable(ZP<Callable_SearcherResultsAvailable> iCallable);

protected:
	void pCollectResultsCalled();
	void pTriggerSearcherResultsAvailable();

private:
	ZMtx fMtx;
	FalseOnce fCalled_SearcherResultsAvailable;
	ZP<Callable_SearcherResultsAvailable> fCallable_SearcherResultsAvailable;
	};

} // namespace Dataspace

template <> struct RelopsTraits_HasEQ<Dataspace::SearchSpec> : public RelopsTraits_Has {};
template <> struct RelopsTraits_HasLT<Dataspace::SearchSpec> : public RelopsTraits_Has {};

} // namespace ZooLib

#endif // __ZooLib_Dataspace_Searcher_h__
