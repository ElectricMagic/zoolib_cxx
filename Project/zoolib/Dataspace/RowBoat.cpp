// Copyright (c) 2014 Andrew Green. MIT License. http://www.zoolib.org

#include "zoolib/Dataspace/RowBoat.h"

#include "zoolib/Callable_PMF.h"
#include "zoolib/Util_STL_map.h"
#include "zoolib/Util_STL_set.h"
#include "zoolib/Util_STL_vector.h"
#include "zoolib/ZMACRO_foreach.h"

#include "zoolib/RelationalAlgebra/PseudoMap.h"
#include "zoolib/RelationalAlgebra/Util_Rel_Operators.h"

#include "zoolib/ValPred/Expr_Bool_ValPred.h"
#include "zoolib/ValPred/ValPred_DB.h"

namespace ZooLib {
namespace Dataspace {

using namespace Util_STL;

using std::vector;

// =================================================================================================
#pragma mark - RowBoat

RowBoat::RowBoat(const ZP<Callable_Register>& iCallable_Register,
	const ZP<Expr_Rel>& iRel,
	const RelHead& iIdentity, const RelHead& iSignificant,
	bool iEmitDummyChanges,
	const ZP<Callable_Make_Callable_Row>& iCallable)
:	fCallable_Register(iCallable_Register)
,	fRel(iRel)
,	fResultDiffer(iIdentity, iSignificant, iEmitDummyChanges)
,	fCallable(iCallable)
	{}

RowBoat::~RowBoat()
	{}

void RowBoat::Initialize()
	{
	Counted::Initialize();
	fRegistration = fCallable_Register->Call(sCallable(sWP(this), &RowBoat::pChanged), fRel);
	}

const vector<ZP<RowBoat::Callable_Row>>& RowBoat::GetRows()
	{ return fRows; }

void RowBoat::pChanged(
	const ZP<Counted>& iRegistration,
	int64 iChangeCount,
	const ZP<Result>& iResult,
	const ZP<ResultDeltas>& iResultDeltas)
	{
	ZP<Result> priorResult;

	vector<size_t> theRemoved;
	vector<std::pair<size_t,size_t>> theAdded;
	vector<Multi3<size_t,size_t,size_t>> theChanged;

	ZP<Result> curResult;
	fResultDiffer.Apply(iResult, &priorResult, null, &curResult, &theRemoved, &theAdded, &theChanged);

	if (priorResult)
		{
		// We generate bindings the first time we're called, so they must be non-empty at this point.
		ZAssert(sNotEmpty(fBindings));

		// RelHeads can't and mustn't change from one result to another.
		ZAssert(priorResult->GetRelHead() == curResult->GetRelHead());
		}
	else
		{
		ZAssert(sIsEmpty(fBindings));
		// Build fBindings the first time we get a result.
		sBuildBindings(curResult, fBindings);
		}

	// Note that we're doing a reverse scan here...
	for (vector<size_t>::reverse_iterator riter = theRemoved.rbegin();
		riter != theRemoved.rend(); ++riter)
		{
		size_t yy = *riter;
		ZAssert(yy < fRows.size());
		if (ZP<Callable_Row> theRow = fRows[yy])
			{
			const PseudoMap thePM_Prior(&fBindings, priorResult->GetValsAt(yy));
			theRow->Call(&thePM_Prior, nullptr);
			}
		fRows.erase(fRows.begin() + yy);
		}

	// ...and forward scans here...
	foreacha (yy, theAdded)
		{
		ZAssert(yy.first <= fRows.size());
		const PseudoMap thePM_New(&fBindings, curResult->GetValsAt(yy.second));
		ZP<Callable_Row> theRow = fCallable->Call(thePM_New);
		fRows.insert(fRows.begin() + yy.first, theRow);
		if (theRow)
			theRow->Call(nullptr, &thePM_New);
		}

	// ...and here...
	foreacha (yy, theChanged)
		{
		if (ZP<Callable_Row> theRow = fRows[yy.f0])
			{
			const PseudoMap thePM_Prior(&fBindings, priorResult->GetValsAt(yy.f1));
			const PseudoMap thePM_New(&fBindings, curResult->GetValsAt(yy.f2));
			theRow->Call(&thePM_Prior, &thePM_New);
			}
		}
	}

} // namespace Dataspace
} // namespace ZooLib
