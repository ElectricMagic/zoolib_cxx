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

#ifndef __ZVisitor_Expr_Bool_DoCompare__
#define __ZVisitor_Expr_Bool_DoCompare__ 1
#include "zconfig.h"

#include "zoolib/ZCompare_T.h"
#include "zoolib/ZExpr_Bool.h"
#include "zoolib/ZVisitor_Do_T.h"

namespace ZooLib {
namespace Visitor_Expr_Bool_DoCompare {

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_DoCompare::Comparer_Bootstrap

struct Comparer_Bootstrap
:	public virtual ZVisitor_Do_T<int>
,	public virtual ZVisitor_Expr_Bool_True
,	public virtual ZVisitor_Expr_Bool_False
,	public virtual ZVisitor_Expr_Bool_Not
,	public virtual ZVisitor_Expr_Bool_And
,	public virtual ZVisitor_Expr_Bool_Or
	{
protected:
	ZRef<ZExpr_Bool> fExpr;

public:
	Comparer_Bootstrap();

	int Compare(const ZRef<ZExpr_Bool>& iLHS, const ZRef<ZExpr_Bool>& iRHS);

	virtual void Visit_Expr_Bool_True(const ZRef<ZExpr_Bool_True>&);
	virtual void Visit_Expr_Bool_False(const ZRef<ZExpr_Bool_False>&);
	virtual void Visit_Expr_Bool_Not(const ZRef<ZExpr_Bool_Not>& iExpr);
	virtual void Visit_Expr_Bool_And(const ZRef<ZExpr_Bool_And>& iExpr);
	virtual void Visit_Expr_Bool_Or(const ZRef<ZExpr_Bool_Or>& iExpr);
	};

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_DoCompare::Comparer

class Comparer
:	public virtual ZVisitor_Do_T<int>
,	public virtual ZVisitor_Expr_Bool_True
,	public virtual ZVisitor_Expr_Bool_False
,	public virtual ZVisitor_Expr_Bool_Not
,	public virtual ZVisitor_Expr_Bool_And
,	public virtual ZVisitor_Expr_Bool_Or
	{
public:
	Comparer();
	Comparer(Comparer_Bootstrap* iBootstrap);

// From ZVisitor_Expr_Bool_XXX
	virtual void Visit_Expr_Bool_True(const ZRef<ZExpr_Bool_True>&);
	virtual void Visit_Expr_Bool_False(const ZRef<ZExpr_Bool_False>&);
	virtual void Visit_Expr_Bool_Not(const ZRef<ZExpr_Bool_Not>&);
	virtual void Visit_Expr_Bool_And(const ZRef<ZExpr_Bool_And>&);
	virtual void Visit_Expr_Bool_Or(const ZRef<ZExpr_Bool_Or>&);

	int CompareUnary(const ZRef<ZExpr_Op1_T<ZExpr_Bool> >& iLHS,
		const ZRef<ZExpr_Op1_T<ZExpr_Bool> >& iRHS);

	int CompareBinary(const ZRef<ZExpr_Op2_T<ZExpr_Bool> >& iLHS,
		const ZRef<ZExpr_Op2_T<ZExpr_Bool> >& iRHS);

private:
	Comparer_Bootstrap* fBootstrap;
	};

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_DoCompare::Comparer_GT_XXX

struct Comparer_GT_True : public virtual Comparer
	{ virtual void Visit_Expr_Bool_True(const ZRef<ZExpr_Bool_True>&); };

struct Comparer_GT_False : public virtual Comparer_GT_True
	{ virtual void Visit_Expr_Bool_False(const ZRef<ZExpr_Bool_False>&); };

struct Comparer_GT_Not : public virtual Comparer_GT_False
	{ virtual void Visit_Expr_Bool_Not(const ZRef<ZExpr_Bool_Not>&); };

struct Comparer_GT_And : public virtual Comparer_GT_Not
	{ virtual void Visit_Expr_Bool_And(const ZRef<ZExpr_Bool_And>&); };

struct Comparer_GT_Or : public virtual Comparer_GT_And
	{ virtual void Visit_Expr_Bool_Or(const ZRef<ZExpr_Bool_Or>&); };

// =================================================================================================
#pragma mark -
#pragma mark * Visitor_Expr_Bool_DoCompare::Comparer_XXX

class Comparer_True : public virtual Comparer
	{
public:
	Comparer_True(Comparer_Bootstrap* iBootstrap);
	virtual void Visit_Expr_Bool_True(const ZRef<ZExpr_Bool_True>&);
	};

class Comparer_False : public virtual Comparer_GT_True
	{
public:
	Comparer_False(Comparer_Bootstrap* iBootstrap);
	virtual void Visit_Expr_Bool_False(const ZRef<ZExpr_Bool_False>&);
	};

class Comparer_Not : public virtual Comparer_GT_False
	{
public:
	Comparer_Not(Comparer_Bootstrap* iBootstrap, const ZRef<ZExpr_Bool_Not>& iExpr);
	virtual void Visit_Expr_Bool_Not(const ZRef<ZExpr_Bool_Not>& iExpr);

private:
	const ZRef<ZExpr_Bool_Not> fExpr;
	};

class Comparer_And : public virtual Comparer_GT_Not
	{
public:
	Comparer_And(Comparer_Bootstrap* iBootstrap, const ZRef<ZExpr_Bool_And>& iExpr);
	virtual void Visit_Expr_Bool_And(const ZRef<ZExpr_Bool_And>& iExpr);

private:
	const ZRef<ZExpr_Bool_And> fExpr;
	};

class Comparer_Or : public virtual Comparer_GT_And
	{
public:
	Comparer_Or(Comparer_Bootstrap* iBootstrap, const ZRef<ZExpr_Bool_Or>& iExpr);
	virtual void Visit_Expr_Bool_Or(const ZRef<ZExpr_Bool_Or>& iExpr);

private:
	const ZRef<ZExpr_Bool_Or> fExpr;
	};

} // namespace Visitor_Expr_Bool_DoCompare
} // namespace ZooLib

#endif // __ZVisitor_Expr_Bool_DoCompare__
