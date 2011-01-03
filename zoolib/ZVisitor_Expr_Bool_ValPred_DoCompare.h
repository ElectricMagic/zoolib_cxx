/* -------------------------------------------------------------------------------------------------
Copyright (c) 2010 Andrew Green
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

#ifndef __ZVisitor_Expr_Bool_ValPred_DoCompare__
#define __ZVisitor_Expr_Bool_ValPred_DoCompare__ 1
#include "zconfig.h"

#include "zoolib/ZExpr_Bool_ValPred.h"
#include "zoolib/ZVisitor_Expr_Bool_DoCompare.h"

namespace ZooLib {
namespace Visitor_Expr_Bool_ValPred_DoCompare {

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_ValPred_DoCompare::Comparer_Bootstrap

struct Comparer_Bootstrap
:	public virtual Visitor_Expr_Bool_DoCompare::Comparer_Bootstrap
,	public virtual ZVisitor_Expr_Bool_ValPred
	{
public:
	virtual void Visit_Expr_Bool_ValPred(const ZRef<ZExpr_Bool_ValPred>& iExpr);
	};

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_ValPred_DoCompare::Comparer

struct Comparer
:	public virtual Visitor_Expr_Bool_DoCompare::Comparer
,	public virtual ZVisitor_Expr_Bool_ValPred
	{
	virtual void Visit_Expr_Bool_ValPred(const ZRef<ZExpr_Bool_ValPred>& iRep);
	};

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_ValPred_DoCompare::Comparer_GT_ValPred

struct Comparer_GT_ValPred
:	public virtual Visitor_Expr_Bool_DoCompare::Comparer_GT_Or
,	public virtual ZVisitor_Expr_Bool_ValPred
	{
	virtual void Visit_Expr_Bool_ValPred(const ZRef<ZExpr_Bool_ValPred>&);
	};

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_ValPred_DoCompare::Comparer_ValPred

class Comparer_ValPred
:	public virtual Visitor_Expr_Bool_DoCompare::Comparer_GT_Or
,	public virtual ZVisitor_Expr_Bool_ValPred
	{
public:
	Comparer_ValPred(
		Visitor_Expr_Bool_DoCompare::Comparer_Bootstrap* iBootstrap,
		const ZRef<ZExpr_Bool_ValPred>& iExpr);

	virtual void Visit_Expr_Bool_ValPred(const ZRef<ZExpr_Bool_ValPred>& iExpr);

private:
	const ZRef<ZExpr_Bool_ValPred> fExpr;
	};

} // namespace Visitor_Expr_Bool_ValPred_DoCompare
} // namespace ZooLib

#endif // __ZVisitor_Expr_Bool_ValPred_DoCompare__
